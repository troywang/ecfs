/*
 * datanode_public_service.cpp
 *
 *  Created on: Apr 28, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/datanode/datanode_thrift_service.h"

void DatanodeThriftService::put_chunk(ThriftResult& _return, const int32_t chunkid, const std::string& data)
{
	if (!mServer.is_running()) {
		_return.code = RESULT_DATANODE_ERR_MAINTAINENCE;
		_return.__isset.verbose = true;
		_return.verbose = "datanode not running";
		return;
	}

	if (NODE_ID(chunkid) != mServer.get_node_id()) {
		_return.code = RESULT_ERR_GENERAL;
		_return.__isset.verbose = true;
		_return.verbose = "thrift request for wrong datanode";
		return;
	}

	try
	{
		DiskEntry &de = mServer.get_disk_manager().find_disk_entry(chunkid);
		de.get_chunk_manager().put_chunk(chunkid, data);
		_return.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.code = err.get_error_code();
		_return.__isset.verbose = true;
		_return.verbose = err.what();
	}
}

void DatanodeThriftService::del_chunk(ThriftResult& _return, const int32_t chunkid)
{
	if (!mServer.is_running()) {
		_return.code = RESULT_DATANODE_ERR_MAINTAINENCE;
		_return.__isset.verbose = true;
		_return.verbose = "datanode not running";
		return;
	}

	if (NODE_ID(chunkid) != mServer.get_node_id()) {
		_return.code = RESULT_ERR_GENERAL;
		_return.__isset.verbose = true;
		_return.verbose = "thrift request for wrong datanode";
		return;
	}

	try
	{
		DiskEntry &de = mServer.get_disk_manager().find_disk_entry(chunkid);
		de.get_chunk_manager().del_chunk(chunkid);
		_return.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.code = err.get_error_code();
		_return.__isset.verbose = true;
		_return.verbose = err.what();
	}

}

void DatanodeThriftService::get_chunk(ResultGetChunk& _return, const int32_t chunkid)
{
	if (!mServer.is_running()) {
		_return.result.code = RESULT_DATANODE_ERR_MAINTAINENCE;
		_return.result.__isset.verbose = true;
		_return.result.verbose = "datanode not running";
		return;
	}

	if (NODE_ID(chunkid) != mServer.get_node_id()) {
		_return.result.code = RESULT_ERR_GENERAL;
		_return.result.__isset.verbose = true;
		_return.result.verbose = "thrift request for wrong datanode";
		return;
	}


	try
	{
		DiskEntry &de = mServer.get_disk_manager().find_disk_entry(chunkid);
		de.get_chunk_manager().get_chunk(chunkid, _return.data);
		_return.result.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.result.code = err.get_error_code();
		_return.result.__isset.verbose = true;
		_return.result.verbose = err.what();
	}
}

void DatanodeThriftService::read_data(ResultReadData& _return, const int32_t chunkid, const int32_t offset, const int32_t size)
{
	if (!mServer.is_running()) {
		_return.result.code = RESULT_DATANODE_ERR_MAINTAINENCE;
		_return.result.__isset.verbose = true;
		_return.result.verbose = "datanode not running";
		return;
	}

	if (NODE_ID(chunkid) != mServer.get_node_id()) {
		_return.result.code = RESULT_ERR_GENERAL;
		_return.result.__isset.verbose = true;
		_return.result.verbose = "thrift request for wrong datanode";
		return;
	}

	try
	{
		DiskEntry &de = mServer.get_disk_manager().find_disk_entry(chunkid);
		de.get_chunk_manager().read_data(chunkid, offset, size, _return.data);
		_return.result.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.result.code = err.get_error_code();
		_return.result.__isset.verbose = true;
		_return.result.verbose = err.what();
	}
}

void DatanodeThriftService::report_chunks(ResultReportChunks& _return)
{
	if (!mServer.is_running()) {
		_return.result.code = RESULT_DATANODE_ERR_MAINTAINENCE;
		_return.result.__isset.verbose = true;
		_return.result.verbose = "datanode not running";
		return;
	}

	try
	{
		mServer.get_disk_manager().get_chunk_ids(_return.chunkUnUse, _return.chunkInUse);
		_return.result.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.result.code = err.get_error_code();
		_return.result.__isset.verbose = true;
		_return.result.verbose = err.what();
	}
}
