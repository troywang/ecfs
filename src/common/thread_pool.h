
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <set>
#include "ecfs/src/common/blocking_queue.h"
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

class	Worker;
class 	ThreadPool;

class Task : public boost::noncopyable
{
public:
	friend class	Worker;
	friend class	ThreadPool;
	enum status { WAITING, RUNNING, COMPLETED, CANCELED };
	Task ();
	virtual ~Task () {}
	virtual std::string str();

	status		stat() { return m_status; }
	uint16_t 	progress() { return m_prog; }
	time_t		create_time() { return m_create; }
	time_t		start_time() { return m_begin; }
	time_t		end_time() { return m_end; }

	void		wait();
	bool		timed_wait(uint32_t timeout_ms);
	bool		timed_wait(const boost::system_time &until);

	//do the real work
	virtual void		run() = 0;
	//interrupt run
	virtual void		cancel() {}
	//called when run completed
	virtual void		on_complete() {}
	//called when cancel completed
	virtual void 		on_cancel() {}

	void 	update_progress(uint16_t prog) { m_prog = prog; }

private:
	void 	start();
	void	stop();
	bool	is_done() { return m_status == COMPLETED || m_status == CANCELED; }

private:
	status		m_status;
	time_t		m_create;
	time_t 		m_begin;
	time_t 		m_end;
	uint16_t 	m_prog;

	boost::mutex		m_mutex;
	boost::condition_variable	m_cond;
};

typedef boost::shared_ptr<Task> taskp;
typedef boost::shared_ptr<Worker> workerp;

class Worker : public boost::noncopyable, public boost::enable_shared_from_this<Worker>
{
public:
	friend class ThreadPool;

	~Worker	();
	Worker	(ThreadPool &mgr);
	void 	operator()();

	taskp   get_task ();

private:
	void 	acquire(taskp task);

private:
	taskp			m_tsk;
	time_t			m_active_time; //last active time
	ThreadPool&		m_mgr;

	boost::mutex	m_mutex;

	boost::condition_variable		 m_cond;
	boost::shared_ptr<boost::thread> m_thread;
};

class ThreadPool : public boost::noncopyable
{
public:
	friend class 	Worker;

	ThreadPool	(uint32_t min = 1,
				 uint32_t max = 128,
				 uint32_t idle_ms = 600 * 1000,
				 uint32_t timeout_sec = 10 * 60);

	~ThreadPool ();

	void 	add(const taskp &task);
	void 	stop(const taskp &task);

	void	addv(const std::vector<taskp> &tasks);
	void	stopv(const std::vector<taskp> &tasks);

	void	wait(const taskp &task);
	bool	timed_wait(const taskp &task, uint32_t timeout_ms);

	void	waitv(const std::vector<taskp> &tasks);
	bool	timed_waitv(const std::vector<taskp> &tasks, uint32_t timeout_ms);

	void	operator()();

private:
	void	put_worker(workerp worker);
	void	get_worker(workerp &worker);

	void 	reclaim_workers ();
	void 	expire_workers ();


private:
	size_t		m_min;
	size_t		m_max;
	uint32_t	m_idle; /* start to reclaim when pass the idle time */
	uint32_t 	m_timeout; /* max time for a task to finish */

	boost::mutex			m_mutex; // mutex for workers

	std::set<workerp>		m_active_workers;
	std::deque<workerp>		m_idle_workers;
	BlockingQueue<taskp> 	m_tasks;

	std::auto_ptr<boost::thread>	m_main;
	std::auto_ptr<boost::thread>	m_expire;

	std::auto_ptr<boost::interprocess::interprocess_semaphore> m_sem;
};

#endif /* THREAD_POOL_H_ */
