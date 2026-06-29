//
//$<<Header>>$
//

#pragma once

//
// include tree not correct!
//
#include <blender/utils.hpp>
#include <blender/reader_client.hpp>
#include <blender/range.hpp>
#include <blender/info.hpp>
#include <blender/sdna.hpp>
#include <blender/pointer.hpp>
#include <blender/block.hpp>
#include <blender/type.hpp>
#include <blender/structure.hpp>
#include <blender/flattened.hpp>
#include <blender/block_data.hpp>
#include <blender/struct_block_data.hpp>
#include <cstring>

namespace spu::oglplus::imports {

class BlendFileStructGlobBlock : public BlendFileFlatStructBlockData {
private:
	friend class BlendFile;

	BlendFileStructGlobBlock(const BlendFileFlatStructBlockData &tmp)
	        : BlendFileFlatStructBlockData(tmp), m_curscreen(Field<void *>("curscreen")),
	          m_curscene(Field<void *>("curscene"))
	{
	}

public:
	BlendFileFlatStructTypedFieldData<void *> m_curscreen;
	BlendFileFlatStructTypedFieldData<void *> m_curscene;
};

class BlendFile : public BlendFileReaderClient {
	// private:
public:
	BlendFileReader m_reader;
	BlendFileInfo m_info;
	std::vector<BlendFileBlock> m_blocks;
	std::size_t m_glob_block_index;
	std::shared_ptr<BlendFileSDNA> m_sdna;
	std::map<BlendFilePointer::ValueType, std::size_t> m_block_map;

	template<std::size_t N> bool equal(const std::array<char, N> &a, const char *b)
	{
		return std::strncmp(a.data(), b, N) == 0;
	}

public:
	// BlendFile(std::istream &input);
	BlendFile(File &input);

	const BlendFileInfo &info() const { return m_info; }

	BlendFileStructRange structures() const { return BlendFileStructRange(m_sdna.get()); }

	const BlendFileBlock &blockByPointer(BlendFilePointerBase pointer, bool allow_offset = false) const;

	template<uint32_t Level> BlendFilePointerTpl<Level - 1> dereference(BlendFilePointerTpl<Level> pptr)
	{
		auto block = BlockByPointer(pptr, true);
		auto offset = pptr - block.Pointer();
		auto block_data = blockData(block);
		return block_data.template do_get_pointer<Level - 1>(pptr.m_type_index, 0, 0, 0, offset);
	}

	BlendFileFlatStructBlockData structuredBlockByPointer(
	        BlendFilePointer pointer, bool allow_offset = false, bool use_pointee_struct = false);

	BlendFileFlatStructBlockData operator[](BlendFilePointer pointer)
	{
		return structuredBlockByPointer(pointer);
	}

	const BlendFileBlock &globalBlock() const { return m_blocks[m_glob_block_index]; }

	BlendFilePointer globalBlockPointer() const { return globalBlock().pointer(); }

	BlendFileStructGlobBlock structuredGlobalBlock()
	{
		return BlendFileStructGlobBlock(structuredBlockByPointer(globalBlockPointer()));
	}

	BlendFileBlockRange blocks() const { return BlendFileBlockRange(m_blocks); }

	BlendFileStruct blockStructure(const BlendFileBlock &block) const
	{
		return BlendFileStruct(m_sdna.get(), block.m_sdna_index);
	}

	BlendFileBlockData blockData(const BlendFileBlock &block);

	BlendFileType typeByIdx(std::size_t type_index) const;

	template<typename T> BlendFileType type() const { return typeByIdx(m_sdna->find_type_index<T>()); }

	BlendFileType pointee(const BlendFilePointer &pointer) const
	{
		return BlendFileType(
		        m_sdna.get(), pointer.m_type_index, m_sdna->m_type_structs[pointer.m_type_index]);
	}
};

}  // namespace spu::oglplus::imports
#include <blender/blender.ipp>
