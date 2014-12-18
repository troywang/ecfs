/*
 * heartbeat_manager.h
 *
 *  Created on: Apr 28, 2014
 *      Author: wanghuan
 */

#ifndef HEARTBEAT_MANAGER_H_
#define HEARTBEAT_MANAGER_H_

#include "ecfs/src/common/common.h"
#include "ecfs/src/datanode/managerbase.h"
#include "ecfs/src/common/thrift_adaptor.h"

class DatanodeServer;

class HeartbeatManager : public ManagerBase
{
public:
	HeartbeatManager(DatanodeServer &server) : ManagerBase(server) {
	}

	virtual ~HeartbeatManager ();

	virtual void init ();

	virtual void uninit ();

	void operator() ();

private:
	void make_heartbeat_info (/* out */DatanodeHeartbeat &info);

private:
    std::auto_ptr<boost::thread> mThread;
};

#endif /* HEARTBEAT_MANAGER_H_ */
