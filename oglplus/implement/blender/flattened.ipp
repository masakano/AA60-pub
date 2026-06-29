// #include <config/basic.hpp>

namespace spu::oglplus::imports {

inline const std::string &BlendFileFlattenedStructField::name() const
{
	return m_flat_fields->field_names[m_flat_field_index];
}

inline BlendFileStruct BlendFileFlattenedStructField::parent() const
{
	return BlendFileStruct(m_sdna, m_flat_fields->field_structs[m_flat_field_index]);
}

inline BlendFileStructField BlendFileFlattenedStructField::field() const
{
	return BlendFileStructField(
	        m_sdna, m_flat_fields->field_structs[m_flat_field_index],
	        m_flat_fields->field_indices[m_flat_field_index]);
}

inline uint32_t BlendFileFlattenedStructField::offset() const
{
	return m_flat_fields->field_offsets[m_flat_field_index];
}

inline BlendFileFlattenedStructFieldRange BlendFileFlattenedStruct::fields() const
{
	const BlendFileSDNA::FlatStructInfo *flat_fields
	        = (m_struct_index == m_sdna->invalid_struct_index()) ?
	                  static_cast<const BlendFileSDNA::FlatStructInfo *>(nullptr) :
	                  (m_sdna->struct_flatten_fields(m_struct_index).get());

	return BlendFileFlattenedStructFieldRange(m_sdna, m_struct_index, flat_fields);
}

inline BlendFileFlattenedStructField BlendFileFlattenedStruct::fieldByName(const std::string &name) const
{
	if (m_struct_index == m_sdna->invalid_struct_index()) {
		std::string what("Requesting field '");
		what.append(name);
		what.append("' in an atomic type");
		throw std::runtime_error(what);
	}

	const BlendFileSDNA::FlatStructInfo *flat_fields = m_sdna->struct_flatten_fields(m_struct_index).get();
	assert(flat_fields);

	auto pos = flat_fields->field_map.find(&name);

	if (pos == flat_fields->field_map.end()) {
		std::string what("Cannot find field '");
		what.append(name);
		what.append("' in flattened structure");
		throw std::runtime_error(what);
	}

	const std::size_t flat_field_index = pos->second;

	return BlendFileFlattenedStructField(m_sdna, m_struct_index, flat_field_index, flat_fields);
}

}  // namespace spu::oglplus::imports
