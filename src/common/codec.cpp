/*
 * codec.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/common/common.h"

ErasureCodec::ErasureCodec(size_t k, size_t n, size_t len)
:mFec(k, n)
{
	K = k;
	N = n;
	mLen = len;
}

void ErasureCodec::encode()
{
	int off = 0;
	std::vector<fecpp::byte> input;
	std::map<size_t, std::string>::iterator it;

	input.resize(FLAGS_K * FLAGS_chunk_len);

	for (int i = 0; i < FLAGS_K; i++)
	{
		int32_t chunkid = mChunkIds[i];

		it = mData.find(chunkid);
		if (it == mData.end()) {
			break;
		}

		memcpy(&input[off], it->second.data(), FLAGS_chunk_len);
		off += FLAGS_chunk_len;
	}

	mFec.encode(&input[0], input.size(), std::tr1::ref(*this));
}

void ErasureCodec::decode()
{
	std::map<size_t, const::fecpp::byte*> tmp;
	std::map<size_t, std::string>::iterator it;

	for (size_t i = 0; i < mChunkIds.size(); i++)
	{
		int32_t chunkid = mChunkIds[i];
		it = mData.find(chunkid);
		if (it != mData.end()) {
			tmp.insert(std::make_pair(i, (::fecpp::byte*)it->second.data()));
		}
	}

	mFec.decode(tmp, mLen, std::tr1::ref(*this));
}

void ErasureCodec::operator()(size_t idx, size_t N, const fecpp::byte share[], size_t len)
{
	int32_t chunkid = mChunkIds[idx];
	if (mData.find(chunkid) == mData.end()) {
		std::string &data = mData.insert(std::make_pair(chunkid, std::string())).first->second;
		data.assign((char *)share, len);
	}
}

void ErasureCodec::add_chunk(int32_t chunkid, const std::string &data)
{
	mData.insert(std::make_pair(chunkid, data));
}

void ErasureCodec::get_chunk (int32_t chunkid, /* out */std::string &data)
{
	std::map<size_t, std::string>::iterator it = mData.find(chunkid);
	if (it == mData.end()) {
		LOG(ERROR) << "no data found for chunk " << chunkid;
		return;
	}

	data = it->second;
}

void ErasureCodec::del_chunk (int32_t chunkid)
{
	std::map<size_t, std::string>::iterator it = mData.find(chunkid);
	if (it == mData.end())
		return;

	mData.erase(it);
}


void ErasureCodec::fix_chunk (int32_t oldid, int32_t newid)
{
	std::map<size_t, std::string>::iterator it = mData.find(oldid);
	if (it == mData.end())
		return;

	mData.insert(std::make_pair(newid, it->second));
	mData.erase(oldid);
}
