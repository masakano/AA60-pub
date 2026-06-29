//
// SpuShader :
//
#include <spu++/spu_shader.h>

namespace spu {
using base_t = spu_object::SpuObject<spu_object::ShaderBody>;

void SpuShader::dispose()
{
	base_t::dispose();
	clearUniforms();
}

void SpuShader::reset(uint32_t shader_id, bool is_owner)
{
	base_t::reset(shader_id, is_owner);
	clearUniforms();
}

void SpuShader::set(const Attrs &attrs)
{
	spu_shader_set(id(), attrs);
	setUniforms(attrs);
}

int32_t SpuShader::get(const hash32_t &key, void *value) const
{
	int32_t ret = 0;
	if ((ret = spu_inventory_get(id(), key, value))) {
		return ret;
	}
	return spu_shader_get(id(), key, value);
}

bool SpuShader::addUniform(const char *name, void *ptr)
{
	if (id() == 0) {
		return false;
	}
	for (auto i = 0u; i < m_names.size(); i++) {
		if (m_names[i] == name) {
			if (m_ptrs[i] == ptr) {
				return false;
			}
		}
	}
	int32_t loc;
	uint32_t size;
	uint32_t type;
	spu_shader_loc(id(), &name, &loc, &size, &type, 1);
	if (loc >= 0) {
		m_names.push_back(name);
		m_ptrs.push_back(ptr);
		m_locs.push_back(loc);
		m_sizes.push_back(size);
		m_types.push_back(type);
		return true;
	}
	return false;
}

int32_t SpuShader::addUniforms(const Attrs &unif_attrs)
{
	auto n = 0;
	for (const auto &attr: unif_attrs) {
		n += addUniform(attr.key().c_str(), attr);
	}
	return n;
}

bool SpuShader::replaceUniform(const char *name, void *ptr)
{
	for (auto i = 0u; i < m_names.size(); i++) {
		if (m_names[i] == name) {
			m_ptrs[i] = ptr;
			return true;
		}
	}
	return false;
}

void *SpuShader::uniformPtr(const char *name) const
{
	for (auto i = 0u; i < m_names.size(); i++) {
		if (m_names[i] == name) {
			return m_locs[i] >= 0 ? m_ptrs[i] : nullptr;
		}
	}
	return nullptr;
}

void SpuShader::clearUniforms()
{
	m_locs.clear();
	m_ptrs.clear();
	m_names.clear();
	m_sizes.clear();
	m_types.clear();
}

void SpuShader::setUniforms(const Attrs &attrs)
{
	auto is_vec = [&](int32_t type) {
		return (type == GL_FLOAT_VEC4 ||
			type == GL_FLOAT_VEC3 ||
			type == GL_FLOAT_VEC2);
	};

	auto is_ivec = [&](int32_t type) {
		return (type == GL_INT_VEC4   ||
			type == GL_INT_VEC3   ||
			type == GL_INT_VEC2   ||
			type == GL_BOOL_VEC4 ||
			type == GL_BOOL_VEC3  ||
			type == GL_BOOL_VEC2);
	};
	// clang-format on
	for (auto i = 0u; i < m_names.size(); i++) {
		const auto *name = m_names[i].c_str();
		const auto size = m_sizes[i];
		const auto type = m_types[i];

		auto *ptr = m_ptrs[i];

		if (type == GL_FLOAT && size == 4) {
			*(float *)ptr = attrs.get(name, *(float *)ptr);
			continue;
		}
		if (type == GL_INT && size == 4) {
			*(int32_t *)ptr = attrs.get(name, *(int32_t *)ptr);
			continue;
		}
		if ((type == GL_UNSIGNED_INT || type == GL_BOOL) && size == 4) {
			*(uint32_t *)ptr = attrs.get(name, *(uint32_t *)ptr);
			continue;
		}
		if (is_vec(type) && size <= 16) {
			auto value = attrs.get(name, *(vec4f_t *)ptr);
			memcpy(ptr, &value, size);
			continue;
		}
		if (is_ivec(type) && size <= 16) {
			auto value = attrs.get(name, *(vec4i_t *)ptr);
			memcpy(ptr, &value, size);  // size might be smaller
			continue;
		}
		if (spu_gl_is_texture(type) || spu_gl_is_image(type)) {
			*(uint32_t *)ptr = attrs.get(name, *(uint32_t *)ptr);
		}
		else {
			auto data_ptr = attrs.get(name, ptr);
			memcpy(ptr, data_ptr, size);
		}
	}
}

void SpuShader::init(const Attrs &attrs) { base_t::init(attrs); }

void SpuShader::init(const char *path, const Attrs &shader_attrs)
{
	if (path) {
		if (shader_attrs.get("reloadable", 0)) {
			auto attrs = shader_attrs.uniq();
			attrs.erase("reloadable");
			attrs.emplace_back("reload_count", 0);
			attrs.emplace_back("abort", 0);
			attrs.preserve();
			ms_reloads()[this] = Reload(path, attrs);
		}
		reset(spu_inventory_new("shader", path, shader_attrs));
	}
}
void SpuShader::reload()
{
	for (auto &pair: ms_reloads()) {
		auto *shader = pair.first;
		auto &reload = pair.second;

		auto reload_count = reload.attrs.get("reload_count", 0);
		reload.attrs.replace({"reload_count", reload_count + 1});

		auto new_shader_id = spu_inventory_new("shader", reload.path.c_str(), reload.attrs);
		aux_message(
		        0, "reload: %p %3d %3d %s\n", shader, shader->id(), new_shader_id, reload.path.c_str());

		if (new_shader_id != 0 && new_shader_id != shader->id()) {
			shader->base_t::reset(new_shader_id);  // preserve uniforms
			std::vector<const char *> names;
			for (auto &name: shader->m_names) {
				names.push_back(name.c_str());
			}
			spu_shader_loc(
			        shader->id(), names.data(), shader->m_locs.data(), shader->m_sizes.data(),
			        shader->m_types.data(), names.size());
		}
	}
}

void SpuShader::report(const char *str) const
{
	if (str && *str) aux_printf("%s:\n", str);

	aux_printf("uniform pointers:\n");
	for (auto i = 0u; i < m_names.size(); i++) {
		aux_printf("\t%08x", m_locs[i]);
		if ((m_locs[i] & ~0xffff) == 0x20000) {  // not uniform
			uint32_t ubo_id;                 // might be changed via set_extra()
			const char *name = m_names[i].c_str();
			spu_shader_loc(id(), &name, nullptr, nullptr, &ubo_id, 1);
			aux_printf(" ubo:%04x%*s", ubo_id, 23, " ");
		}
		else {
			aux_printf(" %-30s ", opengl_const(m_types[i]));
		}
		aux_printf("%p %5d %s\n", m_ptrs[i], m_sizes[i], m_names[i].c_str());
	}
	spu_shader_report(id());
}
}  // namespace spu
