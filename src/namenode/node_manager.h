/*
 * node_manager.h
 *
 *  Created on: Sep 1, 2014
 *      Author: wanghuan
 */

#ifndef NODE_MANAGER_H_
#define NODE_MANAGER_H_

#include "ecfs/src/common/common.h"
#include "ecfs/src/namenode/managerbase.h"

struct NodeInfo
{
	NodeInfo() {
		memset(this, 0, sizeof(NodeInfo));
	}

	int8_t nodeid;
	int32_t ipv4;
	int16_t port;
	time_t active;
	ServerStatus status;

	int32_t load;

	bool is_running () {
		return status == Server_Running;
	}
};

typedef std::map<int16_t, NodeInfo> NodeMap;

class NamenodeServer;
class NodeManager : public ManagerBase
{
public:
	NodeManager (NamenodeServer &server);

	virtual ~NodeManager ();

	virtual void init ();

	virtual void uninit ();

	void operator() ();

	void add_datanode_raw (int16_t nodeid);

	void handle_datanode_heartbeat (const DatanodeHeartbeat &dhb);

	void handle_recovery_heartbeat (const RecoveryHeartbeat &rhb);

	void done_datanode_chunk_report (int16_t nodeid, bool success);

	void get_recovery_nodes (/* out */ std::set<int16_t> &nodeids);

	int pick_data_nodes (size_t count, const std::set<int16_t> &excludes, std::vector<int16_t> &out);

	int make_recovery_endpoint(int16_t nodeid, /* out */DatanodeEndpoint &endpoint);

	int make_datanode_endpoint(int16_t nodeid, /* out */DatanodeEndpoint &endpoint);

	int make_chunkid_endpoints(std::vector<int32_t> &chunkids, /* out */std::vector<DatanodeEndpoint> &endpoints);


private:
	RWLock mRWLock;
	NodeMap mDatanodes;
	NodeMap mRecoveries;
	std::auto_ptr<boost::thread> mLiveChecker;
};

#endif /* NODE_MANAGER_H_ */
