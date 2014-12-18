/*
 * datanode_public_service.h
 *
 *  Created on: Apr 28, 2014
 *      Author: wanghuan
 */

#ifndef DATANODE_PUBLIC_SERVICE_H_
#define DATANODE_PUBLIC_SERVICE_H_

#include "ecfs/src/datanode/data_node.h"
#include "ecfs/src/gen-cpp/DatanodeThriftService.h"

class DatanodeThriftService : public DatanodeThriftServiceIf
{
public:

	DatanodeThriftService(DatanodeServer &server) : mServer(server) {}

	virtual void put_chunk(ThriftResult& _return, const int32_t chunkid, const std::string& data);

	virtual void del_chunk(ThriftResult& _return, const int32_t chunkid);

	virtual void get_chunk(ResultGetChunk& _return, const int32_t chunkid);

	virtual void read_data(ResultReadData& _return, const int32_t chunkid, const int32_t offset, const int32_t size);

	virtual void report_chunks(ResultReportChunks& _return);

private:
	DatanodeServer &mServer;
};




#endif /* DATANODE_PUBLIC_SERVICE_H_ */
