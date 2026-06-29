//
// Image :
//
#pragma once

#include <smath/vec.h>
#include <spu++/spu++.h>

/// classic DDS
namespace spu::dds {

/// DDS image loader
class Image {
public:
	Image() = default;

	Image(const char *filename, bool is_flip = true) { read(filename, is_flip); }
	~Image() = default;

	uint32_t upload(const Attrs &aux_attrs);
	void read(const char *filename, bool is_flip = true);

	const void *pixels(int32_t level, uint32_t face = GL_TEXTURE_CUBE_MAP_POSITIVE_X) const;
	int32_t size(int32_t level, uint32_t face = GL_TEXTURE_CUBE_MAP_POSITIVE_X) const;

	uint32_t width() const { return m_width; }
	uint32_t height() const { return m_height; }
	uint32_t depth() const { return m_depth; }
	int32_t get(const hash32_t &key, void *value) const;
	bool hasAlpha() const;

private:
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	uint32_t m_depth = 1;
	int32_t m_levels = 1;
	int32_t m_layers = 1;
	uint32_t m_elementSize = 4;
	uint32_t m_iformat = 0;
	uint32_t m_pformat = 0;
	bool m_isCubemap = false;
	uint32_t m_type = 0;
	bool m_isCompressed = false;

	std::vector<std::vector<uint8_t>> m_pixv;  // not pointer array

	void flip(void *ptr, int32_t width, int32_t height, int32_t depth) const;
	void parseDDS(const void *ddsh_ptr, const void *ddsh10_ptr);
	bool parseDX10(const void *ptr);
	bool setFormat(uint32_t fourcc);
};
}  // namespace spu::dds
