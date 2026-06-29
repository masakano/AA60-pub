// #include <config/basic.hpp>

namespace spu::oglplus::imports {

template<uint32_t Level>
BlendFilePointerTpl<Level> BlendFileBlockData::do_make_pointer(const char *pos, std::size_t type_index) const
{
	if (m_ptr_size == 4) {
		return BlendFilePointerTpl<Level>(*reinterpret_cast<const uint32_t *>(pos), type_index);
	}
	if (m_ptr_size == 8) {
		return BlendFilePointerTpl<Level>(*reinterpret_cast<const uint64_t *>(pos), type_index);
	}
	assert(!"Invalid pointer size!");
	return BlendFilePointerTpl<Level>();
}

template<uint32_t Level>
BlendFilePointerTpl<Level> BlendFileBlockData::do_get_pointer(
        std::size_t type_index, std::size_t field_offset, std::size_t block_element, std::size_t field_element,
        std::size_t data_offset) const
{
	const char *pos = m_block_data.data() + data_offset + block_element * m_struct_size
	                + field_element * m_ptr_size + field_offset;
	return do_make_pointer<Level>(pos, type_index);
}

template<uint32_t Level>
BlendFilePointerTpl<Level> BlendFileBlockData::get_pointer(
        const BlendFileFlattenedStructField &flat_field, std::size_t block_element, std::size_t field_element,
        std::size_t data_offset) const
{
	return do_get_pointer<Level>(
	        flat_field.m_sdna
	                ->m_structs[flat_field.m_flat_fields->field_structs[flat_field.m_flat_field_index]]
	                .field_type_indices[flat_field.m_flat_fields
	                                            ->field_indices[flat_field.m_flat_field_index]],
	        flat_field.offset(), block_element, field_element, data_offset);
}

inline BlendFilePointer BlendFileBlockData::asPointerTo(
        const BlendFileType &type, std::size_t index, std::size_t data_offset) const
{
	const char *pos = m_block_data.data() + data_offset + index * m_ptr_size;
	return do_make_pointer<1>(pos, type.m_type_index);
}

inline BlendFilePointer BlendFileBlockData::getPointer(
        const BlendFileFlattenedStructField &flat_field, std::size_t block_element, std::size_t field_element,
        std::size_t data_offset) const
{
	return get_pointer<1>(flat_field, block_element, field_element, data_offset);
}

inline BlendFilePointerToPointer BlendFileBlockData::getPointerToPointer(
        const BlendFileFlattenedStructField &flat_field, std::size_t block_element, std::size_t field_element,
        std::size_t data_offset) const
{
	return get_pointer<2>(flat_field, block_element, field_element, data_offset);
}

inline std::string BlendFileBlockData::getString(
        std::size_t field_size, std::size_t field_offset, std::size_t block_element, std::size_t field_element,
        std::size_t data_offset) const
{
	const char *pos = m_block_data.data() + data_offset + block_element * m_struct_size
	                + field_element * field_size + field_offset;
	return std::string(pos, field_size);
}

inline void BlendFileBlockData::valueVisitRef(
        BlendFileVisitor &visitor, const BlendFileFlattenedStructField &flat_field, std::size_t block_element,
        std::size_t field_element, std::size_t data_offset) const
{
	auto f = flat_field.field();
	if (f.isPointer()) {
		auto bt = f.baseType();
		if (bt.isNative<char>()) {
			visitor.visitStr(getString(flat_field, block_element, field_element, data_offset));
		}
		else {
			visitor.visitPtr(getPointer(flat_field, block_element, field_element, data_offset));
		}
	}
	else if (f.isPointerToPointer()) {
		visitor.visitPPtr(getPointerToPointer(flat_field, block_element, field_element, data_offset));
	}
	else {
		auto bt = f.baseType();
		if (bt.isNative<char>()) {
			if (f.isArray()) {
				visitor.visitStr(
				        getString(flat_field, block_element, field_element, data_offset));
			}
			else {
				visitor.visitChr(
				        getInt<char>(flat_field, block_element, field_element, data_offset));
			}
		}
		else if (bt.isNative<uint8_t>()) {
			visitor.visitU8(getInt<uint8_t>(flat_field, block_element, field_element, data_offset));
		}
		else if (bt.isNative<int8_t>()) {
			visitor.visitI8(getInt<int8_t>(flat_field, block_element, field_element, data_offset));
		}
		else if (bt.isNative<uint16_t>()) {
			visitor.visitU16(
			        getInt<uint16_t>(flat_field, block_element, field_element, data_offset));
		}
		else if (bt.isNative<int16_t>()) {
			visitor.visitI16(
			        getInt<int16_t>(flat_field, block_element, field_element, data_offset));
		}
		else if (bt.isNative<uint32_t>()) {
			visitor.visitU32(
			        getInt<uint32_t>(flat_field, block_element, field_element, data_offset));
		}
		else if (bt.isNative<int32_t>()) {
			visitor.visitI32(
			        getInt<int32_t>(flat_field, block_element, field_element, data_offset));
		}
		else if (bt.isNative<float>()) {
			visitor.visitFlt(
			        getFloat<float>(flat_field, block_element, field_element, data_offset));
		}
		else if (bt.isNative<double>()) {
			visitor.visitDbl(
			        getFloat<double>(flat_field, block_element, field_element, data_offset));
		}
		else {
			visitor.visitRaw(
			        m_block_data.data() + data_offset + block_element * m_struct_size
			                + flat_field.offset(),
			        flat_field.size());
		}
	}
}

}  // namespace spu::oglplus::imports
