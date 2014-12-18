/*
 * name_node.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/namenode/name_node.h"
#include "ecfs/src/namenode/node_manager.h"
#include "ecfs/src/namenode/block_manager.h"

ChunkReportTask::ChunkReportTask (NamenodeServer &server, int16_t nodeid)
: mServer(server)
{
	mNodeId = nodeid;
}

void ChunkReportTask::run ()
{
	mServer.get_block_manager().do_chunk_report(mNodeId);
}

NamenodeServer::NamenodeServer()
:mNodeMgr(*this), mBlockMgr(*this), mLogMgr(*this),
 mRecMgr(*this), mCleanMgr(*this)
{
	mStatus = Server_Starting;
}

NamenodeServer::~NamenodeServer()
{
	uninit();
}

void NamenodeServer::init ()
{
	mDatanodePool.reset(new DatanodeThriftConnectionPool(FLAGS_conn_max,
		FLAGS_conn_maxperhost,
		FLAGS_conn_timeout_ms,
		FLAGS_recv_timeout_ms,
		FLAGS_send_timeout_ms,
		FLAGS_idle_timeout_sec,
		FLAGS_block_wait_ms));

	mRecoveryPool.reset(new RecoveryThriftConnectionPool(FLAGS_conn_max,
		FLAGS_conn_maxperhost,
		FLAGS_conn_timeout_ms,
		FLAGS_recv_timeout_ms,
		FLAGS_send_timeout_ms,
		FLAGS_idle_timeout_sec,
		FLAGS_block_wait_ms));

	mLogMgr.init();
	mNodeMgr.init();
	mBlockMgr.init();
	mRecMgr.init();
	mCleanMgr.init();
	mStatus = Server_Running;
}

void NamenodeServer::uninit ()
{
	mStatus = Server_Suspended;

	mCleanMgr.uninit();
	mRecMgr.uninit();
	mBlockMgr.uninit();
	mNodeMgr.uninit();
	mLogMgr.uninit();

	mRecoveryPool.reset();
	mDatanodePool.reset();
}

bool NamenodeServer::is_running()
{
	return mStatus == Server_Running;
}

void NamenodeServer::get_write_token (/* out */ FileWriteToken &token)
{
	token.blockid = get_block_manager().get_block_id();

	std::set<int32_t> excludes;
	int ret = get_block_manager().pick_chunks(FLAGS_N, excludes, token.chunkids);
	CHECK_RET(ret, "failed to pick chunks");

	for (size_t i = 0; i < token.chunkids.size(); i++) {
		int32_t chunkid = token.chunkids[i];
		DatanodeEndpoint endpoint;
		get_node_manager().make_datanode_endpoint(NODE_ID(chunkid), endpoint);
		token.endpoints.push_back(endpoint);
	}

	get_block_manager().create_block(token);
}

void NamenodeServer::fix_write_token (/* out */ FileWriteToken &token, const int32_t blockid, const std::set<int32_t> &failed_ids)
{
	token.blockid = blockid;
	int ret = get_block_manager().get_chunk_ids(blockid, token.chunkids, false);
	CHECK_RET(ret, "failed to get chunk ids");

	std::set<int32_t> excludes;
	excludes.insert(token.chunkids.begin(), token.chunkids.end());

	std::vector<int32_t> chunks;
	ret = get_block_manager().pick_chunks(failed_ids.size(), excludes, chunks);
	CHECK_RET(ret, "failed to pick chunks");

	for (size_t i = 0, j = 0; i < token.chunkids.size(); i++)
	{
		DatanodeEndpoint endpoint;
		int32_t cid = token.chunkids[i];

		if (failed_ids.count(cid) == 0) {
			ret = get_node_manager().make_datanode_endpoint(NODE_ID(cid), endpoint);
		} else {
			int32_t newid = chunks[j++];
			token.chunkids[i] = newid;
			ret = get_node_manager().make_datanode_endpoint(NODE_ID(newid), endpoint);
		}

		CHECK_RET(ret, "failed to make datanode endpoint");
		token.endpoints.push_back(endpoint);
	}

	ret = get_block_manager().adjust_block(token);
	CHECK_RET(ret, "failed to ajust block");
}

void NamenodeServer::commit_write (const int32_t blockid)
{
	std::vector<int32_t> chunkids;
	int ret = get_block_manager().get_chunk_ids(blockid, chunkids, false);
	CHECK_RET(ret, "block not found");

	ret = get_block_manager().commit_block(blockid);
	CHECK_RET(ret, "block not found");

	ret = get_oplog_manager().add_block(blockid, time(NULL));
	CHECK_RET(ret, "add block failed");

	for (size_t i = 0; i < chunkids.size(); i++)
	{
		int32_t cid = chunkids[i];
		get_oplog_manager().add_chunk(blockid, cid);
	}
}

void NamenodeServer::get_read_token (/* out */ FileReadToken &token, const int32_t blockid)
{
	int ret = get_block_manager().get_chunk_ids(blockid, token.chunkids, true);
	CHECK_RET(ret, "block not found");

	for (size_t i = 0; i < token.chunkids.size(); i++) {
		DatanodeEndpoint endpoint;
		int32_t chunkid = token.chunkids[i];
		get_node_manager().make_datanode_endpoint(NODE_ID(chunkid), endpoint);
		token.endpoints.push_back(endpoint);
	}
}

void NamenodeServer::request_recover (const int32_t blockid, const int32_t chunkid)
{
	get_recover_manager().start_recover(blockid, chunkid);
}

void NamenodeServer::done_recovery (const int32_t blockid, const int32_t oldchunk, const int32_t newchunk)
{
	bool found = get_recover_manager().done_recovery(blockid, oldchunk, newchunk);
	if (found) {
		get_block_manager().update_chunk(blockid, oldchunk, newchunk);
		get_oplog_manager().update_chunk(blockid, oldchunk, newchunk);
	}
}

void NamenodeServer::delete_block (const int32_t blockid)
{
	std::vector<int32_t> chunkids;
	int ret = get_block_manager().get_chunk_ids(blockid, chunkids, true);
	CHECK_RET(ret, "block not found");

	ret = get_block_manager().delete_block(blockid);
	CHECK_RET(ret, "failed to delete block");

	for (size_t i = 0; i < chunkids.size(); i++)
	{
		get_clean_manager().request_delete(chunkids[i]);
	}
}

void NamenodeServer::start_chunk_report (int16_t nodeid)
{
	LOG_WARNING_SMART << "start chunk report for datanode " << nodeid;

	boost::shared_ptr<ChunkReportTask> pTask(new ChunkReportTask(*this, nodeid));
	mPool.add(pTask);
}

void NamenodeServer::done_chunk_report(int16_t nodeid, bool success)
{
	mNodeMgr.done_datanode_chunk_report(nodeid, success);
}

void NamenodeServer::handle_datanode_dead (int16_t nodeid)
{
	get_block_manager().handle_datanode_dead(nodeid);
}

boost::shared_ptr<DatanodeThriftConnection> NamenodeServer::get_datanode_connection(int16_t nodeid)
{
	DatanodeEndpoint endpoint;
	int ret = mNodeMgr.make_datanode_endpoint(nodeid, endpoint);
	CHECK_RET(ret, "failed to make datanode endpoint");

	return mDatanodePool->get_connection(endpoint.ipv4, endpoint.port);
}

boost::shared_ptr<RecoveryThriftConnection> NamenodeServer::get_recovery_connection(int16_t nodeid)
{
	DatanodeEndpoint endpoint;
	int ret = mNodeMgr.make_recovery_endpoint(nodeid, endpoint);
	CHECK_RET(ret, "failed to make recovery endpoint");

	return mRecoveryPool->get_connection(endpoint.ipv4, endpoint.port);
}



