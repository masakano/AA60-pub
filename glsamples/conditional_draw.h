//
// ConditionalDraw :
//
#pragma once
#include <spu++/spu++.h>

namespace spu {

class ConditionalDraw {
public:
	void init(uint32_t array_id, uint32_t query_id)
	{
		m_arrayId = array_id;
		m_queryId = query_id;
	}

	void probe(bool is_test = true, bool is_nonblock = true)
	{
		SpuScopedRenderstate renderstate(true);
		renderstate.write_mask = {0, 0, 0, 0, 0};
		renderstate.use();

		spu_query_begin(m_queryId);
		if (is_test) {
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
		spu_query_end(m_queryId, &m_value, is_nonblock);
	}

	void begin(uint32_t mode = GL_QUERY_NO_WAIT) { spu_query_begin(m_queryId, mode); }
	void end() { spu_query_end(m_queryId, nullptr, true); }
	u_int64_t get() { return m_value; }

private:
	uint32_t m_arrayId = 0;
	uint32_t m_queryId = 0;
	u_int64_t m_value = 0;
};

}  // namespace spu
