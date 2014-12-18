/*
 * chunkmanager.cpp
 *
 *  Created on: Aug 27, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/common/utils.h"

#include "ecfs/src/datanode/data_node.h"
#include "ecfs/src/datanode/disk_manager.h"
#include "ecfs/src/datanode/chunk_manager.h"

ChunkManager::ChunkManager (DatanodeServer &server, DiskEntry &entry)
: ManagerBase(server), mEntry(entry)
{
}

ChunkManager::~ChunkManager ()
{
	uninit();
}

void ChunkManager::init ()
{
	int16_t nodeid = mServer.get_node_id();
	int8_t diskid = mEntry.get_id();
    std::string root = mEntry.get_path();
    ChunkManager::scan_sub_dirs(root, mSubDirs);

    LOG_WARNING_SMART << "start initialize datanode " << nodeid << ", disk " << (int)diskid << ": " << root;

    for (uint8_t i = 0; i < mSubDirs.size(); i++)
    {
    	std::set<int32_t> chunkInUse;
    	std::set<int32_t> chunkUnUse;
    	std::set<int32_t>::iterator it;

    	const std::string &dir = mSubDirs[i];

    	std::ostringstream oss;
    	oss << root << "/" << dir;
    	ChunkManager::scan_chunk_files(oss.str(), chunkInUse, chunkUnUse);

    	mChunkInUse.push_back(chunkInUse);
    	mChunkUnUse.push_back(chunkUnUse);

    	for (it = chunkInUse.begin(); it != chunkInUse.end(); it++) {
    		if (nodeid == NODE_ID(*it) && diskid == DISK_ID(*it)) {
    			mChunks.insert(std::make_pair(*it, i));
    		} else {
    			LOG(ERROR) << "found misplaced chunk " << *it;
    		}
    	}

    	for (it = chunkUnUse.begin(); it != chunkUnUse.end(); it++) {
    		if (nodeid == NODE_ID(*it) && diskid == DISK_ID(*it)) {
    			mChunks.insert(std::make_pair(*it, i));
    		} else {
    			LOG(ERROR) << "found misplace chunk " << *it;
    		}
    	}

    	mLocks.push_back(new boost::mutex());
    }
}

void ChunkManager::uninit ()
{
	mSubDirs.clear();
	mChunks.clear();
	mChunkInUse.clear();
	mChunkUnUse.clear();

	for (size_t i = 0; i < mLocks.size(); i++) {
		delete mLocks[i];
	}
	mLocks.clear();
}

void ChunkManager::put_chunk (int32_t chunkid, const std::string &data)
	throw (ErasureException)
{
	std::map<int32_t, int8_t>::iterator it = mChunks.find(chunkid);
	if (it == mChunks.end()) {
		LOG(ERROR) << "chunk not found " << chunkid;
		ErasureUtils::throw_exception(RESULT_ERR_CHUNK_NOT_FOUND, "chunk not on this node");
	}

	if (data.size() > (size_t)FLAGS_chunk_len)
		ErasureUtils::throw_exception(RESULT_ERR_CHUNK_LEN_OVERFLOW, "chunk len overflow");

	int8_t idx = it->second;
	std::set<int32_t> &chunkInUse = mChunkInUse[idx];
	std::set<int32_t> &chunkUnUse = mChunkUnUse[idx];

	boost::mutex::scoped_lock lock(*mLocks[idx]); //lock subdir

	if (chunkUnUse.count(chunkid) == 0) {
		LOG(ERROR) << "chunk not in unuse set " << chunkid;
		ErasureUtils::throw_exception(RESULT_ERR_CHUNK_NOT_FOUND, "chunk not in unuse set");
	}

	const std::string &root = mEntry.get_path();
	const std::string &subdir = mSubDirs[idx];

	std::ostringstream oss;
	oss << root << "/" << subdir << "/" << chunkid << CHUNK_UNUSE_SUFFIX;
	std::string unuse = oss.str();

	oss.str("");
	oss << root << "/" << subdir << "/" << chunkid << CHUNK_INUSE_SUFFIX;
	std::string inuse = oss.str();

	if (!ErasureFile::exists(unuse)) {
		LOG(ERROR) << "chunk not found on disk " << unuse;
		ErasureUtils::throw_exception(RESULT_ERR_CHUNK_NOT_FOUND, "chunk not found on disk");
	}

	ErasureFile ef;
	ef.open(unuse, "rb+");
	ef.seek(0, SEEK_SET);

	ef.write(data.data(), data.size());
	ef.close();

	ErasureFile::rename(unuse, inuse);

	chunkInUse.insert(chunkid);
	chunkUnUse.erase(chunkid);
}

void ChunkManager::del_chunk (int32_t chunkid)
	throw (ErasureException)
{
	std::map<int32_t, int8_t>::iterator it = mChunks.find(chunkid);
	if (it == mChunks.end()) {
		LOG(ERROR) << "chunk not on this node";
		return;
	}

	const std::string &root = mEntry.get_path();
	const std::string &subdir = mSubDirs[it->second];

	int8_t idx = it->second;
	boost::mutex::scoped_lock lock(*mLocks[idx]); //lock subdir

	std::set<int32_t> &chunkInUse = mChunkInUse[idx];
	std::set<int32_t> &chunkUnUse = mChunkUnUse[idx];

	if (chunkInUse.count(chunkid) == 0) {
		LOG(ERROR) << "chunk not in inuse set";
		return;
	}

	std::ostringstream oss;
	oss << root << "/" << subdir << "/" << chunkid << CHUNK_INUSE_SUFFIX;
	std::string inuse = oss.str();

	oss.str("");
	oss << root << "/" << subdir << "/" << chunkid << CHUNK_UNUSE_SUFFIX;
	std::string unuse = oss.str();

	if (ErasureFile::exists(inuse)) {
		ErasureFile::rename(inuse, unuse);
		chunkUnUse.insert(chunkid);
	}

	chunkInUse.erase(chunkid);

	LOG_WARNING_SMART << "successfully delete chunk " << chunkid;
}

void ChunkManager::get_chunk (int32_t chunkid, /* out */std::string &data)
	throw (ErasureException)
{
	read_data(chunkid, 0, FLAGS_chunk_len, data);
}

void ChunkManager::read_data (int32_t chunkid, int32_t offset, int32_t size, /* out */std::string &data)
	throw (ErasureException)
{
	std::map<int32_t, int8_t>::iterator it = mChunks.find(chunkid);
	if (it == mChunks.end()) {
		LOG(ERROR) << "chunk " << chunkid << " not found in inuse set";
		ErasureUtils::throw_exception(RESULT_ERR_CHUNK_NOT_FOUND, "chunk not in inuse set");
	}

	const std::string &root = mEntry.get_path();
	const std::string &subdir = mSubDirs[it->second];

	std::ostringstream oss;
	oss << root << "/" << subdir << "/" << chunkid << CHUNK_INUSE_SUFFIX;
	std::string inuse = oss.str();

	if (!ErasureFile::exists(inuse)) {
		LOG(ERROR) << "chunk not found on disk " << inuse;
		ErasureUtils::throw_exception(RESULT_ERR_CHUNK_NOT_FOUND, "chunk not found on disk");
	}

	ErasureFile ef;
	ef.open(inuse, "rb");
	ef.seek(offset, SEEK_SET);

	data.resize(size);

	ef.read((char *)&data.c_str()[0], size);
	ef.close();

}

void ChunkManager::get_chunk_ids (/* out */std::set<int32_t> &unuse, /* out */std::set<int32_t> &inuse)
{
	//do not clear unuse & inuse
	for (size_t i = 0; i < mSubDirs.size(); i++)
	{
		boost::mutex::scoped_lock lock(*mLocks[i]);
		unuse.insert(mChunkUnUse[i].begin(), mChunkUnUse[i].end());
		inuse.insert(mChunkInUse[i].begin(), mChunkInUse[i].end());
	}
}

void ChunkManager::scan_sub_dirs(const std::string &dir, /* out */std::vector<std::string> &subdirs)
	throw (boost::filesystem::filesystem_error)
{
	boost::filesystem::directory_iterator enditr;
	boost::filesystem::directory_iterator itr(dir);

	subdirs.clear();

	for (; itr != enditr; itr++)
	{
		if (boost::filesystem::is_directory(itr->status()))
		{
			if (boost::iends_with(itr->path().filename().string(), CHUNK_SUB_DIR_SUFFIX))
			{
				std::string dirname = itr->path().filename().string();
				subdirs.push_back(dirname);
			}
		}
	}
}

void ChunkManager::scan_chunk_files (const std::string &dir, /* out */std::set<int32_t> &chunkInUse, std::set<int32_t> &chunkUnUse)
	throw (boost::filesystem::filesystem_error)
{
	boost::filesystem::directory_iterator enditr;
	boost::filesystem::directory_iterator itr(dir);

	LOG_WARNING_SMART << "scan sub dir " << dir;

	for (; itr != enditr; itr++)
	{
		if (boost::filesystem::is_regular_file(itr->status()))
		{
			if (boost::iends_with(itr->path().filename().string(), CHUNK_INUSE_SUFFIX))
			{
				std::string filename = itr->path().filename().string();
				filename = filename.substr(0, filename.size() - CHUNK_INUSE_SUFFIX_LEN);

				int32_t id = ErasureUtils::parse_int(filename);
				chunkInUse.insert(id);
			}

			if (boost::iends_with(itr->path().filename().string(), CHUNK_UNUSE_SUFFIX))
			{
				std::string filename = itr->path().filename().string();
				filename = filename.substr(0, filename.size() - CHUNK_UNUSE_SUFFIX_LEN);

				int32_t id = ErasureUtils::parse_int(filename);
				chunkUnUse.insert(id);
			}
		}
	}
}
