// #include <config/basic.hpp>

namespace spu::oglplus::imports {

inline bool BlendFileInfo::read_header(BlendFileReader &bfr)
{
	return expect(bfr, "BLENDER", 7, "Failed to read header");
}

inline std::size_t BlendFileInfo::read_pointer_size(BlendFileReader &bfr)
{
	char c = expect_one_of(bfr, "_-", 2, "Failed to read pointer size");
	std::size_t ptr_size = 0;
	if (c == '_') {
		ptr_size = 4;
	}
	if (c == '-') {
		ptr_size = 8;
	}
	adjust_ptr_size(bfr, ptr_size);
	assert(ptr_size);
	return ptr_size;
}

inline Endian BlendFileInfo::read_endianness(BlendFileReader &bfr)
{
	char c = expect_one_of(bfr, "vV", 2, "Failed to read endianness");
	if (c == 'v') {
		return Endian::Little;
	}
	if (c == 'V') {
		// return Endian::Big;
		assert(!"Bigendian not supported");
	}
	assert(!"Logic error!");
	return Endian();
}

inline int32_t BlendFileInfo::read_version(BlendFileReader &bfr)
{
	char buffer[4];
	read(bfr, buffer, 3, "Failed to read version");
	return (buffer[0] - '0') * 100 + (buffer[1] - '0') * 10 + (buffer[2] - '0') * 1;
}

}  // namespace spu::oglplus::imports
