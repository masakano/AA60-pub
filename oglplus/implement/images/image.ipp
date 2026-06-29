
namespace spu::oglplus::images {

inline bool Image::is_initialized() const { return (!m_storage.empty()) && (m_convert != nullptr); }

inline uint32_t Image::get_def_pdf(uint32_t n)
{
	if (n == 1) {
		return GL_RED;
	}
	if (n == 2) {
		return GL_RG;
	}
	if (n == 3) {
		return GL_RGB;
	}
	if (n == 4) {
		return GL_RGBA;
	}
	assert(!"Invalid number of color channels!");
	return GL_RED;
}

inline uint32_t Image::get_def_pdif(uint32_t n)
{
	if (n == 1) {
		return GL_RED;
	}
	if (n == 2) {
		return GL_RG;
	}
	if (n == 3) {
		return GL_RGB;
	}
	if (n == 4) {
		return GL_RGBA;
	}
	assert(!"Invalid number of color channels!");
	return GL_RED;
}
}  // namespace spu::oglplus::images
