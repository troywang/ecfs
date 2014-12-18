/*
 * recoverynode.h
 *
 *  Created on: May 28, 2014
 *      Author: wanghuan
 */

#ifndef RECOVERYNODE_H_
#define RECOVERYNODE_H_

#include "ecfs/src/common/common.h"
#include "ecfs/src/common/configs.h"
#include "ecfs/src/common/thread_pool.h"
#include "ecfs/src/common/thrift_adaptor.h"

class RecoveryServer;
class RecoverTask : public Task
{
public:
	RecoverTask (RecoveryServer &server, const FileRecoverToken &token);

	virtual void run ();

private:
	RecoveryServer &mServer;
	FileRecoverToken mToken;
};

class RecoveryServer
{
public:
	RecoveryServer(int16_t nodeid);

	virtual ~RecoveryServer() {
	}

	void init ();

	void uninit ();

	void operator() ();

	void recover_chunk (const FileRecoverToken &token);

	void do_recover_chunk (FileRecoverToken token);

	void make_heartbeat_info (/* out */RecoveryHeartbeat &info);

	virtual boost::shared_ptr<NamenodeThriftConnection> get_namenode_connection ();

	virtual boost::shared_ptr<DatanodeThriftConnection> get_datanode_connection (int16_t nodeid);

private:
	uint16_t mNodeId;
	ThreadPool mPool;

    std::auto_ptr<boost::thread> mHeart;

	NamenodeThriftConnectionPool mNamenodeThriftPool;
	DatanodeThriftConnectionPool mDatanodeThriftPool;
};


#endif /* RECOVERYNODE_H_ */
