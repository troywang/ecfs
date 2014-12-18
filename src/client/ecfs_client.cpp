/*
 * client.cpp
 *
 *  Created on: Sep 17, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/common/codec.h"
#include "ecfs/src/client/ecfs_client.h"

ErasureClient::ErasureClient()
:mNamenodePool(FLAGS_conn_max,
		FLAGS_conn_maxperhost,
		FLAGS_conn_timeout_ms,
		FLAGS_recv_timeout_ms,
		FLAGS_send_timeout_ms,
		FLAGS_idle_timeout_sec,
		FLAGS_block_wait_ms),
 mDatanodePool(FLAGS_conn_max,
		FLAGS_conn_maxperhost,
		FLAGS_conn_timeout_ms,
		FLAGS_recv_timeout_ms,
		FLAGS_send_timeout_ms,
		FLAGS_idle_timeout_sec,
		FLAGS_block_wait_ms)
{
}

int ErasureClient::put_block(const std::string &data) throw (ErasureException)
{
	time_t start = time(NULL);
	int32_t blockid = -1;
	FileWriteToken token;

	size_t max = FLAGS_chunk_len * FLAGS_K;
	CHECK_TRUE(data.size() <= max, "file size exceed limit");

	boost::shared_ptr<NamenodeThriftConnection> pNameConn;
	try
	{
		ResultGetWriteToken ret;
		pNameConn = get_namenode_connection();
		pNameConn->get_client()->get_write_token(ret);

		CHECK_RET(ret.result.code, "failed to get write token");

		token = ret.token;
		blockid = token.blockid;
	} catch (apache::thrift::TException &err) {
		if (pNameConn.get())
			pNameConn->destroy();
		CHECK_TRUE(false, std::string("failed to get write token ") + std::string(err.what()));
	}

	ErasureCodec codec(FLAGS_K, FLAGS_N, FLAGS_chunk_len);
	codec.set_chunk_ids(token.chunkids);

	int32_t offset = 0;
	int32_t remain = data.size();
	for (int i = 0; i < FLAGS_K; i++)
	{
		std::string tmp;
		int32_t chunkid = token.chunkids[i];
		tmp.resize(FLAGS_chunk_len, 0);

		if (remain > 0)
		{
			std::memcpy((char *)&tmp.c_str()[0], (char *)&data.c_str()[offset], std::min(remain, FLAGS_chunk_len));
		}
		codec.add_chunk(chunkid, tmp);

		offset += FLAGS_chunk_len;
		remain -= FLAGS_chunk_len;
	}

	codec.encode();

	int retry = 0;
	std::set<int32_t> success;

	while (retry++ < 5)
	{
		std::vector<int32_t> &chunkids = token.chunkids;
		std::vector<DatanodeEndpoint> &endpoints = token.endpoints;

		for (size_t i = 0; i < chunkids.size(); i++)
		{
			int32_t chunkid = chunkids[i];
			DatanodeEndpoint &endpoint = endpoints[i];
			if (success.find(chunkid) != success.end()) {
				continue;
			}

			boost::shared_ptr<DatanodeThriftConnection> pDataConn;
			try
			{
				ThriftResult ret;
				pDataConn = get_datanode_connection(endpoint.nodeid);

				std::string data;
				codec.get_chunk(chunkid, data);

				time_t s = time(NULL);
				pDataConn->get_client()->put_chunk(ret, chunkid, data);
				LOG(ERROR) << "put chunk (" << blockid << ", " << chunkid << ") cost " << time(NULL) - s << " seconds, ret = " << ret.code;
				if (RESULT_IS_SUCCESS(ret.code)) {
					success.insert(chunkid);
				} else {
					LOG(ERROR) << "failed to put chunk " << ret.verbose;
				}
			} catch (apache::thrift::TException &err) {
				if (pDataConn.get())
					pDataConn->destroy();
				LOG(ERROR) << "failed to put chunk: " << err.what();
			} catch (ErasureException &err) {
				LOG(ERROR) << "failed to put chunk: " << err.what();
			}
		}

		if (success.size() == (size_t)FLAGS_N)
			break;

		std::set<int32_t> tmp;
		std::set<int32_t> failed;
		tmp.insert(token.chunkids.begin(), token.chunkids.end());
		std::set_difference(tmp.begin(), tmp.end(), success.begin(), success.end(),
				std::insert_iterator<std::set<int32_t> >(failed, failed.begin()));

		LOG(ERROR) << "retry put block " << blockid << ", failed count " << failed.size();

		try
		{
			ResultFixWriteToken ret;
			boost::shared_ptr<NamenodeThriftConnection> pNameConn = get_namenode_connection();
			pNameConn->get_client()->fix_write_token(ret, blockid, failed);
			CHECK_RET(ret.result.code, "failed to fix write token");

			for (size_t i = 0; i < token.chunkids.size(); i++)
			{
				int32_t oldid = token.chunkids[i];
				int32_t newid = ret.token.chunkids[i];
				if (oldid != newid) {
					codec.fix_chunk(oldid, newid);
					LOG(ERROR) << "fix chunk " << blockid << "(" << oldid << "->" << newid << ")";
				}
			}

			token = ret.token;
		} catch (apache::thrift::TException &err) {
			CHECK_TRUE(false, std::string("failed to fix write token ") + std::string(err.what()));
		}
	}

	CHECK_TRUE(retry < 5, "failed to put block after 5 retries");

	try
	{
		ThriftResult ret;
		pNameConn = get_namenode_connection();
		pNameConn->get_client()->commit_write(ret, blockid);
		CHECK_RET(ret.code, "failed to commit write");
	} catch (apache::thrift::TException &err) {
		if (pNameConn.get())
			pNameConn->destroy();
		CHECK_TRUE(false, std::string("failed to commit write: ") + std::string(err.what()));
	}

	LOG(ERROR) << "done put block " << blockid << ", cost " << time(NULL) - start << " seconds";
	return blockid;
}

void ErasureClient::get_block (int32_t blockid, /* out */std::string &data) throw (ErasureException)
{
	boost::shared_ptr<NamenodeThriftConnection> pNameConn;
	pNameConn = get_namenode_connection();

	ResultGetReadToken ret;
	pNameConn->get_client()->get_read_token(ret, blockid);
	CHECK_RET(ret.result.code, "failed to get read token");

	std::vector<int32_t> &chunkids = ret.token.chunkids;
	std::vector<DatanodeEndpoint> &endpoints = ret.token.endpoints;

	ErasureCodec decoder(FLAGS_K, FLAGS_N, FLAGS_chunk_len);

	int count = 0;
	bool need_decode = false;
	for (int i = 0; i < FLAGS_N; i++)
	{
		int32_t chunkid = chunkids[i];
		DatanodeEndpoint &endpoint = endpoints[i];
		boost::shared_ptr<DatanodeThriftConnection> pDataConn;

		try
		{
			pDataConn = get_datanode_connection(endpoint.nodeid);

			ResultGetChunk _ret;
			pDataConn->get_client()->get_chunk(_ret, chunkid);
			if (RESULT_IS_SUCCESS(_ret.result.code)) {
				decoder.add_chunk(chunkid, _ret.data);
				count++;
			} else {
				LOG(ERROR) << "failed to get chunk: " << _ret.result.verbose;
			}
		} catch (apache::thrift::TException &err) {
			if (pDataConn.get())
				pDataConn->destroy();
			LOG(ERROR) << "failed to get chunk: " << err.what();
		} catch (ErasureException &err) {
			LOG(ERROR) << "failed to get chunk: " << err.what();
		}

		if (i == FLAGS_K)
			need_decode = true;

		if (count == FLAGS_K)
			break;
	}

	if (count < FLAGS_K) {
		ErasureUtils::throw_exception(RESULT_ERR_BLOCK_FILE_BROKEN, "failed to get enough chunks");
	}

	if (need_decode) {
		decoder.set_chunk_ids(chunkids);
		decoder.decode();
	}

	for (int i = 0; i < FLAGS_K; i++)
	{
		std::string tmp;
		decoder.get_chunk(chunkids[i], /* out */tmp);
		data.append(tmp);
	}
}

void ErasureClient::read_data(int32_t blockid, int32_t offset, int32_t size, /* out */std::string &data) throw (ErasureException)
{
	boost::shared_ptr<NamenodeThriftConnection> pNameConn;
	pNameConn = get_namenode_connection();

	ResultGetReadToken ret;
	pNameConn->get_client()->get_read_token(ret, blockid);
	CHECK_RET(ret.result.code, "failed to get read token");

	std::vector<int32_t> &chunkids = ret.token.chunkids;
	std::vector<DatanodeEndpoint> &endpoints = ret.token.endpoints;

	offset = std::max(offset, 0);
	int remain = std::min(size, FLAGS_K * FLAGS_chunk_len - offset);

	int idx = offset / FLAGS_chunk_len;
	int off = offset % FLAGS_chunk_len;

	boost::shared_ptr<DatanodeThriftConnection> pDataConn;
	try
	{
		while (idx < FLAGS_K && remain > 0)
		{
			int32_t chunkid = chunkids[idx];
			DatanodeEndpoint &endpoint = endpoints[idx];

			ResultReadData _ret;
			int32_t sz = std::min(remain, FLAGS_chunk_len - off);

			pDataConn = get_datanode_connection(endpoint.nodeid);
			pDataConn->get_client()->read_data(_ret, chunkid, off, sz);

			if (RESULT_IS_SUCCESS(_ret.result.code)) {
				idx++;
				off = 0;
				remain -= sz;
				data.append(_ret.data);
			} else {
				LOG(ERROR) << "failed to read data " << _ret.result.code;
				break;
			}
		}

		if (remain == 0)
			return;

	} catch (apache::thrift::TException &err) {
		LOG(ERROR) << "failed to read data " << err.what();
	} catch (ErasureException &err) {
		LOG(ERROR) << "failed to read data " << err.what();
	}

	LOG(ERROR) << "decode to get data ";

	std::string tmp;
	get_block(blockid, tmp);
	data.assign((char *)&tmp.c_str()[offset], size);
}

boost::shared_ptr<NamenodeThriftConnection> ErasureClient::get_namenode_connection()
{
	uint32_t ip = ErasureUtils::str2ip(FLAGS_namenode_service_ip);
	return mNamenodePool.get_connection(ip, FLAGS_namenode_service_port);
}

boost::shared_ptr<DatanodeThriftConnection> ErasureClient::get_datanode_connection(int16_t nodeid)
{
	ResultGetNodeInfo info;
	get_namenode_connection()->get_client()->get_node_info(info, nodeid);
	return mDatanodePool.get_connection(info.endpoint.ipv4, info.endpoint.port);
}

