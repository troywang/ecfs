/*
 * heartbeat_manager.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/gen-cpp/common_types.h"
#include "ecfs/src/datanode/data_node.h"
#include "ecfs/src/datanode/heartbeat_manager.h"

HeartbeatManager::~HeartbeatManager ()
{
	uninit();
}

void HeartbeatManager::init()
{
	mThread.reset(new boost::thread(boost::ref(*this)));
}

void HeartbeatManager::uninit()
{
	if (mThread.get()) {
		mThread->interrupt();
		mThread->join();
		mThread.reset();
	}
}

void HeartbeatManager::operator ()()
{
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			boost::shared_ptr<NamenodeThriftConnection> pConn;
			try
			{
				ThriftResult _ret;
				DatanodeHeartbeat info;
				make_heartbeat_info(/* out */info);

				pConn = get_server().get_namenode_connection();
				pConn->get_client()->datanode_heartbeat(_ret, info);

			} catch (ErasureException &err) {
				LOG(ERROR) << "datanode heartbeat failed: " << err.what();
			} catch (apache::thrift::TException &err) {
				LOG(ERROR) << "datanode heartbeat failed: " << err.what();
				if (pConn.get())
					pConn->destroy();
			}

			boost::this_thread::sleep(boost::posix_time::seconds(FLAGS_datanode_heartbeat_interval));
		}
    } catch (boost::thread_interrupted &err) {
		LOG(ERROR) << "datanode heartbeat interrupted";
	}
}

void HeartbeatManager::make_heartbeat_info (/* out */DatanodeHeartbeat &info)
{
	info.endpoint.nodeid = mServer.get_node_id();
	info.endpoint.ipv4 = ErasureUtils::str2ip(FLAGS_datanode_service_ip);
	info.endpoint.port = FLAGS_datanode_service_port;
}
