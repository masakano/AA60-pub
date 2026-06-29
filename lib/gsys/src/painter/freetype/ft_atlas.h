//
// FTAtlas :
//
#pragma once
#include <spu++/spu++.h>

namespace spu::gs_painter::freetype {

class FTAtlas {
public:
	FTAtlas() = default;
	~FTAtlas() = default;

	void init(const Vec4i &size);

	void upload();
	void clear();

	Recti getRegion(const uint32_t width, const uint32_t height);
	void setRegion(const Recti &region, const uint8_t *data, const uint32_t stride);

	void enlargeTexture(uint32_t width_new, uint32_t height_new);

	Vec4i size() const { return m_size; }
	uint32_t used() const { return m_used; }
	const SpuTexture &texture() const { return m_texture; }

private:
	SpuTexture m_texture;         // exture identity (OpenGL)
	std::vector<Vec3f> m_nodes;   // Allocated nodes
	std::vector<uint8_t> m_data;  // Atlas data
	Vec4i m_size;                 // size (in pixels) of the underlying texture
	uint32_t m_used;              // Allocated surface size
	bool m_isModified;            // Atlas has been modified

	int fit(const uint32_t index, const uint32_t width, const uint32_t height);
	void merge();
};

class FTFont;
struct FTMarkup;
class FTFontAtlas : public FTAtlas {
public:
	FTFontAtlas() = default;
	~FTFontAtlas();

	FTFont *getFontFromFile(const char *filename, const float size);
	FTFont *getFontFromMarkup(const FTMarkup &markup);

private:
	std::vector<FTFont *> m_fonts;  // Cached textures.
};
}  // namespace spu::gs_painter::freetype
