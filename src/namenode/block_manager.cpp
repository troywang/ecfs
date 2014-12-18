/*
 * file_manager.cpp
 *
 *  Created on: Sep 1, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/namenode/name_node.h"
#include "ecfs/src/namenode/block_manager.h"

void NodeChunkInfo::get_inuse_chunkids(/* out */std::set<int32_t> &ids)
{
	std::map<int32_t, int32_t>::iterator it;
	for (it = mChunkInuse.begin(); it != mChunkInuse.end(); it++)
		ids.insert(it->first);
}

bool NodeChunkInfo::has_enough_chunks ()
{
	int cnt = 0;
	std::map<int8_t, DiskChunks>::iterator it;
	for (it = mChunkUnuse.begin(); it != mChunkUnuse.end(); it++) {
		cnt += it->second.chunks.size();
	}

	return cnt > 100;
}

void NodeChunkInfo::clear ()
{
	mChunkUnuse.clear();
	mChunkInuse.clear();
}

BlockManager::BlockManager (NamenodeServer &server)
:ManagerBase(server)
{
	mBlockId = 10000;
}

BlockManager::~BlockManager ()
{
	uninit();
}

void BlockManager::init ()
{
	mExpireThread.reset(new boost::thread(boost::ref(*this)));
}

void BlockManager::uninit ()
{
	boost::mutex::scoped_lock lock(mMutex);

	if (mExpireThread.get()) {
		mExpireThread->interrupt();
		mExpireThread->join();
		mExpireThread.reset();
	}

	for (BlockMap::iterator it = mCommitted.begin(); it != mCommitted.end(); it++) {
		delete it->second;
	}
	mCommitted.clear();

	for (BlockMap::iterator it = mUnCommitted.begin(); it != mUnCommitted.end(); it++) {
		delete it->second;
	}
	mUnCommitted.clear();

	for (NodeChunkMap::iterator it = mNodeChunks.begin(); it != mNodeChunks.end(); it++) {
		delete it->second;
	}
	mNodeChunks.clear();
}

void BlockManager::operator ()()
{
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			BlockMap::iterator it;
			{
				time_t now = time(NULL);
				boost::mutex::scoped_lock lock(mMutex);
				for (it = mUnCommitted.begin(); it != mUnCommitted.end();)
				{
					if (now - it->second->ctime > 600) {
						LOG_WARNING_SMART << "uncommitted block " << it->first << " expired, remove it";
						mUnCommitted.erase(it++);
					} else {
						it++;
					}
				}
			}

			boost::this_thread::sleep(boost::posix_time::seconds(600));
		}
	} catch (boost::thread_interrupted &err) {
		LOG(ERROR) << "block manager interrupted";
	}
}

void BlockManager::add_block_raw (int32_t blockid)
{
	mCommitted.insert(std::make_pair(blockid, BlockInfo::new_one()));
	if (blockid > mBlockId)
		mBlockId = blockid;
}

int32_t BlockManager::get_block_id ()
{
	boost::mutex::scoped_lock lock(mMutex);
	return mBlockId++;
}

void BlockManager::add_chunk_raw (int32_t blockid, int32_t chunkid)
{
	{
		BlockMap::iterator it = mCommitted.find(blockid);
		if (it == mCommitted.end()) {
			LOG(ERROR) << "block not found " << blockid;
			return;
		}

		BlockInfo *bi = it->second;

		for (int i = 0; i < FLAGS_N; i++) {
			if (bi->chunkids[i] == 0) {
				bi->chunkids[i] = chunkid;
				break;
			}
		}
	}

	NodeChunkInfo *info;
	int16_t nodeid = NODE_ID(chunkid);
	NodeChunkMap::iterator it = mNodeChunks.find(nodeid);
	if (it == mNodeChunks.end()) {
		info = new NodeChunkInfo();
		mNodeChunks.insert(std::make_pair(nodeid, info));
		mServer.get_node_manager().add_datanode_raw(nodeid);
	} else {
		info = it->second;
	}

	info->mChunkInuse.insert(std::make_pair(chunkid, blockid));
}

void BlockManager::expire_chunk_raw (int32_t blockid, int32_t chunkid)
{
	int16_t nodeid = NODE_ID(chunkid);
	NodeChunkMap::iterator it = mNodeChunks.find(nodeid);
	if (it == mNodeChunks.end()) {
		LOG(ERROR) << "block not found " << blockid;
		return;
	}

	it->second->mChunkInuse.erase(chunkid);
}

void BlockManager::update_chunk (int32_t blockid, int32_t oldid, int32_t newid)
{
	boost::mutex::scoped_lock lock(mMutex);
	update_chunk_raw(blockid, oldid, newid);
}

void BlockManager::update_chunk_raw (int32_t blockid, int32_t oldid, int32_t newid)
{
	{
		BlockMap::iterator it = mCommitted.find(blockid);
		if (it == mCommitted.end()) {
			LOG(ERROR) << "block not found " << blockid;
			return;
		}

		int i;
		BlockInfo *bi = it->second;
		for (i = 0; i < FLAGS_N; i++) {
			if (bi->chunkids[i] == oldid) {
				bi->chunkids[i] = newid;
				break;
			}
		}

		if (i == FLAGS_N) {
			LOG(ERROR) << "!!!BUG!!!, not found chunk id " << oldid << " in block " << blockid;
			return;
		}
	}

	NodeChunkInfo *info;
	int16_t nodeid = NODE_ID(oldid);
	NodeChunkMap::iterator it = mNodeChunks.find(nodeid);
	if (it != mNodeChunks.end()) {
		info = it->second;
		info->mChunkInuse.erase(oldid);
	}

	nodeid = NODE_ID(newid);
	it = mNodeChunks.find(nodeid);
	if (it == mNodeChunks.end()) {
		info = new NodeChunkInfo();
		mNodeChunks.insert(std::make_pair(nodeid, info));
		mServer.get_node_manager().add_datanode_raw(nodeid);
	} else {
		info = it->second;
	}

	info->mChunkInuse.insert(std::make_pair(newid, blockid));
}

void BlockManager::del_block_raw (int32_t blockid)
{
	BlockMap::iterator it = mCommitted.find(blockid);
	if (it == mCommitted.end()) {
		LOG(ERROR) << "block not found " << blockid;
		return;
	}

	BlockInfo *info = it->second;
	for (int i = 0; i < FLAGS_N; i++)
	{
		int32_t cid = info->chunkids[i];
		int16_t nid = NODE_ID(cid);

		NodeChunkMap::iterator it = mNodeChunks.find(nid);
		if (it != mNodeChunks.end()) {
			it->second->mChunkInuse.erase(cid);
		}
	}

	mCommitted.erase(it);
	delete info;
}

void BlockManager::do_chunk_report (int16_t nodeid)
{
	bool success = false;
	boost::shared_ptr<DatanodeThriftConnection> pConn;
	try
	{
		ResultReportChunks _ret;
		pConn = mServer.get_datanode_connection(nodeid);
		pConn->get_client()->report_chunks(_ret);
		success = RESULT_IS_SUCCESS(_ret.result.code);
		if (success) {
			process_chunks(nodeid, _ret.chunkUnUse, _ret.chunkInUse);
		}
	} catch (ErasureException &err) {
		LOG(ERROR) << "failed to do chunk report: " << err.what();
	} catch (apache::thrift::TException &err) {
		LOG(ERROR) << "failed to do chunk report: " << err.what();
		if (pConn.get())
			pConn->destroy();
	}

	mServer.done_chunk_report(nodeid, success);
}

void BlockManager::handle_datanode_dead (int16_t nodeid)
{
	boost::mutex::scoped_lock lock(mMutex);

	NodeChunkMap::iterator it = mNodeChunks.find(nodeid);
	if (it == mNodeChunks.end()) {
		LOG(ERROR) << "data node not found " << nodeid;
		return;
	}

	NodeChunkInfo *info = it->second;

	LOG_WARNING_SMART << "expire and start recover all chunks inuse on data " << nodeid;

	for (std::map<int32_t, int32_t>::iterator it = info->mChunkInuse.begin(); it != info->mChunkInuse.end(); it++)
	{
		int32_t chunkid = it->first;
		int32_t blockid = it->second;
		mServer.get_oplog_manager().expire_chunk(blockid, chunkid);
		mServer.get_recover_manager().start_recover(blockid, chunkid);
	}

	info->clear();
}

int BlockManager::pick_chunks (int32_t count, const std::set<int32_t> &excludes, /* out */std::vector<int32_t> &chunks)
{
	std::set<int16_t> exclude_nodes;
	for (std::set<int32_t>::iterator it = excludes.begin(); it != excludes.end(); it++)
	{
		int16_t nodeid = NODE_ID(*it);
		exclude_nodes.insert(nodeid);
	}

	boost::mutex::scoped_lock lock(mMutex);
	for (NodeChunkMap::iterator it = mNodeChunks.begin(); it != mNodeChunks.end(); it++)
	{
		if (!it->second->has_enough_chunks())
			exclude_nodes.insert(it->first);
	}

	std::vector<int16_t> picked_nodes;
	int ret = mServer.get_node_manager().pick_data_nodes(count, exclude_nodes, picked_nodes);
	if (!RESULT_IS_SUCCESS(ret))
		return ret;

	for (size_t i = 0; i < picked_nodes.size(); i++)
	{
		int16_t nodeid = picked_nodes[i];
		NodeChunkMap::iterator it = mNodeChunks.find(nodeid);
		if (it == mNodeChunks.end()) {
			LOG(ERROR) << "!!!BUG!!!, not found node chunk info " << nodeid;
			return RESULT_ERR_PROG_BUG;
		}

		NodeChunkInfo *info = it->second;
		int32_t chunkid = pick_one_chunk(info);
		if (chunkid == -1) {
			LOG(ERROR) << "!!!BUG!!!, cannot pick one chunk ";
			return RESULT_ERR_PROG_BUG;
		}

		chunks.push_back(chunkid);

		LOG_INFO_SMART << "pick chunk " << chunkid << " from " << nodeid;
	}

	return RESULT_SUCCESS;
}

void BlockManager::create_block (/* ref */const FileWriteToken &token)
{
	boost::mutex::scoped_lock lock(mMutex);

	int32_t blockid = token.blockid;
	const std::vector<int32_t> &chunkids = token.chunkids;
	BlockInfo *info = BlockInfo::new_one();
	mUnCommitted.insert(std::make_pair(blockid, info));

	for (size_t i = 0; i < chunkids.size(); i++)
	{
		info->chunkids[i] = chunkids[i];
	}
	info->blockid = blockid;
	info->ctime = time(NULL);
}

int BlockManager::adjust_block (/* ref */const FileWriteToken &token)
{
	boost::mutex::scoped_lock lock(mMutex);

	int32_t blockid = token.blockid;
	const std::vector<int32_t> &chunkids = token.chunkids;

	BlockMap::iterator it = mUnCommitted.find(blockid);
	if (it == mUnCommitted.end())
		return RESULT_ERR_BLOCK_NOT_FOUND;

	BlockInfo *bi = it->second;
	for (size_t i = 0; i < chunkids.size(); i++)
	{
		bi->chunkids[i] = chunkids[i];
	}

	return RESULT_SUCCESS;
}

int BlockManager::commit_block (int32_t blockid)
{
	boost::mutex::scoped_lock lock(mMutex);

	BlockMap::iterator it = mUnCommitted.find(blockid);
	if (it == mUnCommitted.end()) {
		return RESULT_ERR_BLOCK_NOT_FOUND;
	}

	BlockInfo *info = it->second;
	mCommitted.insert(std::make_pair(blockid, info));

	for (int i = 0; i < FLAGS_N; i++)
	{
		int32_t cid = info->chunkids[i];
		int16_t nid = NODE_ID(cid);
		NodeChunkMap::iterator it = mNodeChunks.find(nid);
		if (it == mNodeChunks.end()) {
			LOG(ERROR) << "!!!BUG!!!, not found not chunk info" << nid;
			return RESULT_ERR_PROG_BUG;
		}

		NodeChunkInfo *info = it->second;
		info->mChunkInuse.insert(std::make_pair(cid, blockid));
	}

	mUnCommitted.erase(it);

	return RESULT_SUCCESS;
}

int BlockManager::get_chunk_ids (int32_t blockid, /* out */std::vector<int32_t> &chunkids, bool committed)
{
	boost::mutex::scoped_lock lock(mMutex);

	BlockMap::iterator it;
	BlockInfo *info  = NULL;
	if (committed)
	{
		it = mCommitted.find(blockid);
		if (it != mCommitted.end()) {
			info = it->second;
		}
	}
	else
	{
		it = mUnCommitted.find(blockid);
		if (it != mCommitted.end()) {
			info = it->second;
		}
	}

	if (info == NULL)
		return RESULT_ERR_BLOCK_NOT_FOUND;

	for (int i = 0; i < FLAGS_N; i++)
	{
		int32_t cid = info->chunkids[i];
		chunkids.push_back(cid);
	}

	return RESULT_SUCCESS;
}

int BlockManager::delete_block (int32_t blockid)
{
	boost::mutex::scoped_lock lock(mMutex);

	BlockMap::iterator it = mCommitted.find(blockid);
	if (it == mCommitted.end())
		return RESULT_SUCCESS;

	int ret = mServer.get_oplog_manager().del_block(blockid);
	if (!RESULT_IS_SUCCESS(ret))
		return ret;

	BlockInfo *info = it->second;
	for (int i = 0; i < FLAGS_N; i++)
	{
		int32_t cid = info->chunkids[i];
		int16_t nid = NODE_ID(cid);
		NodeChunkMap::iterator it = mNodeChunks.find(nid);
		if (it == mNodeChunks.end()) {
			LOG(ERROR) << "!!!BUG!!!, not found node chunks " << nid;
			continue;
		}

		it->second->mChunkInuse.erase(cid);
	}
	mCommitted.erase(it);

	return RESULT_SUCCESS;
}

void BlockManager::get_committed_blocks (/* out */ std::set<int32_t> &blockids)
{
	boost::mutex::scoped_lock lock(mMutex);

	BlockMap::iterator it;
	for (it = mCommitted.begin(); it != mCommitted.end(); it++) {
		blockids.insert(it->first);
	}
}

int32_t BlockManager::pick_one_chunk (NodeChunkInfo *info)
{
	int32_t chunkid = -1;
	int min_load = INT_MAX;

	DiskChunks *pDisk = NULL;
	for (std::map<int8_t, DiskChunks>::iterator it = info->mChunkUnuse.begin();
			it != info->mChunkUnuse.end(); it++)
	{
		int32_t cur = it->second.load;
		std::set<int32_t> &chunks = it->second.chunks;
		if (cur < min_load && !chunks.empty()) {
			min_load = cur;
			pDisk = &it->second;
		}
	}

	if (pDisk != NULL) {
		chunkid = *pDisk->chunks.begin();
		pDisk->chunks.erase(chunkid);
		pDisk->load++;
	}

	return chunkid;
}

void BlockManager::process_chunks (int16_t nodeid, const std::set<int32_t> &chunkUnUse, const std::set<int32_t> &chunkInUse)
{
	boost::mutex::scoped_lock lock(mMutex);

	LOG_WARNING_SMART << "start to process chunks from data node " << nodeid;

	NodeChunkInfo *info;
	NodeChunkMap::iterator it = mNodeChunks.find(nodeid);
	if (it == mNodeChunks.end()) {
		info = new NodeChunkInfo();
		mNodeChunks.insert(std::make_pair(nodeid, info));
	} else {
		info = it->second;
	}

	info->mChunkUnuse.clear();
	for (std::set<int32_t>::iterator it = chunkUnUse.begin(); it != chunkUnUse.end(); it++)
	{
		int32_t chunkid = *it;

		std::pair<std::map<int8_t, DiskChunks>::iterator, bool> pair;
		pair = info->mChunkUnuse.insert(std::make_pair(DISK_ID(chunkid), DiskChunks()));
		pair.first->second.chunks.insert(chunkid);
	}

	std::vector<int32_t> c1; //inuse chunks on datanode newly reported
	std::vector<int32_t> c2; //inuse chunks missed on datanode

	std::set<int32_t> inuse;
	info->get_inuse_chunkids(inuse);

	std::set_difference(chunkInUse.begin(), chunkInUse.end(), inuse.begin(), inuse.end(), std::back_inserter(c1));
	std::set_difference(inuse.begin(), inuse.end(), chunkInUse.begin(), chunkInUse.end(), std::back_inserter(c2));

	if (!c1.empty()) //chunks inuse on datanode but not on namenode
	{
		for (size_t i = 0; i < c1.size(); i++)
		{
			int32_t chunkid = c1[i];
			int32_t blockid = mServer.get_recover_manager().cancel_recovery(chunkid);
			if (blockid != -1) {
				info->mChunkInuse.insert(std::make_pair(chunkid, blockid));
				mServer.get_oplog_manager().update_chunk(blockid, chunkid, chunkid);
			} else {
				mServer.get_clean_manager().request_delete(chunkid);
			}
		}
	}

	if (!c2.empty()) //chunks inuse on namenode but not on datanode
	{
		for (size_t i = 0; i < c2.size(); i++)
		{
			int32_t chunkid = c2[i];
			int32_t blockid = info->mChunkInuse.find(chunkid)->second;
			info->mChunkInuse.erase(chunkid);
			mServer.get_oplog_manager().expire_chunk(blockid, chunkid);
			mServer.get_recover_manager().start_recover(blockid, chunkid);
		}
	}
}

