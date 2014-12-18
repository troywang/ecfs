/*
 * recovery_main.cpp
 *
 *  Created on: May 29, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/common/configs.h"
#include "ecfs/src/common/thrift_service_base.h"

#include "ecfs/src/recovery/recovery_node.h"
#include "ecfs/src/recovery/recovery_thrift_service.h"

using namespace ::apache::thrift;

int main(int argc, char **argv)
{
    // Initialize Google's logging and flag library.
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    RecoveryServer server(FLAGS_recovery_id);

    if (!ErasureUtils::is_local_ip4_address(FLAGS_recovery_service_ip)) {
        LOG(FATAL) << "EXIT: the service ip address is not hosted locally";
        return -1;
    }

    RecoveryThriftService* recovery = new RecoveryThriftService(server);
	boost::shared_ptr<RecoveryThriftService> handler(recovery);
	boost::shared_ptr<TProcessor> processor_internal(new RecoveryThriftServiceProcessor(handler));

	// start recovery internal thrift service
	boost::shared_ptr<ThriftServiceThreadWorker> worker_recovery(new ThriftServiceThreadWorker(processor_internal, FLAGS_thrift_recovery_thread_num, FLAGS_recovery_service_port));
	boost::shared_ptr<boost::thread> worker_thread_recovery(new boost::thread(boost::ref(*(worker_recovery.get()))));

	server.init();

	worker_thread_recovery->join();

    return 0;
}


