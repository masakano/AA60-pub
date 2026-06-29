//
// Uniform :
//
#pragma once

#include "spu_object.h"

namespace spu {
extern bool spu_texture_use(uint32_t texture_id, int32_t slot);
}  // namespace spu

namespace spu::libspu::spu_shader {

#define setMat(TYPE, DIM, FUNC, VALUE)                                                  \
	if (!equal(VALUE, sizeof(TYPE) * m_elemSize * DIM * DIM)) {                     \
		F(FUNC, m_loc, m_elemSize, GL_FALSE, static_cast<const TYPE *>(VALUE)); \
	}

#define setVec(TYPE, DIM, FUNC, VALUE)                                        \
	if (!equal(VALUE, sizeof(TYPE) * m_elemSize * DIM)) {                 \
		F(FUNC, m_loc, m_elemSize, static_cast<const TYPE *>(VALUE)); \
	}

class Uniform {
private:
	std::string m_name;
	std::vector<uint8_t> m_cache;

	uint32_t m_type;
	int32_t m_elemSize;
	int32_t m_loc;
	int32_t m_textureSlot;
	int32_t m_imageSlot;
	int32_t m_imageFormat;
	Attrs m_extra;

	friend class Uniforms;

public:
	Uniform(uint32_t shader_id, int32_t index, int32_t texture_slot, int32_t image_slot)
	        : m_textureSlot(texture_slot), m_imageSlot(image_slot), m_imageFormat(0)
	{
		constexpr int32_t c_maxlen = 256;
		char name[c_maxlen];
		int32_t len;

		F(glGetActiveUniform, shader_id, index, sizeof(name), &len, &m_elemSize, &m_type, name);
		assert(len < c_maxlen - 1);

		m_loc = glGetUniformLocation(shader_id, name);
		if (m_loc >= 0) {
			char *sqbr;
			if ((sqbr = strchr(name, '['))) {
				*sqbr = 0;  // suppress '[0]'
			}
			m_name = name;
		}
	}

	int32_t byteSize() const { return spu_gl_sizeof(m_type) * m_elemSize; }

	int32_t type() const { return m_type; }

	void clear() { m_cache.clear(); }

	bool set(const char *key, const void *value)
	{
		auto len = m_name.length();
		if (strncmp(key, m_name.c_str(), len) == 0 && key[len] == '.') {
			key += len + 1;
			Attr attr(key, *((const uint32_t *)value));
			if ((int32_t)m_extra.replace(attr) == 0) {
				m_extra.push_back(attr);
			}
			return true;
		}
		return false;
	}

	int32_t get(const char *key, void *value)
	{
		auto len = m_name.length();
		if (strncmp(key, m_name.c_str(), len) == 0 && key[len] == '.') {
			key += len + 1;
			uint32_t extra = m_extra.get(key, ~0u);
			if (extra != ~0u) {
				*((uint32_t *)value) = extra;
				return sizeof(uint32_t);
				;
			}
		}
		return 0;
	}

	void setUnif(const void *value)
	{
		if (spu_gl_is_texture(m_type)) {
			setTexture(value);
			return;
		}
		if (spu_gl_is_image(m_type)) {
			setImage(value);
			return;
		}

		switch (m_type) {
		case GL_FLOAT_MAT4: setMat(float, 4, glUniformMatrix4fv, value); return;
		case GL_FLOAT_MAT3: setMat(float, 3, glUniformMatrix3fv, value); return;
		case GL_FLOAT_MAT2: setMat(float, 2, glUniformMatrix2fv, value); return;
		case GL_FLOAT_VEC4: setVec(float, 4, glUniform4fv, value); return;
		case GL_FLOAT_VEC3: setVec(float, 3, glUniform3fv, value); return;
		case GL_FLOAT_VEC2: setVec(float, 2, glUniform2fv, value); return;
		case GL_FLOAT: setVec(float, 1, glUniform1fv, value); return;
		case GL_INT_VEC4: setVec(int, 4, glUniform4iv, value); return;
		case GL_INT_VEC3: setVec(int, 3, glUniform3iv, value); return;
		case GL_INT_VEC2: setVec(int, 2, glUniform2iv, value); return;
		case GL_INT: setVec(int, 1, glUniform1iv, value); return;
		case GL_BOOL_VEC4: setVec(int, 4, glUniform4iv, value); return;
		case GL_BOOL_VEC3: setVec(int, 3, glUniform3iv, value); return;
		case GL_BOOL_VEC2: setVec(int, 2, glUniform2iv, value); return;
		case GL_BOOL: setVec(int, 1, glUniform1iv, value); return;
		case GL_UNSIGNED_INT: setVec(uint32_t, 1, glUniform1uiv, value); return;
		case GL_DOUBLE_MAT4: setMat(double, 4, glUniformMatrix4dv, value); return;
		case GL_DOUBLE_MAT3: setMat(double, 3, glUniformMatrix3dv, value); return;
		case GL_DOUBLE_MAT2: setMat(double, 2, glUniformMatrix2dv, value); return;
		case GL_DOUBLE_VEC4: setVec(double, 4, glUniform4dv, value); return;
		case GL_DOUBLE_VEC3: setVec(double, 3, glUniform3dv, value); return;
		case GL_DOUBLE_VEC2: setVec(double, 2, glUniform2dv, value); return;
		case GL_DOUBLE: setVec(double, 1, glUniform1dv, value); return;
		case GL_GPU_ADDRESS_NV: setVec(GLuint64EXT, 1, glUniformui64vNV, value); return;
		}

		aux_error(
		        true, "spu_shader.set_unif: %s: unknown type (%x:%s)\n", m_name.c_str(), m_type,
		        opengl_const(m_type));
	}

	void report() const
	{
		aux_printf("\t%2d %-30s %s", m_loc, opengl_const(m_type), m_name.c_str());
		if (m_elemSize > 1) {
			aux_printf("[%d]", m_elemSize);
		}
		aux_printf("\n");
		for (const auto &attr: m_extra) {
			aux_printf("\t   %s=%s\n", attr.key().c_str(), attr.toString().c_str());
		}
	}

	void reportDetail(int32_t shader_id, int32_t idx) const
	{
		auto loci = m_loc + idx;
		auto nx = 0;
		auto is_double = false;
		auto is_float = false;
		auto is_long = false;

		float fv[16];
		double dv[16];
		int32_t iv[16];
		uint64_t lv[16];

		memset(fv, 0, sizeof(fv));
		memset(dv, 0, sizeof(dv));
		memset(iv, 0, sizeof(iv));
		memset(lv, 0, sizeof(lv));

		switch (m_type) {
		case GL_FLOAT_MAT4:
			nx = 16;
			is_float = true;
			F(glGetUniformfv, shader_id, loci, fv);
			break;
		case GL_FLOAT_MAT3:
			nx = 9;
			is_float = true;
			F(glGetUniformfv, shader_id, loci, fv);
			break;
		case GL_FLOAT_MAT2:
			nx = 4;
			is_float = true;
			F(glGetUniformfv, shader_id, loci, fv);
			break;
		case GL_FLOAT_VEC4:
			nx = 4;
			is_float = true;
			F(glGetUniformfv, shader_id, loci, fv);
			break;
		case GL_FLOAT_VEC3:
			nx = 3;
			is_float = true;
			F(glGetUniformfv, shader_id, loci, fv);
			break;
		case GL_FLOAT_VEC2:
			nx = 2;
			is_float = true;
			F(glGetUniformfv, shader_id, loci, fv);
			break;
		case GL_FLOAT:
			nx = 1;
			is_float = true;
			F(glGetUniformfv, shader_id, loci, fv);
			break;
		case GL_DOUBLE_MAT4:
			nx = 32;
			is_double = true;
			F(glGetUniformdv, shader_id, loci, dv);
			break;
		case GL_DOUBLE_MAT3:
			nx = 18;
			is_double = true;
			F(glGetUniformdv, shader_id, loci, dv);
			break;
		case GL_DOUBLE_MAT2:
			nx = 8;
			is_double = true;
			F(glGetUniformdv, shader_id, loci, dv);
			break;
		case GL_DOUBLE_VEC4:
			nx = 8;
			is_double = true;
			F(glGetUniformdv, shader_id, loci, dv);
			break;
		case GL_DOUBLE_VEC3:
			nx = 6;
			is_double = true;
			F(glGetUniformdv, shader_id, loci, dv);
			break;
		case GL_DOUBLE_VEC2:
			nx = 4;
			is_double = true;
			F(glGetUniformdv, shader_id, loci, dv);
			break;
		case GL_DOUBLE:
			nx = 2;
			is_double = true;
			F(glGetUniformdv, shader_id, loci, dv);
			break;
		case GL_BOOL_VEC4:
			nx = 4;
			F(glGetUniformiv, shader_id, loci, iv);
			break;
		case GL_BOOL_VEC3:
			nx = 3;
			F(glGetUniformiv, shader_id, loci, iv);
			break;
		case GL_BOOL_VEC2:
			nx = 2;
			F(glGetUniformiv, shader_id, loci, iv);
			break;
		case GL_BOOL:
			nx = 1;
			F(glGetUniformiv, shader_id, loci, iv);
			break;
		case GL_INT_VEC4:
			nx = 4;
			F(glGetUniformiv, shader_id, loci, iv);
			break;
		case GL_INT_VEC3:
			nx = 3;
			F(glGetUniformiv, shader_id, loci, iv);
			break;
		case GL_INT_VEC2:
			nx = 2;
			F(glGetUniformiv, shader_id, loci, iv);
			break;
		case GL_INT:
			nx = 1;
			F(glGetUniformiv, shader_id, loci, iv);
			break;

		case GL_UNSIGNED_INT: {
			auto *gl_iv = reinterpret_cast<uint32_t *>(iv);
			nx = 1;
			F(glGetUniformuiv, shader_id, loci, gl_iv);
			break;
		}
		case GL_GPU_ADDRESS_NV:
			nx = 1;
			is_long = true;
			F(glGetUniformui64vNV, shader_id, loci, lv);
			break;
		default:
			if (spu_gl_is_texture(m_type) || spu_gl_is_image(m_type)) {
				nx = 1;
				F(glGetUniformiv, shader_id, loci, iv);
				break;
			}
			aux_printf("unknown type (%x:%s)\n", m_type, opengl_const(m_type));
		}

		aux_printf(
		        "\t%2d %-23s ", loci,
		        m_elemSize == 1 ? m_name.c_str() :
		                          string_printf("%s[%d]", m_name.c_str(), idx).c_str());

		for (auto x = 0; x < nx; x++) {
			if (nx > 1 && x % 4 == 0) {
				aux_printf("\n\t");
			}
			if (is_double) {
				aux_printf("%8.3f ", dv[x]);
			}
			else if (is_float) {
				aux_printf("%8.3f ", fv[x]);
			}
			else if (is_long) {
				aux_printf("%08lx ", lv[x]);
			}
			else {
				aux_printf("%8d ", iv[x]);
			}
		}
		aux_printf("\n");
	}

private:
	bool equal(const void *value, uint32_t byte_size)
	{
		if (m_cache.size() != byte_size) {
			m_cache.resize(byte_size);
			memcpy(m_cache.data(), value, byte_size);
			return false;
		}
		if (memcmp(m_cache.data(), value, byte_size) != 0) {
			memcpy(m_cache.data(), value, byte_size);
			return false;
		}
		return true;
	}

	void setTexture(const void *value)
	{
		uint32_t base_slot = m_textureSlot;
		std::vector<int32_t> texture_slot(m_elemSize);
		for (auto i = 0; i < m_elemSize; i++) {
			uint32_t texture_id = ((const uint32_t *)value)[i];
			texture_slot[i] = base_slot + i;

			if (texture_id && spu_texture_use(texture_id, texture_slot[i]) == 0u) {
				spu_message(
				        0 /*1*/, "%s: suspicious texture_id (%08x)\n", m_name.c_str(),
				        texture_id);
			}
		}
		setVec(int, 1, glUniform1iv, texture_slot.data());
	}

	// should marge into spu_texture
	void setImage(const void *value)
	{
		bool layered = GL_FALSE;
		int32_t level = 0;
		int32_t layer = 0;
		uint32_t base_slot = m_imageSlot;
		uint32_t access = GL_READ_WRITE;

		std::vector<int32_t> image_slot(m_elemSize);

		if (!m_extra.empty()) {
			base_slot = m_extra.get("binding", base_slot);
			level = m_extra.get("level", level);
			layered = m_extra.get("layered", layered);
			layer = m_extra.get("layer", layer);
			access = m_extra.get("access", access);
			m_imageFormat = m_extra.get("format", m_imageFormat);

			if (layer == -1) {  // use "all" layers
				layered = GL_TRUE;
				layer = 0;
			}
		}

		for (auto i = 0; i < m_elemSize; i++) {
			auto texture = ((const Handle *)value)[i];
			image_slot[i] = base_slot + i;

			// this destrys current binding.
			if (texture.id) {
				if (m_imageFormat == 0) {
					if (texture.target == GL_TEXTURE_CUBE_MAP) {
						texture.target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
					}

					F(glBindTexture, texture.target, texture.id);
					F(glGetTexLevelParameteriv, texture.target, 0,
					  GL_TEXTURE_INTERNAL_FORMAT, &m_imageFormat);

					// printf("imageFormat=%s\n", opengl_const(m_imageFormat));

					spu_message(
					        1, "%s (%s:%d): set to ", m_name.c_str(),
					        opengl_const(texture.target), texture.id);
					spu_message(
					        1, "[%s] (valid at the next time)\n",
					        opengl_const(m_imageFormat));
				}
				F(glBindImageTexture, image_slot[i], texture.id, level, layered, layer, access,
				  m_imageFormat);
			}
// #define MAINTENANCE
#ifdef MAINTENANCE
			aux_printf("bindImageTexture:\n");
			aux_printf("    slot    : %d\n", image_slot[i]);
			aux_printf("    id      : %d\n", texture.id);
			aux_printf("    level   : %d\n", level);
			aux_printf("    layered : %d\n", layered);
			aux_printf("    layer   : %d\n", layer);
			aux_printf("    access  : %s\n", opengl_const(access));
			aux_printf("    format  : %s\n", opengl_const(m_imageFormat));
#endif
		}
		F(glUniform1iv, m_loc, image_slot.size(), image_slot.data());
	}
};

class Uniforms final : public std::vector<Uniform> {
public:
	void init(uint32_t shader_id)
	{
		auto unif_count = 0;
		auto texture_slot = 0;
		auto image_slot = 0;

		F(glGetProgramiv, shader_id, GL_ACTIVE_UNIFORMS, &unif_count);

		clear();
		for (auto i = 0; i < unif_count; i++) {
			Uniform unif(shader_id, i, texture_slot, image_slot);
			if (unif.m_loc >= 0) {
				push_back(unif);
				if (spu_gl_is_texture(unif.m_type)) {
					texture_slot += unif.m_elemSize;
				}
				if (spu_gl_is_image(unif.m_type)) {
					image_slot += unif.m_elemSize;
				}
			}
		}
	}

	int32_t getLoc(const char *name) const
	{
		for (const auto &u: *this) {
			if (u.m_name == name) {
				return &u - &(*this)[0];
			}
		}
		return -1;
	}

	bool set(const char *key, const void *value)
	{
		for (auto &unif: *this) {
			if (unif.set(key, value)) {
				return true;
			}
		}
		return false;
	}

	bool get(const char *key, void *value)
	{
		for (auto &unif: *this) {
			if (unif.get(key, value)) {
				return true;
			}
		}
		return false;
	}

	void clear()
	{
		for (auto &u: *this) {
			u.clear();
		}
	}

	void report() const
	{
		for (const auto &u: *this) {
			u.report();
		}
	}

	void reportDetail(int32_t shader_id) const
	{
		for (const auto &u: *this) {
			for (auto index = 0; index < u.m_elemSize; index++) {
				u.reportDetail(shader_id, index);
			}
		}
	}
};
}  // namespace spu::libspu::spu_shader
