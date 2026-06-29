//
//$<<Header>>$
//

#pragma once

namespace spu::oglplus::imports {

class BlendFileStruct;

class BlendFileType {
protected:
	BlendFileSDNA *m_sdna;
	const std::size_t m_type_index;
	const std::size_t m_struct_index;

	friend class BlendFile;
	friend class BlendFileBlockData;
	friend class BlendFileStructField;
	friend class BlendFileFlatStructBlockData;

	BlendFileType(BlendFileSDNA *sdna, std::size_t type_index, std::size_t struct_index)
	        : m_sdna(sdna), m_type_index(type_index), m_struct_index(struct_index)
	{
		assert(m_sdna);
	}

public:
	const std::string &name() const { return m_sdna->m_type_names[m_type_index]; }

	std::size_t size() const { return m_sdna->m_type_sizes[m_type_index]; }

	template<typename T> bool isNative() const { return m_sdna->type_matches<T>(m_type_index); }

	bool isStructure() const { return m_struct_index != m_sdna->invalid_struct_index(); }

	BlendFileStruct asStructure() const;
};
}  // namespace spu::oglplus::imports
