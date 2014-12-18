#!/usr/local/bin/thrift --cpp --java

# Interface definition for NameNode Public Service
#

include "common.thrift"

################################################
service NamenodeThriftService {
  common.ThriftResult datanode_heartbeat(
  1: required common.DatanodeHeartbeat info
  ),
  
  common.ThriftResult recovery_heartbeat(
  1: required common.RecoveryHeartbeat info
  ),
 
  common.ResultGetWriteToken get_write_token(
  ),

  common.ResultFixWriteToken fix_write_token(
  1: required i32 blockid,
  2: required set<i32> failed_ids,
  ),
  
  common.ResultGetReadToken get_read_token(
  1: required i32 blockid,
  ),

  common.ResultGetNodeInfo get_node_info(
  1: required i16 nodeid,
  ),

  common.ThriftResult commit_write(
  1: required i32 blockid,
  ),

  common.ThriftResult request_recover(
  1: required i32 blockid,
  2: required i32 chunkid
  ),

  common.ThriftResult done_recovery(
  1: required i32 blockid,
  2: required i32 oldchunk,
  3: required i32 newchunk,
  ),
  
  common.ThriftResult delete_block(
  1: required i32 blockid 
  ),

  common.ResultGetBlocks get_committed_blocks(
  )
}
