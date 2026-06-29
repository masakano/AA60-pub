//
//$<<Header>>$
//

#pragma once

namespace spu::oglplus::imports {

class BlendFileFlattenedStruct;

class BlendFileStructField {
private:
	BlendFileSDNA *m_sdna;
	std::size_t m_struct_index;
	std::size_t m_field_index;

	friend class BlendFileStructFieldRange;
	friend class BlendFileFlattenedStructField;

	BlendFileStructField(BlendFileSDNA *sdna, std::size_t struct_index, std::size_t field_index)
	        : m_sdna(sdna), m_struct_index(struct_index), m_field_index(field_index)
	{
		assert(m_sdna);
	}

public:
	BlendFileType baseType() const;

	const std::string &definition() const;

	std::string name() const;

	bool isPointer() const;

	bool isPointerToPointer() const;

	bool isPointerToFunc() const;

	bool isArray() const;

	std::size_t elementCount() const;

	std::size_t size() const;
};

class BlendFileStructFieldRange : public BlendFileRangeTpl<BlendFileStructFieldRange, BlendFileStructField> {
private:
	BlendFileSDNA *m_sdna;
	std::size_t m_struct_index;

	static std::size_t field_count(BlendFileSDNA *sdna, std::size_t struct_index);

	using Base = BlendFileRangeTpl<BlendFileStructFieldRange, BlendFileStructField>;

	friend class BlendFileStruct;

	BlendFileStructFieldRange(BlendFileSDNA *sdna, std::size_t struct_index)
	        : Base(field_count(sdna, struct_index)), m_sdna(sdna), m_struct_index(struct_index)
	{
	}

public:
	BlendFileStructField get(std::size_t index) const
	{
		return BlendFileStructField(m_sdna, m_struct_index, index);
	}
};

class BlendFileStruct : public BlendFileType {
private:
	friend class BlendFile;
	friend class BlendFileStructRange;
	friend class BlendFileStructField;
	friend class BlendFileFlattenedStructField;

	BlendFileStruct(BlendFileSDNA *sdna, std::size_t struct_index)
	        : BlendFileType(sdna, sdna->m_structs[struct_index].type_index, struct_index)
	{
	}

public:
	BlendFileStruct(const BlendFileType &type) : BlendFileType(type) {}

	BlendFileStructFieldRange fields() const { return BlendFileStructFieldRange(m_sdna, m_struct_index); }

	BlendFileFlattenedStruct flattened() const;
};

inline BlendFileStruct BlendFileType::asStructure() const { return BlendFileStruct(*this); }

class BlendFileStructRange : public BlendFileRangeTpl<BlendFileStructRange, BlendFileStruct> {
private:
	BlendFileSDNA *m_sdna;

	using Base = BlendFileRangeTpl<BlendFileStructRange, BlendFileStruct>;

	friend class BlendFile;

	BlendFileStructRange(BlendFileSDNA *sdna) : Base(sdna->m_structs.size()), m_sdna(sdna) {}

public:
	BlendFileStruct get(std::size_t index) const { return BlendFileStruct(m_sdna, index); }
};

}  // namespace spu::oglplus::imports
#include <blender/structure.ipp>
