//
//
//
#pragma once

#include "spu_object.h"

#if 0
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#endif
#endif

namespace spu::libspu::spu_renderstate {

int32_t s_blend_func_channel = -1;  // all channel

union value_t {
	void *p;
	uint32_t u;  // This is 32bit. NOT size_t
	int32_t s;
	float f;

	value_t() = default;
	explicit value_t(void *p) : p(p) {}
	explicit value_t(uint32_t u) : u(u) {}
	explicit value_t(int32_t s) : s(s) {}
	explicit value_t(float f) : f(f) {}
};

union vec2 {
	int32_t i[2];
	float f[2];
};

union vec4 {
	int32_t i[4];
	float f[4];
	struct {
		float r, g, b, a;
	};
};

// get
inline int32_t getInt(int32_t param)
{
	int32_t val[4];
	F(glGetIntegerv, param, val);
	return val[0];
}

inline float getFloat(int32_t param)
{
	float val[4];
	F(glGetFloatv, param, val);
	return val[0];
}

inline vec4 getFloat4(int32_t param)
{
	vec4 val;
	F(glGetFloatv, param, val.f);
	return val;
}

inline vec4 getInt4(int32_t param)
{
	vec4 val;
	F(glGetIntegerv, param, val.i);
	return val;
}

// print
inline void printEnum(const char *sym, uint32_t val)
{
	std::string s_val = opengl_const(val);
	if (s_val.length() > 24) {
		s_val = s_val.substr(0, 24) + "...";
	}
	aux_printf("\t%-24s : [%08x] %s\n", sym, val, s_val.c_str());
}

inline void printInt(const char *sym, uint32_t val) { aux_printf("\t%-24s : 0x%08x\n", sym, val); }

inline void printInt4(const char *sym, const vec4 &val)
{
	aux_printf("\t%-24s : %d %d %d %d\n", sym, val.i[0], val.i[1], val.i[2], val.i[3]);
}

inline void printFloat(const char *sym, float val) { aux_printf("\t%-24s : %f\n", sym, val); }

inline void printFloat4(const char *sym, const vec4 &val)
{
	aux_printf("\t%-24s : %f %f %f %f\n", sym, val.r, val.g, val.b, val.a);
}

// copy
template<class T> value_t copyDeep(value_t val, const T &src)
{
	if (val.p) {
		*static_cast<T *>(val.p) = src;
	}
	return val;
}

//  cache
template<class T> bool cache(T *prev, const void *curr)
{
	constexpr int32_t size = sizeof(T);
	if (curr && memcmp(prev, curr, size) != 0) {
		memcpy(prev, curr, size);
		return true;
	}
	return false;
}
}  // namespace spu::libspu::spu_renderstate
#if 0
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif
