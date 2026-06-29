//
// Object :
//
#pragma once

#include <ssys/ssys.h>
#include <spu++/spu++.h>
#include "sb6mfile.h"

namespace spu::sb6 {
class Object {
public:
	Object() = default;
	~Object() = default;

	void draw(uint32_t instance_count = 1, uint32_t base_instance = ~0u);
	void getModelInfo(uint32_t model_index, uint32_t &first, uint32_t &count)
	{
		if (model_index >= m_modelCount) {
			first = 0;
			count = 0;
		}
		else {
			first = m_models[model_index].first;
			count = m_models[model_index].count;
		}
	}

	void load(
	        const char *filename, const Attrs &aux_attrs = Attrs(), const char **sym = nullptr,
	        uint32_t shader_id = 0);

	uint32_t modelCount() const { return m_modelCount; }
	uint32_t slotCount() const { return m_slotCount; }

	SpuArray &getArray() { return m_array; }
	const SpuArray &getArray() const { return m_array; }

private:
	enum { MAX_MODEL = 256 };

	SpuArray m_array;
	uint32_t m_indexCount;
	uint32_t m_modelCount;
	uint32_t m_slotCount;

	SB6M_SUB_OBJECT_DECL m_models[MAX_MODEL];
};
}  // namespace spu::sb6
