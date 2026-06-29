//
//$<<Header>>$
//

#pragma once

namespace spu::oglplus::imports {

class BlendFileFlattenedStructField {
private:
	BlendFileSDNA *m_sdna;
	// std::size_t m_struct_index;
	std::size_t m_flat_field_index;
	const BlendFileSDNA::FlatStructInfo *m_flat_fields;

	BlendFileFlattenedStructField(
	        BlendFileSDNA *sdna, std::size_t /*struct_index*/, std::size_t flat_field_index,
	        const BlendFileSDNA::FlatStructInfo *flat_fields)
	        : m_sdna(sdna)
	          //, m_struct_index(struct_index)
	          ,
	          m_flat_field_index(flat_field_index), m_flat_fields(flat_fields)
	{
	}

	friend class BlendFileBlockData;
	friend class BlendFileFlattenedStruct;
	friend class BlendFileFlattenedStructFieldRange;

public:
	const std::string &name() const;

	BlendFileStruct parent() const;

	BlendFileStructField field() const;

	uint32_t offset() const;

	uint32_t size() const { return field().size(); }
};

class BlendFileFlattenedStructFieldRange
        : public BlendFileRangeTpl<BlendFileFlattenedStructFieldRange, BlendFileFlattenedStructField> {
private:
	BlendFileSDNA *m_sdna;
	std::size_t m_struct_index;
	const BlendFileSDNA::FlatStructInfo *m_flat_fields;

	using Base = BlendFileRangeTpl<BlendFileFlattenedStructFieldRange, BlendFileFlattenedStructField>;

	static std::size_t field_count(const BlendFileSDNA::FlatStructInfo *flat_fields)
	{
		if (flat_fields != nullptr) {
			return flat_fields->field_count();
		}
		{
			return 0;
		}
	}

	BlendFileFlattenedStructFieldRange(
	        BlendFileSDNA *sdna, std::size_t struct_index, const BlendFileSDNA::FlatStructInfo *flat_fields)
	        : Base(field_count(flat_fields)), m_sdna(sdna), m_struct_index(struct_index),
	          m_flat_fields(flat_fields)
	{
	}

	friend class BlendFileFlattenedStruct;

public:
	BlendFileFlattenedStructField get(std::size_t index) const
	{
		assert(m_flat_fields);
		return BlendFileFlattenedStructField(m_sdna, m_struct_index, index, m_flat_fields);
	}
};

class BlendFileFlattenedStruct : public BlendFileType {
public:
	BlendFileFlattenedStruct(const BlendFileType &type) : BlendFileType(type) {}

	BlendFileFlattenedStructFieldRange fields() const;

	BlendFileFlattenedStructField fieldByName(const std::string &name) const;
};

inline BlendFileFlattenedStruct BlendFileStruct::flattened() const { return BlendFileFlattenedStruct(*this); }

}  // namespace spu::oglplus::imports
#include <blender/flattened.ipp>
