#!/usr/local/bin/thrift --cpp --java

include "common.thrift"

################################################
#

service DatanodeThriftService {
        common.ThriftResult put_chunk(
        1: required i32 chunkid,
        2: required string data
        ),

        common.ThriftResult del_chunk(
        1: required i32 chunkid
        ),

        common.ResultGetChunk get_chunk(
        1: required i32 chunkid
        ),
    
        common.ResultReadData read_data(
        1: required i32 chunkid,
        2: required i32 offset,
        3: required i32 size
        ),

        common.ResultReportChunks report_chunks(
        ),
}
