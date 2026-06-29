//
// Attrib :
//
#pragma once

#include "spu_object.h"

namespace spu::libspu::spu_array {

class Attrib {
public:
	Attrib(const char *name, int32_t nelem, int32_t size, uint32_t iformat, uint32_t oformat, bool is_norm)
	        : m_name(name), m_elemSize(nelem), m_size(size), m_iformat(iformat), m_oformat(oformat),
	          m_isNorm(is_norm)
	{
		if (isdigit(m_name[0])) {
			m_loc.id = std::stoi(m_name);
			m_loc.target = GL_ARRAY_BUFFER;
			m_isFixed = true;
		}
		else if (m_name[0] == '+' && isdigit(m_name[1])) {
			m_loc.id = std::stoi(m_name.c_str() + 1);
			m_loc.target = GL_SHADER_STORAGE_BUFFER;
			m_isFixed = true;
		}
		else if (m_name[0] == '=' && isdigit(m_name[1])) {
			aux_error(true, "atomic counter ('%s') deprecated\n", m_name.c_str());
		}
	}

	uint32_t fixup(uint32_t shader_id)
	{
		if (!m_isFixed) {
			if (shader_id == 0) {
				return 0xffff;
			}
			const char *name = m_name.c_str();
			spu_shader_loc(shader_id, &name, &m_loc.i, nullptr, nullptr, 1);
		}
		if (m_loc.id != 0xffff) {
			aux_error(
			        !isValidBuffer(m_loc.target), "%s: invalid attribute buffer [%d]\n",
			        m_name.c_str(), shader_id);
		}

		// safety
		m_elemSize = std::max(m_elemSize, 1);
		m_size = std::max(m_size, 1);

		return m_loc.target;
	}

	size_t bind(size_t buf_ptr, int32_t buf_size, int32_t divisor) const
	{
		if (m_loc.id != 0xffff) {
			for (auto ofs = 0; ofs < std::max(1, m_elemSize / 4); ofs++) {
				auto loc = m_loc.id + ofs;
				auto elem_size = std::min(4, m_elemSize);
				size_t ptr = buf_ptr + size_t(ofs) * 16;

				attribPointer(loc, elem_size, buf_size, size_t(ptr));
				F(glVertexAttribDivisor, loc, divisor);
				F(glEnableVertexAttribArray, loc);
			}
		}
		return buf_ptr + buf_size;
	}

	static void reportTitle()
	{
		aux_printf("\t    ");
		aux_printf("%4s ", "loc");
		aux_printf("%4s ", "elem");
		aux_printf("%4s ", "size");
		aux_printf("%16s ", "iformat");
		aux_printf("%9s ", "oformat");
		aux_printf("%4s ", "norm");
		aux_printf("%s", "name");
		aux_printf("\n");
	}

	void report() const
	{
		aux_printf("\t    ");
		aux_printf("%04x ", m_loc.id);
		aux_printf("%4d ", m_elemSize);
		aux_printf("%4d ", m_size);
		if (m_loc.target == GL_SHADER_STORAGE_BUFFER) {
			aux_printf("%16s ", "(ssbo)");
			aux_printf("%9s ", "(ssbo)");
		}
		else {
			aux_printf("%16s ", opengl_const(m_iformat));
			aux_printf("%9s ", opengl_const(m_oformat));
		}
		aux_printf("%4d ", m_isNorm);
		aux_printf("%s", m_name.c_str());
		aux_printf("\n");
	}
	const std::string &name() const { return m_name; }
	int32_t size() const { return m_size; }
	int32_t loc() const { return m_loc.id; }

private:
	std::string m_name;
	Handle m_loc = -1;
	int32_t m_elemSize = 0;
	int32_t m_size = 0;
	uint32_t m_iformat = 0;
	uint32_t m_oformat = 0;
	bool m_isNorm = false;
	bool m_isFixed = false;

	static bool isValidBuffer(uint32_t target)
	{
		return (target == GL_ARRAY_BUFFER /*|| target == GL_TRANSFORM_FEEDBACK_BUFFER*/
		        || target == GL_SHADER_STORAGE_BUFFER /*|| target == GL_ATOMIC_COUNTER_BUFFER*/);
	}

	void attribPointer(int32_t loc, int32_t elem_size, int32_t buf_size, size_t ptr) const
	{
		const void *gl_ptr = reinterpret_cast<const void *>(ptr);

		switch (m_oformat) {
		case GL_INT: F(glVertexAttribIPointer, loc, elem_size, m_iformat, buf_size, gl_ptr); break;
		case GL_DOUBLE: F(glVertexAttribLPointer, loc, elem_size, m_iformat, buf_size, gl_ptr); break;
		case GL_FLOAT:
			F(glVertexAttribPointer, loc, elem_size, m_iformat, m_isNorm, buf_size, gl_ptr);
			break;
		default: aux_error(true, "invalid oformat (%s)\n", opengl_const(m_oformat));
		}
	}
};
}  // namespace spu::libspu::spu_array
