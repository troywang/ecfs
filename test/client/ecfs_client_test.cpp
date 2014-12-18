/*
 * ecfs_client_test.cpp
 *
 *  Created on: Sep 16, 2014
 *      Author: wanghuan
 */

#include "ecfs/test/common/ecfs_test_base.h"

class ClientTest : public ::testing::Test
{
public:
	ClientTest ();

	~ClientTest ();

	void test_put ();

private:
	MockErasureClient *mClient;

protected:
	ErasureTestCluster mCluster;
};

ClientTest::ClientTest()
{
	mClient = mCluster.get_client();

	mCluster.prepare_namenode();
	mCluster.prepare_datanodes(FLAGS_N, 1, 1, 2);

	mCluster.get_namenode_server().get_oplog_manager().init();
	mCluster.get_namenode_server().get_block_manager().init();
	mCluster.set_namenode_status(Server_Running);

	for (int i = 1; i <= FLAGS_N; i++) {
		mCluster.get_datanode_sever(i).get_disk_manager().init();
		mCluster.get_datanode_sever(i).set_status(Server_Running);

		ThriftResult result;
		DatanodeHeartbeat hti;
		hti.endpoint.nodeid = i;
		mCluster.get_namenode_if()->datanode_heartbeat(result, hti);
	}

	sleep(2);
}

ClientTest::~ClientTest()
{
}

void ClientTest::test_put()
{
	int len = FLAGS_K * FLAGS_chunk_len;
	char *buf = new char[len];
	std::string data;
	data.assign(buf, len);

	int32_t blockid = mClient->put_block(data);

	std::string copy;
	mClient->get_block(blockid, copy);

	ASSERT_TRUE(copy == data);
}

TEST_F(ClientTest, TestAll)
{
	test_put();
}


