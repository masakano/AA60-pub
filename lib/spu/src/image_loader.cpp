//
// Loader :
//
#include <tinyexr.h>
#include "image_loader.h"

namespace spu::libspu::image {

const int32_t c_tofu_size = 32;  // maybe 32*32*4byte mininum: need check

void Loader::loadImage(const std::filesystem::path &path, const Attrs &attrs, bool is_full_convert)
{
	m_path = path;
	m_attrs = attrs;
	m_target = m_attrs.get("target", GL_TEXTURE_2D);
	m_clamp = m_attrs.get("clamp", 0.0f);

	m_attrs.peek("nz", "use 'gray_scale' instead");
	m_alpha = m_attrs.get("alpha", 0.0f);
	m_limitSize = m_attrs.get("limit_size", 0);
	m_smoothEdge = m_attrs.get("smooth_edge", 1);
	m_grayScale = m_attrs.get("gray_scale", 0);

	memset(m_flip, 0, sizeof(m_flip));
	strncpy(m_flip, m_attrs.get("flip", ""), 7);

	if (m_path.extension() == ".cache") {
		m_format = m_attrs.get("iformat", 0);
		m_width = m_attrs.get("width", 0);
		m_height = m_attrs.get("height", 0);

		aux_error(
		        m_format == 0 || m_width == 0 || m_height == 0,
		        "explicit format/width/height needed for %s\n", path.c_str());

		auto full_path = File::searchPath(m_path);
		auto size = File(full_path).size();
		if (size > 0) {
			File file(full_path, "rb");
			m_dat.resize(size);
			file.read(m_dat.data(), m_dat.size());
		}
		aux_error(m_dat.data() == nullptr, "\"%s\": cannot read\n", m_path.c_str());
		m_pixels = m_dat.data();
		return;  // no additional work
	}
	else if (m_path.string()[0] == '#') {
		aux_error(
		        m_path.string().length() != 9, "\"%s\": bad path. must be '#%%02x%%02x%%02x%%02x'\n",
		        m_path.c_str());

		alloc(GL_RGBA8, c_tofu_size, c_tofu_size);
		m_rgba8.alloc(c_tofu_size, c_tofu_size);

		uint32_t r, g, b, a;
		sscanf(m_path.string().c_str(), "#%02x%02x%02x%02x", &r, &g, &b, &a);
		m_rgba8.fill(RGBA8(r / 256.0f, g / 256.0f, b / 256.0f, a / 256.0f));
	}
	else if (m_path == "checker") {
		alloc(GL_RGB8, 256, 256);
		m_rgb8.makeChecker();
	}
	else if (m_path == "rand3") {
		alloc(GL_RGB8, 256, 256);
		m_rgb8.makeRand();
	}
	else if (m_path == "rand4") {
		alloc(GL_RGBA8, 256, 256);
		m_rgba8.makeRand();
	}
	else if (m_path.extension() == ".exr") {
		auto full_path = File::searchPath(m_path, false);  // no abort
		if (full_path.empty()) {
			spu_message(0, "'%s' : no such file\n", m_path.c_str());
			assert(0);  // temporary
		}
		float *pix;
		int32_t width;
		int32_t height;

		const char *err = nullptr;
		auto ret = LoadEXR(&pix, &width, &height, full_path.string().c_str(), &err);
		aux_error(ret != TINYEXR_SUCCESS, "%s\n", err ? err : "tinyexr: something wrong\n");
		if (err) FreeEXRErrorMessage(err);

		alloc(GL_RGBA32F, width, height);
		memcpy(m_rgba32f.pixels().data(), pix, width * height * 4 * sizeof(float));
		free(pix);
	}
	else if (loadFromFile()) {
		/* no post process */
	}
	else {  // tofu image
		alloc(GL_RGB8, c_tofu_size, c_tofu_size);
		m_rgb8.alloc(c_tofu_size, c_tofu_size);
		m_rgb8.fill(RGB8(1, 1, 1, 1));
	}

	switch (m_format) {
	case GL_R8: buildFace(m_r8, m_r8Cube); break;
	case GL_RGB8: buildFace(m_rgb8, m_rgb8Cube); break;
	case GL_RGBA8: buildFace(m_rgba8, m_rgba8Cube); break;
	case GL_RGB16: buildFace(m_rgb16, m_rgb16Cube); break;
	case GL_RGBA16: buildFace(m_rgba16, m_rgba16Cube); break;
	case GL_RGB32F: buildFace(m_rgb32f, m_rgb32fCube); break;
	case GL_RGBA32F: buildFace(m_rgba32f, m_rgba32fCube); break;
	default: aux_error(true, "%s: unknown format\n", opengl_const(m_format));
	}

	auto channel = m_attrs.get("channel", -1);
	if (channel != -1) {
		convert(GL_R8, true);
	}
	else {
		auto iformat = m_attrs.get("iformat", 0);
		convert(iformat, is_full_convert);
	}
}

void Loader::alloc(uint32_t format, int32_t width, int32_t height)
{
	aux_error(
	        m_format != 0 || m_width != 0 || m_height != 0, "%s: duplicated alloc\n",
	        m_path.string().c_str());

	m_format = format;
	m_width = width;
	m_height = height;

	aux_error(
	        m_width <= 0 || m_height <= 0, "%s: bad image size (%dx%d)\n", m_path.string().c_str(), m_width,
	        m_height);

	switch (m_format) {
	case GL_R8: m_r8.alloc(m_width, m_height); return;
	case GL_RGB8: m_rgb8.alloc(m_width, m_height); return;
	case GL_RGBA8: m_rgba8.alloc(m_width, m_height); return;
	case GL_RGB16: m_rgb16.alloc(m_width, m_height); return;
	case GL_RGBA16: m_rgba16.alloc(m_width, m_height); return;
	case GL_RGB32F: m_rgb32f.alloc(m_width, m_height); return;
	case GL_RGBA32F: m_rgba32f.alloc(m_width, m_height); return;
	default: aux_error(true, "unknown format (%s)\n", opengl_const(format));
	}
}

void Loader::convert(uint32_t format, bool is_full_convert)
{
	if (is_full_convert && format && format != m_format) {
		switch (format) {
		case GL_R8: m_pixels = copy(m_r8); break;
		case GL_RGB8: m_pixels = copy(m_rgb8); break;
		case GL_RGBA8: m_pixels = copy(m_rgba8); break;
		case GL_RGB16: m_pixels = copy(m_rgb16); break;
		case GL_RGBA16: m_pixels = copy(m_rgba16); break;
		case GL_RGB32F: m_pixels = copy(m_rgb32f); break;
		case GL_RGBA32F: m_pixels = copy(m_rgba32f); break;
		default: aux_error(true, "convert cannot convert to (%s)\n", opengl_const(format));
		}
		m_format = format;
	}

	auto is_srgb = m_attrs.get("srgb", 0);
	if (is_srgb && m_format == GL_RGB8) m_format = GL_SRGB8;
	if (is_srgb && m_format == GL_RGBA8) m_format = GL_SRGB8_ALPHA8;
}
}  // namespace spu::libspu::image

// #define SPU_NO_FREEIMAGE
#ifdef SPU_NO_FREEIMAGE
#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#include <external/stb/stb_image.h>

namespace spu::libspu::image {

bool Loader::loadFromFile()
{
	auto full_path = File::searchPath(m_path, false);  // no abort
	if (full_path.empty()) {
		spu_message(0, "'%s' : no such file\n", m_path.c_str());
		return false;
	}
	auto bitstream = read_from_file<std::vector<uint8_t>>(full_path);

	std::vector<uint8_t> pixels;
	int32_t width = 0;
	int32_t height = 0;
	int32_t channel = 3;
	int32_t desired_channel = 3;

	uint8_t *pix = stbi_load_from_memory(
	        bitstream.data(), bitstream.size(), &width, &height, &channel, desired_channel);

	std::vector<uint8_t> buf(width * 3);
	;
	for (auto y = 0; y < height / 2; y++) {
		auto *swp = buf.data();
		auto *src = &pix[y * width * 3];
		auto *dst = &pix[(height - y - 1) * width * 3];
		memcpy(swp, dst, width * 3);
		memcpy(dst, src, width * 3);
		memcpy(src, swp, width * 3);
	}

	if (pix == nullptr || channel != 3) {
		spu_message(
		        0, "%s: cannot open by stb. try freeimage instead. (undef 'SPU_NO_FREEIMAGE')\n",
		        full_path.string().c_str());
		return false;
	}
	alloc(GL_RGB8, width, height);
	memcpy(m_rgb8.pixels().data(), pix, width * height * 3);
	stbi_image_free(pix);
	return true;
}
}  // namespace spu::libspu::image
#else

#include <FreeImage.h>
namespace spu::libspu::image {
bool Loader::loadFromFile()
{
	auto full_path = File::searchPath(m_path, false);  // no abort
	if (full_path.empty()) {
		spu_message(0, "'%s' : no such file\n", m_path.c_str());
		return false;
	}

	auto full_path_str = full_path.string();
	const char* full_path_cstr = full_path_str.c_str();

	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(full_path_cstr, 0);

	if (format == FIF_UNKNOWN) {
		format = FreeImage_GetFIFFromFilename(full_path_cstr);
		if (FreeImage_FIFSupportsReading(format) == 0) {
			spu_message(0, "'%s' : cannot read\n", full_path_cstr);
			return false;
		}
	}

	FIBITMAP* bitmap;
	{
		auto size = File(full_path_cstr).size();
		if (size == 0) {
			spu_message(0, "%s: cannot read\n", full_path_cstr);
			return false;
		}

		File file(full_path_cstr, "rb");
		std::vector<uint8_t> buf(size);
		file.read(buf.data(), buf.size());

		FIMEMORY* mp = FreeImage_OpenMemory(buf.data(), buf.size());
		bitmap = FreeImage_LoadFromMemory(format, mp);
		FreeImage_CloseMemory(mp);
	}

	FREE_IMAGE_TYPE type = FreeImage_GetImageType(bitmap);

	int32_t width = FreeImage_GetWidth(bitmap);
	int32_t height = FreeImage_GetHeight(bitmap);
	int32_t bpp = FreeImage_GetBPP(bitmap);
	size_t size = size_t(width) * size_t(height);
	int32_t pitch = FreeImage_GetPitch(bitmap);
	void* pix = FreeImage_GetBits(bitmap);

	aux_error(pix == nullptr, "%s: broken data\n", full_path_cstr);

	switch (type) {
	case FIT_BITMAP: {
		switch (bpp) {
		case 8: {
			alloc(GL_R8, width, height);
			memcpy(m_r8.pixels().data(), pix, size * bpp / 8);
			break;
		}
		case 16: {
			alloc(GL_RGB8, width, height);
			RGB8* rgb = m_rgb8.pixels().data();
			for (auto y = 0; y < height; y++) {
				auto* p = (uint16_t*)pix + size_t(y) * pitch;
				for (auto x = 0; x < width; x++) {
					// assume 16bit BMP
					float b = float((p[x] >> 0) & 0x1f) / 512;
					float g = float((p[x] >> 5) & 0x1f) / 512;
					float r = float((p[x] >> 10) & 0x1f) / 512;
					rgb->setf(r, g, b, 1.0);
					rgb++;
				}
			}
			break;
		}
		case 24: {
			if (width % 4 == 0) {
				alloc(GL_RGB8, width, height);
				RGB8* rgb = m_rgb8.pixels().data();
				for (auto y = 0; y < height; y++) {
					auto* p = (uint8_t*)pix + size_t(y) * pitch;
					for (auto x = 0; x < width; x++) {
						rgb->setc(p[x * 3 + 2], p[x * 3 + 1], p[x * 3 + 0], 255);
						rgb++;
					}
				}
			}
			else {
				alloc(GL_RGBA8, width, height);
				RGBA8* rgba = m_rgba8.pixels().data();
				for (auto y = 0; y < height; y++) {
					auto* p = (uint8_t*)pix + size_t(y) * pitch;
					for (auto x = 0; x < width; x++) {
						rgba->setc(p[x * 3 + 2], p[x * 3 + 1], p[x * 3 + 0], 255);
						rgba++;
					}
				}
			}
			break;
		}
		case 32: {
			alloc(GL_RGBA8, width, height);
			auto* p = (uint8_t*)pix;
			for (auto& rgba: m_rgba8.pixels()) {
				rgba.setc(p[2], p[1], p[0], p[3]);
				p += 4;
			}
			break;
		}

		default: spu_message(0, "'%s' : unsupported bpp (%d)\n", full_path_cstr, bpp); return false;
		}
		break;
	}
	case FIT_RGB16: {
		alloc(GL_RGB16, width, height);
		memcpy(m_rgb16.pixels().data(), pix, size * bpp / 8);
		break;
	}
	case FIT_RGBA16: {
		alloc(GL_RGBA16, width, height);
		memcpy(m_rgba16.pixels().data(), pix, size * bpp / 8);
		break;
	}
	case FIT_RGBF: {
		alloc(GL_RGB32F, width, height);
		memcpy(m_rgb32f.pixels().data(), pix, size * bpp / 8);
		break;
	}
	case FIT_RGBAF: {
		alloc(GL_RGBA32F, width, height);
		memcpy(m_rgba32f.pixels().data(), pix, size * bpp / 8);
		break;
	}
	default: aux_error(true, "%s: unsupported freeimage type (%d)\n", full_path_cstr, type);
	}

	FreeImage_Unload(bitmap);
	return true;
}
}  // namespace spu::libspu::image
#endif  // SPU_FREEIMAGE
