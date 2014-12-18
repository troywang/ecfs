/*
 * name_node.h
 *
 *  Created on: Sep 1, 2014
 *      Author: wanghuan
 */

#ifndef NAME_NODE_H_
#define NAME_NODE_H_

#include "ecfs/src/common/common.h"
#include "ecfs/src/common/thread_pool.h"
#include "ecfs/src/common/thrift_adaptor.h"

#include "ecfs/src/namenode/node_manager.h"
#include "ecfs/src/namenode/block_manager.h"
#include "ecfs/src/namenode/oplog_manager.h"
#include "ecfs/src/namenode/recover_manager.h"
#include "ecfs/src/namenode/clean_manager.h"

class NamenodeServer;

class ChunkReportTask : public Task
{
public:
	ChunkReportTask (NamenodeServer &server, int16_t nodeid);

	virtual void run ();

private:
	NamenodeServer &mServer;
	int16_t mNodeId;
};

class NamenodeServer : public boost::noncopyable
{
public:
	NamenodeServer ();

	void init ();

	void uninit ();

	bool is_running ();

	virtual ~NamenodeServer ();

	virtual NodeManager& get_node_manager () {
		return mNodeMgr;
	}

	virtual BlockManager& get_block_manager () {
		return mBlockMgr;
	}

	virtual OpLogManager& get_oplog_manager () {
		return mLogMgr;
	}

	virtual RecoverManager& get_recover_manager () {
		return mRecMgr;
	}

	virtual CleanManager& get_clean_manager () {
		return mCleanMgr;
	}

	void get_write_token (/* out */ FileWriteToken &token);

	void fix_write_token (/* out */ FileWriteToken &token, const int32_t blockid, const std::set<int32_t> &failed_ids);

	void commit_write (const int32_t blockid);

	void get_read_token (/* out */ FileReadToken &token, const int32_t blockid);

	void request_recover (const int32_t blockid, const int32_t chunkid);

	void done_recovery (const int32_t blockid, const int32_t oldchunk, const int32_t newchunk);

	void delete_block (const int32_t blockid);

	void start_chunk_report (int16_t nodeid);

	void done_chunk_report (int16_t nodeid, bool success);

	void handle_datanode_dead (int16_t nodeid);

	virtual boost::shared_ptr<DatanodeThriftConnection> get_datanode_connection(int16_t nodeid);

	virtual boost::shared_ptr<RecoveryThriftConnection> get_recovery_connection(int16_t nodeid);

protected:
	ThreadPool mPool;
	boost::mutex mMutex;

	volatile ServerStatus mStatus;

	NodeManager mNodeMgr;
	BlockManager mBlockMgr;
	OpLogManager mLogMgr;
	RecoverManager mRecMgr;
	CleanManager mCleanMgr;

	std::auto_ptr<DatanodeThriftConnectionPool> mDatanodePool;
	std::auto_ptr<RecoveryThriftConnectionPool> mRecoveryPool;
};

#endif /* NAME_NODE_H_ */
