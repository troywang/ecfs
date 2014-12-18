/*
 * codec_test.cpp
 *
 *  Created on: Jun 30, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/common/codec.h"
#include "ecfs/test/common/ecfs_test_base.h"

class ErasureCodecTest : public ::testing::Test
{
public:
	void test ();

	ErasureCodecTest ();
};

ErasureCodecTest::ErasureCodecTest()
{
}

void ErasureCodecTest::test ()
{
	const std::string dir = "/tmp/ecfs/codec/";
	if (!boost::filesystem::exists(dir)) {
		boost::filesystem::create_directories(dir);
	}

	int len = FLAGS_K * FLAGS_chunk_len;
	char *buf = new char [len];

	ErasureCodec codec(FLAGS_K, FLAGS_N, FLAGS_chunk_len);

	int32_t chunkid = 10000;
	std::vector<int32_t> chunkids;
	for (int i = 0; i < FLAGS_N; i++) {
		chunkids.push_back(chunkid++);
	}

	codec.set_chunk_ids(chunkids);

	for (int i = 0; i < FLAGS_K; i++) {
		std::string data;
		data.resize(FLAGS_chunk_len);
		data.assign(buf + i * FLAGS_chunk_len, FLAGS_chunk_len);

		codec.add_chunk(chunkids[i], data);
	}

	codec.encode();

	for (int i = 0; i < FLAGS_K; i++) {
		std::string data;
		codec.get_chunk(chunkids[i], data);
		assert(0 == memcmp(data.data(), buf + i * FLAGS_chunk_len, FLAGS_chunk_len));
	}

	for (int i = FLAGS_K; i < FLAGS_N; i++) {
		std::string data;
		codec.get_chunk(chunkids[i], data);
		assert(data.empty() == false);
	}

	codec.del_chunk(chunkids[0]);
	codec.del_chunk(chunkids[3]);
	codec.del_chunk(chunkids[7]);
	codec.del_chunk(chunkids[11]);

	codec.decode();

	for (int i = 0; i < FLAGS_K; i++) {
		std::string data;
		codec.get_chunk(chunkids[i], data);
		assert(0 == memcmp(data.data(), buf + i * FLAGS_chunk_len, FLAGS_chunk_len));
	}

	try
	{
		for (int i = 0; i < 6; i++) {
			codec.del_chunk(chunkids[i]);
		}
		codec.decode();
		ASSERT_TRUE(false);
	} catch (std::logic_error &err) {

	}
}

TEST_F(ErasureCodecTest, TestAll)
{
	test();

//	int K = 8;
//	int N = 12;
//	ErasureEncoder encoder(K, N, "/tmp/codec/");
//	encoder.encode("/tmp/codec/test.png");
//
//	int len = 8*1024*1024;
//	ErasureDecoder decoder(K, N, len, "/tmp/codec/");
//
//	int start = 100;
//	decoder.set_start_id(start);
//
//	for (int i = 2; i < N; i++)
//	{
//		decoder.add_chunk(i + start, encoder.get_data(i));
//	}
//
//	decoder.decode();
}



