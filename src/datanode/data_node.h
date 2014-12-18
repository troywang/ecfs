/*
 * datanode.h
 *
 *  Created on: Apr 25, 2014
 *      Author: wanghuan
 */

#ifndef DATANODE_H_
#define DATANODE_H_

#include "ecfs/src/common/configs.h"
#include "ecfs/src/common/thread_pool.h"
#include "ecfs/src/datanode/disk_manager.h"
#include "ecfs/src/datanode/heartbeat_manager.h"
#include "ecfs/src/common/thrift_adaptor.h"

class DatanodeServer
{
public:
	DatanodeServer(int16_t nodeid);

	virtual ~DatanodeServer();

	void init ();

	void uninit ();

	int16_t get_node_id() {
		return mNodeId;
	}

    bool is_running() {
    	boost::mutex::scoped_lock lock(mMutex);
        return (mStatus == Server_Running);
    }

	virtual DiskManager& get_disk_manager() {
		return mDisks;
	}

	virtual HeartbeatManager& get_heartbeat_manager() {
		return mHeart;
	}

	virtual boost::shared_ptr<NamenodeThriftConnection> get_namenode_connection();

protected:
	int16_t mNodeId;
    DiskManager mDisks;
	HeartbeatManager mHeart;

	boost::mutex mMutex;
	volatile ServerStatus mStatus;

	std::auto_ptr<NamenodeThriftConnectionPool> mNamenodeThriftConnectionPool;
	std::auto_ptr<DatanodeThriftConnectionPool> mDatanodeThriftConnectionPool;
};

#endif /* DATANODE_H_ */
