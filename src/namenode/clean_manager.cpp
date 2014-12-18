/*
 * clean_manager.cpp
 *
 *  Created on: Sep 9, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/namenode/name_node.h"
#include "ecfs/src/namenode/clean_manager.h"
#include "ecfs/src/common/thrift_connection_pool.h"

CleanManager::CleanManager(NamenodeServer &server)
: ManagerBase(server)
{
}

CleanManager::~CleanManager ()
{
	uninit();
}

void CleanManager::init ()
{
	mThread.reset(new boost::thread(boost::ref(*this)));
}

void CleanManager::uninit ()
{
	if (mThread.get()) {
		mThread->interrupt();
		mThread->join();
		mThread.reset();
	}
}

void CleanManager::operator ()()
{
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			int32_t chunkid;
			{
				boost::mutex::scoped_lock lock(mMutex);
				while (mDels.empty()) {
					mCond.wait(lock);
				}
				chunkid = mDels.front();
				mDels.pop_front();
			}

			int16_t nodeid = NODE_ID(chunkid);
			boost::shared_ptr<DatanodeThriftConnection> pConn;

			try
			{
				ThriftResult result;
				pConn = mServer.get_datanode_connection(nodeid);
				pConn->get_client()->del_chunk(result, chunkid);
				LOG_WARNING_SMART << "ask delete chuck " << chunkid;
			} catch (ErasureException &err) {
				LOG(ERROR) << "failed to del chunk " << chunkid << " : " << err.what();
			} catch (apache::thrift::TException &err) {
				LOG(ERROR) << "failed to del chunk " << chunkid << " : " << err.what();
				if (pConn.get())
					pConn->destroy();
			}
		}

	} catch (boost::thread_interrupted &err) {
		LOG(ERROR) << "clean manager interrupted";
	}
}

void CleanManager::request_delete (int32_t chunkid)
{
	boost::mutex::scoped_lock lock(mMutex);
	mDels.push_back(chunkid);
	mCond.notify_one();
}




