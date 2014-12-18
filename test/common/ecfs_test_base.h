/*
 * ecfs_test_base.h
 *
 *  Created on: Jun 4, 2014
 *      Author: wanghuan
 */

#ifndef ERASURE_TEST_BASE_H_
#define ERASURE_TEST_BASE_H_

#include "thirdparty/gtest/gtest.h"

#include "ecfs/src/common/common.h"
#include "ecfs/src/datanode/datanode_thrift_service.h"
#include "ecfs/src/namenode/namenode_thrift_service.h"
#include "ecfs/src/recovery/recovery_thrift_service.h"
#include "ecfs/src/client/ecfs_client.h"
#include "ecfs/src/recovery/recovery_node.h"
#include "ecfs/src/datanode/data_node.h"
#include "ecfs/src/namenode/name_node.h"

template<typename TInterface>
class MockThriftConnection: public ThriftConnection<TInterface>
{
public:
	~MockThriftConnection() {
	}
	MockThriftConnection(TInterface *client)
	: ThriftConnection<TInterface>(NULL, client, 0, 0) {
    }
};

class MockDatanodeThriftService : public DatanodeThriftService
{
public:
	MockDatanodeThriftService(DatanodeServer &server) : DatanodeThriftService(server) {
	}
};

class MockNamenodeThriftService : public NamenodeThriftService
{
public:
	MockNamenodeThriftService(NamenodeServer &server): NamenodeThriftService(server) {}
};

class MockRecoveryThriftService : public RecoveryThriftService
{
public:
	MockRecoveryThriftService(RecoveryServer &server) : RecoveryThriftService(server) {}
};

class ErasureTestCluster;
class MockErasureClient : public ErasureClient
{
public:
	MockErasureClient(ErasureTestCluster &cluster) : ErasureClient(), mCluster(cluster) {
	}

	virtual boost::shared_ptr<DatanodeThriftConnection> get_datanode_connection(int16_t nodeid) throw (ErasureException);

	virtual boost::shared_ptr<NamenodeThriftConnection> get_namenode_connection() throw (ErasureException);

private:
	ErasureTestCluster &mCluster;
};

class MockDiskManager : public DiskManager
{
public:
	MockDiskManager(DatanodeServer &server);

	virtual ~MockDiskManager() {}

	virtual void make_heartbeat_info(/* out */DatanodeHeartbeat &info);
};

class MockDatanodeServer : public DatanodeServer
{
public:
	MockDatanodeServer(int16_t nodeid, ErasureTestCluster &cluster);

	virtual ~MockDatanodeServer() {}

	ErasureTestCluster& get_cluster() {
		return mCluster;
	}

	virtual DiskManager& get_disk_manager() {
		return mDisk;
	}

	void set_status (ServerStatus status) {
		mStatus = status;
	}

	virtual boost::shared_ptr<NamenodeThriftConnection> get_namenode_connection() throw (ErasureException);

private:
	ErasureTestCluster &mCluster;
	MockDiskManager mDisk;
};

class MockRecoveryServer : public RecoveryServer
{
public:
	MockRecoveryServer(int16_t nodeid, ErasureTestCluster &cluster);

	virtual ~MockRecoveryServer() {
	}

	ErasureTestCluster& get_cluster() {
		return mCluster;
	}

	virtual boost::shared_ptr<NamenodeThriftConnection> get_namenode_connection() throw (ErasureException);

	virtual boost::shared_ptr<DatanodeThriftConnection> get_datanode_connection(int16_t nodeid) throw (ErasureException);

private:
	ErasureTestCluster &mCluster;
};

class MockNamenodeServer : public NamenodeServer
{
public:
	MockNamenodeServer(ErasureTestCluster &cluster) : mCluster(cluster) {
	}

	virtual boost::shared_ptr<DatanodeThriftConnection> get_datanode_connection(int16_t nodeid) throw (ErasureException);

	virtual boost::shared_ptr<RecoveryThriftConnection> get_recovery_connection(int16_t nodeid) throw (ErasureException);

	void set_status (ServerStatus status) {
		mStatus = status;
	}

private:
	ErasureTestCluster &mCluster;
};

class ErasureTestCluster
{
public:
    ErasureTestCluster();

    ~ErasureTestCluster();

    /*
     * n: # of data node
     * d: # of disk for each data node
     * s: # of sub dir for each disk
     * c: # of chunk for each sub dir
     */
    void prepare_namenode ();

    void prepare_datanodes (int n, int d, int s, int c);

    void init ();

    void uninit ();

    MockDatanodeServer& get_datanode_sever(const int16_t nodeid) {
    	return *(mDataNodes.find(nodeid)->second);
    }

    MockNamenodeServer& get_namenode_server () {
    	return mNameServer;
    }

    MockErasureClient* get_client () {
    	return mClient;
    }

    DatanodeThriftServiceIf* get_datanode_if(int16_t nodeid) {
    	return mDataNodeThriftIfs.find(nodeid)->second;
    }

    RecoveryThriftServiceIf* get_recovery_if(int16_t nodeid) {
    	return mRecoveryThriftIfs.find(nodeid)->second;
    }

    NamenodeThriftServiceIf* get_namenode_if() {
    	return mNamenodeThriftIf;
    }

    MockDatanodeServer& add_datanode(int16_t nodeid, const std::vector<std::string> &disks);

    void set_namenode_status (ServerStatus status);

    void set_datanode_status (int16_t nodeid, ServerStatus status);

    MockRecoveryServer& add_recovery(int16_t nodeid, const std::string &dir);

private:
    void create_chunk_file (const std::string &filepath);

private:
    std::map<int16_t, DatanodeThriftServiceIf*> mDataNodeThriftIfs;
    std::map<int16_t, RecoveryThriftServiceIf*> mRecoveryThriftIfs;

    NamenodeThriftServiceIf* mNamenodeThriftIf;

    std::map<int16_t, MockDatanodeServer*> mDataNodes;
    std::map<int16_t, MockRecoveryServer*> mRecoveries;
    MockNamenodeServer mNameServer;

    MockErasureClient *mClient;
};

#endif /* ERASURE_TEST_BASE_H_ */
