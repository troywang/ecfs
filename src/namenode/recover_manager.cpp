/*
 * recover_manager.cpp
 *
 *  Created on: Sep 3, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/namenode/name_node.h"
#include "ecfs/src/namenode/recover_manager.h"

RecoverManager::RecoverManager(NamenodeServer &server)
: ManagerBase(server)
{
}

RecoverManager::~RecoverManager()
{
	uninit();
}

void RecoverManager::init ()
{
	mMain.reset(new boost::thread(boost::ref(*this)));
	mExpire.reset(new boost::thread(boost::bind(&RecoverManager::expire_recovery, this)));
}

void RecoverManager::uninit ()
{
	if (mMain.get()) {
		mMain->interrupt();
		mMain->join();
		mMain.reset();
	}

	if (mExpire.get()) {
		mExpire->interrupt();
		mExpire->join();
		mExpire.reset();
	}
}

void RecoverManager::expire_recovery ()
{
	const int timeout = 20;

	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			{
				boost::mutex::scoped_lock lock(mMutex);

				while (mWorkList.empty()) {
					mWorkCond.wait(lock);
				}

				std::list<RecoveryItem>::iterator it;
				for (it = mWorkList.begin(); it != mWorkList.end();)
				{
					RecoveryItem item = *it;

					if (time(NULL) - item.active > timeout) {
						LOG_WARNING_SMART << "recovery expired, chunkid = " << item.chunkid;

						it = mWorkList.erase(it);
						del_recovery_raw(item.nodeid);

						mWaitList.push_back(item);
						mWaitCond.notify_one();

					} else {
						it++;
					}
				}
			}

			boost::this_thread::sleep(boost::posix_time::seconds(timeout + 2));
		}

	} catch (boost::thread_interrupted &err) {
		LOG(ERROR) << "expire manager interrupted";
	}
}

void RecoverManager::operator ()()
{
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			bool success = false;
			RecoveryItem item;
			int16_t recovery_node;
			FileRecoverToken token;

			do
			{
				//get one valid chunkid from waiting list
				while(true)
				{
					boost::mutex::scoped_lock lock(mMutex);
					while (mWaitList.empty()) {
						mWaitCond.wait(lock);
					}

					item = mWaitList.front();
					mWaitList.pop_front();

					std::map<int32_t, RecoveryInfo>::iterator it;
					it = mInfos.find(item.chunkid);
					if (it != mInfos.end()) { //recovery may have been cancelled
						RecoveryInfo ri = it->second;
						token.blockid = ri.blockid;
						token.oldchunk = ri.chunkid;
						break;
					}
				}

				//select new chunkid for recovering
				{
					int ret = mServer.get_block_manager().get_chunk_ids(token.blockid, /* out */token.chunkids, true);
					if (!RESULT_IS_SUCCESS(ret)) {
						LOG(ERROR) << "chunk ids not found for block " << token.blockid;
						break; //give up recovery
					}

					//ignore failure
					mServer.get_node_manager().make_chunkid_endpoints(token.chunkids, /* out */token.endpoints);

					std::set<int32_t> excludes;
					excludes.insert(token.chunkids.begin(), token.chunkids.end());
					excludes.erase(token.oldchunk);

					std::vector<int32_t> chunks;
					ret = mServer.get_block_manager().pick_chunks(1, excludes, chunks);
					if (!RESULT_IS_SUCCESS(ret)) {
						LOG(ERROR) << "failed to pick chunk for recovery, errcode = " << ret;
						break;
					}

					token.newchunk = chunks[0];

					ret = pick_recovery_node(/* out */recovery_node);
					if (!RESULT_IS_SUCCESS(ret)) {
						LOG(ERROR) << "failed to pick recovery node, errcode = " << ret;
						break;
					}
				}

				boost::shared_ptr<RecoveryThriftConnection> pConn;
				try
				{
					ThriftResult result;
					pConn = mServer.get_recovery_connection(recovery_node);
					pConn->get_client()->recover_chunk(result, token);

					success = true;
					LOG_WARNING_SMART << recovery_node << " start to recover block " << token.blockid << ", " << token.oldchunk << "=>" << token.newchunk;

				} catch (ErasureException &err) {
					LOG(ERROR) << "failed to recover: " << err.what();
				} catch (apache::thrift::TException &err) {
					if (pConn.get())
						pConn->destroy();
					LOG(ERROR) << "failed to recover: " << err.what();
				}
			} while (0);

			//process result
			{
				boost::mutex::scoped_lock lock(mMutex);
				if (success) {
					item.nodeid = recovery_node;
					item.active = time(NULL);

					mWorkList.push_back(item);
					mWorkCond.notify_one();

					add_recovery_raw(recovery_node);
				} else {
					mWaitList.push_back(item);
				}
			}

			if (!success)
				boost::this_thread::sleep(boost::posix_time::seconds(3));
		}
	} catch (boost::thread_interrupted &err) {
		LOG(ERROR) << "recover manager interrupted";
	}
}

void RecoverManager::start_recover(int32_t blockid, int32_t chunkid)
{
	boost::mutex::scoped_lock lock(mMutex);

	std::map<int32_t, RecoveryInfo>::iterator it;
	it = mInfos.find(chunkid);
	if (it != mInfos.end())
		return;

	mInfos.insert(std::make_pair(chunkid, RecoveryInfo(blockid, chunkid)));

	RecoveryItem item;
	item.chunkid = chunkid;
	mWaitList.push_back(item);
	mWaitCond.notify_one();

	LOG_WARNING_SMART << "start to recover " << blockid << ", " << chunkid;
}

int32_t RecoverManager::cancel_recovery (int32_t chunkid)
{
	boost::mutex::scoped_lock lock(mMutex);

	int32_t blockid = -1;
	std::map<int32_t, RecoveryInfo>::iterator it;
	it = mInfos.find(chunkid);
	if (it != mInfos.end()) {
		blockid = it->second.blockid;
		mInfos.erase(it);
	}

	if (blockid != -1) {
		LOG_WARNING_SMART << "cancel recovery blockid = " << blockid << ", chunkid = " << chunkid;
	}

	//process working list
	{
		std::list<RecoveryItem>::iterator it;
		for (it = mWorkList.begin(); it != mWorkList.end(); it++)
		{
			if (it->chunkid == chunkid) {
				del_recovery_raw(it->nodeid);
				mWorkList.erase(it);
				break;
			}
		}
	}

	return blockid;
}

bool RecoverManager::done_recovery (int32_t blockid, int32_t oldchunk, int32_t newchunk)
{
	boost::mutex::scoped_lock lock(mMutex);

	//process working list
	{
		std::list<RecoveryItem>::iterator it;
		for (it = mWorkList.begin(); it != mWorkList.end(); it++)
		{
			if (it->chunkid == oldchunk) {
				del_recovery_raw(it->nodeid);
				mWorkList.erase(it);
				break;
			}
		}
	}

	std::map<int32_t, RecoveryInfo>::iterator it;
	it = mInfos.find(oldchunk);
	if (it != mInfos.end())
	{
		LOG_WARNING_SMART << "done recovery blockid = " << blockid << ", oldid = " << oldchunk << ", newid = " << newchunk;
		mInfos.erase(it);
		return true;
	}

	return false;
}

void RecoverManager::add_recovery_raw (int16_t recovery_node)
{
	typedef std::map<int16_t, int16_t> RecoMap;
	RecoMap::iterator it = mRecos.find(recovery_node);
	if (it == mRecos.end()) {
		mRecos.insert(std::make_pair(recovery_node, 1));
	} else {
		it->second++;
	}
}

void RecoverManager::del_recovery_raw (int16_t recovery_node)
{
	typedef std::map<int16_t, int16_t> RecoMap;
	RecoMap::iterator it = mRecos.find(recovery_node);
	if (it != mRecos.end() && it->second > 0) {
		it->second--;
	}

	mRecoCond.notify_one();
}

int RecoverManager::pick_recovery_node (/* out */ int16_t &nodeid)
{
	typedef std::map<int16_t, int16_t> RecoMap;

	while (true)
	{
		std::set<int16_t> nodeids;
		mServer.get_node_manager().get_recovery_nodes(nodeids);

		if (nodeids.empty())
			return RESULT_ERR_INSUFFICIENT_RECOVERNODE;

		boost::mutex::scoped_lock lock(mMutex);

		for (std::set<int16_t>::iterator it = nodeids.begin(); it != nodeids.end(); it++)
		{
			nodeid = *it;
			RecoMap::iterator iit = mRecos.find(nodeid);
			if (iit == mRecos.end() || iit->second < FLAGS_recovery_maxperhost) {
				return RESULT_SUCCESS;
			}
		}

		mRecoCond.wait(lock);
	}

	return RESULT_ERR_PROG_BUG;
}


