//
// RGBA8 :
//
#include "spu_object.h"
#include <ssys/bmp.h>

namespace spu {
namespace libspu {
namespace {

struct RGBA8 {
	uint8_t r, g, b, a;
};

void save_bmp_rgba(const char *path, const RGBA8 *rgba, int32_t width, int32_t height)
{
	std::vector<uint8_t> pix3(size_t(width) * size_t(height) * 3);
	for (auto i = 0; i < width * height; i++) {
		pix3[i * 3 + 2] = rgba[i].r;
		pix3[i * 3 + 1] = rgba[i].g;
		pix3[i * 3 + 0] = rgba[i].b;
	}
	bmp_write(path, pix3.data(), width, height, 0);
}

void save_bmp_r(const char *path, const uint8_t *r, int32_t width, int32_t height)
{
	std::vector<uint8_t> pix3(size_t(width) * size_t(height) * 3);
	for (auto i = 0; i < width * height; i++) {
		pix3[i * 3 + 0] = r[i];
		pix3[i * 3 + 1] = r[i];
		pix3[i * 3 + 2] = r[i];
	}
	bmp_write(path, pix3.data(), width, height, 0);
}

void save_depth(const char *path, const float *depth32f, int32_t width, int32_t height)
{
	std::vector<uint8_t> depth8(size_t(width) * size_t(height));
	auto n = 0;
	auto s1 = 0.0;
	auto s2 = 0.0;

	for (auto i = 0; i < width * height; i++) {
		auto d = double(depth32f[i]);
		if (0.0 < d && d < 1.0) {
			s1 += d;
			s2 += d * d;
			n++;
		}
	}
	auto mean = n != 0 ? s1 / n : 1.0;
	auto var2 = n != 0 ? std::max(0.0, s2 / n - mean * mean) : 0.0;
	auto var = sqrt(var2);

	for (auto i = 0; i < width * height; i++) {
		auto d = double(depth32f[i]);
		if (0.0 < d && d < 1.0) {
			auto z = var != 0.0 ? int32_t(((d - mean) / (3 * var) + 0.5) * 255) : 128;  // 3 sigma
			depth8[i] = std::max(0, std::min(255, z));
		}
		else if (d == 1.0) {
			depth8[i] = 255;
		}
		else {
			depth8[i] = 0;
		}
	}
	save_bmp_r(path, depth8.data(), width, height);
}

template<class T> class Pixels {
public:
	Pixels(uint32_t texture_id, uint32_t pformat) : m_target(texture_id >> 16)
	{
		uint32_t width;
		uint32_t height;
		uint32_t iformat;

		spu_texture_get(texture_id, "iformat", &iformat);
		spu_texture_get(texture_id, "width", &width);
		spu_texture_get(texture_id, "height", &height);
		spu_texture_get(texture_id, "depth", &m_depth);

		if ((texture_id != 0u)
		    && spu_gl_is_depth_component(iformat) != spu_gl_is_depth_component(pformat)) {
			aux_printf("Warning: depth component mismatch (wrong spu_texture_save() format)\n");
			aux_printf("    target:  %s\n", opengl_const(m_target));
			aux_printf("    iformat: %s\n", opengl_const(iformat));
			aux_printf("    pformat: %s\n", opengl_const(pformat));
			aux_printf("    width:   %d\n", width);
			aux_printf("    height:  %d\n", height);
			aux_printf("    depth:   %d\n", m_depth);
		}

// #define MAINTENANCE
#ifdef MAINTENANCE
		aux_printf("spu_texture_recv:\n");
		aux_printf("    target:  %s\n", opengl_const(m_target));
		aux_printf("    iformat: %s\n", opengl_const(iformat));
		aux_printf("    pformat: %s\n", opengl_const(pformat));
		aux_printf("    width:   %d\n", width);
		aux_printf("    height:  %d\n", height);
		aux_printf("    depth:   %d\n", m_depth);
#endif
		auto layer_count = m_depth;

		if (m_target == GL_TEXTURE_CUBE_MAP || m_target == GL_TEXTURE_CUBE_MAP_ARRAY) {
			layer_count *= 6;
		}

		while (width >= 1 && height >= 1) {
			Slice slice = {
			        std::vector<T>(width * height * layer_count),
			        width,
			        height,
			};
			m_slices.push_back(slice);
			width >>= 1;
			height >>= 1;
		}

		for (auto &slice: m_slices) {
			int32_t i = &slice - &m_slices[0];
			int32_t loc[4] = {0, 0, 0, i};
			spu_texture_recv(texture_id, m_slices[i].pixels.data(), pformat, loc);
		}
	}

	void save(const char *path, int32_t level, void (*save_func)(const char *, const T *, int, int))
	{
		auto width = getWidth(level);
		auto height = getHeight(level);
		const T *data = get(level);
		auto layer_count = m_target == GL_TEXTURE_CUBE_MAP ? 6 : m_depth;
		for (auto layer = 0u; layer < layer_count; layer++) {
			const auto layer_path = string_printf(path, layer);
			aux_printf("save layer %d/%d to [%s]...\n", layer, layer_count, layer_path.c_str());
			save_func(layer_path.data(), data, width, height);
			data += width * height;
		}
	}
	const T *get(int32_t level) const { return m_slices[level].pixels.data(); }
	int32_t getWidth(int32_t level) const { return m_slices[level].width; }
	int32_t getHeight(int32_t level) const { return m_slices[level].height; }
	int32_t getDepth() const { return m_depth; }

private:
	struct Slice {
		std::vector<T> pixels;
		uint32_t width;
		uint32_t height;
	};
	std::vector<Slice> m_slices;
	uint32_t m_depth;
	uint32_t m_target;
};

}  // namespace
}  // namespace libspu

using namespace libspu;

void spu_texture_save(uint32_t texture_id, const char *path, const hash32_t &target, uint32_t level)
{
	uint32_t multisample;
	spu_texture_get(texture_id, "multisample", &multisample);
	if (multisample > 0) {
		aux_message(0, "%s: cannot save multisample(%d) texture\n", path, multisample);
		return;
	}

	if (target == "color"_h32) {
		Pixels<RGBA8> pixbuf(texture_id, GL_RGBA8);
		pixbuf.save(path, level, save_bmp_rgba);
	}
	else if (target == "depth"_h32) {
		uint32_t iformat;
		spu_texture_get(texture_id, "iformat", &iformat);
		if (iformat == GL_R32F) {
			Pixels<float> pixbuf(texture_id, GL_R32F);
			pixbuf.save(path, level, save_depth);
		}
		else {
			Pixels<float> pixbuf(texture_id, GL_DEPTH_COMPONENT32F);
			pixbuf.save(path, level, save_depth);
		}
	}
	else if (target == "stencil"_h32) {
		Pixels<uint8_t> pixbuf(texture_id, GL_STENCIL_INDEX8);
		pixbuf.save(path, level, save_bmp_r);
	}
	else {
		aux_error(true, "bad target. (hash must be 'color', 'depth', or 'stencil'\n");
	}
}
}  // namespace spu
