// #include <config/basic.hpp>

namespace spu::oglplus::imports {

inline BlendFile::BlendFile(File &input)
        : m_reader(input), m_info(m_reader), m_glob_block_index(std::size_t(-1))
{
	std::size_t block_idx = 0;
	while (!eof(m_reader)) {
		std::array<char, 4> code = read_array<4>(m_reader, "Failed to read file block code");

		if (equal(code, "GLOB")) {
			m_glob_block_index = block_idx;
		}

		if (equal(code, "DNA1")) {
			m_blocks.emplace_back(m_reader, m_info, code, false);
			m_sdna = std::make_shared<BlendFileSDNA>(m_reader, m_info);
		}
		else {
			m_blocks.emplace_back(m_reader, m_info, code, true);
		}
		m_block_map[m_blocks.back().m_old_ptr] = block_idx++;
	}
	if (m_glob_block_index == std::size_t(-1)) {
		throw std::runtime_error("Blend file does not contain GLOB block");
	}
	if (!m_sdna) {
		throw std::runtime_error("Blend file does not contain SDNA block");
	}
}

inline const BlendFileBlock &BlendFile::blockByPointer(BlendFilePointerBase pointer, bool allow_offset) const
{
	auto ptr = pointer.value();
	auto pos = m_block_map.find(ptr);
	if (allow_offset && (pos == m_block_map.end())) {
		auto pos2 = m_block_map.lower_bound(ptr);
		if (pos2 != m_block_map.end()) {
			if (pos2 != m_block_map.begin()) {
				--pos2;
				assert(pos2->first < ptr);
				assert(pos2->second < m_blocks.size());
				std::size_t size = m_blocks[pos2->second].size();
				if (ptr - pos2->first < size) {
					pos = pos2;
				}
			}
		}
	}
	if (pos == m_block_map.end()) {
		throw std::runtime_error("Unable to find block by pointer");
	}
	assert(pos->second < m_blocks.size());

	return m_blocks[pos->second];
}

inline BlendFileFlatStructBlockData BlendFile::structuredBlockByPointer(
        BlendFilePointer pointer, bool allow_offset, bool use_pointee_struct)
{
	auto block = blockByPointer(pointer, allow_offset);
	auto offset = static_cast<int>(pointer) - static_cast<int>(block.pointer());
	auto block_data = blockData(block);
	auto flat_struct = (use_pointee_struct) ? pointee(pointer).asStructure().flattened() :
	                                          blockStructure(block).flattened();

	return BlendFileFlatStructBlockData(flat_struct, block, block_data, offset);
}

inline BlendFileBlockData BlendFile::blockData(const BlendFileBlock &block)
{
	std::vector<char> data;
	if (block.size() != 0u) {
		data.resize(block.size());
		go_to(m_reader, block.dataPosition());
		raw_read(m_reader, data.data(), data.size(), "Failed to read blend file block data");
	}
	return BlendFileBlockData(
	        data, m_info.pointerSize(),
	        m_sdna->m_type_sizes[m_sdna->m_structs[block.m_sdna_index].type_index]);
}

inline BlendFileType BlendFile::typeByIdx(std::size_t type_index) const
{
	if (type_index == m_sdna->invalid_type_index()) {
		throw std::runtime_error("Unknown blender type");
	}
	return BlendFileType(m_sdna.get(), type_index, m_sdna->m_type_structs[type_index]);
}

}  // namespace spu::oglplus::imports
