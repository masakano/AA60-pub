//
// SpuQuery :
//
#pragma once
#include <spu/spu.h>
namespace spu {

class SpuQuery {
public:
	SpuQuery() = default;

	~SpuQuery()
	{
		if (m_queryId) spu_query_delete(m_queryId);
	}

	void settarget(uint32_t target)
	{
		assert(m_queryId == 0);
		m_queryTarget = target;
	}

	void reset()
	{
		m_duration = 0;
		m_total = 0;
		m_count = 0;
	}

	void start()
	{
		if (m_queryId == 0) {
			Attrs attrs = {
			        {"target", m_queryTarget}
                        };
			m_queryId = spu_query_new(attrs);
		}
		spu_query_begin(m_queryId);
	}

	void stop(bool is_nonblock = true)
	{
		uint64_t duration_nsec;
		spu_query_end(m_queryId, &duration_nsec, is_nonblock);  // non-block
		m_duration = duration_nsec / 1000;
		m_total += m_duration;
		m_count++;
	}

	uint64_t count() const { return m_count; }
	uint64_t usec() const { return m_duration; }
	uint64_t utotal() const { return m_total; }
	uint64_t uaverage() const { return m_count != 0u ? m_total / m_count : 0; }

	SpuQuery(const SpuQuery &) = delete;
	SpuQuery &operator=(const SpuQuery &) = delete;

private:
	uint32_t m_queryTarget = GL_TIME_ELAPSED;
	uint32_t m_queryId = 0;
	uint64_t m_duration = 0;
	uint64_t m_total = 0;
	uint64_t m_count = 0;
};

/// RAII of SpuQuery
template<class T = SpuQuery> class SpuScopedQuery {
public:
	SpuScopedQuery(T &query) : m_query(query) { m_query.start(); }
	~SpuScopedQuery() { m_query.stop(); }

private:
	T &m_query;
};
}  // namespace spu
