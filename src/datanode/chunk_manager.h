/*
 * chunkmanager.h
 *
 *  Created on: Aug 27, 2014
 *      Author: wanghuan
 */

#ifndef CHUNKMANAGER_H_
#define CHUNKMANAGER_H_

#include "ecfs/src/common/common.h"
#include "ecfs/src/datanode/managerbase.h"

#define CHUNK_SUB_DIR_SUFFIX ".sub"
#define CHUNK_INUSE_SUFFIX ".ck"
#define CHUNK_INUSE_SUFFIX_LEN 3
#define CHUNK_UNUSE_SUFFIX ".un"
#define CHUNK_UNUSE_SUFFIX_LEN 3

class DiskEntry;
class DatanodeServer;
class ChunkManager : public ManagerBase
{
public:
	ChunkManager (DatanodeServer &server, DiskEntry &entry);

	virtual ~ChunkManager ();

	virtual void init ();

	virtual void uninit ();

	void put_chunk (int32_t chunkid, const std::string &data) throw (ErasureException);

	void del_chunk (int32_t chunkid) throw (ErasureException);

	void get_chunk (int32_t chunkid, /* out */std::string &data) throw (ErasureException);

	void read_data (int32_t chunkid, int32_t offset, int32_t size, /* out */std::string &data) throw (ErasureException);

	void get_chunk_ids (/* out */std::set<int32_t> &unuse, /* out */std::set<int32_t> &inuse);

private:
	void scan_sub_dirs (const std::string &dir, /* out */std::vector<std::string> &subdirs)
		throw (boost::filesystem::filesystem_error);

	void scan_chunk_files (const std::string &dir, /* out */std::set<int32_t> &chunkInUse, std::set<int32_t> &chunkUnUse)
		throw (boost::filesystem::filesystem_error);

private:
	DiskEntry &mEntry;
	std::vector<boost::mutex*> mLocks; //lock for each subdir
	std::vector<std::string> mSubDirs; //subdirs
	std::map<int32_t, int8_t> mChunks; //chunkid => subdir index
	std::vector<std::set<int32_t> > mChunkInUse; //chunk inuse in each subdir
	std::vector<std::set<int32_t> > mChunkUnUse; //chunk unused in each subdir
};

#endif /* CHUNKMANAGER_H_ */
