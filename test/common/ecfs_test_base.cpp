/*
 * ecfs_test_base.cpp
 *
 *  Created on: Jun 4, 2014
 *      Author: wanghuan
 */

#include "ecfs/test/common/ecfs_test_base.h"

MockDiskManager::MockDiskManager(DatanodeServer &server)
: DiskManager(server) {

}

void MockDiskManager::make_heartbeat_info(/* out */DatanodeHeartbeat &info)
{

}

ErasureTestCluster::ErasureTestCluster() : mNameServer(*this)
{
	mClient = new MockErasureClient(*this);
	mNamenodeThriftIf = new MockNamenodeThriftService(mNameServer);
}

ErasureTestCluster::~ErasureTestCluster()
{
	uninit();

	if (mNamenodeThriftIf)
		delete mNamenodeThriftIf;

	std::map<int16_t, DatanodeThriftServiceIf*>::iterator it;
	for (it = mDataNodeThriftIfs.begin(); it != mDataNodeThriftIfs.end(); it++)
		delete it->second;

	std::map<int16_t, MockDatanodeServer*>::iterator it2;
	for (it2 = mDataNodes.begin(); it2 != mDataNodes.end(); it2++)
		delete it2->second;

	if (mClient)
		delete mClient;
}

void ErasureTestCluster::prepare_namenode ()
{
	std::string dir = "/tmp/ecfs/namenode/";

	boost::filesystem::remove_all(dir);
	boost::filesystem::create_directories(dir);
	mNameServer.get_oplog_manager().set_log_dir(dir);
}

void ErasureTestCluster::prepare_datanodes (int n, int d, int s, int c)
{
	for (int i = 1; i <= n; i++)
	{
		std::vector<std::string> disks;
		for (int j = 1; j <= d; j++)
		{
			std::ostringstream oss;
			oss << "/tmp/ecfs/datanode/" << i << "/disk" << j;
			disks.push_back(oss.str());
		}

		add_datanode(i, disks);

		for (int j = 1; j <= d; j++)
		{
			int m = 1;
			for (int k = 1; k <= s; k++)
			{
				std::ostringstream oss;
				oss << disks[j - 1] << "/" << k << CHUNK_SUB_DIR_SUFFIX;

				boost::filesystem::remove_all(oss.str());
				boost::filesystem::create_directories(oss.str());

				for (int l = 1; l <= c; l++)
				{
					std::ostringstream oss2;
					oss2 << oss.str() << "/" << MAKE_CHUNK_ID(i, j, m++) << CHUNK_UNUSE_SUFFIX;
					create_chunk_file (oss2.str());
				}
			}
		}
	}
}


void ErasureTestCluster::create_chunk_file (const std::string &filepath)
{
    int fd = ::open(filepath.c_str(), O_RDWR | O_CREAT, 0644);
    ::posix_fallocate(fd, 0, FLAGS_chunk_len);
    ::close(fd);
}

void ErasureTestCluster::init ()
{
	mNameServer.init();
	std::map<int16_t, MockDatanodeServer*>::iterator it;
	for (it = mDataNodes.begin(); it != mDataNodes.end(); it++) {
		it->second->init();
	}
}

void ErasureTestCluster::uninit ()
{
	std::map<int16_t, MockDatanodeServer*>::iterator it;
	for (it = mDataNodes.begin(); it != mDataNodes.end(); it++) {
		it->second->uninit();
	}
}

MockDatanodeServer& ErasureTestCluster::add_datanode(int16_t nodeid, const std::vector<std::string> &disks)
{
	std::map<int16_t, MockDatanodeServer*>::iterator it = mDataNodes.find(nodeid);
	std::pair<std::map<int16_t, MockDatanodeServer*>::iterator, bool> pair;

	if (it != mDataNodes.end())
		return *(it->second);

	MockDatanodeServer *server = new MockDatanodeServer(nodeid, *this);
	mDataNodes.insert(std::make_pair(nodeid, server));

	for (size_t i = 0; i < disks.size(); i++) {
		boost::filesystem::remove_all(disks[i]);
		boost::filesystem::create_directories(disks[i]);

		server->get_disk_manager().add_disk_path(disks[i], i + 1);
	}

	mDataNodeThriftIfs.insert(std::make_pair(nodeid, new MockDatanodeThriftService(*server)));

	return *server;
}

void ErasureTestCluster::set_namenode_status (ServerStatus status)
{
	mNameServer.set_status(status);
}

void ErasureTestCluster::set_datanode_status (int16_t nodeid, ServerStatus status)
{
	mDataNodes.find(nodeid)->second->set_status(status);
}

MockRecoveryServer& ErasureTestCluster::add_recovery(int16_t nodeid, const std::string &dir)
{
	std::map<int16_t, MockRecoveryServer*>::iterator it = mRecoveries.find(nodeid);
	std::pair<std::map<int16_t, MockRecoveryServer*>::iterator, bool> pair;

	if (it != mRecoveries.end())
		return *(it->second);

	MockRecoveryServer *server = new MockRecoveryServer(nodeid, *this);
	mRecoveries.insert(std::make_pair(nodeid, server));

	mRecoveryThriftIfs.insert(std::make_pair(nodeid, new MockRecoveryThriftService(*server)));

	return *server;
}

boost::shared_ptr<DatanodeThriftConnection> MockErasureClient::get_datanode_connection(int16_t nodeid) throw (ErasureException)
{
	DatanodeThriftServiceIf* interf = mCluster.get_datanode_if(nodeid);
	return boost::shared_ptr<DatanodeThriftConnection>(new MockThriftConnection<DatanodeThriftServiceIf>(interf));
}

boost::shared_ptr<NamenodeThriftConnection> MockErasureClient::get_namenode_connection() throw (ErasureException)
{
	NamenodeThriftServiceIf* interf = mCluster.get_namenode_if();
	return boost::shared_ptr<NamenodeThriftConnection>(new MockThriftConnection<NamenodeThriftServiceIf>(interf));
}

MockDatanodeServer::MockDatanodeServer(int16_t nodeid, ErasureTestCluster &cluster)
: DatanodeServer(nodeid), mCluster(cluster), mDisk(*this)
{
}

boost::shared_ptr<NamenodeThriftConnection> MockDatanodeServer::get_namenode_connection() throw (ErasureException)
{
	NamenodeThriftServiceIf *interf = mCluster.get_namenode_if();
	return boost::shared_ptr<NamenodeThriftConnection>(new MockThriftConnection<NamenodeThriftServiceIf>(interf));
}

MockRecoveryServer::MockRecoveryServer(int16_t nodeid, ErasureTestCluster &cluster)
: RecoveryServer(nodeid), mCluster(cluster)
{
}

boost::shared_ptr<NamenodeThriftConnection> MockRecoveryServer::get_namenode_connection() throw (ErasureException)
{
	NamenodeThriftServiceIf *interf = mCluster.get_namenode_if();
	return boost::shared_ptr<NamenodeThriftConnection>(new MockThriftConnection<NamenodeThriftServiceIf>(interf));
}

boost::shared_ptr<DatanodeThriftConnection> MockRecoveryServer::get_datanode_connection(int16_t nodeid) throw (ErasureException)
{
	DatanodeThriftServiceIf *interf = mCluster.get_datanode_if(nodeid);
	return boost::shared_ptr<DatanodeThriftConnection>(new MockThriftConnection<DatanodeThriftServiceIf>(interf));
}

boost::shared_ptr<DatanodeThriftConnection> MockNamenodeServer::get_datanode_connection(int16_t nodeid) throw (ErasureException)
{
	DatanodeThriftServiceIf *interf = mCluster.get_datanode_if(nodeid);
	return boost::shared_ptr<DatanodeThriftConnection>(new MockThriftConnection<DatanodeThriftServiceIf>(interf));
}

boost::shared_ptr<RecoveryThriftConnection> MockNamenodeServer::get_recovery_connection(int16_t nodeid) throw (ErasureException)
{
	RecoveryThriftServiceIf *interf = mCluster.get_recovery_if(nodeid);
	return boost::shared_ptr<RecoveryThriftConnection>(new MockThriftConnection<RecoveryThriftServiceIf>(interf));
}
