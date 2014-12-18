
#include "ecfs/test/common/ecfs_test_base.h"

class ErasureRecoveryTest : public ::testing::Test
{
public:
	void test ();

	ErasureRecoveryTest ();

private:
	ErasureTestCluster mCluster;
};

ErasureRecoveryTest::ErasureRecoveryTest()
{
	mCluster.prepare_namenode();
	mCluster.prepare_datanodes(FLAGS_N + 1, 1, 1, 2);
	mCluster.add_recovery(100, "/tmp/ecfs/recovery");

	mCluster.get_namenode_server().get_oplog_manager().init();
	mCluster.get_namenode_server().get_block_manager().init();
	mCluster.set_namenode_status(Server_Running);

	for (int i = 1; i <= FLAGS_N + 1; i++) {
		mCluster.get_datanode_sever(i).get_disk_manager().init();
		mCluster.get_datanode_sever(i).set_status(Server_Running);

		ThriftResult result;
		DatanodeHeartbeat hti;
		hti.endpoint.nodeid = i;
		mCluster.get_namenode_if()->datanode_heartbeat(result, hti);
	}

	ThriftResult result;
	RecoveryHeartbeat hti;
	hti.endpoint.nodeid = 100;
	mCluster.get_namenode_if()->recovery_heartbeat(result, hti);

	mCluster.get_namenode_server().get_recover_manager().init();
}

void ErasureRecoveryTest::test()
{
	std::string data;
	int len = FLAGS_K * FLAGS_chunk_len;
	char *buf = new char[len];
	data.assign(buf, len);

	int32_t blockid = mCluster.get_client()->put_block(data);

	ResultGetReadToken ret;
	mCluster.get_namenode_if()->get_read_token(ret, blockid);

	std::vector<int32_t> &chunkids = ret.token.chunkids;
	int32_t chunkid = chunkids[0];

	ThriftResult _ret;
	mCluster.get_datanode_if(NODE_ID(chunkid))->del_chunk(_ret, chunkid);

	mCluster.get_namenode_if()->request_recover(_ret, blockid, chunkid);

	sleep(10000);
}

TEST_F(ErasureRecoveryTest, TestAll)
{
	test();
}
