//
//$<<Header>>$
//

#pragma once

#include <smath/vec.h>

namespace spu::oglplus::images {

class Image {
public:
	template<typename T>
	Image(int32_t width, int32_t height, int32_t depth, int32_t channels, const T *data, uint32_t format,
	      uint32_t internal)
	        : m_format(format), m_internal(internal), m_width(width), m_height(height), m_depth(depth),
	          m_channels(channels), m_type(spu::spu_gl_typeof<T>()), m_size(spu::spu_gl_sizeof(m_type)),
	          m_convert(&do_convert<T>)
	{
		m_storage.resize(m_width * m_height * m_depth * m_channels * sizeof(T), 0);
		if (data) {
			memcpy(m_storage.data(), data, m_storage.size());
		}
	}

	template<typename T>
	Image(int32_t width, int32_t height, int32_t depth, int32_t channels, const T *data)
	        : Image(width, height, depth, channels, data, get_def_pdf(channels), get_def_pdif(channels))
	{
	}

	const void *data() const { return m_storage.data(); }
	size_t dataSize() const { return m_storage.size(); }

	int32_t width() const { return m_width; }
	int32_t height() const { return m_height; }
	int32_t depth() const { return m_depth; }
	int32_t channels() const { return m_channels; }
	uint32_t type() const { return m_type; }
	uint32_t format() const { return m_format; }
	uint32_t internalFormat() const { return m_internal; }

	size_t pixelPos(int32_t w, int32_t h, int32_t d) const
	{
		assert(is_initialized());
		assert(w >= 0 && w < width());
		assert(h >= 0 && h < height());
		assert(d >= 0 && d < depth());

		auto ppos = d * height() * width() + h * width() + w;
		return size_t(ppos * channels());
	}

	Vec4f pixel(int32_t w, int32_t h, int32_t d) const
	{
		assert(m_convert);
		auto ppos = pixelPos(w, h, d);
		return Vec4f(
		        m_channels > 0 ? m_convert(get(ppos + 0)) : 0.0,
		        m_channels > 1 ? m_convert(get(ppos + 1)) : 0.0,
		        m_channels > 2 ? m_convert(get(ppos + 2)) : 0.0,
		        m_channels > 3 ? m_convert(get(ppos + 3)) : 0.0);
	}

protected:
	uint32_t m_format;
	uint32_t m_internal;

	Image() = default;

	void bzero() { fill(std::begin(m_storage), std::end(m_storage), 0x00); }
	void *get(int32_t offset) { return &m_storage[offset * m_size]; }
	const void *get(int32_t offset) const { return &m_storage[offset * m_size]; }

	template<typename T> static T one(T) { return std::numeric_limits<T>::max(); }

	static float one(float) { return 1.0f; }
	static double one(double) { return 1.0; }

	template<typename T> bool type_ok() const { return m_type == spu::spu_gl_typeof<T>(); }

	template<typename T = uint8_t> T *begin()
	{
		assert(is_initialized());
		assert(type_ok<T>());
		return static_cast<T *>(get(0));
	}

	template<typename T = uint8_t> T *end()
	{
		assert(is_initialized());
		assert(type_ok<T>());
		return static_cast<T *>(get(dataSize() / m_size));
	}

	template<typename T> T &at(uint32_t x, uint32_t y = 0, uint32_t z = 0)
	{
		return *(T *)get(pixelPos(x, y, z));
	}

private:
	int32_t m_width = 0;
	int32_t m_height = 0;
	int32_t m_depth = 0;
	int32_t m_channels = 0;

	uint32_t m_type = 0;
	uint32_t m_size = 0;

	std::vector<uint8_t> m_storage;

	double (*m_convert)(const void *){nullptr};

	bool is_initialized() const;

	template<typename T> static double do_convert(const void *ptr)
	{
		assert(ptr != nullptr);
		const auto v = double(*(const T *)ptr);
		const auto n = double(one(T(0)));
		return v / n;
	}

	static uint32_t get_def_pdf(uint32_t N);
	static uint32_t get_def_pdif(uint32_t N);
};

}  // namespace spu::oglplus::images

#include <images/image.ipp>
