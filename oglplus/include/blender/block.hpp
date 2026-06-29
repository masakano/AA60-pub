//
//$<<Header>>$
//

#pragma once

#include <blender/range.hpp>

namespace spu::oglplus::imports {

class BlendFile;

class BlendFileBlock : public BlendFileReaderClient {
private:
	std::array<char, 4> m_code;
	uint32_t m_size;
	uint64_t m_old_ptr;
	uint32_t m_sdna_index;
	uint32_t m_count;
	std::streampos m_data_pos;

	uint32_t read_size(BlendFileReader &bfr, const BlendFileInfo &bfi);
	uint64_t read_old_ptr(BlendFileReader &bfr, const BlendFileInfo &bfi);
	uint32_t read_index(BlendFileReader &bfr, const BlendFileInfo &bfi);
	uint32_t read_count(BlendFileReader &bfr, const BlendFileInfo &bfi);

	friend class BlendFile;

public:
	BlendFileBlock(
	        BlendFileReader &bfr, const BlendFileInfo &bfi, const std::array<char, 4> &code, bool do_skip);

	std::string code() const { return std::string(m_code.data(), m_code.size()); }

	uint32_t size() const { return m_size; }

	uint32_t elementCount() const { return m_count; }

	BlendFilePointer pointer() const { return BlendFilePointer(m_old_ptr, m_sdna_index); }

	std::streampos dataPosition() const { return m_data_pos; }
};

class BlendFileBlockRange : public BlendFileRangeTpl<BlendFileBlockRange, const BlendFileBlock &> {
private:
	const std::vector<BlendFileBlock> &m_blocks;

	using base_t = BlendFileRangeTpl<BlendFileBlockRange, const BlendFileBlock &>;

	BlendFileBlockRange(const std::vector<BlendFileBlock> &blocks) : base_t(blocks.size()), m_blocks(blocks)
	{
	}

	friend class BlendFile;

public:
	const BlendFileBlock &get(std::size_t index) const { return m_blocks[index]; }
};

}  // namespace spu::oglplus::imports
#include <blender/block.ipp>
