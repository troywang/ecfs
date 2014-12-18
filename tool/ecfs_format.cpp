/*
 * ecfs_tool.cpp
 *
 *  Created on: Sep 28, 2014
 *      Author: wanghuan
 */

#include "thirdparty/gflags/gflags.h"
#include "thirdparty/glog/logging.h"

#include "ecfs/src/datanode/chunk_manager.h"

DEFINE_string(root_dir, "/tmp/namenode/", "root dir for chunk file");

DEFINE_int32(node_id, 1, "node id");
DEFINE_int32(disk_id, 1, "disk id");

DEFINE_int32(seq_start, 1, "chunk id seq start");
DEFINE_int32(seq_end, 1000, "chunk id seq end");

DEFINE_int32(sub_capacity, 100, "chunk number in each sub dir");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    int subidx = 1;
    std::string subpath;

    for (int seq = FLAGS_seq_start; seq <= FLAGS_seq_end; seq++)
    {
    	if ((seq - FLAGS_seq_start) % FLAGS_sub_capacity == 0)
    	{
    		std::ostringstream oss;
    		oss << FLAGS_root_dir << "/" << subidx++ << CHUNK_SUB_DIR_SUFFIX << "/";
    		subpath = oss.str();

    		if (!boost::filesystem::exists(oss.str())) {
    			boost::filesystem::create_directories(oss.str());
    		}
    	}

    	std::ostringstream oss;
    	oss << subpath << MAKE_CHUNK_ID(FLAGS_node_id, FLAGS_disk_id, seq) << CHUNK_UNUSE_SUFFIX;

    	int fd = ::open(oss.str().c_str(), O_RDWR | O_CREAT, 0644);
    	::posix_fallocate(fd, 0, FLAGS_chunk_len);
    	::close(fd);
    }

	return 0;
}


