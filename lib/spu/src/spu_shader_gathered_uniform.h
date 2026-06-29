//
// GatheredUniform :
//
#pragma once
#include "spu_shader_uniform_block.h"
#include "spu_shader_gathered_uniform_source.h"

namespace spu::libspu::spu_shader {

class GatheredUniform {
public:
	GatheredUniform() = default;
	GatheredUniform(const GatheredUniformSourceDesc &desc, uint32_t index)
	        : m_name(desc.symbol), m_type(desc.type), m_nelem(desc.nelem), m_offset(desc.offset),
	          m_size(desc.size), m_stride(desc.stride)
	{
		aux_error(m_name.empty(), "gathered uniform[%d] has no symbol\n", index);
		aux_error(
		        m_type == 0u || m_size == 0u, "%s: invalid gathered uniform metadata\n",
		        m_name.c_str());
	}

	const std::string &name() const { return m_name; }
	uint32_t type() const { return m_type; }
	uint32_t byteSize() const { return m_size; }

	void setUnif(UniformBlock &block, const void *value) const
	{
		if (m_stride == 0u || m_nelem <= 1u) {
			changeUnif(block, value, m_offset);
			return;
		}

		auto elem_size = m_size / m_nelem;
		auto *src = static_cast<const uint8_t *>(value);
		for (auto i = 0u; i < m_nelem; i++) {
			changeUnif(block, src + elem_size * i, m_offset + m_stride * i);
		}
	}

	void report() const
	{
		aux_printf(
		        "\t%08x %-30s offset=%d size=%d nelem=%d stride=%d\n", m_type, m_name.c_str(), m_offset,
		        m_size, m_nelem, m_stride);
	}

	void report(const UniformBlock &block) const
	{
		auto name = m_name;
		if (m_nelem > 1u) {
			name += string_printf("[%d]", m_nelem);
		}
		if (m_stride > 0u) {
			aux_printf(
			        "\t%s %s offset=%d size=%d stride=%d\n", opengl_const(m_type), name.c_str(),
			        m_offset, m_size, m_stride);
		}
		else {
			aux_printf(
			        "\t%s %s offset=%d size=%d\n", opengl_const(m_type), name.c_str(), m_offset,
			        m_size);
		}

		ReportInfo info;
		if (!getReportInfo(info)) {
			aux_printf("\t      unknown type (%x:%s)\n", m_type, opengl_const(m_type));
			return;
		}

		auto *cache = block.cacheData();
		if (cache == nullptr) {
			aux_printf("\t      (cache empty)\n");
			return;
		}

		for (auto i = 0u; i < m_nelem; i++) {
			auto offset = m_offset + elemStride() * i;
			if (offset + info.byte_size > uint32_t(block.cacheSize())) {
				aux_printf(
				        "\t      (cache out of range: %d + %d > %d)\n", offset, info.byte_size,
				        block.cacheSize());
				return;
			}
			reportValue(cache + offset, info);
		}
	}

private:
	enum ReportKind {
		e_float,
		e_double,
		e_int,
		e_uint,
	};

	struct ReportInfo {
		ReportKind kind = e_float;
		uint32_t component_count = 0;
		uint32_t column_count = 0;
		uint32_t byte_size = 0;
		uint32_t component_size = 4;
		uint32_t column_stride = 0;
	};

	std::string m_name;
	uint32_t m_type = 0;
	uint32_t m_nelem = 1;
	uint32_t m_offset = 0;
	uint32_t m_size = 0;
	uint32_t m_stride = 0;

	uint32_t elemStride() const { return m_stride > 0u ? m_stride : m_size / m_nelem; }

	void changeUnif(UniformBlock &block, const void *value, uint32_t offset) const
	{
		if (m_type == GL_DOUBLE_MAT2) {
			auto *src = static_cast<const uint8_t *>(value);
			for (auto column = 0u; column < 2u; column++) {
				block.changeUnif(src + 16 * column, offset + 32 * column, 16);
			}
			return;
		}

		if (m_type == GL_DOUBLE_MAT3) {
			auto *src = static_cast<const uint8_t *>(value);
			for (auto column = 0u; column < 3u; column++) {
				block.changeUnif(src + 24 * column, offset + 32 * column, 24);
			}
			return;
		}

		block.changeUnif(value, offset, m_size / m_nelem);
	}

	bool getReportInfo(ReportInfo &info) const
	{
		switch (m_type) {
		case GL_FLOAT_MAT4: info = {e_float, 16, 4, 64, 4, 16}; return true;
		case GL_FLOAT_VEC4: info = {e_float, 4, 4, 16, 4, 0}; return true;
		case GL_FLOAT_VEC3: info = {e_float, 3, 3, 12, 4, 0}; return true;
		case GL_FLOAT_VEC2: info = {e_float, 2, 2, 8, 4, 0}; return true;
		case GL_FLOAT: info = {e_float, 1, 1, 4, 4, 0}; return true;
		case GL_DOUBLE_MAT4: info = {e_double, 16, 4, 128, 8, 32}; return true;
		case GL_DOUBLE_MAT3: info = {e_double, 9, 3, 96, 8, 32}; return true;
		case GL_DOUBLE_MAT2: info = {e_double, 4, 2, 64, 8, 32}; return true;
		case GL_DOUBLE_VEC4: info = {e_double, 4, 4, 32, 8, 0}; return true;
		case GL_DOUBLE_VEC3: info = {e_double, 3, 3, 24, 8, 0}; return true;
		case GL_DOUBLE_VEC2: info = {e_double, 2, 2, 16, 8, 0}; return true;
		case GL_INT_VEC4: info = {e_int, 4, 4, 16, 4, 0}; return true;
		case GL_INT_VEC3: info = {e_int, 3, 3, 12, 4, 0}; return true;
		case GL_INT_VEC2: info = {e_int, 2, 2, 8, 4, 0}; return true;
		case GL_INT: info = {e_int, 1, 1, 4, 4, 0}; return true;
		case GL_UNSIGNED_INT_VEC4: info = {e_uint, 4, 4, 16, 4, 0}; return true;
		case GL_UNSIGNED_INT_VEC3: info = {e_uint, 3, 3, 12, 4, 0}; return true;
		case GL_UNSIGNED_INT_VEC2: info = {e_uint, 2, 2, 8, 4, 0}; return true;
		case GL_UNSIGNED_INT: info = {e_uint, 1, 1, 4, 4, 0}; return true;
		case GL_BOOL: info = {e_uint, 1, 1, 4, 4, 0}; return true;
		default: return false;
		}
	}

	void reportValue(const uint8_t *data, const ReportInfo &info) const
	{
		for (auto x = 0u; x < info.component_count; x++) {
			if (x % info.column_count == 0u) {
				aux_printf("\t");
			}

			auto column = x / info.column_count;
			auto row = x % info.column_count;
			auto offset = info.column_stride > 0u ?
			                      info.column_stride * column + info.component_size * row :
			                      info.component_size * x;
			auto *ptr = data + offset;
			if (info.kind == e_float) {
				aux_printf("%8.3f ", *reinterpret_cast<const float *>(ptr));
			}
			else if (info.kind == e_double) {
				aux_printf("%8.3f ", *reinterpret_cast<const double *>(ptr));
			}
			else if (info.kind == e_int) {
				aux_printf("%8d ", *reinterpret_cast<const int32_t *>(ptr));
			}
			else {
				aux_printf("%8u ", *reinterpret_cast<const uint32_t *>(ptr));
			}
			if (x % info.column_count == info.column_count - 1u) {
				aux_printf("\n");
			}
		}
	}
};

class GatheredUniforms final : public std::vector<GatheredUniform> {
public:
	void init(const GatheredUniformSourceResult &result, const UniformBlocks &blocks)
	{
		clear();
		m_blockId = -1;
		if (result.empty()) {
			return;
		}

		auto block_name = result.block_symbol.c_str();
		m_blockId = blocks.loc(block_name);
		aux_error(m_blockId < 0, "gathered uniform block '%s' not found\n", block_name);

		auto expected_size = result.block_size;
		auto actual_size = uint32_t(blocks[m_blockId].size());
		aux_error(
		        expected_size > actual_size, "gathered uniform block '%s' size mismatch (%d > %d)\n",
		        block_name, expected_size, actual_size);

		reserve(result.uniforms.size());
		for (auto i = 0u; i < result.uniforms.size(); i++) {
			emplace_back(result.uniforms[i], i);
		}
	}

	int32_t loc(const char *name) const
	{
		for (const auto &u: *this) {
			if (u.name() == name) {
				return &u - &(*this)[0];
			}
		}
		return -1;
	}

	int32_t blockId() const { return m_blockId; }

	void setUnif(int32_t id, UniformBlocks &blocks, const void *value) const
	{
		aux_error(m_blockId < 0, "gathered uniform block is not initialized\n");
		aux_error(id < 0 || id >= int32_t(size()), "gathered uniform id (%d) out of range\n", id);
		(*this)[id].setUnif(blocks[m_blockId], value);
	}

	void flushUnif(UniformBlocks &blocks) const
	{
		if (m_blockId >= 0) {
			blocks[m_blockId].flushUnif();
		}
	}

	void report(const UniformBlocks &blocks) const
	{
		if (m_blockId < 0) {
			return;
		}

		const auto &block = blocks[m_blockId];
		for (const auto &u: *this) {
			u.report(block);
		}
	}

private:
	int32_t m_blockId = -1;
};
}  // namespace spu::libspu::spu_shader
