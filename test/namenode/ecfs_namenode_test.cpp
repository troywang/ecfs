#include "ecfs/test/common/ecfs_test_base.h"

class NamenodeTest : public ::testing::Test
{
public:
	NamenodeTest();

	void test_oplog_manager ();

	void test_namenode_server ();

protected:
	ErasureTestCluster mCluster;
	NamenodeServer *mServer;
};

void NamenodeTest::test_oplog_manager ()
{
	OpLogManager &oplog = mServer->get_oplog_manager();
	BlockManager &block = mServer->get_block_manager();

	int32_t blockid = 1000;
	size_t N = FLAGS_N;

	std::vector<int32_t> chunkids;

	oplog.init();

	block.get_chunk_ids(blockid, chunkids, true);
	ASSERT_TRUE(chunkids.size() == 0);

	oplog.add_block(blockid, time(NULL));
	for (size_t i = 1; i <= N; i++) {
		oplog.add_chunk(blockid, MAKE_CHUNK_ID(i, 1, 1));
	}

	oplog.init();
	block.get_chunk_ids(blockid, chunkids, true);
	ASSERT_TRUE(chunkids.size() == N);

	oplog.update_chunk(blockid, MAKE_CHUNK_ID(1, 1, 1), MAKE_CHUNK_ID(2, 1, 2));
	oplog.uninit();
	block.uninit();

	oplog.init();
	chunkids.clear();
	block.get_chunk_ids(blockid, chunkids, true);
	ASSERT_TRUE(chunkids.size() == N);

	oplog.expire_chunk(blockid, MAKE_CHUNK_ID(2, 1, 2));
	oplog.uninit();
	block.uninit();

	oplog.init();
	chunkids.clear();
	block.get_chunk_ids(blockid, chunkids, true);
	ASSERT_TRUE(chunkids.size() == N);

	oplog.del_block(blockid);
	oplog.uninit();
	block.uninit();

	oplog.init();
	chunkids.clear();
	block.get_chunk_ids(blockid, chunkids, true);
	ASSERT_TRUE(chunkids.size() == 0);
}

NamenodeTest::NamenodeTest()
{
	mCluster.prepare_namenode();
	mCluster.prepare_datanodes(FLAGS_N + 2, 1, 2, 3);

	for (int i = 1; i <= FLAGS_N + 2; i++)
	{
		mCluster.get_datanode_sever(i).get_disk_manager().init();
		mCluster.set_datanode_status(i, Server_Running);
	}

	mServer = &mCluster.get_namenode_server();
	mCluster.set_namenode_status(Server_Running);
}

void NamenodeTest::test_namenode_server()
{
	for (int i = 1; i <= FLAGS_N; i++)
	{
		ThriftResult result;
		DatanodeHeartbeat hti;
		hti.endpoint.nodeid = i;
		mCluster.get_namenode_if()->datanode_heartbeat(result, hti);
	}

	sleep(2); //wait for heartbeat finish

	FileWriteToken token;
	mServer->get_write_token(token);

	int32_t blockid = token.blockid;

	ASSERT_TRUE(token.chunkids.size() == (size_t)FLAGS_N);

	std::set<int32_t> failed;
	int32_t failid = *token.chunkids.begin();
	LOG(ERROR) << "failed block id " << failid;
	failed.insert(failid);

	FileWriteToken fix_token;
	mServer->fix_write_token(fix_token, blockid, failed);

	ASSERT_TRUE(fix_token.chunkids[0] != token.chunkids[0]);

	for (int i = 1; i < FLAGS_N; i++) {
		ASSERT_TRUE(fix_token.chunkids[i] == token.chunkids[i]);
	}

	for (int i = 0; i < FLAGS_N; i++)
	{
		ThriftResult result;
		std::string data("hello world");
		int32_t chunkid = fix_token.chunkids[i];
		mCluster.get_datanode_if(NODE_ID(chunkid))->put_chunk(result, chunkid, data);
	}

	mServer->commit_write(blockid);

	FileReadToken read_token;
	mServer->get_read_token(read_token, blockid);

	for (int i = 0; i < FLAGS_N; i++) {
		ASSERT_TRUE(fix_token.chunkids[i] == read_token.chunkids[i]);
	}

	mServer->get_oplog_manager().uninit();
	mServer->get_oplog_manager().init();

	FileReadToken new_read;
	mServer->get_read_token(new_read, blockid);

	for (int i = 0; i < FLAGS_N; i++) {
		ASSERT_TRUE(fix_token.chunkids[i] == new_read.chunkids[i]);
	}

	mServer->delete_block(blockid);

	mServer->get_oplog_manager().uninit();
	mServer->get_oplog_manager().init();

	std::vector<int32_t> chunkids;
	mServer->get_block_manager().get_chunk_ids(blockid, chunkids, true);
	ASSERT_TRUE(chunkids.empty());
}

TEST_F(NamenodeTest, TestAll)
{
	test_oplog_manager();
}
