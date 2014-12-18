/*
 * namenode_main.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: wanghuan
 */

#include <signal.h>
#include "thirdparty/gflags/gflags.h"
#include "ecfs/src/common/configs.h"
#include "ecfs/src/common/common.h"
#include "ecfs/src/common/utils.h"
#include "ecfs/src/common/thrift_service_base.h"

#include "ecfs/src/namenode/name_node.h"
#include "ecfs/src/namenode/namenode_thrift_service.h"

#include "ecfs/src/gen-cpp/NamenodeThriftService.h"

static NamenodeServer *sServer = NULL;

static void namenode_sig_handler(int sig)
{
	if (sig == SIGTERM)
	{
		LOG(ERROR) << "name node will be shut down by sigterm";
		sServer->uninit();
		exit(0);
	}
}

int main(int argc, char **argv) {
    // Initialize Google's logging and flag library.
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    sServer = new NamenodeServer();

    if (!ErasureUtils::is_local_ip4_address(FLAGS_namenode_service_ip)) {
        LOG(FATAL) << "EXIT: the service ip address is not hosted locally";
        return -1;
    }

    sServer->get_oplog_manager().set_log_dir(FLAGS_namenode_oplog_dir);

    NamenodeThriftService* namenode_service = new NamenodeThriftService(*sServer);
	boost::shared_ptr<NamenodeThriftService> handler_internal(namenode_service);
	boost::shared_ptr<TProcessor> processor_internal(new NamenodeThriftServiceProcessor(handler_internal));
	// start datanode internal thrift service
	boost::shared_ptr<ThriftServiceThreadWorker> worker_namenode(new ThriftServiceThreadWorker(processor_internal, FLAGS_thrift_namenode_thread_num, FLAGS_namenode_service_port));
	boost::shared_ptr<boost::thread> worker_thread_namenode(new boost::thread(boost::ref(*(worker_namenode.get()))));

	signal(SIGTERM, &namenode_sig_handler);

	sServer->init();

	worker_thread_namenode->join();

    return 0;
}
