//
//
//
#pragma once
#include <vector>
#include <smath/quatf.h>

namespace spu {

struct mat4d_t {
	mat4d_t() = default;
	mat4d_t(const Mat4f &mat)
	{
		for (auto i = 0; i < 4; i++) {
			for (auto j = 0; j < 4; j++) {
				v[i][j] = mat.c[i].f[j];
			}
		}
	}
	double v[4][4];
};

// packed vector
template<class T> struct vec4_t {
	T x, y, z, w;
};

template<class T> struct vec3_t {
	T x, y, z;
};

template<class T> struct vec2_t {
	T x, y;
};

using vec3sf_t = vec3_t<float>;
using vec2sf_t = vec2_t<float>;

using vec4ub_t = vec4_t<uint8_t>;
using vec3ub_t = vec3_t<uint8_t>;
using vec2ub_t = vec2_t<uint8_t>;

using vec4sb_t = vec4_t<int8_t>;
using vec3sb_t = vec3_t<int8_t>;
using vec2sb_t = vec2_t<int8_t>;

using vec4d_t = vec4_t<double>;
using vec3d_t = vec3_t<double>;
using vec2d_t = vec2_t<double>;

template<class T = vec2sf_t> std::vector<T> squareTriangles(const Vec2f &s = Vec2f(1));
template<class T = vec2sf_t> std::vector<T> squareQuads(const Vec2f &s = Vec2f(1));
template<class T = vec2sf_t> uint32_t squareQuadsArray(const Attrs &attrs, const Vec2f &s = Vec2f(1));

inline uint32_t packSnorm3x10_1x2(Vec4f const &v)
{
	union {
		struct {
			int32_t x: 10;
			int32_t y: 10;
			int32_t z: 10;
			int32_t w: 2;
		};
		uint32_t ui;
	} result;

	result.x = roundf(std::clamp(v.x, -1.0F, +1.0F)) * 511.0F;
	result.y = roundf(std::clamp(v.y, -1.0F, +1.0F)) * 511.0F;
	result.z = roundf(std::clamp(v.z, -1.0F, +1.0F)) * 511.0F;
	result.w = roundf(std::clamp(v.w, -1.0F, +1.0F)) * 1.0F;
	return result.ui;
}

struct v2fv2f_t {
	float px, py;
	float tx, ty;
};

struct v3fv2f_t {
	float px, py, pz;
	float tx, ty;
};

struct v3fv4u8_t {
	float px, py, pz;
	uint8_t cx, cy, cz, cw;
};

struct v2fv3f_t {
	float px, py;
	float tx, ty, tz;
};

struct v3fv3f_t {
	float px, py, pz;
	float tx, ty, tz;
};

struct v3fv3fv1i_t {
	float px, py, pz;
	float tx, ty, tz;
	int32_t DrawID;
};

struct v4fv2f_t {
	float px, py, pz, pw;
	float tx, ty;
};

struct v2fc4f_t {
	float px, py;
	float cx, cy, cz, cw;
};

struct v2fc4d_t {
	float px, py;
	double cx, cy, cz, cw;
};

struct v4fc4f_t {
	float px, py, pz, pw;
	float cx, cy, cz, cw;
};

struct v2fc4ub_t {
	float px, py;
	uint8_t cx, cy, cz, cw;
};

struct v2fv2fv4ub_t {
	float px, py;
	float tx, ty;
	uint8_t cx, cy, cz, cw;
};

struct v2fv2fv4f_t {
	float px, py;
	float tx, ty;
	float cx, cy, cz, cw;
};

struct v4fv4f_t {
	float px, py, pz, pw;
	float tx, ty, tz, tw;
};

struct v4fv4fv4f_t {
	float px, py, pz, pw;
	float tx, ty, tz, tw;
	float cx, cy, cz, cw;
};

/*
   3-----2
   |     |     quads:     0-1-2-3
   |     |     triangles: 0-1-2, 2-2-0
   0-----1
*/
template<> inline std::vector<vec2sf_t> squareTriangles(const Vec2f &s)
{
	return (std::vector<vec2sf_t>){
	        {-s.x, -s.y},
                {+s.x, -s.y},
                {+s.x, +s.y},
                {+s.x, +s.y},
                {-s.x, +s.y},
                {-s.x, -s.y},
	};
}

template<> inline std::vector<vec2sf_t> squareQuads(const Vec2f &s)
{
	return (std::vector<vec2sf_t>){
	        {-s.x, -s.y},
	        {+s.x, -s.y},
	        {+s.x, +s.y},
	        {-s.x, +s.y},
	};
}

template<> inline std::vector<v2fv2f_t> squareQuads(const Vec2f &s)
{
	return (std::vector<v2fv2f_t>){
	        {-s.x, -s.y, +0, +1},
	        {+s.x, -s.y, +1, +1},
	        {+s.x, +s.y, +1, +0},
	        {-s.x, +s.y, +0, +0},
	};
}

template<> inline std::vector<v2fv2f_t> squareTriangles(const Vec2f &s)
{
	return (std::vector<v2fv2f_t>){
	        {-s.x, -s.y, +0, +1},
                {+s.x, -s.y, +1, +1},
                {+s.x, +s.y, +1, +0},
	        {+s.x, +s.y, +1, +0},
                {-s.x, +s.y, +0, +0},
                {-s.x, -s.y, +0, +1},
	};
}

template<> inline std::vector<Vec4f> squareQuads(const Vec2f &s)
{
	return (std::vector<Vec4f>){
	        {-s.x, -s.y, 0, 1},
	        {+s.x, -s.y, 0, 1},
	        {+s.x, +s.y, 0, 1},
	        {-s.x, +s.y, 0, 1},
	};
}

template<> inline std::vector<Vec4f> squareTriangles(const Vec2f &s)
{
	return (std::vector<Vec4f>){
	        {-s.x, -s.y, 0, 1},
                {+s.x, -s.y, 0, 1},
                {+s.x, +s.y, 0, 1},
	        {+s.x, +s.y, 0, 1},
                {-s.x, +s.y, 0, 1},
                {-s.x, -s.y, 0, 1},
	};
}

template<class T> inline uint32_t squareQuadsArray(const Attrs &attrs, const Vec2f &s)
{
	auto vertices = spu::squareQuads<T>(s);
	std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
	auto array_id = spu_array_new(attrs);
	spu_array_send(array_id, vertices.data(), vertices.size());
	spu_array_send(array_id, indices.data(), indices.size(), -1, 2);
	return array_id;
}
}  // namespace spu
