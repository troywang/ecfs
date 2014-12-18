/*
 * recovery_internal_service.h
 *
 *  Created on: Jun 10, 2014
 *      Author: wanghuan
 */

#ifndef RECOVERY_INTERNAL_SERVICE_H_
#define RECOVERY_INTERNAL_SERVICE_H_

#include "ecfs/src/recovery/recovery_node.h"
#include "ecfs/src/gen-cpp/RecoveryThriftService.h"

class RecoveryThriftService : public RecoveryThriftServiceIf
{
public:
	RecoveryThriftService (RecoveryServer &server) : mServer(server) {
	}

	virtual void recover_chunk(ThriftResult& _return, const FileRecoverToken& token);

private:
	RecoveryServer &mServer;
};

#endif /* RECOVERY_INTERNAL_SERVICE_H_ */
