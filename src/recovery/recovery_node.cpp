/*
 * recoverynode.cpp
 *
 *  Created on: May 28, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/common/configs.h"
#include "ecfs/src/common/codec.h"
#include "ecfs/src/recovery/recovery_node.h"

#include "thirdparty/fecpp/fecpp.h"

RecoverTask::RecoverTask (RecoveryServer &server, const FileRecoverToken &token)
: mServer(server), mToken(token)
{
}

void RecoverTask::run ()
{
	mServer.do_recover_chunk(mToken);
}

RecoveryServer::RecoveryServer(int16_t nodeid)
:mNamenodeThriftPool(FLAGS_conn_max,
		FLAGS_conn_maxperhost,
		FLAGS_conn_timeout_ms,
		FLAGS_recv_timeout_ms,
		FLAGS_send_timeout_ms,
		FLAGS_idle_timeout_sec,
		FLAGS_block_wait_ms),

 mDatanodeThriftPool(FLAGS_conn_max,
		FLAGS_conn_maxperhost,
		FLAGS_conn_timeout_ms,
		FLAGS_recv_timeout_ms,
		FLAGS_send_timeout_ms,
		FLAGS_idle_timeout_sec,
		FLAGS_block_wait_ms)
{
	mNodeId = nodeid;
}

void RecoveryServer::init()
{
	mHeart.reset(new boost::thread(boost::ref(*this)));
}

void RecoveryServer::uninit()
{
	if (mHeart.get() != NULL) {
		mHeart->interrupt();
		mHeart->join();
		mHeart.reset();
	}
}

void RecoveryServer::operator ()()
{
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			boost::shared_ptr<NamenodeThriftConnection> pConn;
			try
			{
				ThriftResult _ret;
				RecoveryHeartbeat info;
				make_heartbeat_info(info);

				pConn = get_namenode_connection();
				pConn->get_client()->recovery_heartbeat(_ret, info);
			} catch (ErasureException &err) {
				LOG(ERROR) << "recovery node heart beat failed: " << err.what();
			} catch (apache::thrift::TException &err) {
				if (pConn.get())
					pConn->destroy();

				LOG(ERROR) << "recovery node heart beat failed: " << err.what();
			}

			boost::this_thread::sleep(boost::posix_time::seconds(FLAGS_datanode_heartbeat_interval));
		}
	} catch (boost::thread_interrupted &err) {
		LOG(ERROR) << "thread interrupted";
	}
}

void RecoveryServer::make_heartbeat_info(RecoveryHeartbeat &info)
{
	info.endpoint.nodeid = FLAGS_recovery_id;
	info.endpoint.ipv4 = ErasureUtils::str2ip(FLAGS_recovery_service_ip);
	info.endpoint.port = FLAGS_recovery_service_port;
}

void RecoveryServer::recover_chunk(const FileRecoverToken &token)
{
	boost::shared_ptr<RecoverTask> pTask(new RecoverTask(*this, token));
	mPool.add(pTask);
	LOG_WARNING_SMART << "start to recover block " << token.blockid << ", " << token.oldchunk << " => " << token.newchunk;
}

void RecoveryServer::do_recover_chunk(FileRecoverToken token)
{
	ErasureCodec codec(FLAGS_K, FLAGS_N, FLAGS_chunk_len);
	codec.set_chunk_ids(token.chunkids);

	boost::shared_ptr<DatanodeThriftConnection> pDataConn;

	int count = 0;
	for (size_t i = 0; i < token.chunkids.size(); i++)
	{
		int32_t chunkid = token.chunkids[i];
		DatanodeEndpoint &endpoint = token.endpoints[i];

		//不能读chunkid, chunkid可能已经属于别的block了
		if (chunkid == token.oldchunk)
			continue;

		try
		{
			pDataConn = get_datanode_connection(endpoint.nodeid);

			ResultGetChunk _ret;
			pDataConn->get_client()->get_chunk(_ret, chunkid);
			if (RESULT_IS_SUCCESS(_ret.result.code)) {
				codec.add_chunk(chunkid, _ret.data);
				if (++count == FLAGS_K)
					break;
			} else {
				LOG(ERROR) << "failed to get chunk: " << _ret.result.code;
			}

		} catch (ErasureException &err) {
			LOG(ERROR) << "failed to get chunk: " << err.what();
		} catch (apache::thrift::TException &err) {
			if (pDataConn.get())
				pDataConn->destroy();
			LOG(ERROR) << "failed to get chunk: " << err.what();
		}
	}

	if (count < FLAGS_K) {
		LOG(ERROR) << "no enough chunks for recovery";
		return;
	}

	codec.decode();
	codec.encode();

	std::string data;
	codec.get_chunk(token.oldchunk, data);
	if (data.empty()) {
		LOG(ERROR) << "data is empty, failed to decode";
		return;
	}

	boost::shared_ptr<NamenodeThriftConnection> pNameConn;
	try
	{
		ThriftResult _ret;
		pDataConn = get_datanode_connection(NODE_ID(token.newchunk));
		pDataConn->get_client()->put_chunk(_ret, token.newchunk, data);

		if (!RESULT_IS_SUCCESS(_ret.code)) {
			LOG(ERROR) << "failed to put chunk " << token.newchunk;
			return;
		}

		pNameConn = get_namenode_connection();
		pNameConn->get_client()->done_recovery(_ret, token.blockid, token.oldchunk, token.newchunk);

		if (!RESULT_IS_SUCCESS(_ret.code)) {
			LOG(ERROR) << "failed to notify namenode " << _ret.verbose;
			return;
		}

		LOG_WARNING_SMART << "done recover block " << token.blockid << ", " << token.oldchunk << " => " << token.newchunk;

	} catch (ErasureException &err) {
		LOG(ERROR) << "failed to recover chunk: " << err.what();
	} catch (apache::thrift::TException &err) {
		if (pDataConn.get())
			pDataConn->destroy();
		if (pNameConn.get())
			pNameConn->destroy();
		LOG(ERROR) << "failed to recover chunk: " << err.what();
	}
}

boost::shared_ptr<DatanodeThriftConnection> RecoveryServer::get_datanode_connection(int16_t nodeid)
{
	ResultGetNodeInfo _ret;
	boost::shared_ptr<NamenodeThriftConnection> pNameConn;

	pNameConn = get_namenode_connection();
	pNameConn->get_client()->get_node_info(_ret, nodeid);

	return mDatanodeThriftPool.get_connection(_ret.endpoint.ipv4, _ret.endpoint.port);
}

boost::shared_ptr<NamenodeThriftConnection> RecoveryServer::get_namenode_connection()
{
	uint32_t ip = ErasureUtils::str2ip(FLAGS_namenode_service_ip);
	return mNamenodeThriftPool.get_connection(ip, FLAGS_namenode_service_port);
}

