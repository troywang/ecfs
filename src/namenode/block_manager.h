/*
 * file_manager.h
 *
 *  Created on: Sep 1, 2014
 *      Author: wanghuan
 */

#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#include "ecfs/src/common/common.h"
#include "ecfs/src/common/configs.h"
#include "ecfs/src/namenode/managerbase.h"

struct BlockInfo
{
	time_t ctime; //create time
	int32_t blockid;
	int32_t chunkids[0];

	static BlockInfo* new_one () {
		return (BlockInfo *)(calloc(1, sizeof(BlockInfo) + FLAGS_N * sizeof(int32_t)));
	}
};

struct DiskChunks
{
	DiskChunks () {
		load = 0;
	}

	int32_t load;
	std::set<int32_t> chunks;
};

struct NodeChunkInfo
{
	void get_inuse_chunkids (/* out */std::set<int32_t> &ids);

	bool has_enough_chunks ();

	void clear ();

	std::map<int8_t, DiskChunks> mChunkUnuse;
	std::map<int32_t, int32_t> mChunkInuse; //chunkid => blockid
};

typedef std::map<int32_t, BlockInfo*> BlockMap;
typedef std::map<int16_t, NodeChunkInfo*> NodeChunkMap;

class NamenodeServer;

class BlockManager : public ManagerBase
{
public:
	BlockManager (NamenodeServer &server);

	virtual ~BlockManager ();

	virtual void init ();

	virtual void uninit ();

	void operator ()();

	int32_t get_block_id ();

	void do_chunk_report (int16_t nodeid);

	void handle_datanode_dead (int16_t nodeid);

	int commit_block (int32_t blockid);

	int delete_block (int32_t blockid);

	void create_block (/* ref */const FileWriteToken &token);

	int adjust_block (/* ref */const FileWriteToken &token);

	void update_chunk (int32_t blockid, int32_t oldid, int32_t newid);

	int get_chunk_ids (int32_t blockid, /* out */std::vector<int32_t> &chunkids, bool committed);

	int pick_chunks (int32_t count, const std::set<int32_t> &exclues, /* out */std::vector<int32_t> &chunks);

	void get_committed_blocks (/* out */ std::set<int32_t> &blockids);

public:
	void add_block_raw (int32_t blockid);

	void add_chunk_raw (int32_t blockid, int32_t chunkid);

	void expire_chunk_raw (int32_t blockid, int32_t chunkid);

	void update_chunk_raw (int32_t blockid, int32_t oldid, int32_t newid);

	void del_block_raw (int32_t blockid);

private:
	int32_t pick_one_chunk (NodeChunkInfo *info);

	void process_chunks (int16_t nodeid, const std::set<int32_t> &chunkUnUse, const std::set<int32_t> &chunkInUse);

private:
	boost::mutex mMutex;

	int32_t mBlockId;
	BlockMap mCommitted;
	BlockMap mUnCommitted;
	NodeChunkMap mNodeChunks;
	std::auto_ptr<boost::thread> mExpireThread;
};

#endif /* FILE_MANAGER_H_ */
