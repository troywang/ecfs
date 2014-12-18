/*
 * datanode.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/datanode/data_node.h"

DatanodeServer::DatanodeServer(int16_t nodeid)
	: mDisks(*this), mHeart(*this)
{
	mNodeId = nodeid;
	mStatus = Server_Suspended;
}

DatanodeServer::~DatanodeServer()
{
	uninit();
}

void DatanodeServer::init()
{
    mNamenodeThriftConnectionPool.reset(new NamenodeThriftConnectionPool(
            /* maxConn */FLAGS_conn_max,
            /* maxConnPerHost */FLAGS_conn_maxperhost,
            /* connect */FLAGS_conn_timeout_ms,
            /* recv */FLAGS_recv_timeout_ms,
            /* send */FLAGS_send_timeout_ms,
            /* idle */FLAGS_idle_timeout_sec,
            /* block wait */ FLAGS_block_wait_ms));

    mDatanodeThriftConnectionPool.reset(new DatanodeThriftConnectionPool(
                    /* maxConn */FLAGS_conn_max,
                    /* maxConnPerHost */FLAGS_conn_maxperhost,
                    /* connect */FLAGS_conn_timeout_ms,
                    /* recv */FLAGS_recv_timeout_ms,
                    /* send */FLAGS_send_timeout_ms,
                    /* idle */FLAGS_idle_timeout_sec,
                    /* block wait */ FLAGS_block_wait_ms));

    mDisks.init();
    mHeart.init();
    mStatus = Server_Running;
}

void DatanodeServer::uninit()
{
	mStatus = Server_Suspended;

	mHeart.uninit();
	mDisks.uninit();
	mNamenodeThriftConnectionPool.reset(NULL);
	mDatanodeThriftConnectionPool.reset(NULL);
}

boost::shared_ptr<NamenodeThriftConnection> DatanodeServer::get_namenode_connection()
{
	uint32_t ip = ErasureUtils::str2ip(FLAGS_namenode_service_ip);
	return mNamenodeThriftConnectionPool->get_connection(ip, FLAGS_namenode_service_port);
}
