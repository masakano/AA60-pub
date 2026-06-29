// #include <config/basic.hpp>

namespace spu::oglplus::imports {

inline BlendFileType BlendFileStructField::baseType() const
{
	auto type_index = m_sdna->m_structs[m_struct_index].field_type_indices[m_field_index];
	return BlendFileType(m_sdna, type_index, m_sdna->m_type_structs[type_index]);
}

inline const std::string &BlendFileStructField::definition() const
{
	return m_sdna->m_names[m_sdna->m_structs[m_struct_index].field_name_indices[m_field_index]];
}

inline std::string BlendFileStructField::name() const { return m_sdna->field_name_from_def(definition()); }

inline bool BlendFileStructField::isPointer() const
{
	bool is_ptr = m_sdna->m_structs[m_struct_index].field_ptr_flags[m_field_index];
	bool is_ptr2 = m_sdna->m_structs[m_struct_index].field_ptr2_flags[m_field_index];
	return is_ptr && !is_ptr2;
}

inline bool BlendFileStructField::isPointerToPointer() const
{
	bool is_ptr = m_sdna->m_structs[m_struct_index].field_ptr_flags[m_field_index];
	bool is_ptr2 = m_sdna->m_structs[m_struct_index].field_ptr2_flags[m_field_index];
	return is_ptr && is_ptr2;
}

inline bool BlendFileStructField::isPointerToFunc() const
{
	bool is_ptr = m_sdna->m_structs[m_struct_index].field_ptr_flags[m_field_index];
	bool is_ptr2 = m_sdna->m_structs[m_struct_index].field_ptr2_flags[m_field_index];
	return !is_ptr && is_ptr2;
}

inline bool BlendFileStructField::isArray() const
{
	return m_sdna->m_structs[m_struct_index].field_array_flags[m_field_index];
}

inline std::size_t BlendFileStructField::elementCount() const
{
	return m_sdna->m_structs[m_struct_index].field_elem_counts[m_field_index];
}

inline std::size_t BlendFileStructField::size() const
{
	bool is_ptr = isPointer() || isPointerToFunc();

	if (isArray()) {
		std::size_t ec = elementCount();
		if (is_ptr) {
			return m_sdna->m_ptr_size * ec;
		}
		{
			return baseType().size() * ec;
		}
	}
	if (is_ptr) {
		return m_sdna->m_ptr_size;
	}
	return baseType().size();
}

inline std::size_t BlendFileStructFieldRange::field_count(BlendFileSDNA *sdna, std::size_t struct_index)
{
	if (struct_index == sdna->invalid_struct_index()) {
		return 0;
	}
	{
		return sdna->m_structs[struct_index].field_count();
	}
}

}  // namespace spu::oglplus::imports
