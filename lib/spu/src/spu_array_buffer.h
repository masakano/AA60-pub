//
// Buffer :
//
#pragma once
#include "spu_object.h"

namespace spu::libspu::spu_array {

class Buffer {
public:
	Buffer() = default;
	virtual ~Buffer() = default;

	void alloc(uint32_t target, uint32_t prefix_id, uint32_t prefix_stride)
	{
		if (target == 0) {
			target = GL_ARRAY_BUFFER;  // use default
		}
		if (m_id != 0) {
			spu_message(1, "buffer already allocated (%d) (replaced)\n", m_id);
			free();
		}
		if (prefix_id != 0u) {
			int32_t total_size;
			m_target = target;
			m_id = prefix_id;
			m_isLink = true;
			bind();
			F(glGetBufferParameteriv, m_target, GL_BUFFER_SIZE, &total_size);
			setStride(prefix_stride);
			setNelem(total_size / prefix_stride);
		}
		else {
			m_target = target;
			F(glGenBuffers, 1, &m_id);
			bind();  // need check
		}
	}

	void free()
	{
		if (!m_isLink && m_id) {
			F(glDeleteBuffers, 1, &m_id);
		}
		m_id = 0;
	}

	void bind() const
	{
		assert(m_target != 0);
		aux_error(m_id == 0, "buffer not allocated\n");
		F(glBindBuffer, m_target, m_id);
	}

	void unbind() const { F(glBindBuffer, m_target, 0); }

	void send(const void *data, uint32_t nelem, uint32_t stride, uint32_t target = 0)
	{
		assert(stride > 0);

		if (m_id == 0) {
			alloc(target, 0, 0);
		}
		else {
			bind();
		}

		if (m_isLink && nelem == 0) {
			int32_t total_size;
			F(glGetBufferParameteriv, m_target, GL_BUFFER_SIZE, &total_size);
			nelem = total_size / stride;
		}

		if (!(m_isLink && data == nullptr)) {  // bindless
			F(glBufferData, m_target, stride * nelem, data, GL_STATIC_DRAW);
		}
		m_nelem = nelem;
		m_stride = stride;
		unbind();
	}

	void update(const void *data, uint32_t first, uint32_t count) const
	{
		bind();

		if (first + count > m_nelem) {
			F(glBufferData, m_target, m_stride * m_nelem, nullptr, GL_STATIC_DRAW);
			spu_message(0, "buffer overflow (reallocate)\n");
		}
		F(glBufferSubData, m_target, first * m_stride, count * m_stride, data);
		unbind();
	}
	void recv(void *data, uint32_t nelem) const
	{
		bind();
		F(glGetBufferSubData, m_target, 0, m_stride * nelem, data);
		unbind();
	}
	void *map(uint32_t access) const
	{
		bind();
		return glMapBufferRange(m_target, 0, m_nelem * m_stride, access);
	}

	void unmap() const
	{
		bind();
		F(glUnmapBuffer, m_target);
		unbind();
	}

	void link(Buffer *src)
	{
		free();
		m_id = src->m_id;

		if (m_target == 0) {
			m_target = src->m_target;
		}

		// change align at same type only (nv/compute_water)
		if (m_target == src->m_target) {
			m_stride = src->m_stride;
		}
		else {
			spu_message(1, "link from different target (%s)\n", opengl_const(src->m_target));
		}
		m_nelem = src->m_nelem * src->m_stride / m_stride;
		m_isLink = true;
	}

	void copy(Buffer *src, uint32_t dst_offset, uint32_t src_offset, uint32_t total_size)
	{
		if (total_size == 0) {
			auto src_size = src->m_nelem * src->m_stride;
			auto dst_size = m_nelem * m_stride;

			if (dst_size < src_size) {
				m_stride = src->m_stride;
				m_nelem = src->m_nelem;
				send(nullptr, m_nelem, m_stride);
			}
			total_size = src_size;
		}

		F(glBindBuffer, GL_COPY_READ_BUFFER, src->m_id);
		F(glBindBuffer, GL_COPY_WRITE_BUFFER, m_id);
#ifdef MAINTENANCE
		if (src_offset || dst_offset) {
			aux_printf("CopyBufferSubData\n");
			aux_printf("    src id     : %d\n", src->m_id);
			aux_printf("    dst id     : %d\n", m_id);
			aux_printf("    src offset : %d\n", src_offset);
			aux_printf("    dst offset : %d\n", dst_offset);
			aux_printf("    size       : %d\n", total_size);
		}
#endif
		F(glCopyBufferSubData, GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, src_offset, dst_offset,
		  total_size);

		F(glBindBuffer, GL_COPY_READ_BUFFER, 0);
		F(glBindBuffer, GL_COPY_WRITE_BUFFER, 0);
	}

	void report() const
	{
		aux_printf("\tid      : %d\n", m_id);
		aux_printf("\ttarget  : %s\n", opengl_const(m_target));
		aux_printf("\tnelem   : %d x %d\n", m_stride, m_nelem);
		aux_printf("\tis_link : %d\n", m_isLink);
	}
#if 0
	void checkOverflow(uint32_t first, uint32_t count) const
	{
		aux_error(
		        m_nelem < first + count, "overflow: id=%d first=%d count=%d nelem=%d target=%s\n", m_id,
		        first, count, m_nelem, opengl_const(m_target));
	}
#endif

	uint32_t target() const { return m_target; }
	uint32_t id() const { return m_id; }
	int32_t nelem() const { return m_nelem; }
	uint32_t stride() const { return m_stride; }
	void setNelem(uint32_t nelem) { m_nelem = nelem; }
	void setStride(uint32_t stride) { m_stride = stride; }

private:
	bool m_isLink = false;
	uint32_t m_id = 0;
	uint32_t m_target = 0;
	uint32_t m_stride = 1;
	uint32_t m_nelem = 0;
};
}  // namespace spu::libspu::spu_array
