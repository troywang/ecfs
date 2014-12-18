/*
 * namenode_public_service.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/namenode/namenode_thrift_service.h"

void NamenodeThriftService::datanode_heartbeat(ThriftResult& _return, const DatanodeHeartbeat &info)
{
	if (!mServer.is_running()) {
		_return.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.__isset.verbose = true;
		_return.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.get_node_manager().handle_datanode_heartbeat(info);
		_return.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.code = err.get_error_code();
		_return.__isset.verbose = true;
		_return.verbose = err.what();
	}
}

void NamenodeThriftService::recovery_heartbeat(ThriftResult& _return, const RecoveryHeartbeat& info)
{
	if (!mServer.is_running()) {
		_return.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.__isset.verbose = true;
		_return.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.get_node_manager().handle_recovery_heartbeat(info);
		_return.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.code = err.get_error_code();
		_return.__isset.verbose = true;
		_return.verbose = err.what();
	}
}


void NamenodeThriftService::get_write_token(ResultGetWriteToken& _return)
{
	if (!mServer.is_running()) {
		_return.result.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.result.__isset.verbose = true;
		_return.result.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.get_write_token(_return.token);
		_return.result.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.result.code = err.get_error_code();
		_return.result.__isset.verbose = true;
		_return.result.verbose = err.what();
	}
}

void NamenodeThriftService::fix_write_token(ResultFixWriteToken& _return, const int32_t blockid,
		const std::set<int32_t> &failed_ids)
{
	if (!mServer.is_running()) {
		_return.result.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.result.__isset.verbose = true;
		_return.result.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.fix_write_token(/* out */_return.token, blockid, failed_ids);
		_return.result.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.result.code = err.get_error_code();
		_return.result.__isset.verbose = true;
		_return.result.verbose = err.what();
	}
}

void NamenodeThriftService::get_read_token(ResultGetReadToken& _return, const int32_t blockid)
{
	if (!mServer.is_running()) {
		_return.result.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.result.__isset.verbose = true;
		_return.result.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.get_read_token(_return.token, blockid);
		_return.result.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.result.code = err.get_error_code();
		_return.result.__isset.verbose = true;
		_return.result.verbose = err.what();
	}
}

void NamenodeThriftService::get_node_info(ResultGetNodeInfo& _return, const int16_t nodeid)
{
	if (!mServer.is_running()) {
		_return.result.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.result.__isset.verbose = true;
		_return.result.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.get_node_manager().make_datanode_endpoint(nodeid, _return.endpoint);
		_return.result.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.result.code = err.get_error_code();
		_return.result.__isset.verbose = true;
		_return.result.verbose = err.what();
	}
}

void NamenodeThriftService::commit_write(ThriftResult& _return, const int32_t blockid)
{
	if (!mServer.is_running()) {
		_return.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.__isset.verbose = true;
		_return.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.commit_write(blockid);
		_return.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.code = err.get_error_code();
		_return.__isset.verbose = true;
		_return.verbose = err.what();
	}
}

void NamenodeThriftService::request_recover(ThriftResult& _return, const int32_t blockid, const int32_t chunkid)
{
	if (!mServer.is_running()) {
		_return.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.__isset.verbose = true;
		_return.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.request_recover(blockid, chunkid);
		_return.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.code = err.get_error_code();
		_return.__isset.verbose = true;
		_return.verbose = err.what();
	}
}

void NamenodeThriftService::done_recovery(ThriftResult& _return, const int32_t blockid, const int32_t oldchunk, const int32_t newchunk)
{
	if (!mServer.is_running()) {
		_return.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.__isset.verbose = true;
		_return.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.done_recovery(blockid, oldchunk, newchunk);
		_return.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.code = err.get_error_code();
		_return.__isset.verbose = true;
		_return.verbose = err.what();
	}
}

void NamenodeThriftService::delete_block(ThriftResult& _return, const int32_t blockid)
{
	if (!mServer.is_running()) {
		_return.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.__isset.verbose = true;
		_return.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.delete_block(blockid);
		_return.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.code = err.get_error_code();
		_return.__isset.verbose = true;
		_return.verbose = err.what();
	}
}

void NamenodeThriftService::get_committed_blocks(ResultGetBlocks& _return)
{
	if (!mServer.is_running()) {
		_return.result.code = RESULT_NAMENODE_ERR_MAINTAINENCE;
		_return.result.__isset.verbose = true;
		_return.result.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.get_block_manager().get_committed_blocks(_return.blockids);
		_return.result.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.result.code = err.get_error_code();
		_return.result.__isset.verbose = true;
		_return.result.verbose = err.what();
	}
}


