/*
 * namenode_public_service.h
 *
 *  Created on: Apr 28, 2014
 *      Author: wanghuan
 */

#ifndef NAMENODE_PUBLIC_SERVICE_H_
#define NAMENODE_PUBLIC_SERVICE_H_

#include "ecfs/src/namenode/name_node.h"
#include "ecfs/src/gen-cpp/NamenodeThriftService.h"

class NamenodeThriftService : public NamenodeThriftServiceIf
{
public:
	NamenodeThriftService(NamenodeServer &server): mServer(server){}

	virtual void datanode_heartbeat(ThriftResult& _return, const DatanodeHeartbeat& info);

	virtual void recovery_heartbeat(ThriftResult& _return, const RecoveryHeartbeat& info);

	virtual void get_write_token(ResultGetWriteToken& _return);

	virtual void fix_write_token(ResultFixWriteToken& _return, const int32_t blockid, const std::set<int32_t> & failed_ids);

	virtual void get_read_token(ResultGetReadToken& _return, const int32_t blockid);

	virtual void get_node_info(ResultGetNodeInfo& _return, const int16_t nodeid);

	virtual void commit_write(ThriftResult& _return, const int32_t blockid);

	virtual void request_recover(ThriftResult& _return, const int32_t blockid, const int32_t chunkid);

	virtual void done_recovery(ThriftResult& _return, const int32_t blockid, const int32_t oldchunk, const int32_t newchunk);

	virtual void delete_block(ThriftResult& _return, const int32_t blockid);

	virtual void get_committed_blocks(ResultGetBlocks& _return);

private:
	NamenodeServer &mServer;
};

#endif /* NAMENODE_PUBLIC_SERVICE_H_ */
