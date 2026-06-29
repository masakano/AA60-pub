//
// SpuBody :
//
#pragma once

#include <ssys/attrs.h>

#define SPU_SET_GET_REPORT(name)                                                 \
	void set(const Attrs &attrs) override { spu_##name##_set(id(), attrs); } \
	int32_t get(const hash32_t &key, void *value) const override             \
	{                                                                        \
		return spu_##name##_get(id(), key, value);                       \
	}                                                                        \
	void report(const char *str) const override                              \
	{                                                                        \
		if (str && *str) aux_printf("%s:\n", str);                       \
		spu_##name##_report(id());                                       \
	}

namespace spu::spu_object {

struct SpuBody {
	SpuBody(uint32_t handle, bool is_owner) : use_count(0), handle(handle), is_owner(is_owner) {}
	int32_t use_count;
	uint32_t handle;
	bool is_owner;
};

template<class body_t> class SpuObject {
public:
	SpuObject() = default;
	SpuObject(const SpuObject &src) { share_ptr(src.m_body); }
	SpuObject(SpuObject &&src) noexcept : m_body(src.m_body) { src.m_body = nullptr; }

	virtual ~SpuObject() { clear_ptr(); }

	SpuObject &operator=(const SpuObject &src)
	{
		share_ptr(src.m_body);
		return *this;
	}

	bool operator==(const SpuObject &o) const { return id() == o.id(); }

	virtual void init(const Attrs &attrs) { share_ptr(new body_t(attrs)); }
	virtual void dispose() { clear_ptr(); }
	virtual void reset(uint32_t handle, bool is_owner = true) { share_ptr(new body_t(handle, is_owner)); }
	virtual void report(const char *str) const = 0;
	virtual void set(const Attrs &attrs) = 0;
	virtual int32_t get(const hash32_t &key, void *value) const = 0;

	const uint32_t &id() const { return m_body ? m_body->handle : ms_nullid; }
	bool is_owner() const { return m_body ? m_body->is_owner : 0; }
	int32_t use_count() const { return m_body ? m_body->use_count : 0; }
	body_t *get_body() const { return m_body; }

	template<class T> void set(const hash32_t &key, const T &value) { set(Attrs({Attr(key, value)})); }

protected:
	void clear_ptr()
	{
		if (m_body) {
			if (--m_body->use_count <= 0) {
				delete m_body;
			}
			m_body = nullptr;
		}
	}

	void share_ptr(body_t *src_body)
	{
		body_t *prev_body = m_body;

		if ((m_body = src_body)) {
			m_body->use_count++;
		}

		if (prev_body) {  // delete after alloc
			if (--prev_body->use_count <= 0) {
				delete prev_body;
			}
		}
	}

private:
	body_t *m_body = nullptr;
	inline static uint32_t ms_nullid = 0;
};
}  // namespace spu::spu_object
