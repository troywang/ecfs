#!/usr/local/bin/thrift --cpp --java

# Interface definition for NameNode Public Service
#

include "common.thrift"

################################################
service RecoveryThriftService {
  common.ThriftResult recover_chunk (
  1: required common.FileRecoverToken token,
  ),
}
