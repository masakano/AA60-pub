//
// FFMpegEncoder :
//
#pragma once

#include <spu/spu.h>
// #include <future>

#ifdef _MSC_VER
#define unlink _unlink
#define popen _popen
#define pclose _pclose
#endif

namespace spu::libspu::video {

class FFMpegEncoder {
public:
	static constexpr const char *c_com
	        = "ffmpeg -loglevel warning -hide_banner -nostats -y -f rawvideo -video_size %dx%d -framerate %d -pixel_format rgba -i pipe:0 -vf vflip %s";

	FFMpegEncoder(const char *path, uint32_t texture_id) : m_path(path), m_textureId(texture_id)
	{
		uint32_t width;
		uint32_t height;
		uint32_t framerate = 60;

		spu_texture_get(m_textureId, "width", &width);
		spu_texture_get(m_textureId, "height", &height);

		const auto com = string_printf(c_com, width, height, framerate, m_path.c_str());

		m_fp = popen(com.data(), "w");
		m_pixels.resize(size_t(width) * size_t(height));
	}

	void encode()
	{
		spu_texture_recv(m_textureId, m_pixels.data(), GL_RGBA8);
		fwrite(m_pixels.data(), sizeof(uint32_t), m_pixels.size(), m_fp);
	};

	~FFMpegEncoder() { pclose(m_fp); }

private:
	FILE *m_fp = nullptr;
	std::string m_path;
	std::vector<uint32_t> m_pixels;
	uint32_t m_textureId;
};
}  // namespace spu::libspu::video
