// #include <config/basic.hpp>

namespace spu::oglplus::imports {

inline BlendFileBlock::BlendFileBlock(
        BlendFileReader &bfr, const BlendFileInfo &bfi, const std::array<char, 4> &code, bool do_skip)
        : m_code(code), m_size(read_size(bfr, bfi)), m_old_ptr(read_old_ptr(bfr, bfi)),
          m_sdna_index(read_index(bfr, bfi)), m_count(read_count(bfr, bfi)), m_data_pos(position(bfr))
{
	if (do_skip) {
		skip(bfr, m_size, "Error skipping file block data");
	}
}

inline uint32_t BlendFileBlock::read_size(BlendFileReader &bfr, const BlendFileInfo & /*bfi*/)
{
	return read_int<uint32_t>(bfr, "Failed to read file block size");
}

inline uint64_t BlendFileBlock::read_old_ptr(BlendFileReader &bfr, const BlendFileInfo &bfi)
{
	if (bfi.pointerSize() == 4) {
		return read_int<uint32_t>(bfr, "Failed to read file block old pointer");
	}
	if (bfi.pointerSize() == 8) {
		return read_int<uint64_t>(bfr, "Failed to read file block old pointer");
	}

	assert(!"Logic error!");
}

inline uint32_t BlendFileBlock::read_index(BlendFileReader &bfr, const BlendFileInfo & /*bfi*/)
{
	return read_int<uint32_t>(bfr, "Failed to read file block SDNA index");
}

inline uint32_t BlendFileBlock::read_count(BlendFileReader &bfr, const BlendFileInfo & /*bfi*/)
{
	return read_int<uint32_t>(bfr, "Failed to read file block object count");
}

}  // namespace spu::oglplus::imports
