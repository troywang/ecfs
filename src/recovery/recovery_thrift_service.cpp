/*
 * recovery_internal_service.cpp
 *
 *  Created on: Jun 10, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/recovery/recovery_thrift_service.h"

void RecoveryThriftService::recover_chunk(ThriftResult& _return, const FileRecoverToken& token)
{
	try
	{
		mServer.recover_chunk(token);
		_return.code = RESULT_SUCCESS;
	} catch (ErasureException &err) {
		_return.code = err.get_error_code();
		_return.__isset.verbose = true;
		_return.verbose = err.what();
	}
}
