/*
 * configs.cpp
 *
 *  Created on: May 7, 2014
 *      Author: wanghuan
 */


#include "ecfs/src/common/configs.h"

DEFINE_int32(conn_max, 100, "max connections for each pool");
DEFINE_int32(conn_maxperhost, 30, "max connections per host");
DEFINE_int32(conn_timeout_ms, 500, "max connect timeout");
DEFINE_int32(recv_timeout_ms, 10000, "max receive timeout");
DEFINE_int32(send_timeout_ms, 10000, "max send timeout");
DEFINE_int32(idle_timeout_sec, 60, "timeout to reclaim idle connections");
DEFINE_int32(block_wait_ms, 500, "timeout to wait for available connections");

DEFINE_int32(thrift_datanode_thread_num, 10, "");
DEFINE_int32(thrift_namenode_thread_num, 100, "");
DEFINE_int32(thrift_recovery_thread_num, 10, "");

DEFINE_string(namenode_service_ip, "127.0.0.1", "ip address for namenode");
DEFINE_int32(namenode_service_port, 8001, "port for public namenode service");

DEFINE_string(namenode_oplog_dir, "/tmp/namenode/oplog/", "directory for namenode oplogs");

DEFINE_int32(datanode_id, 1, "node id for this datanode");
DEFINE_string(datanode_service_ip, "127.0.0.1", "ip address for this datanode");
DEFINE_int32(datanode_service_port, 9001, "port for public datanode service");

DEFINE_int32(datanode_heartbeat_interval, 3, "seconds for heartbeat interval");
DEFINE_int32(datanode_dead_interval, 60 * 60, "seconds for death interval");
DEFINE_int32(datanode_suspend_interval, 3 * 3, "seconds for suspend interval");

DEFINE_string(datanode_storage_dirs, "/tmp/datanode/s1:/tmp/datanode/s2", "storage dirs separated by colons");


DEFINE_int32(recovery_id, 100, "id for this recovery node");
DEFINE_string(recovery_service_ip, "127.0.0.1", "ip address for recovery node");
DEFINE_int32(recovery_service_port, 7001, "port for public recovery service");
DEFINE_int32(recovery_maxperhost, 3, "max recovery task per host");

DEFINE_int32(K, 8, "the number of shares needed for recovery");
DEFINE_int32(N, 12, "the number of shares generated");
DEFINE_int32(chunk_len, 8*1024*1024, "chunk len default 8M");
