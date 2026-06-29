//
//$<<Header>>$
//

#pragma once
#include <cassert>

namespace spu::oglplus::imports {

enum struct Endian : bool {
	Little = false,
	Big = true,
};

class BlendFileInfo : public BlendFileReaderClient {
private:
	const bool m_header_ok;
	bool read_header(BlendFileReader &bfr);

	const std::size_t m_pointer_size;
	std::size_t read_pointer_size(BlendFileReader &bfr);

	const Endian m_byte_order;
	Endian read_endianness(BlendFileReader &bfr);

	const int32_t m_version;
	int32_t read_version(BlendFileReader &bfr);

public:
	BlendFileInfo(BlendFileReader &bfr)
	        : m_header_ok(read_header(bfr)), m_pointer_size(read_pointer_size(bfr)),
	          m_byte_order(read_endianness(bfr)), m_version(read_version(bfr))
	{
	}

	Endian byteOrder() const { return m_byte_order; }

	std::size_t pointerSize() const { return m_pointer_size; }

	int32_t versionMajor() const { return m_version / 100; }

	int32_t versionMinor() const { return m_version % 100; }
};

}  // namespace spu::oglplus::imports
#include <blender/info.ipp>
