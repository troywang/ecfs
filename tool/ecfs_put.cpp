/*
 * ecfs_put.cpp
 *
 *  Created on: Oct 10, 2014
 *      Author: wanghuan
 */
#include "thirdparty/gflags/gflags.h"
#include "thirdparty/glog/logging.h"

#include "ecfs/src/common/blocking_queue.h"
#include "ecfs/src/client/ecfs_client.h"

DEFINE_int32(threads, 1, "concurrency");
DEFINE_int32(block_cnt, 10, "blocks to write");
DEFINE_string(work_dir, "/run/shm", "Work Directory");

std::string data;
std::string copy;
BlockingQueue<int32_t> queue;

void prepare_data ()
{
	int cutoff = FLAGS_chunk_len - 1;
	int len = FLAGS_K * FLAGS_chunk_len;
	char *buf = new char[len];
	memset(buf + len - cutoff, 0, cutoff);

	data.assign(buf, len - cutoff);
	copy.assign(buf, len);
}

void put_file ()
{
	ErasureClient client;

	std::vector<int32_t> blockids;
	for (int i = 0; i < FLAGS_block_cnt; i++)
	{
		try
		{
			int32_t blockid = client.put_block(data);
			queue.push_back(blockid);
		} catch (ErasureException &err) {
			LOG(ERROR) << err.what();
		}
	}
}

void get_file ()
{
	ErasureClient client;

	while (true)
	{
		int32_t blockid;
		queue.pop_front(blockid);

		try
		{
			std::string tmp;
			client.get_block(blockid, tmp);

			if (copy == tmp) {
				LOG(ERROR) << "done get file";
			} else {
				LOG(ERROR) << blockid << " corrupted";
			}
		} catch (ErasureException &err) {
			LOG(ERROR) << err.what();
		}
	}
}

void do_print_status ()
{

}

void print_status ()
{
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			boost::this_thread::sleep(boost::posix_time::seconds(10));
			do_print_status();
		}
	}
	catch (boost::thread_interrupted&)
	{
		printf("Test Done\n");
	}
}

void put_files ()
{
    boost::thread_group grp;

    for (int i = 0; i < FLAGS_threads; i++)
    {
        boost::thread *thread = new boost::thread(put_file);
    	grp.add_thread(thread);
    }

//    boost::thread *rthread = new boost::thread(get_file);
//    grp.add_thread(rthread);

	grp.join_all();
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    printf("============================================================================================\n");
    printf(" Erasure Performance Test Tools\n");
    printf("============================================================================================\n");

    prepare_data();

    boost::thread* thread = new boost::thread(print_status);

    boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
    put_files();
    boost::posix_time::time_duration duration = boost::posix_time::microsec_clock::local_time() - start;

    thread->interrupt();
    thread->join();
    do_print_status();

    printf("Test Cost: %9.2f sec\n", (double) duration.total_milliseconds() / 1000);

	return 0;
}

