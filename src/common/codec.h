/*
 * codecer.h
 *
 *  Created on: Jun 11, 2014
 *      Author: wanghuan
 */

#ifndef CODECER_H_
#define CODECER_H_

#include <map>
#include <vector>
#include <stdint.h>
#include <stddef.h>

#include "thirdparty/fecpp/fecpp.h"

class ErasureCodec
{
public:
	ErasureCodec (size_t k, size_t n, size_t len);

	void set_chunk_ids (const std::vector<int32_t> &chunkids) {
		mChunkIds = chunkids;
	}

	void add_chunk (int32_t chunkid, const std::string &data);

	void get_chunk (int32_t chunkid, /* out */std::string &data);

	//for test
	void del_chunk (int32_t chunkid);

	void fix_chunk (int32_t oldid, int32_t newid);

	void operator() (size_t idx, size_t, const fecpp::byte share[], size_t len);

	void decode ();

	void encode ();

private:
	size_t K, N;
	fecpp::fec_code mFec;

	size_t mLen;
	std::vector<int32_t> mChunkIds;
	std::map<size_t, std::string> mData; //chunkid => data
};

#endif /* CODECER_H_ */
