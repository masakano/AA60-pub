//
// ResourceObject :
//
#include "resource_object.h"
#include <atomic>
#include <deque>
#include <mutex>
#include <thread>

namespace spu::libspu::resource {

class Queue {
public:
	// foreground
	void push(ResourceObject *res)
	{
#ifdef MAINTENANCE
		res->m_isBgRun = true;
		res->doBackground();
		res->m_isBgRun = false;
#else
		std::lock_guard<std::mutex> lock(m_mutex);
		res->m_isBgRun = true;
		m_queue.push_back(res);

		if (!m_isBgRun) {
			m_isBgRun = true;
			m_thread = std::thread([&]() { background(); });
			m_thread.detach();
		}
#endif
	}

	int32_t size() const { return m_queue.size(); }

private:
	std::thread m_thread;
	std::deque<ResourceObject *> m_queue;
	std::mutex m_mutex;
	bool m_isBgRun = false;

	// background
	ResourceObject *pop()
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_queue.empty()) {
			auto *res = m_queue.front();
			m_queue.pop_front();
			return res;
		}
		return nullptr;
	}

	void background()
	{
		ResourceObject *res;
		while ((res = pop())) {
			spu_message(1, "load \"%s\"\n", res->m_path.c_str());
			res->doBackground();
			res->m_isBgRun = false;
			if (res->m_reqDispose) {
				ResourceObject::dispose(res);
			}
		}
		m_isBgRun = false;
	}
};

enum { e_queue_max = 4 };
Queue s_queues[e_queue_max];

ResourceObject::ResourceObject(const char *path, const Attrs &attrs) : m_path(path), m_attrs(attrs) {}

void ResourceObject::dispose(ResourceObject *res)
{
	if (res->m_isBgRun) {
		spu_message(1, "lazy dispose resource: %s\n", res->m_path.c_str());
		res->m_reqDispose = true;
	}
	else {
		delete res;
	}
}

void ResourceObject::start()
{
	if (m_reqBackground) {
		auto q_index = 0;
		auto q_size = s_queues[0].size();

		for (auto index = 1; index < e_queue_max; index++) {
			auto size = s_queues[index].size();
			if (size < q_size) {
				q_size = size;
				q_index = index;
			}
		}
		s_queues[q_index].push(this);
	}
}

bool ResourceObject::sync()
{
	// if (!m_isBgRun && m_reqEpilogue) {
	if (!m_isBgRun && m_reqBackground) {
		doEpilogue();
		// m_reqEpilogue = false;
		m_reqBackground = false;
	}
	return m_isBgRun;
}
}  // namespace spu::libspu::resource
