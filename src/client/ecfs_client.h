/*
 * client.h
 *
 *  Created on: Sep 17, 2014
 *      Author: wanghuan
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "thirdparty/fecpp/fecpp.h"
#include "ecfs/src/common/common.h"
#include "ecfs/src/common/configs.h"
#include "ecfs/src/common/thread_pool.h"
#include "ecfs/src/common/thrift_adaptor.h"

class ErasureClient
{
public:
	ErasureClient ();

	virtual ~ErasureClient () {
	}

	int32_t put_block (const std::string &data) throw (ErasureException);

	void get_block (int32_t blockid, /* out */std::string &data) throw (ErasureException);

	void read_data (int32_t blockid, int32_t offset, int32_t size, /* out */std::string &data) throw (ErasureException);

	virtual boost::shared_ptr<NamenodeThriftConnection> get_namenode_connection();

	virtual boost::shared_ptr<DatanodeThriftConnection> get_datanode_connection(int16_t nodeid);

private:
	NamenodeThriftConnectionPool mNamenodePool;
	DatanodeThriftConnectionPool mDatanodePool;
};

#endif /* CLIENT_H_ */
