/*
 * configs.h
 *
 *  Created on: May 7, 2014
 *      Author: wanghuan
 */

#ifndef CONFIGS_H_
#define CONFIGS_H_

#include "thirdparty/gflags/gflags.h"
#include "thirdparty/glog/logging.h"

DECLARE_int32(conn_max);
DECLARE_int32(conn_maxperhost);
DECLARE_int32(conn_timeout_ms);
DECLARE_int32(recv_timeout_ms);
DECLARE_int32(send_timeout_ms);
DECLARE_int32(idle_timeout_sec);
DECLARE_int32(block_wait_ms);

DECLARE_int32(thrift_datanode_thread_num);
DECLARE_int32(thrift_namenode_thread_num);
DECLARE_int32(thrift_recovery_thread_num);

DECLARE_string(namenode_service_ip);
DECLARE_int32(namenode_service_port);

DECLARE_string(namenode_oplog_dir);

DECLARE_int32(datanode_id);
DECLARE_string(datanode_service_ip);
DECLARE_int32(datanode_service_port);

DECLARE_int32(datanode_heartbeat_interval);
DECLARE_int32(datanode_dead_interval);
DECLARE_int32(datanode_suspend_interval);
DECLARE_string(datanode_storage_dirs);

DECLARE_int32(recovery_id);
DECLARE_string(recovery_service_ip);
DECLARE_int32(recovery_service_port);
DECLARE_int32(recovery_maxperhost);

DECLARE_int32(K);
DECLARE_int32(N);
DECLARE_int32(chunk_len);

#endif /* CONFIGS_H_ */
