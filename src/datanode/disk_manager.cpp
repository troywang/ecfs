/*
 * diskmanager.cpp
 *
 *  Created on: May 8, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/datanode/chunk_manager.h"
#include "ecfs/src/datanode/disk_manager.h"
#include "ecfs/src/datanode/data_node.h"

DiskEntry::DiskEntry(DiskManager &mgr, const std::string &path, int8_t diskid)
: mPath(path), mDiskMgr(mgr), mChunkMgr(mgr.get_server(), *this)
{
	mDiskId = diskid;
	mDisabled = false;
}

void DiskEntry::init ()
{
	mChunkMgr.init();
}

void DiskEntry::uninit ()
{
	mChunkMgr.uninit();
}

DiskManager::DiskManager(DatanodeServer &server)
: ManagerBase(server)
{
}

DiskManager::~DiskManager ()
{
	uninit();
}

void DiskManager::init ()
{
	std::map<int8_t, DiskEntry*>::iterator it;
	for (it = mEntries.begin(); it != mEntries.end(); it++) {
		it->second->init();
	}
}

void DiskManager::uninit ()
{
	std::map<int8_t, DiskEntry*>::iterator it;
	for (it = mEntries.begin(); it != mEntries.end(); it++) {
		it->second->uninit();
		delete it->second;
	}

	mEntries.clear();
}

void DiskManager::add_disk_path (const std::string &path, int8_t id)
{
	std::string p = path;
	if (!boost::iends_with(path, "/"))
		p += "/";

	boost::filesystem::path bpath(p);
	if (!boost::filesystem::exists(bpath.branch_path())) {
		LOG(ERROR) << "disk not exists " << path;
		return;
	}

	RWLockScopedGuard lock(mRWLock, RWLOCK_WRITE);
	std::map<int8_t, DiskEntry*>::iterator it = mEntries.find(id);
	if (it != mEntries.end()) {
		if (it->second->get_path() != p) {
			LOG(ERROR) << "disk path unmatch with previously added one ";
		}

		return;
	}

	mEntries.insert(std::make_pair(id, new DiskEntry(*this, p, id)));
}

void DiskManager::disable_disk_path (const std::string &path)
{
	RWLockScopedGuard lock(mRWLock, RWLOCK_WRITE);

	std::string p = path;
	if (!boost::iends_with(path, "/"))
		p += "/";

	std::map<int8_t, DiskEntry*>::iterator it;
	for (it = mEntries.begin(); it != mEntries.end(); it++)
	{
		if (it->second->get_path() == p) {
			it->second->disable();
			return;
		}
	}
}

void DiskManager::get_disk_paths (/* out */std::vector<std::string> &disks)
{
	RWLockScopedGuard lock(mRWLock, RWLOCK_READ);
	std::map<int8_t, DiskEntry*>::iterator it;
	for (it = mEntries.begin(); it != mEntries.end(); it++)
	{
		if (!it->second->is_disabled()) {
			disks.push_back(it->second->get_path());
		}
	}
}

DiskEntry& DiskManager::find_disk_entry(int32_t chunkid) throw (ErasureException)
{
	return get_disk_entry(DISK_ID(chunkid));
}

DiskEntry& DiskManager::get_disk_entry(int8_t diskid) throw (ErasureException)
{
	RWLockScopedGuard lock(mRWLock, RWLOCK_READ);
	std::map<int8_t, DiskEntry*>::iterator it;
	it = mEntries.find(diskid);
	if (it != mEntries.end())
		return *it->second;

	LOG(ERROR) << "disk entry not found " << diskid;
	ErasureUtils::throw_exception(RESULT_ERR_DISK_ENTRY_NOT_FOUND, "disk entry not found");
}

void DiskManager::get_chunk_ids (/* out */std::set<int32_t> &unuse, /* out */std::set<int32_t> &inuse)
{
	RWLockScopedGuard lock(mRWLock, RWLOCK_READ);
	std::map<int8_t, DiskEntry*>::iterator it;
	for (it = mEntries.begin(); it != mEntries.end(); it++)
	{
		it->second->get_chunk_manager().get_chunk_ids(unuse, inuse);
	}
}
