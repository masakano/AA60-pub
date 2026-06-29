//
// SpuArray : array object
//
#pragma once

#include "spu_renderstate.h"
#include "spu_shader.h"
#include <smath/mat4f.h>

namespace spu {

namespace spu_object {
struct ArrayBody : SpuBody {
	ArrayBody(uint32_t handle, bool is_owner) : SpuBody(handle, is_owner) {}
	ArrayBody(const Attrs &attrs) : SpuBody(0, true) { handle = spu_array_new(attrs); }
	~ArrayBody()
	{
		assert(use_count == 0);
		if (is_owner) {
			spu_array_delete(handle);
		}
	}
};
}  // namespace spu_object

struct SpuCommand {
	uint32_t count = 0;
	uint32_t instance_count = 1;
	uint32_t first = 0;
	uint32_t base_vertex = 0;
	uint32_t base_instance = 0;
	uint32_t mode = GL_TRIANGLES;
	uint32_t target = 0;
	uint32_t flags = 0;  // reserved (zero filled)
};

/// c++ interface of spu_aray_*()
class SpuArray : public spu_object::SpuObject<spu_object::ArrayBody> {
public:
	using callback_t = std::function<std::pair<void *, uint32_t>(uint32_t)>;
	using spu_object::SpuObject<spu_object::ArrayBody>::set;
	using spu_object::SpuObject<spu_object::ArrayBody>::get;

	SpuArray() = default;

	explicit SpuArray(uint32_t handle, bool is_owner = true) { reset(handle, is_owner); }
	explicit SpuArray(const Attrs &attrs) { init(attrs); }

	virtual void draw(const callback_t &callback) { spu_array_draw(id(), callback); }

	virtual void draw(
	        uint32_t mode, uint32_t first = 0, uint32_t count = 0, uint32_t instance_count = 1,
	        uint32_t target = 0, uint32_t base_vertex = 0, uint32_t base_instance = 0)
	{
		spu_array_draw(id(), mode, first, count, instance_count, target, base_vertex, base_instance);
	}

	template<class T = void *> T map(const char *mode, int32_t slot = 0) const
	{
		auto bits = 0u;
		for (; *mode; mode++) {
			if (*mode == 'r') bits |= GL_MAP_READ_BIT;
			if (*mode == 'w') bits |= GL_MAP_WRITE_BIT;
		}
		return static_cast<T>(spu_array_map(id(), bits, slot));
	}

	void unmap(int32_t slot) const { return spu_array_unmap(id(), slot); }

	void aux(const Attrs &attrs, int32_t slot)
	{
		slot == 0 ? init(attrs) : spu_array_aux(id(), attrs, slot);
	}

	void send(const void *data, uint32_t nelem, int32_t slot = 0, uint32_t stride = 4)
	{
		spu_array_send(id(), data, nelem, slot, stride);
	}

	void update(const void *data, uint32_t first, uint32_t count, int32_t slot = 0)
	{
		spu_array_update(id(), data, first, count, slot);
	}

	void recv(void *data, uint32_t nelem, int32_t slot = 0) const
	{
		spu_array_recv(id(), data, nelem, slot);
	}

	void copy(
	        int32_t dst_slot, const SpuArray &src_array, int32_t src_slot, uint32_t dst_offset = 0,
	        uint32_t src_offset = 0, uint32_t size = 0)
	{
		spu_array_copy(id(), src_array.id(), dst_slot, src_slot, dst_offset, src_offset, size);
	}

	void link(int32_t dst_slot, const SpuArray &src_array, int32_t src_slot)
	{
		spu_array_link(id(), src_array.id(), dst_slot, src_slot);
	}

	template<class T> void send(const std::vector<T> &data, int32_t slot = 0, uint32_t stride = 4)
	{
		send(data.data(), data.size(), slot, stride);
	}

	template<class T> void recv(std::vector<T> &data, int32_t slot = 0) const
	{
		recv(data.data(), data.size(), slot);
	}
	SPU_SET_GET_REPORT(array);
};

struct SpuComputeArray : public SpuArray {
public:
	SpuComputeArray()
	{
		SpuArray::init({
		        {"nelem", 1}
                });
	}

	Vec4i &getDim() { return m_dim; }
	const Vec4i &getDim() const { return m_dim; }

	SpuShader &getShader() { return m_shader; }
	const SpuShader &getShader() const { return m_shader; }

	virtual void compute()
	{
		assert(id() != 0);
		m_shader.use();
		SpuArray::draw(0xffff, m_dim.x, m_dim.y, m_dim.z);
	}

protected:
	Vec4i m_dim = {1, 1, 1, 1};
	SpuShader m_shader;
};
}  // namespace spu
