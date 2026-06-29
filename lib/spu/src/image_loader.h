//
// R8 :
//
#pragma once
#include "spu_object.h"
#include <ssys/random_generator.h>

#ifndef GL_RGB16
#define GL_RGB16 0x8054
#endif

#ifndef GL_RGBA16
#define GL_RGBA16 0x805B
#endif

namespace spu::libspu::image {

inline uint8_t to_uchar(float c) { return std::clamp(c, 0.0f, 1.0f) * float(0xff) + 0.5f; }

inline uint8_t to_uchar(uint16_t c) { return c >> 8; }

inline uint16_t to_ushort(double c) { return std::clamp(c, 0.0, 1.0) * double(0xffff) + 0.5; }

inline float to_float(uint8_t c) { return c / float(0xff); }

inline double to_double(uint16_t c) { return c / double(0xffff); }

struct R8 {
	uint8_t r;

	R8() = default;
	R8(float r, float g, float b, float) { this->r = to_float((r + g + b) / 3); }

	void setc(uint8_t r, uint8_t g, uint8_t b, uint8_t)
	{
		this->r = (int32_t(r) + int32_t(g) + int32_t(b)) / 3;
	}
	void setf(float r, float g, float b, float) { this->r = to_uchar((r + g + b) / 3); }
	void get(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const { r = g = b = a = this->r; }
	void get(float &r, float &g, float &b, float &a) const { r = g = b = a = to_float(this->r); }
};

struct RGB8 {
	uint8_t r, g, b;

	RGB8() = default;
	RGB8(float r, float g, float b, float a) { setf(r, g, b, a); }

	void setc(uint8_t r, uint8_t g, uint8_t b, uint8_t)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}
	void setf(float r, float g, float b, float)
	{
		this->r = to_uchar(r);
		this->g = to_uchar(g);
		this->b = to_uchar(b);
	}
	void get(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const
	{
		r = this->r;
		g = this->g;
		b = this->b;
		a = 255;
	}
	void get(float &r, float &g, float &b, float &a) const
	{
		r = to_float(this->r);
		g = to_float(this->g);
		b = to_float(this->b);
		a = 1.0;
	}
};

struct RGBA8 {
	uint8_t r, g, b, a;

	RGBA8() = default;
	RGBA8(float r, float g, float b, float a) { setf(r, g, b, a); }
	void setc(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}
	void setf(float r, float g, float b, float a)
	{
		this->r = to_uchar(r);
		this->g = to_uchar(g);
		this->b = to_uchar(b);
		this->a = to_uchar(a);
	}
	void get(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const
	{
		r = this->r;
		g = this->g;
		b = this->b;
		a = this->a;
	}
	void get(float &r, float &g, float &b, float &a) const
	{
		r = to_float(this->r);
		g = to_float(this->g);
		b = to_float(this->b);
		a = to_float(this->a);
	}
};

struct RGB16 {
	uint16_t r, g, b;

	RGB16() = default;
	RGB16(float r, float g, float b, float a) { setf(r, g, b, a); }
	void setc(uint8_t r, uint8_t g, uint8_t b, uint8_t /*a*/)
	{
		this->r = to_ushort(r);
		this->g = to_ushort(g);
		this->b = to_ushort(b);
	}
	void setf(float r, float g, float b, float /*a*/)
	{
		this->r = to_ushort(r);
		this->g = to_ushort(g);
		this->b = to_ushort(b);
	}
	void get(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const
	{
		r = to_uchar(this->r);
		g = to_uchar(this->g);
		b = to_uchar(this->b);
		a = 255;
	}
	void get(float &r, float &g, float &b, float &a) const
	{
		r = to_double(this->r);
		g = to_double(this->g);
		b = to_double(this->b);
		a = 1.0;
	}
};

struct RGBA16 {
	uint16_t r, g, b, a;

	RGBA16() = default;
	RGBA16(float r, float g, float b, float a) { setf(r, g, b, a); }

	void setc(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		this->r = to_ushort(r);
		this->g = to_ushort(g);
		this->b = to_ushort(b);
		this->a = to_ushort(a);
	}
	void setf(float r, float g, float b, float a)
	{
		this->r = to_ushort(r);
		this->g = to_ushort(g);
		this->b = to_ushort(b);
		this->a = to_ushort(a);
	}
	void get(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const
	{
		r = to_uchar(this->r);
		g = to_uchar(this->g);
		b = to_uchar(this->b);
		a = to_uchar(this->a);
	}
	void get(float &r, float &g, float &b, float &a) const
	{
		r = to_double(this->r);
		g = to_double(this->g);
		b = to_double(this->b);
		a = to_double(this->a);
	}
};

struct RGB32F {
	float r, g, b;

	RGB32F() = default;

	RGB32F(float r, float g, float b, float a) { setf(r, g, b, a); }

	void setc(uint8_t r, uint8_t g, uint8_t b, uint8_t)
	{
		this->r = to_float(r);
		this->g = to_float(g);
		this->b = to_float(b);
	}
	void setf(float r, float g, float b, float)
	{
		this->r = r;
		this->g = g;
		this->b = b;
	}
	void get(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const
	{
		r = to_uchar(this->r);
		g = to_uchar(this->g);
		b = to_uchar(this->b);
		a = 255;
	}
	void get(float &r, float &g, float &b, float &a) const
	{
		r = this->r;
		g = this->g;
		b = this->b;
		a = 1.0;
	}
};

struct RGBA32F {
	float r, g, b, a;

	RGBA32F() = default;

	RGBA32F(float r, float g, float b, float a) { setf(r, g, b, a); }

	void setc(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		this->r = to_float(r);
		this->g = to_float(g);
		this->b = to_float(b);
		this->a = to_float(a);
	}

	void setf(float r, float g, float b, float a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	void get(uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) const
	{
		r = to_uchar(this->r);
		g = to_uchar(this->g);
		b = to_uchar(this->b);
		a = to_uchar(this->a);
	}

	void get(float &r, float &g, float &b, float &a) const
	{
		r = this->r;
		g = this->g;
		b = this->b;
		a = this->a;
	}
};

struct FlipPoint {
	int32_t x, y, w, h;

	FlipPoint trans(const char c)
	{
		switch (c) {
		case 'x': return {w - 1 - x, y, w, h};
		case 'y': return {x, h - 1 - y, w, h};
		case 's': return {std::min(y, w), std::min(x, h), w, h};
		default: return *this;
		}
	}

	FlipPoint trans(const char *str)
	{
		while (*str) {
			*this = trans(*str++);
		}
		return *this;
	}
};

template<class T> class Face {
public:
	int32_t sx() const { return m_sx; }
	int32_t sy() const { return m_sy; }

	const std::vector<T> &pixels() const { return m_pixels; }
	std::vector<T> &pixels() { return m_pixels; }

	Face() = default;

	Face(int32_t width, int32_t height) { alloc(width, height); }

	void alloc(int32_t width, int32_t height)
	{
		m_sx = width;
		m_sy = height;
		m_pixels.resize(m_sx * m_sy);
	}

	template<class TS> void *copy(const Face<TS> &src)
	{
		alloc(src.sx(), src.sy());
		for (auto i = 0u; i < src.pixels().size(); i++) {
			float r, g, b, a;
			src.pixels()[i].get(r, g, b, a);
			m_pixels[i].setf(r, g, b, a);
		}
		return m_pixels.data();
	}

	void fill(const T &pixel) { m_pixels.assign(m_sx * m_sy, pixel); }

	void swap(Face<T> &src)
	{
		m_sx = src.sx();
		m_sy = src.sy();
		m_pixels.swap(src);
	}

	void flip(const char *command)
	{
		auto buf = m_pixels;  // copy
		for (auto i = 0u; i < m_pixels.size(); i++) {
			int32_t x = i % m_sx;
			int32_t y = i / m_sx;
			FlipPoint p = {x, y, m_sx, m_sy};
			FlipPoint fp = p.trans(command);
			m_pixels[i] = buf[fp.y * m_sx + fp.x];
		}
	}

	void halfsize(int32_t limit_size)
	{
		if (limit_size == 0 || m_sx < limit_size || m_sy < limit_size) {
			return;
		}

		auto org = m_pixels;  // copy
		alloc(m_sx / 2, m_sy / 2);

		for (auto i = 0u; i < m_pixels.size(); i++) {
			int32_t x0 = (i % m_sx) * 2;
			int32_t y0 = (i / m_sx) * 2;

			float sr = 0;
			float sg = 0;
			float sb = 0;
			float sa = 0;

			for (auto y = y0; y < y0 + 2; y++) {
				for (auto x = x0; x < x0 + 2; x++) {
					auto &sp = org[y * (m_sx * 2) + x];

					float r, g, b, a;
					sp.get(r, g, b, a);

					sr += r;
					sg += g;
					sb += b;
					sa += a;
				}
			}
			m_pixels[i].setf(sr / 4, sg / 4, sb / 4, sa / 4);
		}
	}

	void fixAlpha(float alpha)
	{
		for (auto &p: m_pixels) {
			float r;
			float g;
			float b;
			float a;
			p.get(r, g, b, a);
			p.setf(r, g, b, alpha);
		}
	}

	void clampColor(float clamp_color)
	{
		for (auto &p: m_pixels) {
			float r, g, b, a;
			p.get(r, g, b, a);

			r = std::clamp(r, 0.0f, clamp_color);
			g = std::clamp(g, 0.0f, clamp_color);
			b = std::clamp(b, 0.0f, clamp_color);
			a = std::clamp(b, 0.0f, clamp_color);

			p.setf(r, g, b, a);
		}
	}

	void grayScale()
	{
		for (auto &p: m_pixels) {
			float r, g, b, a;
			p.get(r, g, b, a);
			auto gray = (r + g + b) / 3;
			p.setf(gray, gray, gray, a);
		}
	}

	void smoothEdge()
	{
		for (auto i = 0u; i < m_pixels.size(); i++) {
			int32_t x0 = i % m_sx;
			int32_t y0 = i / m_sx;

			auto &p = m_pixels[i];
			float r, g, b, a;
			p.get(r, g, b, a);

			if (a != 1.0) {
				int32_t sy = m_pixels.size() / m_sx;
				float sr = 0;
				float sg = 0;
				float sb = 0;
				float sa = 0;

				// 5x5 sum
				for (auto y = y0 - 2; y < y0 + 2; y++) {
					if (y < 0 || y >= sy) {
						continue;
					}
					for (auto x = x0 - 2; x < x0 + 2; x++) {
						if (x < 0 || x >= m_sx) {
							continue;
						}
						float r, g, b, a;
						m_pixels[y * m_sx + x].get(r, g, b, a);

						sr += r * a;
						sg += g * a;
						sb += b * a;
						sa += a;
					}
				}
				if (sa > 0) {
					sr /= sa;
					sg /= sa;
					sb /= sa;
				}
				r = r * a + sr * (1 - a);
				g = g * a + sg * (1 - a);
				b = b * a + sb * (1 - a);
				m_pixels[i].setf(r, g, b, a);
			}
		}
	}

	Face extract(int32_t ox, int32_t oy, int32_t sx, int32_t sy, const char *flip_command = "") const
	{
		Face dst(sx, sy);

		for (auto y = 0; y < sy; y++) {
			memcpy(&dst.m_pixels[y * sx], &m_pixels[(oy + y) * m_sx + ox], sx * sizeof(T));
		}

		if (flip_command[0]) {
			dst.flip(flip_command);
		}

		return dst;
	}

	void makeChecker()
	{
		const T c0(1.000, 0.125, 0.125, 1.000);
		const T c1(1, 1, 1, 1);

		for (auto i = 0; i < m_sx * m_sy; i++) {
			int32_t x = i % m_sx;
			int32_t y = i / m_sx;
			m_pixels[i] = ((x & 32) ^ (y & 32)) ? c0 : c1;
		}
	}

	void makeRand()
	{
		RandomGenerator<float> frand;
		for (auto i = 0; i < m_sx * m_sy; i++) {
			m_pixels[i].setf(frand(), frand(), frand(), frand());
		}
	}

private:
	int32_t m_sx = 0;
	int32_t m_sy = 0;
	std::vector<T> m_pixels;
};

template<class T> class Cube {
public:
	std::array<Face<T>, 6> &getFaces() { return m_faces; }
	int32_t unit() const { return m_unit; }

	void extract(const Face<T> &src)
	{
		if (src.sx() / 3 == src.sy() / 4) {
			m_unit = src.sx() / 3;
			loadVerticalCross(src);
		}
		else if (src.sx() / 4 == src.sy() / 3) {
			m_unit = src.sx() / 4;
			loadHorizontalCross(src);
		}
		else if (src.sx() == src.sy() / 6) {
			m_unit = src.sx();
			loadVerticalStraight(src);
		}
		else {
			assert(src.sx() == src.sy());  // expect fallback image is square
			m_unit = src.sx();
			m_faces[e_py] = src;
			m_faces[e_nx] = src;
			m_faces[e_pz] = src;
			m_faces[e_px] = src;
			m_faces[e_ny] = src;
			m_faces[e_nz] = src;
		}
	}

private:
	enum {
		e_px = 0,
		e_nx = 1,
		e_py = 2,
		e_ny = 3,
		e_pz = 4,
		e_nz = 5,
	};

	// Face<T> m_faces[6];
	std::array<Face<T>, 6> m_faces;

	int32_t m_unit;

	void loadVerticalCross(const Face<T> &src)
	{
		//       +----+
		//       | py |
		//  +----+----+----+
		//  | nx | pz | px |
		//  +----+----+----+
		//       | ny |
		//       +----+
		//       | nz |
		//       +----+
		int32_t u = m_unit;

		m_faces[e_py] = src.extract(1 * u, 3 * u, u, u, "y");
		m_faces[e_nx] = src.extract(0 * u, 2 * u, u, u, "y");
		m_faces[e_pz] = src.extract(1 * u, 2 * u, u, u, "y");
		m_faces[e_px] = src.extract(2 * u, 2 * u, u, u, "y");
		m_faces[e_ny] = src.extract(1 * u, 1 * u, u, u, "y");
		m_faces[e_nz] = src.extract(1 * u, 0 * u, u, u, "x");
	}

	void loadHorizontalCross(const Face<T> &src)
	{
		//       +----+
		//       | py |
		//  +----+----+----+----+
		//  | nx | pz | px | nz |
		//  +----+----+----+----+
		//       | ny |
		//       +----+
		int32_t u = m_unit;
		m_faces[e_py] = src.extract(1 * u, 2 * u, u, u, "y");
		m_faces[e_nx] = src.extract(0 * u, 1 * u, u, u, "y");
		m_faces[e_pz] = src.extract(1 * u, 1 * u, u, u, "y");
		m_faces[e_px] = src.extract(2 * u, 1 * u, u, u, "y");
		m_faces[e_nz] = src.extract(3 * u, 1 * u, u, u, "y");
		m_faces[e_ny] = src.extract(1 * u, 0 * u, u, u, "y");
	}

	// vertical straight
	void loadVerticalStraight(const Face<T> &src)
	{
		//  +----+
		//  | px |
		//  +----+
		//  | nx |
		//  +----+
		//  | py |
		//  +----+
		//  | ny |
		//  +----+
		//  | pz |
		//  +----+
		//  | nz |
		//  +----+
		int32_t u = m_unit;
		m_faces[e_px] = src.extract(0 * u, 5 * u, u, u, "y");
		m_faces[e_nx] = src.extract(0 * u, 4 * u, u, u, "y");
		m_faces[e_py] = src.extract(0 * u, 3 * u, u, u, "y");
		m_faces[e_ny] = src.extract(0 * u, 2 * u, u, u, "y");
		m_faces[e_pz] = src.extract(0 * u, 1 * u, u, u, "y");
		m_faces[e_nz] = src.extract(0 * u, 0 * u, u, u, "y");
	}
};

class Loader {
public:
	static constexpr int32_t c_cubeTarget[6] = {
	        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
	};

	void loadImage(const std::filesystem::path &path, const Attrs &attrs, bool is_full_convert);
	uint32_t width() const { return m_width; }
	uint32_t height() const { return m_height; }
	uint32_t depth() const { return 1; }
	uint32_t format() const { return m_format; }
	const void *pixels() const { return m_pixels; }

	void embedR8(uint32_t *rgba8, int32_t channel) const
	{
		assert(m_format == GL_R8);
		auto r8 = (const uint8_t *)m_pixels;
		auto shift = channel * 8;
		auto mask = 0xff << shift;
		for (auto i = 0u; i < m_width * m_height; i++) {
			rgba8[i] = (rgba8[i] & ~mask) | ((r8[i] << shift) & mask);
		}
	}

private:
	const void *m_pixels = nullptr;  // not owner

	std::filesystem::path m_path;
	Attrs m_attrs;
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_format = 0;

	uint32_t m_target;
	uint32_t m_limitSize;
	float m_clamp;

	float m_alpha;
	char m_flip[8];
	bool m_smoothEdge;
	bool m_grayScale;

	Face<R8> m_r8;
	Face<RGB8> m_rgb8;
	Face<RGBA8> m_rgba8;
	Face<RGB16> m_rgb16;
	Face<RGBA16> m_rgba16;
	Face<RGB32F> m_rgb32f;
	Face<RGBA32F> m_rgba32f;

	Cube<R8> m_r8Cube;
	Cube<RGB8> m_rgb8Cube;
	Cube<RGBA8> m_rgba8Cube;
	Cube<RGB16> m_rgb16Cube;
	Cube<RGBA16> m_rgba16Cube;
	Cube<RGB32F> m_rgb32fCube;
	Cube<RGBA32F> m_rgba32fCube;

	std::vector<uint8_t> m_dat;

	bool loadFromFile();
	void alloc(uint32_t format, int32_t width, int32_t height);

	template<class T> void buildFace(Face<T> &face, Cube<T> &cubes)
	{
		if (m_target == GL_TEXTURE_CUBE_MAP) {
			cubes.extract(face);
			for (auto &cube_face: cubes.getFaces()) {
				doEverything(cube_face);
			}
			m_width = cubes.unit();
			m_height = cubes.unit();

			int32_t n = 0;
			int32_t size = m_width * m_height;

			face.alloc(m_width, m_height * 6);
			for (auto &cube_face: cubes.getFaces()) {
				memcpy(&face.pixels()[n], cube_face.pixels().data(), size * sizeof(T));
				n += size;
			}
		}
		else {
			doEverything(face);
			face.halfsize(m_limitSize);
			m_width = face.sx();
			m_height = face.sy();
		}
		m_pixels = face.pixels().data();
	}

	template<class T> void *copy(Face<T> &dst) const
	{
		switch (m_format) {
		case GL_R8: return dst.copy(m_r8);
		case GL_RGB8: return dst.copy(m_rgb8);
		case GL_RGBA8: return dst.copy(m_rgba8);
		case GL_RGB16: return dst.copy(m_rgb16);
		case GL_RGBA16: return dst.copy(m_rgba16);
		case GL_RGB32F: return dst.copy(m_rgb32f);
		case GL_RGBA32F: return dst.copy(m_rgba32f);
		default: aux_error(true, "%s: unknown format\n", opengl_const(m_format));
		}
	}

	template<class T> void doEverything(Face<T> &face)
	{
		// return;
		if (m_flip[0]) {
			face.flip(m_flip);
		}
		if (m_clamp != 0.0) {
			face.clampColor(m_clamp);
		}
		if (m_alpha != 0.0) {
			face.fixAlpha(m_alpha);
		}
		if (m_smoothEdge) {
			face.smoothEdge();
		}
		if (m_grayScale) {
			face.grayScale();
		}
	}
	void convert(uint32_t format, bool is_full_convert);
};
}  // namespace spu::libspu::image
