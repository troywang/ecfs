/*
 * diskmanager.h
 *
 *  Created on: May 8, 2014
 *      Author: wanghuan
 */

#ifndef DISKMANAGER_H_
#define DISKMANAGER_H_

#include "ecfs/src/common/common.h"
#include "ecfs/src/datanode/managerbase.h"
#include "ecfs/src/datanode/chunk_manager.h"

class DiskManager;
class ChunkManager;

class DiskEntry
{
public:
	DiskEntry(DiskManager &mgr, const std::string &path, int8_t diskid);

	int8_t get_id () {
		return mDiskId;
	}

	bool is_disabled () {
		return mDisabled;
	}

	void disable () {
		mDisabled = true;
	}

	const std::string& get_path () {
		return mPath;
	}

	ChunkManager& get_chunk_manager () {
		return mChunkMgr;
	}

	DiskManager& get_disk_manager () {
		return mDiskMgr;
	}

	void init ();

	void uninit ();

private:
	int8_t mDiskId;
	bool mDisabled;
	std::string mPath;
	DiskManager &mDiskMgr;
	ChunkManager mChunkMgr;
};

class DiskManager : public ManagerBase
{
public:
	DiskManager (DatanodeServer &server);

	~DiskManager ();

	virtual void init ();

	virtual void uninit ();

	void add_disk_path (const std::string &path, int8_t id);

	void disable_disk_path (const std::string &path);

	void get_disk_paths (/* out */std::vector<std::string> &disks);

	DiskEntry& get_disk_entry(int8_t diskid) throw (ErasureException);

	DiskEntry& find_disk_entry(int32_t chunkid) throw (ErasureException);

	void get_chunk_ids (/* out */std::set<int32_t> &unuse, /* out */std::set<int32_t> &inuse);

private:
	RWLock mRWLock;
	std::map<int8_t, DiskEntry*> mEntries;
};

#endif /* DISKMANAGER_H_ */
