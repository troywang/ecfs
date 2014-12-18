/*
 * thrift_adaptor.h
 *
 *  Created on: Nov 15, 2014
 *      Author: wanghuan
 */

#ifndef THRIFT_ADAPTOR_H_
#define THRIFT_ADAPTOR_H_

#include "ecfs/src/gen-cpp/common_types.h"
#include "ecfs/src/common/thrift_connection_pool.h"

#include "ecfs/src/gen-cpp/NamenodeThriftService.h"
#include "ecfs/src/gen-cpp/DatanodeThriftService.h"
#include "ecfs/src/gen-cpp/RecoveryThriftService.h"

typedef ThriftConnectionPool<NamenodeThriftServiceIf, NamenodeThriftServiceClient> NamenodeThriftConnectionPool;
typedef ThriftConnectionPool<DatanodeThriftServiceIf, DatanodeThriftServiceClient> DatanodeThriftConnectionPool;
typedef ThriftConnectionPool<RecoveryThriftServiceIf, RecoveryThriftServiceClient> RecoveryThriftConnectionPool;

typedef DatanodeThriftConnectionPool::TThriftConnection DatanodeThriftConnection;
typedef NamenodeThriftConnectionPool::TThriftConnection NamenodeThriftConnection;
typedef RecoveryThriftConnectionPool::TThriftConnection RecoveryThriftConnection;


#endif /* THRIFT_ADAPTOR_H_ */
