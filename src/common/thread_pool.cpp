#include "ecfs/src/common/thread_pool.h"

Task::Task()
{
	m_status = WAITING;
	m_create = time(NULL);
	m_begin = m_end = 0;
	m_prog = 0;
}

void Task::start()
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_begin = time(NULL);
	m_status = RUNNING;
	lock.unlock();
	run();
	lock.lock();
	if (m_status == RUNNING) {
		m_status = COMPLETED;
		m_end = time(NULL);
		m_cond.notify_all(); //可能有多个线程同时在wait task
		lock.unlock();
		on_complete();
	}
}

void Task::stop()
{
	boost::mutex::scoped_lock lock(m_mutex);
	lock.unlock();
	cancel();
	lock.lock();
	if (!is_done()) {
		m_end = time(NULL);
		m_cond.notify_all();
	}
	m_status = CANCELED; //m_status should always be CANCELED once stop() has been called
	lock.unlock();
	on_cancel();
}

std::string Task::str()
{
	std::ostringstream oss;
	oss << "[";
	oss << "create_time: " << m_create << ", ";
	oss << "begin_time: " << m_begin << ", ";
	oss << "end_time: " << m_end << ", ";
	oss << "progress: " << m_prog << ", ";
	oss << "status: " << m_status;
	oss << "]";
	return oss.str();
}

void Task::wait()
{
	boost::mutex::scoped_lock lock(m_mutex);
	if (is_done())
		return;
	m_cond.wait(lock);
}

bool Task::timed_wait(uint32_t timeout)
{
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout);
	return timed_wait(until);
}

bool Task::timed_wait(const boost::system_time &until)
{
	boost::mutex::scoped_lock lock(m_mutex);
	if (is_done())
		return true;
	if (!m_cond.timed_wait(lock, until)) {
		return false;
	}
	return true;
}

Worker::Worker(ThreadPool &mgr) : m_mgr(mgr)
{
	m_active_time = 0;
	m_thread.reset(new boost::thread(boost::ref(*this)));
}

Worker::~Worker()
{
	if (m_thread.get()) {
		m_thread->interrupt();
		m_thread->join();
		m_thread.reset();
	}
}

void Worker::acquire(taskp task)
{
	boost::mutex::scoped_lock lock(m_mutex);
	if (m_tsk.get()) { // should never happen
		return;
	}

	m_tsk = task;
	m_cond.notify_one();
}

taskp Worker::get_task()
{
	boost::mutex::scoped_lock lock(m_mutex);
	return m_tsk;
}

void Worker::operator()()
{
	boost::mutex::scoped_lock lock(m_mutex);
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			if (!m_tsk.get()) {
				m_cond.wait(lock);
			}

			m_active_time = time(NULL);
			m_tsk->start();
			m_active_time = 0;

			m_tsk.reset();
			m_mgr.put_worker(shared_from_this());
		}
	} catch (boost::thread_interrupted &msg) {
	}
}


ThreadPool::ThreadPool(uint32_t min, uint32_t max, uint32_t idle, uint32_t timeout)
{
	if (min > max) {
		max = min;
	}
	m_min = min;
	m_max = max;
	m_idle = idle;
	m_timeout = timeout;

	m_main.reset(new boost::thread(boost::ref(*this)));
	for (uint32_t i = 0; i < m_min; i++) {
		m_idle_workers.push_back(workerp(new Worker(*this)));
	}

	m_expire.reset(new boost::thread(boost::bind(&ThreadPool::expire_workers, this)));

	m_sem.reset(new boost::interprocess::interprocess_semaphore(m_max));
}

ThreadPool::~ThreadPool()
{
	if (m_main.get()) {
		m_main->interrupt(); //interrupt before post
		m_sem->post();
		m_main->join();
		m_main.reset();
	}

	if (m_expire.get()) {
		m_expire->interrupt();
		m_expire->join();
		m_expire.reset();
	}

	if (m_sem.get()) {
		m_sem.reset();
	}
}

void ThreadPool::add(const taskp &task)
{
	m_tasks.push_back(task);
}

void ThreadPool::stop(const taskp &task)
{
	task->stop();
}

void ThreadPool::addv(const std::vector<taskp> &tasks)
{
	for (size_t i = 0; i < tasks.size(); i++)
		m_tasks.push_back(tasks[i]);
}

void ThreadPool::stopv(const std::vector<taskp> &tasks)
{
	for (size_t i = 0; i < tasks.size(); i++)
		tasks[i]->stop();
}

void ThreadPool::wait(const taskp &task)
{
	task->wait();
}

bool ThreadPool::timed_wait(const taskp &task, uint32_t timeout)
{
	return task->timed_wait(timeout);
}

void ThreadPool::waitv(const std::vector<taskp> &tasks)
{
	for (size_t i = 0; i < tasks.size(); i++)
		tasks[i]->wait();
}

bool ThreadPool::timed_waitv(const std::vector<taskp> &tasks, uint32_t timeout)
{
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout);
	for (size_t i = 0; i < tasks.size(); i++) {
		if (!tasks[i]->timed_wait(until))
			return false;
	}
	return true;
}

void ThreadPool::operator()()
{
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			taskp task;
			if (!m_tasks.timed_pop_front(task, m_idle)) {
				reclaim_workers();
			} else {
				workerp worker;
				get_worker(worker);
				worker->acquire(task);
			}
		}
	} catch (boost::thread_interrupted &msg) {
	}
}

void ThreadPool::get_worker (workerp &wkr)
{
	m_sem->wait(); // cannot be interrupted
	boost::this_thread::interruption_point();

	boost::mutex::scoped_lock lock(m_mutex);
	if (!m_idle_workers.empty()) {
		wkr = m_idle_workers.front();
		//reuse the youngest one
		m_idle_workers.pop_front();
		m_active_workers.insert(wkr);
	} else {
		wkr.reset(new Worker(*this));
		m_active_workers.insert(wkr);
	}
}

void ThreadPool::put_worker (workerp worker)
{
	boost::mutex::scoped_lock lock(m_mutex);

	std::set<workerp>::iterator it = m_active_workers.find(worker);
	if (it != m_active_workers.end()) //woker may have been deleted by timeout
	{
		m_active_workers.erase(it);

		//put the youngest one in front of the queue
		m_idle_workers.push_front(worker);
		m_sem->post();
	}
}

void ThreadPool::reclaim_workers ()
{
	boost::mutex::scoped_lock lock(m_mutex);

	size_t td = m_idle_workers.size() + m_active_workers.size() - m_min;
	while (td-- != 0 && !m_idle_workers.empty()) {
		m_idle_workers.pop_back(); //reclaim the oldest first
	}
}

void ThreadPool::expire_workers ()
{
	try
	{
		while (!boost::this_thread::interruption_requested())
		{
			{
				boost::mutex::scoped_lock lock(m_mutex);
				std::set<workerp>::iterator it;
				for (it = m_active_workers.begin(); it != m_active_workers.end();)
				{
					workerp p = *it;
					if (p->m_active_time != 0 && time(NULL) - p->m_active_time > m_timeout)
					{
						taskp tsk = p->get_task();
						if (tsk.get()) {
							tsk->stop();
						}

						m_active_workers.erase(it++);
						m_idle_workers.push_front(workerp(new Worker(*this)));
						m_sem->post();
					} else {
						it++;
					}
				}
			}

			boost::this_thread::sleep(boost::posix_time::seconds(m_timeout));
		}

	} catch (boost::thread_interrupted &msg) {
	}
}



