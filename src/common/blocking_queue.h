#ifndef _BLOCKING_QUEUE__
#define _BLOCKING_QUEUE__

#include <deque>
#include <stdint.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

template<typename Data>
class BlockingQueue : public boost::noncopyable
{
public:
	BlockingQueue   (size_t cap = -1) : m_cap(cap) {}
	~BlockingQueue  () {}

	void 	push_back(const Data &data);
	void 	pop_front(/* out */ Data &data);

	void	push_front(const Data &data);
	void	pop_back(/* out */ Data &data);

	/* timeout in milliseconds */
	bool 	timed_push_back(const Data &data, uint32_t timeout_ms);
	bool 	timed_pop_front(/* out */ Data &data, uint32_t timeout_ms);

	bool	timed_push_front(const Data &data, uint32_t timeout_ms);
	bool	timed_pop_back(/* out */ Data &data, uint32_t timeout_ms);

	size_t	cap() { return m_cap; }
	size_t	size() { boost::mutex::scoped_lock lock(m_mutex); return m_queue.size(); }
	bool	empty() { boost::mutex::scoped_lock lock(m_mutex); return m_queue.empty(); }

private:
	size_t 							m_cap; //max object can stay in this queue, -1 means INT_MAX
	boost::mutex					m_mutex;
	boost::condition_variable		m_cond_en; //notify producer, en means enqueue
	boost::condition_variable		m_cond_de; //notify consumer, de means dequeue
	std::deque<Data> 				m_queue;
};

//implementation
template<typename Data>
void BlockingQueue<Data>::push_back(const Data &data)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.size() >= m_cap) {
		m_cond_en.wait(lock);
	}
	m_queue.push_back(data);
	m_cond_de.notify_one();
}

template<typename Data>
void BlockingQueue<Data>::pop_front(/* out */ Data &data)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.empty()) {
		m_cond_de.wait(lock);
	}
	data = m_queue.front();
	m_queue.pop_front();
	m_cond_en.notify_one();
}

template<typename Data>
void BlockingQueue<Data>::push_front(const Data &data)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.size() >= m_cap) {
		m_cond_en.wait(lock);
	}
	m_queue.push_front(data);
	m_cond_de.notify_one();
}

template<typename Data>
void BlockingQueue<Data>::pop_back(/* out */ Data &data)
{
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.empty()) {
		m_cond_de.wait(lock);
	}
	data = m_queue.back();
	m_queue.pop_back();
	m_cond_en.notify_one();
}

template<typename Data>
bool BlockingQueue<Data>::timed_push_back(const Data &data, uint32_t timeout_ms)
{
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.size() >= m_cap) {
		if (!m_cond_en.timed_wait(lock, until)) {
			return false;
		}
	}
	m_queue.push_back(data);
	m_cond_de.notify_one();
	return true;
}

template<typename Data>
bool BlockingQueue<Data>::timed_pop_front(/* out */ Data &data, uint32_t timeout_ms)
{
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.empty()) {
		if (!m_cond_de.timed_wait(lock, until)) {
			return false;
		}
	}
	data = m_queue.front();
	m_queue.pop_front();
	m_cond_en.notify_one();
	return true;
}

template<typename Data>
bool BlockingQueue<Data>::timed_push_front(const Data &data, uint32_t timeout_ms)
{
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.size() >= m_cap) {
		if (!m_cond_en.timed_wait(lock, until)) {
			return false;
		}
	}
	m_queue.push_front(data);
	m_cond_de.notify_one();
	return true;
}

template<typename Data>
bool BlockingQueue<Data>::timed_pop_back(/* out */ Data &data, uint32_t timeout_ms)
{
	boost::system_time until = boost::get_system_time() + boost::posix_time::milliseconds(timeout_ms);
	boost::mutex::scoped_lock lock(m_mutex);
	while (m_queue.empty()) {
		if (!m_cond_de.timed_wait(lock, until)) {
			return false;
		}
	}
	data = m_queue.back();
	m_queue.pop_back();
	m_cond_en.notify_one();
	return true;
}

#endif


