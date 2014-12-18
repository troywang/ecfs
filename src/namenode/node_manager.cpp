/*
 * node_manager.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/namenode/name_node.h"
#include "ecfs/src/namenode/node_manager.h"

NodeManager::NodeManager(NamenodeServer &server)
: ManagerBase(server)
{
}

NodeManager::~NodeManager()
{
	uninit();
}

void NodeManager::init ()
{
	mLiveChecker.reset(new boost::thread(boost::ref(*this)));
}

void NodeManager::uninit ()
{
	if (mLiveChecker.get()) {
		mLiveChecker->interrupt();
		mLiveChecker->join();
		mLiveChecker.reset();
	}
}

void NodeManager::add_datanode_raw (int16_t nodeid)
{
	NodeInfo info;
	info.active = time(NULL);
	info.nodeid = nodeid;
	info.status = Server_Suspended;
	mDatanodes.insert(std::make_pair(nodeid, info));
}

void NodeManager::operator ()()
{
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			{
				NodeMap::iterator it;
				time_t now = time(NULL);

				RWLockScopedGuard lock(mRWLock, RWLOCK_WRITE);

				for (it = mDatanodes.begin(); it != mDatanodes.end(); it++)
				{
					int16_t nodeid = it->first;
					NodeInfo &info = it->second;
					if (now - info.active  > FLAGS_datanode_dead_interval) {
						if (info.status != Server_Dead) {
							info.status = Server_Dead;
							mServer.handle_datanode_dead(nodeid);
							LOG(ERROR) << "move datanode " << nodeid << " to dead state";
						}
					} else if (now - info.active > FLAGS_datanode_suspend_interval) {
						if (info.status != Server_Suspended) {
							info.status = Server_Suspended;
							LOG(ERROR) << "move datanode " << nodeid << " to suspended state";
						}
					}
				}

				for (it = mRecoveries.begin(); it != mRecoveries.end(); it++)
				{
					uint16_t nodeid = it->first;
					NodeInfo &info = it->second;

					if (now - info.active > FLAGS_datanode_suspend_interval) {
						if (info.status != Server_Suspended) {
							info.status = Server_Suspended;
							LOG(ERROR) << "move recovery node " << nodeid << " to suspended state";
						}
					}
				}
			}

			boost::this_thread::sleep(boost::posix_time::seconds(FLAGS_datanode_heartbeat_interval) * 3);
		}
	} catch (boost::thread_interrupted &err) {
		LOG(ERROR) << "node manager live checker interrupted";
	}
}

void NodeManager::handle_datanode_heartbeat (const DatanodeHeartbeat &dhb)
{
	RWLockScopedGuard lock(mRWLock, RWLOCK_WRITE);

	uint16_t nodeid = dhb.endpoint.nodeid;
	NodeMap::iterator it = mDatanodes.find(nodeid);
	if (it == mDatanodes.end())
	{
		LOG_WARNING_SMART << "receive new heartbeat from datanode " << nodeid;

		NodeInfo info;
		info.active = time(NULL);
		info.nodeid = dhb.endpoint.nodeid;
		info.ipv4 = dhb.endpoint.ipv4;
		info.port = dhb.endpoint.port;
		info.status = Server_Starting;

		mDatanodes.insert(std::make_pair(nodeid, info));
		mServer.start_chunk_report(nodeid);
	}
	else
	{
		NodeInfo &info = it->second;
		info.active = time(NULL);
		if (info.status == Server_Dead || info.status == Server_Suspended)
		{
			LOG_WARNING_SMART << "receive heartbeat from suspended datanode " << nodeid;

			info.nodeid = dhb.endpoint.nodeid;
			info.ipv4 = dhb.endpoint.ipv4;
			info.port = dhb.endpoint.port;
			info.status = Server_Starting;

			mServer.start_chunk_report(nodeid);
		}
	}
}

void NodeManager::handle_recovery_heartbeat (const RecoveryHeartbeat &rhb)
{
	RWLockScopedGuard lock(mRWLock, RWLOCK_WRITE);

	uint16_t nodeid = rhb.endpoint.nodeid;
	NodeMap::iterator it = mRecoveries.find(nodeid);
	if (it == mRecoveries.end())
	{
		NodeInfo info;
		info.active = time(NULL);
		info.nodeid = rhb.endpoint.nodeid;
		info.ipv4 = rhb.endpoint.ipv4;
		info.port = rhb.endpoint.port;
		info.status = Server_Running;
		mRecoveries.insert(std::make_pair(nodeid, info));
		LOG_WARNING_SMART << "receive new heartbeat from recovery node " << nodeid;
	}
	else
	{
		NodeInfo &info = it->second;
		info.active = time(NULL);
		if (info.status == Server_Suspended) {
			info.status = Server_Running;
			LOG_WARNING_SMART << "receive heartbeat from suspended recovery node " << nodeid;
		}
	}
}

void NodeManager::done_datanode_chunk_report (int16_t nodeid, bool success)
{
	RWLockScopedGuard lock(mRWLock, RWLOCK_WRITE);

	NodeMap::iterator it = mDatanodes.find(nodeid);
	if (it == mDatanodes.end()) {
		LOG(ERROR) << "!!!BUG!!!, not found data node " << nodeid;
		return;
	}

	if (success)
	{
		it->second.status = Server_Running;
		LOG_WARNING_SMART << "chunk report from datanode " << nodeid << " succeeded";
	}
	else
	{
		it->second.status = Server_Suspended; //will trigger another chunk report
		LOG_WARNING_SMART << "chunk report from datanode " << nodeid << " failed!";
	}
}

int NodeManager::pick_data_nodes (size_t count, const std::set<int16_t> &excludes, std::vector<int16_t> &out)
{
	RWLockScopedGuard guard(mRWLock, RWLOCK_READ);

	std::vector<NodeMap::iterator> nodeList;
	nodeList.reserve(mDatanodes.size());

	for (NodeMap::iterator it = mDatanodes.begin(); it != mDatanodes.end(); it++)
	{
		NodeInfo &info = it->second;
		int16_t nodeid = info.nodeid;
		if (excludes.count(nodeid) != 0)
			continue;

		if (!info.is_running())
			continue;

		nodeList.push_back(it);
	}

	if (nodeList.size() < count)
		return RESULT_ERR_INSUFFICIENT_DATANODE;

	std::random_shuffle(nodeList.begin(), nodeList.end());

	out.clear();
	std::vector<NodeMap::iterator>::iterator it = nodeList.begin();

	while (count-- != 0) {
		out.push_back((*it)->first);
		++it;
	}

	return RESULT_SUCCESS;
}

void NodeManager::get_recovery_nodes (/* out */ std::set<int16_t> &nodeids)
{
	RWLockScopedGuard guard(mRWLock, RWLOCK_READ);
	for (NodeMap::iterator it = mRecoveries.begin(); it != mRecoveries.end(); it++)
	{
		NodeInfo &info = it->second;
		if (!info.is_running())
			continue;

		nodeids.insert(it->first);
	}
}

int NodeManager::make_datanode_endpoint(int16_t nodeid, /* out */DatanodeEndpoint &endpoint)
{
	RWLockScopedGuard guard(mRWLock, RWLOCK_READ);

	NodeMap::iterator it = mDatanodes.find(nodeid);
	if (it != mDatanodes.end()) {
		NodeInfo &info = it->second;
		endpoint.nodeid = info.nodeid;
		endpoint.ipv4 = info.ipv4;
		endpoint.port = info.port;
		return RESULT_SUCCESS;
	}

	return RESULT_ERR_DATANODE_NOT_FOUND;
}

int NodeManager::make_chunkid_endpoints(std::vector<int32_t> &chunkids, /* out */std::vector<DatanodeEndpoint> &endpoints)
{
	int ret = 0;
	for (size_t i = 0; i < chunkids.size(); i++)
	{
		int32_t chunkid = chunkids[i];
		DatanodeEndpoint endpoint;
		ret |= make_datanode_endpoint(NODE_ID(chunkid), /* out */ endpoint);
		endpoints.push_back(endpoint);
	}

	return ret;
}

int NodeManager::make_recovery_endpoint(int16_t nodeid, /* out */DatanodeEndpoint &endpoint)
{
	RWLockScopedGuard guard(mRWLock, RWLOCK_READ);

	NodeMap::iterator it = mRecoveries.find(nodeid);
	if (it != mRecoveries.end()) {
		NodeInfo &info = it->second;
		endpoint.nodeid = info.nodeid;
		endpoint.ipv4 = info.ipv4;
		endpoint.port = info.port;
		return RESULT_SUCCESS;
	}

	return RESULT_ERR_RECOVERY_NOT_FOUND;
}
