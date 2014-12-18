struct DatanodeEndpoint {
  1: required i32 ipv4,
  2: required i16 port,
  3: required i16 nodeid,
}

struct ThriftResult {
  1: required i32 code,
  2: optional string verbose, 
}

struct FileWriteToken {
  1: required i32 blockid,
  2: required list<i32> chunkids,
  3: required list<DatanodeEndpoint> endpoints
}

struct FileReadToken {
  1: required list<i32> chunkids,
  2: required list<DatanodeEndpoint> endpoints
}

struct FileRecoverToken {
  1: required list<DatanodeEndpoint> endpoints,
  2: required list<i32> chunkids,
  3: required i32 blockid,
  4: required i32 oldchunk,
  5: required i32 newchunk
}

struct ResultGetWriteToken {
  1: required ThriftResult result,
  2: required FileWriteToken token,
}

struct ResultFixWriteToken {
  1: required ThriftResult result,
  2: required FileWriteToken token
}

struct ResultGetReadToken {
  1: required ThriftResult result,
  2: required FileReadToken token,
}

struct ResultGetNodeInfo {
  1: required ThriftResult result,
  2: required DatanodeEndpoint endpoint,
}

struct ResultFixRecoverToken {
  1: required ThriftResult result,
  2: required FileRecoverToken token
}

struct DatanodeHeartbeat {
  1: required DatanodeEndpoint endpoint
}

struct RecoveryHeartbeat {
  1: required DatanodeEndpoint endpoint,
  2: required i32 load,
}


struct ResultGetChunk {
  1: required ThriftResult result,
  2: required string data
}

struct ResultGetCode {
  1: required ThriftResult result,
  2: required string data
}

struct ResultReadData {
  1: required ThriftResult result,
  2: required string data
}

struct ResultReportChunks {
  1: required ThriftResult result,
  2: required set<i32> chunkInUse,
  3: required set<i32> chunkUnUse
}

struct ResultGetBlocks {
  1: required ThriftResult result,
  2: required set<i32> blockids,
}
