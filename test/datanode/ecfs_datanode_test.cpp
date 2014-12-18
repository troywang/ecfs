/*
 * chunk_manager_test.cpp
 *
 *  Created on: Sep 10, 2014
 *      Author: wanghuan
 */

#include "ecfs/test/common/ecfs_test_base.h"

class DatanodeTest : public ::testing::Test
{
public:
	DatanodeTest ();

	~DatanodeTest ();

	void test_chunk_manager ();

	void test_disk_manager ();

protected:
	ChunkManager *mChunkMgr;
	DiskManager *mDiskMgr;
	ErasureTestCluster mCluster;
};

DatanodeTest::DatanodeTest()
{
	mCluster.prepare_datanodes(1, 3, 5, 10);

	mDiskMgr = &mCluster.get_datanode_sever(1).get_disk_manager();
	mChunkMgr = &mDiskMgr->get_disk_entry(1).get_chunk_manager();
}

DatanodeTest::~DatanodeTest()
{
}

void DatanodeTest::test_disk_manager ()
{
	mDiskMgr->init();
	DiskEntry &de = mDiskMgr->get_disk_entry(1);
	ASSERT_EQ(1, de.get_id());

	std::vector<std::string> paths;
	mDiskMgr->get_disk_paths(paths);
	ASSERT_TRUE(paths.size() == 3);

	mDiskMgr->disable_disk_path(de.get_path());
	paths.clear();
	mDiskMgr->get_disk_paths(paths);
	ASSERT_TRUE(paths.size() == 2);

	int32_t chunkid = MAKE_CHUNK_ID(1, 1, 1);
	DiskEntry &de2 = mDiskMgr->find_disk_entry(chunkid);
	ASSERT_EQ(de2.get_id(), 1);
}

void DatanodeTest::test_chunk_manager ()
{
	std::set<int32_t> unuse;
	std::set<int32_t> inuse;
	mChunkMgr->get_chunk_ids(unuse, inuse);
	ASSERT_TRUE(inuse.size() == 0);
	ASSERT_TRUE(unuse.size() == 5 * 10);

	int32_t chunkid = *unuse.begin();

	const std::string txt = "hello world";
	mChunkMgr->put_chunk(chunkid, txt);

	unuse.clear();
	inuse.clear();
	mChunkMgr->get_chunk_ids(unuse, inuse);
	ASSERT_TRUE(inuse.size() == 1);
	ASSERT_TRUE(unuse.size() == 5 * 10 - 1);

	std::string data;
	mChunkMgr->get_chunk(chunkid, data);

	data.clear();
	mChunkMgr->read_data(chunkid, 0, txt.length(), data);
	ASSERT_TRUE(data == txt);

	data.clear();
	mChunkMgr->read_data(chunkid, 6, 5, data);
	ASSERT_TRUE(data == "world");

	mChunkMgr->uninit();
	mChunkMgr->init();

	unuse.clear();
	inuse.clear();
	mChunkMgr->get_chunk_ids(unuse, inuse);
	ASSERT_TRUE(inuse.size() == 1);
	ASSERT_TRUE(unuse.size() == 5 * 10 - 1);

	mChunkMgr->del_chunk(chunkid);
	unuse.clear();
	inuse.clear();
	mChunkMgr->get_chunk_ids(unuse, inuse);
	ASSERT_TRUE(inuse.size() == 0);
	ASSERT_TRUE(unuse.size() == 5 * 10);

}

TEST_F(DatanodeTest, TestAll)
{
	test_disk_manager();
	test_chunk_manager();
}
