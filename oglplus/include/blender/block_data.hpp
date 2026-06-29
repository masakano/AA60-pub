//
//$<<Header>>$
//

#pragma once

#include <blender/visitor.hpp>

namespace spu::oglplus::imports {

class BlendFileBlockData {
private:
	std::vector<char> m_block_data;
	// Endian m_byte_order;
	std::size_t m_ptr_size;
	std::size_t m_struct_size;

	friend class BlendFile;

	BlendFileBlockData(
	        const std::vector<char> &block_data, /*Endian byte_order,*/ std::size_t ptr_size,
	        std::size_t struct_size)
	        : m_block_data(block_data)
	          //, m_byte_order(byte_order)
	          ,
	          m_ptr_size(ptr_size), m_struct_size(struct_size)
	{
	}

	template<uint32_t Level>
	BlendFilePointerTpl<Level> do_make_pointer(const char *pos, std::size_t type_index) const;

	template<uint32_t Level>
	BlendFilePointerTpl<Level> do_get_pointer(
	        std::size_t type_index, std::size_t field_offset, std::size_t block_element,
	        std::size_t field_element, std::size_t data_offset) const;

	template<uint32_t Level>
	BlendFilePointerTpl<Level> get_pointer(
	        const BlendFileFlattenedStructField &flat_field, std::size_t block_element,
	        std::size_t field_element, std::size_t data_offset) const;

public:
	BlendFileBlockData(const BlendFileBlockData &tmp)

	        = default;

	const char *data() const { return m_block_data.data(); }

	char RawByte(std::size_t i) const
	{
		assert(i < m_block_data.size());
		return m_block_data[i];
	}

	std::size_t dataSize() const { return m_block_data.size(); }

	BlendFilePointer asPointerTo(
	        const BlendFileType &type, std::size_t index = 0, std::size_t data_offset = 0) const;

	BlendFilePointer getPointer(
	        const BlendFileFlattenedStructField &flat_field, std::size_t block_element = 0,
	        std::size_t field_element = 0, std::size_t data_offset = 0) const;

	BlendFilePointerToPointer getPointerToPointer(
	        const BlendFileFlattenedStructField &flat_field, std::size_t block_element = 0,
	        std::size_t field_element = 0, std::size_t data_offset = 0) const;

	template<typename Int>
	Int getInt(
	        std::size_t field_offset, std::size_t block_element, std::size_t field_element,
	        std::size_t data_offset) const
	{
		const char *pos = m_block_data.data() + data_offset + block_element * m_struct_size
		                + field_element * sizeof(Int) + field_offset;
		// return aux::reorderToNative(m_byte_order, *reinterpret_cast<const Int *>(pos));
		return *reinterpret_cast<const Int *>(pos);
	}

	template<typename Int>
	Int getInt(
	        const BlendFileFlattenedStructField &flat_field, std::size_t block_element = 0,
	        std::size_t field_element = 0, std::size_t data_offset = 0) const
	{
		assert(sizeof(Int) == flat_field.field().baseType().size());
		return getInt<Int>(flat_field.offset(), block_element, field_element, data_offset);
	}

	template<typename Float>
	Float GetFloat(
	        std::size_t field_offset, std::size_t block_element, std::size_t field_element,
	        std::size_t data_offset) const
	{
		const char *pos = m_block_data.data() + data_offset + block_element * m_struct_size
		                + field_element * sizeof(Float) + field_offset;
		return *reinterpret_cast<const Float *>(pos);
	}

	template<typename Float>
	Float getFloat(
	        const BlendFileFlattenedStructField &flat_field, std::size_t block_element = 0,
	        std::size_t field_element = 0, std::size_t data_offset = 0) const
	{
		assert(sizeof(Float) == flat_field.field().baseType().size());
		return GetFloat<Float>(flat_field.offset(), block_element, field_element, data_offset);
	}

	std::string getString(
	        std::size_t field_size, std::size_t field_offset, std::size_t block_element,
	        std::size_t field_element, std::size_t data_offset) const;

	std::string getString(
	        const BlendFileFlattenedStructField &flat_field, std::size_t block_element = 0,
	        std::size_t field_element = 0, std::size_t data_offset = 0) const
	{
		return getString(
		        flat_field.size(), flat_field.offset(), block_element, field_element, data_offset);
	}

	void valueVisitRef(
	        BlendFileVisitor &visitor, const BlendFileFlattenedStructField &flat_field,
	        std::size_t block_element = 0, std::size_t field_element = 0,
	        std::size_t data_offset = 0) const;

	template<typename Visitor>
	void valueVisit(
	        Visitor visitor, const BlendFileFlattenedStructField &flat_field, std::size_t block_element = 0,
	        std::size_t field_element = 0) const
	{
		valueVisitRef(visitor, flat_field, block_element, field_element);
	}
};

}  // namespace spu::oglplus::imports
#include <blender/block_data.ipp>
#include <utility>
