/*
 * oplog.h
 *
 *  Created on: Jul 8, 2014
 *      Author: wanghuan
 */

#ifndef OPLOG_H_
#define OPLOG_H_

#include "ecfs/src/common/common.h"
#include "ecfs/src/common/configs.h"
#include "ecfs/src/namenode/managerbase.h"

#define OP_LOG_FILE_EXT ".opl"
#define OP_LOG_FILE_EXT_LEN 4

#define OP_LOG_FILE_MAX_LEN 512*1024

struct OpLogHeader
{
	enum Action
	{
		OpLogAddBlock, //new block
		OpLogAddChunk, //new chunk
		OpLogUpdateChunk,//has been recovered
		OpLogExpireChunk, //need to be recovered
		OpLogDelBlock, //del block
	};

	int32_t blockid;
	Action action;
	uint8_t crc;

	OpLogHeader () {
		memset(this, 0, sizeof(OpLogHeader));
	}

	bool is_empty() {
		return blockid == 0 && action == 0 && crc == 0;
	}
};

struct AddBlockOp
{
	OpLogHeader header;
	int32_t tm;
	AddBlockOp () {
		tm = 0;
	}
};

struct AddChunkOp
{
	OpLogHeader header;
	int32_t chunkid;
	AddChunkOp () {
		chunkid = 0;
	}
};

struct UpdateChunkOp
{
	OpLogHeader header;
	int32_t oldid;
	int32_t newid;
	UpdateChunkOp () {
		oldid = newid = 0;
	}
};

struct ExpireChunkOp
{
	OpLogHeader header;
	int32_t chunkid;
	ExpireChunkOp () {
		chunkid = 0;
	}
};

struct DelBlockOp
{
	OpLogHeader header;
	int32_t tm;
	DelBlockOp () {
		tm = 0;
	}
};

class NamenodeServer;
class OpLogManager : public ManagerBase
{
public:
	OpLogManager(NamenodeServer &server);

	virtual ~OpLogManager ();

	virtual void init();

	virtual void uninit ();

	void set_log_dir (const std::string &dir);

	int add_block (int32_t blockid, time_t tm);

	int add_chunk (int32_t blockid, int32_t chunkid);

	int expire_chunk (int32_t blockid, int32_t chunkid);

	int update_chunk (int32_t blockid, int32_t oldid, int32_t newid);

	int del_block (int32_t blockid);

private:
	void load_oplogs (uint32_t id, const std::string &filepath);

	int flush (void *data, uint32_t len);

private:
	boost::mutex mMutex;
	uint32_t mOffset;
	uint32_t mMaxId;
	std::string mLogDir;
	ErasureFile mFile;
};

#endif /* OPLOG_H_ */
