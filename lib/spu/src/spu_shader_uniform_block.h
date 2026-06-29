//
// UniformBlock :
//
#pragma once
#include "spu_object.h"

namespace spu::libspu::spu_shader {

class UniformBlock {
public:
	UniformBlock(uint32_t shader_id, int32_t index)
	{
		char name[256];
		int32_t len;

		F(glGetActiveUniformBlockName, shader_id, index, sizeof(name), &len, name);
		m_loc = glGetUniformBlockIndex(shader_id, name);
		assert(m_loc >= 0);
		if (m_loc >= 0) {
			F(glGetActiveUniformBlockiv, shader_id, m_loc, GL_UNIFORM_BLOCK_DATA_SIZE, &m_size);
			F(glGenBuffers, 1, &m_ubo);
			F(glBindBuffer, GL_UNIFORM_BUFFER, m_ubo);
			F(glBufferData, GL_UNIFORM_BUFFER, m_size, nullptr, GL_STATIC_DRAW);
			F(glUniformBlockBinding, shader_id, m_loc, m_loc);
			F(glBindBuffer, GL_UNIFORM_BUFFER, 0);
			m_name = name;
		}
	}

	UniformBlock(UniformBlock &&ub) noexcept
	        : m_name(ub.m_name), m_ubo(ub.m_ubo), m_size(ub.m_size), m_loc(ub.m_loc), m_isLink(ub.m_isLink)
	{
		ub.m_isLink = true;
	}
	UniformBlock(const UniformBlock &ub) = delete;

	~UniformBlock()
	{
		if (!m_isLink && m_ubo) {
			F(glDeleteBuffers, 1, &m_ubo);
		}
	}
	int32_t size() const { return m_size; }
	uint32_t ubo() const { return m_ubo; }
	const uint8_t *cacheData() const { return m_cache.empty() ? nullptr : m_cache.data(); }
	int32_t cacheSize() const { return int32_t(m_cache.size()); }
	void clearCache()
	{
		m_cache.clear();
		m_isDirty = false;
	}
	void use() const { F(glBindBufferRange, GL_UNIFORM_BUFFER, m_loc, m_ubo, 0, m_size); }

	bool set(const char *key, const void *value)
	{
		auto len = m_name.length();
		if (strncmp(key, m_name.c_str(), len) == 0 && key[len] == '.') {
			key += len + 1;

			if (strcmp(key, "buffer_id") == 0) {
				auto new_ubo = *static_cast<const uint32_t *>(value);
				if (!m_isLink && (m_ubo != 0u) && new_ubo != m_ubo) {
					F(glDeleteBuffers, 1, &m_ubo);
				}
				m_ubo = new_ubo;
				m_isLink = true;
				clearCache();
				return true;
			}
			if (strcmp(key, "size") == 0) {
				m_size = *static_cast<const uint32_t *>(value);
				clearCache();
				return true;
			}
			if (strcmp(key, "data") == 0) {
				F(glNamedBufferData, m_ubo, m_size, value, GL_STREAM_DRAW);
				if (value == nullptr) {
					clearCache();
				}
				else {
					m_cache.resize(m_size);
					memcpy(m_cache.data(), value, m_size);
					m_isDirty = false;
				}
				return true;
			}
		}
		return false;
	}

	int32_t get(const char *key, void *value)
	{
		auto len = m_name.length();
		if (strncmp(key, m_name.c_str(), len) == 0 && key[len] == '.') {
			key += len + 1;
			if (strcmp(key, "buffer_id") == 0) {
				*static_cast<uint32_t *>(value) = m_ubo;
				return sizeof(uint32_t);
			}
			if (strcmp(key, "bindless_id") == 0) {
				F(glBindBuffer, GL_UNIFORM_BUFFER, m_ubo);
				F(glGetBufferParameterui64vNV, GL_UNIFORM_BUFFER, GL_BUFFER_GPU_ADDRESS_NV,
				  static_cast<uint64_t *>(value));
				F(glMakeBufferResidentNV, GL_UNIFORM_BUFFER, GL_READ_ONLY);
				F(glBindBuffer, GL_UNIFORM_BUFFER, 0);
				return sizeof(uint32_t);
			}
		}
		return 0;
	}

	void changeUnif(const void *value, int32_t offset, int32_t size)
	{
		aux_error(
		        offset < 0 || size < 0 || offset + size > m_size,
		        "%s: uniform block update out of range (%d + %d > %d)\n", m_name.c_str(), offset, size,
		        m_size);
		if (int32_t(m_cache.size()) != m_size) {
			m_cache.resize(m_size);
			memset(m_cache.data(), 0, m_cache.size());
		}
		if (memcmp(m_cache.data() + offset, value, size) != 0) {
			memcpy(m_cache.data() + offset, value, size);
			m_isDirty = true;
		}
	}

	void flushUnif()
	{
		if (!m_isDirty) {
			return;
		}
		F(glBindBuffer, GL_UNIFORM_BUFFER, m_ubo);
		F(glBufferData, GL_UNIFORM_BUFFER, m_size, m_cache.data(), GL_STATIC_DRAW);
		F(glBindBuffer, GL_UNIFORM_BUFFER, 0);
		m_isDirty = false;
	}

	void setUnif(const void *value)
	{
		changeUnif(value, 0, m_size);
		flushUnif();
	}

	void report() const
	{
		const auto c_max_size = 48;

		F(glBindBuffer, GL_UNIFORM_BUFFER, m_ubo);
		aux_printf("    %s: [%d (dec)]; %8d bytes", m_name.c_str(), m_ubo, m_size);

		auto *ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_ONLY);
		if (ptr == nullptr) {
			aux_printf("\n\t(unmapped)\n");
		}
		else {
			int32_t n;
			for (n = 0; n < std::min(c_max_size, m_size / 4); n++) {
				if (n % 4 == 0) {
					aux_printf("\n\t");
				}

				auto fval = ((float *)ptr)[n];
				auto ival = ((int32_t *)ptr)[n];
				if (std::isnan(fval)) {
					aux_printf("%10s [%08x]", "(nan)", ival);
				}
				else if (fval < -10000) {
					aux_printf("%10s [%08x]", "<-10000", ival);
				}
				else if (fval > 10000) {
					aux_printf("%10s [%08x]", ">10000", ival);
				}
				else {
					aux_printf("%10.3f [%08x]", fval, ival);
				}
			}
			if (n % 4 == 0) {
				aux_printf("\n");
			}
			if (c_max_size < m_size / 4) {
				aux_printf("\t%8s\n", "...");
			}
		}
		F(glUnmapBuffer, GL_UNIFORM_BUFFER);
	}

private:
	friend class UniformBlocks;
	std::string m_name;
	std::vector<uint8_t> m_cache;
	uint32_t m_ubo = 0;
	int32_t m_size = 0;
	int32_t m_loc = 0;
	bool m_isLink = false;
	bool m_isDirty = false;
};

class UniformBlocks final : public std::vector<UniformBlock> {
public:
	void init(uint32_t shader_id)
	{
		int32_t unif_count;
		F(glGetProgramiv, shader_id, GL_ACTIVE_UNIFORM_BLOCKS, &unif_count);

		clear();
		for (auto i = 0; i < unif_count; i++) {
			UniformBlock block(shader_id, i);
			if (block.m_loc >= 0) {
				push_back(std::move(block));
			}
		}
		m_shaderId = shader_id;
	}

	int32_t loc(const char *name) const
	{
		auto name_s = std::string(name);
		auto name_c = name_s;  // capital

		std::transform(std::begin(name_c), std::end(name_c), std::begin(name_c), ::toupper);

		for (const auto &b: *this) {
			if (b.m_name == name_s) {
				return &b - &(*this)[0];
			}
			if (b.m_name == name_s + "[0]") {
				spu_message(
				        2 /*1*/, "%04x changed \"%s\" to \"%s[0]\"\n", m_shaderId, name, name);

				return &b - &(*this)[0];
			}
			if (b.m_name == name_c) {
				spu_message(
				        2 /*1*/, "%04x changed \"%s\" to \"%s\"\n", m_shaderId, name,
				        name_c.c_str());
				return &b - &(*this)[0];
			}
		}
		return -1;
	}
	bool set(const char *key, const void *value)
	{
		for (auto &b: *this) {
			if (b.set(key, value)) {
				return true;
			}
		}
		return false;
	}
	bool get(const char *key, void *value)
	{
		for (auto &b: *this) {
			if (b.get(key, value)) {
				return true;
			}
		}
		return false;
	}
	void use()
	{
		for (auto &b: *this) {
			b.use();
		}
	}
	void clearCache()
	{
		for (auto &b: *this) {
			b.clearCache();
		}
	}
	void report() const
	{
		for (const auto &b: *this) {
			b.report();
		}
	}

private:
	uint32_t m_shaderId;  // debug only
};
}  // namespace spu::libspu::spu_shader
