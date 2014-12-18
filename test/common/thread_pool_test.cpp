/*
 * thread_pool_test.cpp
 *
 *  Created on: May 30, 2014
 *      Author: wanghuan
 */

#include "ecfs/src/common/thread_pool.h"
#include "ecfs/test/common/ecfs_test_base.h"

class TestTask : public Task
{
public:
	virtual void run () {
		pthread_exit(NULL);
	}
};

class ThreadPoolTest : public ::testing::Test
{
public:
	void test_all ();

private:
	ThreadPool mPool;
};

void ThreadPoolTest::test_all()
{
	boost::shared_ptr<Task> pTask(new TestTask());
	mPool.add(pTask);
	pTask->wait();
}

TEST_F(ThreadPoolTest, TestAll)
{
	test_all ();
}



