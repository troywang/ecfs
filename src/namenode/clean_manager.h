/*
 * clean_manager.h
 *
 *  Created on: Sep 9, 2014
 *      Author: wanghuan
 */

#ifndef CLEAN_MANAGER_H_
#define CLEAN_MANAGER_H_

#include "ecfs/src/common/common.h"
#include "ecfs/src/namenode/managerbase.h"

struct DelInfo
{
	int32_t chunkid;

	DelInfo () {
		chunkid = 0;
	}

	DelInfo (int32_t cid) {
		chunkid = cid;
	}
};

class NamenodeServer;
class CleanManager : public ManagerBase
{
public:
	CleanManager (NamenodeServer &server);

	virtual ~CleanManager ();

	virtual void init ();

	virtual void uninit ();

	void operator ()();

	void request_delete (int32_t chunkid);

private:
	boost::mutex mMutex;
	std::list<int32_t> mDels;
	boost::condition_variable mCond;
	std::auto_ptr<boost::thread> mThread;
};

#endif /* CLEAN_MANAGER_H_ */
