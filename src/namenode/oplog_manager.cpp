/*
 * oplog.cpp
 *
 *  Created on: Jul 8, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/common/utils.h"
#include "ecfs/src/namenode/name_node.h"
#include "ecfs/src/namenode/oplog_manager.h"

OpLogManager::OpLogManager (NamenodeServer &server)
: ManagerBase(server)
{
	mMaxId = 0;
	mOffset = 0;
}

OpLogManager::~OpLogManager()
{
	uninit();
}

void OpLogManager::set_log_dir (const std::string &dir)
{
	std::string path = dir;
    if (!boost::iends_with(path, "/"))
        path = path + "/";

    if (!ErasureFile::exists(path)) {
        LOG(ERROR) << "oplog dir not exists: " << dir;
        return;
    }

	mLogDir = path;
}

void OpLogManager::init()
{
	LOG_WARNING_SMART << "init oplog manager, logdir: " << mLogDir;

	boost::filesystem::directory_iterator end_itr;
	boost::filesystem::directory_iterator itr(mLogDir);

	std::set<uint32_t> ids;
	for (; itr != end_itr; ++itr)
	{
		if (boost::filesystem::is_regular_file(itr->status()))
		{
			std::string filename = itr->path().filename().string();
			if (boost::iends_with(filename, OP_LOG_FILE_EXT))
			{
				uint32_t id = ErasureUtils::parse_int(filename);
				ids.insert(id);
			}
		}
	}

	if (!ids.empty()) {
		mMaxId = *ids.rbegin();
	}

	for (std::set<uint32_t>::iterator it = ids.begin(); it != ids.end(); it++)
	{
		std::ostringstream oss;
		oss << mLogDir << "/" << *it << OP_LOG_FILE_EXT;

		LOG_WARNING_SMART << "load oplogs from " << oss.str();

		load_oplogs(*it, oss.str());
	}
}

void OpLogManager::uninit()
{
	mMaxId = 0;
	mOffset = 0;
}

void OpLogManager::load_oplogs (uint32_t id, const std::string &filepath)
{
	AddBlockOp *op1;
	AddChunkOp *op2;
	ExpireChunkOp *op3;
	UpdateChunkOp *op4;
	DelBlockOp *op5;
	OpLogHeader *header;

	char buf[128];
	size_t offset = 0;

	try
	{
		ErasureFile ef;
		ef.open(filepath, "rb");

		int32_t sz = ef.size();

		while (offset < sz - sizeof(OpLogHeader))
		{
			ef.read(buf, sizeof(OpLogHeader));

			header = (OpLogHeader *)buf;

			switch (header->action)
			{
			case OpLogHeader::OpLogAddBlock:
				ef.read(buf + sizeof(OpLogHeader), sizeof(AddBlockOp) - sizeof(OpLogHeader));
				op1 = (AddBlockOp *)buf;
				mServer.get_block_manager().add_block_raw(header->blockid);
				offset += sizeof(AddBlockOp);
				break;

			case OpLogHeader::OpLogAddChunk:
				ef.read(buf + sizeof(OpLogHeader), sizeof(AddChunkOp) - sizeof(OpLogHeader));
				op2 = (AddChunkOp *)buf;
				mServer.get_block_manager().add_chunk_raw(header->blockid, op2->chunkid);
				offset += sizeof(AddChunkOp);
				break;

			case OpLogHeader::OpLogExpireChunk:
				ef.read(buf + sizeof(OpLogHeader), sizeof(ExpireChunkOp) - sizeof(OpLogHeader));
				op3 = (ExpireChunkOp *)buf;
				mServer.get_block_manager().expire_chunk_raw(header->blockid, op3->chunkid);
				mServer.get_recover_manager().start_recover(header->blockid, op3->chunkid);
				offset += sizeof(ExpireChunkOp);
				break;

			case OpLogHeader::OpLogUpdateChunk:
				ef.read(buf + sizeof(OpLogHeader), sizeof(UpdateChunkOp) - sizeof(OpLogHeader));
				op4 = (UpdateChunkOp *)buf;
				mServer.get_block_manager().update_chunk_raw(header->blockid, op4->oldid, op4->newid);
				mServer.get_recover_manager().cancel_recovery(op4->oldid);
				offset += sizeof(UpdateChunkOp);
				break;

			case OpLogHeader::OpLogDelBlock:
				ef.read(buf + sizeof(OpLogHeader), sizeof(DelBlockOp) - sizeof(OpLogHeader));
				mServer.get_block_manager().del_block_raw(header->blockid);
				offset += sizeof(DelBlockOp);
				break;

			default:
				break;
			}
		}
		ef.close();

		if (id == mMaxId) {
			mOffset = offset;
			LOG_WARNING_SMART << "done load the latest oplog file, id = " << id << ", offset = " << offset;
		}
	} catch (ErasureException& err) {
		LOG(ERROR) << "failed to load oplogs from " << offset << ", err: " << err.what();
		if (id == mMaxId) {
			mMaxId++;
		}
	}
}

int OpLogManager::add_block (int32_t blockid, time_t tm)
{
	AddBlockOp op;
	op.header.blockid = blockid;
	op.header.action = OpLogHeader::OpLogAddBlock;
	op.tm = tm;
	op.header.crc = ErasureUtils::crc_8((char *)&op, sizeof(op));

	return flush(&op, sizeof(op));
}

int OpLogManager::add_chunk (int32_t blockid, int32_t chunkid)
{
	AddChunkOp op;
	op.header.blockid = blockid;
	op.header.action = OpLogHeader::OpLogAddChunk;
	op.chunkid = chunkid;
	op.header.crc = ErasureUtils::crc_8((char *)&op, sizeof(op));

	return flush(&op, sizeof(op));
}

int OpLogManager::update_chunk (int32_t blockid, int32_t oldid, int32_t newid)
{
	UpdateChunkOp op;
	op.header.blockid = blockid;
	op.header.action = OpLogHeader::OpLogUpdateChunk;
	op.oldid = oldid;
	op.newid = newid;
	op.header.crc = ErasureUtils::crc_8((char *)&op, sizeof(op));

	return flush(&op, sizeof(op));
}

int OpLogManager::expire_chunk (int32_t blockid, int32_t chunkid)
{
	ExpireChunkOp op;
	op.header.blockid = blockid;
	op.header.action = OpLogHeader::OpLogExpireChunk;
	op.chunkid = chunkid;
	op.header.crc = ErasureUtils::crc_8((char *)&op, sizeof(op));

	return flush(&op, sizeof(op));
}

int OpLogManager::del_block (int32_t blockid)
{
	DelBlockOp op;
	op.header.blockid = blockid;
	op.header.action = OpLogHeader::OpLogDelBlock;
	op.header.crc = ErasureUtils::crc_8((char *)&op, sizeof(op));

	return flush(&op, sizeof(op));
}

int OpLogManager::flush (void *data, uint32_t len)
{
	boost::mutex::scoped_lock lock(mMutex);

	try
	{
		if (len > OP_LOG_FILE_MAX_LEN - mOffset) {
			mMaxId++;
			mOffset = 0;
			mFile.close();
		}

		if (!mFile.is_open())
		{
			std::ostringstream oss;
			oss << mLogDir << "/" << mMaxId << OP_LOG_FILE_EXT;
			std::string path = oss.str();

			if (!ErasureFile::exists(path)) {
				LOG_WARNING_SMART << "create new oplog file " << path;
				mFile.open(path, "wb");
				mOffset = 0;
			} else {
				mFile.open(path, "rb+");
			}

			mFile.seek(mOffset, SEEK_SET);
		}

		mFile.write(data, len);
		mFile.flush();

		mOffset += len;
		return RESULT_SUCCESS;
	} catch (ErasureException &err) {
		LOG(ERROR) << "failed to flush data " << err.what();
		mOffset = OP_LOG_FILE_MAX_LEN;
		return err.get_error_code();
	}
}


