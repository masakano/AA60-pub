//
// QueryObject :
//
#include "spu_object.h"

namespace spu {
namespace libspu {
namespace {

class QueryObject : public Object {
public:
	uint32_t m_target;
	uint64_t m_value;
	uint64_t m_count;

	enum {
		e_idle = 0,
		e_measuring,
		e_counting,
		e_rendering,
	} m_state;

	explicit QueryObject(const Attrs &attrs)
	        : m_target(attrs.get("target", GL_TIME_ELAPSED)), m_value(0), m_count(0), m_state(e_idle)
	{
		F(glGenQueries, 1, &m_handle.ui);
	}

	~QueryObject() override { F(glDeleteQueries, 1, &m_handle.ui); }

	void begin(uint32_t mode)
	{
		sync(true);
		if (mode == 0) {
			aux_error(m_state == e_measuring, "begin() withount end()\n");
			if (m_state == e_idle) {
				if (is_duration()) {
					F(glBeginQuery, m_target, m_handle.ui);
					m_state = e_measuring;
				}
				else {
					F(glQueryCounter, m_handle.ui, m_target);
					m_state = e_counting;  // no need of end()
				}
			}
		}
		else {
			aux_error(m_state == e_rendering, "begin() withount end()\n");
			F(glBeginConditionalRender, m_handle.ui, mode);
			m_state = e_rendering;
		}
	}

	uint64_t end(uint64_t *value, bool is_nonblock)
	{
		switch (m_state) {
		case e_rendering:
			F(glEndConditionalRender);
			m_state = e_idle;
			break;
		case e_measuring:
			F(glEndQuery, m_target);
			m_state = e_counting;
			sync(is_nonblock);
			break;
		case e_counting: sync(is_nonblock); break;
		default: spu_message(0, "end() without begin() (ignored)\n"); break;
		}
		if (value) {
			*value = m_value;
		}
		return m_count;
	}

	bool sync_and_copy(uint64_t *value, bool is_nonblock)
	{
		bool ret = sync(is_nonblock);
		if (!ret && value) {
			*value = m_value;
		}
		return ret;
	}

private:
	bool sync(bool is_nonblock)
	{
		if (m_state == e_counting) {
			uint32_t is_available = 0;
			F(glGetQueryObjectuiv, m_handle.ui, GL_QUERY_RESULT_AVAILABLE, &is_available);
			if ((is_available != 0u) || !is_nonblock) {
				F(glGetQueryObjectui64v, m_handle.ui, GL_QUERY_RESULT, &m_value);
				m_count++;
				m_state = e_idle;
				return false;
			}
			return true;
		}
		return false;
	}

	bool is_duration() const { return m_target != GL_TIMESTAMP; }
};
Manager<QueryObject> s_objects("query", 0);
}  // namespace
}  // namespace libspu

using namespace libspu;

uint32_t spu_query_new(const Attrs &attrs)
{
	SET();
	return s_objects.add(attrs).ui;
}

void spu_query_delete(uint32_t query_id)
{
	SET();
	s_objects.remove(query_id);
}

int32_t spu_query_get(uint32_t query_id, const hash32_t &key, void *)
{
	GET_CHK(query_id, key);
	return 0;
}

bool spu_query_begin(uint32_t query_id, uint32_t mode)
{
	SET();

	auto *current = s_objects.at(query_id);
	if (current->m_target == GL_TIME_ELAPSED) {  // GL_TIME_ELAPSED only
		for (auto &query: s_objects.objects()) {
			if (query && query->m_state != QueryObject::e_idle && query != current
			    && query->m_target == current->m_target) {
				spu_message(
				        0, "target (%s) already used at query #%d and #%d. (ignored)\n",
				        opengl_const(query->m_target), query_id,
				        &query - &s_objects.objects()[0]);
				return false;
			}
		}
	}
	current->begin(mode);
	return true;
}

uint64_t spu_query_end(uint32_t query_id, uint64_t *value, bool is_nonblock)
{
	SET();
	return s_objects.at(query_id)->end(value, is_nonblock);
}

bool spu_query_sync(uint32_t query_id, uint64_t *value, bool is_nonblock)
{
	SET();
	return s_objects.at(query_id)->sync_and_copy(value, is_nonblock);
}
}  // namespace spu
