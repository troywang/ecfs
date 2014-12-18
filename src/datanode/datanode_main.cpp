/*
 * datanode_main.cpp
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

#include "ecfs/src/datanode/data_node.h"
#include "ecfs/src/datanode/datanode_thrift_service.h"

#include "ecfs/src/gen-cpp/DatanodeThriftService.h"

static DatanodeServer *sServer = NULL;

static void datanode_sig_handler(int sig)
{
	if (sig == SIGTERM)
	{
		LOG(ERROR) << "data node will be shut down by sigterm";
		sServer->uninit();
		exit(0);
	}
}

int main(int argc, char **argv) {
    // Initialize Google's logging and flag library.
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    sServer = new DatanodeServer(FLAGS_datanode_id);

    if (!ErasureUtils::is_local_ip4_address(FLAGS_datanode_service_ip)) {
        LOG(FATAL) << "EXIT: the service ip address is not hosted locally";
        return -1;
    }

    std::string paths = FLAGS_datanode_storage_dirs;
    while (!paths.empty())
    {
        size_t idx = paths.find(";");
        if (idx == std::string::npos)
            idx = paths.length();

        std::string part = paths.substr(0, idx);
        paths = paths.substr(idx + 1 > paths.length() ? paths.length() : idx + 1);

        idx = part.find(":");
        if (idx == std::string::npos){
        	LOG(FATAL) << "wrong format of storage dirs";
        	return -1;
        }

        std::string path = part.substr(0, idx);
        std::string idstr = part.substr(idx + 1);
        sServer->get_disk_manager().add_disk_path(path, atoi(idstr.c_str()));
    }

    DatanodeThriftService* datanode_internal = new DatanodeThriftService(*sServer);
	boost::shared_ptr<DatanodeThriftService> handler_internal(datanode_internal);
	boost::shared_ptr<TProcessor> processor_internal(new DatanodeThriftServiceProcessor(handler_internal));
	// start datanode internal thrift service
	boost::shared_ptr<ThriftServiceThreadWorker> worker_datanode_internal(new ThriftServiceThreadWorker(processor_internal, FLAGS_thrift_datanode_thread_num, FLAGS_datanode_service_port));
	boost::shared_ptr<boost::thread> worker_thread_datanode(new boost::thread(boost::ref(*(worker_datanode_internal.get()))));

	signal(SIGTERM, &datanode_sig_handler);

	sServer->init();

	worker_thread_datanode->join();

    return 0;
}

