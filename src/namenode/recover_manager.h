/*
 * recover_manager.h
 *
 *  Created on: Sep 3, 2014
 *      Author: wanghuan
 */

#ifndef RECOVER_MANAGER_H_
#define RECOVER_MANAGER_H_

#include "ecfs/src/common/common.h"
#include "ecfs/src/namenode/managerbase.h"

struct RecoveryInfo
{
	RecoveryInfo () {
		memset(this, 0, sizeof(RecoveryInfo));
	}

	RecoveryInfo (int32_t bid, int32_t cid) {
		blockid = bid;
		chunkid = cid;
	}

	int32_t blockid;
	int32_t chunkid;
};

struct RecoveryItem
{
	int32_t chunkid;
	int16_t nodeid;
	time_t active;
};

class NamenodeServer;
class RecoverManager : public ManagerBase
{
public:
	RecoverManager (NamenodeServer &server);

	virtual ~RecoverManager ();

	void operator ()();

	virtual void init ();

	virtual void uninit ();

	void expire_recovery ();

	void start_recover (int32_t blockid, int32_t chunkid);

	bool done_recovery (int32_t blockid, int32_t oldchunk, int32_t newchunk);

	int32_t cancel_recovery (int32_t chunkid);

private:
	void add_recovery_raw (int16_t recovery_node);

	void del_recovery_raw (int16_t recovery_node);

	int pick_recovery_node (/* out */ int16_t &nodeid);

private:
	boost::mutex mMutex;
	boost::condition_variable mWaitCond; //cond for waiting list
	boost::condition_variable mWorkCond; //cond for working list

	boost::condition_variable mRecoCond; //cond for picking recovery node

	std::list<RecoveryItem> mWaitList;
	std::list<RecoveryItem> mWorkList;

	std::map<int32_t, RecoveryInfo> mInfos;
	std::map<int16_t, int16_t> mRecos;

	std::auto_ptr<boost::thread> mMain;
	std::auto_ptr<boost::thread> mExpire;
};

#endif /* RECOVER_MANAGER_H_ */
