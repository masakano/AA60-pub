//
// Uniform :
//
#pragma once
#include "spu_object.h"

namespace spu::libspu {
class Uniform {
public:
	void init(uint32_t shaderId) { m_shaderId = shaderId; }
	bool addUniform(const std::string &name, void *ptr)
	{
		int32_t loc;
		uint32_t size;
		uint32_t type;
		const auto *name_cstr = name.c_str();
		spu_shader_loc(m_shaderId, &name_cstr, &loc, &size, &type, 1);

		if (loc >= 0) {
			m_names.push_back(name);
			m_ptrs.push_back(ptr);
			m_locs.push_back(loc);
			m_sizes.push_back(size);
			return true;
		}
		return false;
	}

	void use() const { spu_shader_use(m_shaderId, m_locs.data(), m_ptrs.data(), m_locs.size()); }

private:
	uint32_t m_shaderId;
	std::vector<std::string> m_names;
	std::vector<int32_t> m_locs;
	std::vector<void *> m_ptrs;
	std::vector<uint32_t> m_sizes;
};
}  // namespace spu::libspu
