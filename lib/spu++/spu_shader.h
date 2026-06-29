//
// SpuShader : shader object
//
#pragma once

#include "spu_object.h"
#include <spu/spu.h>

namespace spu {
namespace spu_object {

struct ShaderBody : SpuBody {
	ShaderBody(uint32_t handle, bool is_owner) : SpuBody(handle, is_owner) {}

	ShaderBody(const Attrs &attrs) : SpuBody(0, true)
	{
		handle = spu_shader_new(attrs);  // no inventory
	}

	~ShaderBody()
	{
		assert(use_count == 0);
		if (is_owner && handle) {
			spu_inventory_delete(handle);  // implies spu_shader_delete()
		}
	}
};
}  // namespace spu_object

class SpuShader : public spu_object::SpuObject<spu_object::ShaderBody> {
public:
	SpuShader() = default;

	explicit SpuShader(uint32_t shader_id, bool is_owner = true) { reset(shader_id, is_owner); }
	explicit SpuShader(const Attrs &shader_attrs) { init(shader_attrs); }
	explicit SpuShader(const char *path, const Attrs &shader_attrs = Attrs()) { init(path, shader_attrs); }
	using spu_object::SpuObject<spu_object::ShaderBody>::set;

	/*virtual */void use() { spu_shader_use(id(), m_locs.data(), m_ptrs.data(), m_locs.size()); }
	/*virtual */void init(const char *path, const Attrs &shader_attrs = Attrs());

	bool reuse(const char *signature) 
	{ 
		uint32_t id;
		if ((id = spu_inventory_reuse("shader", signature)) != ~0u) {
			reset(id);
			return true;
		}
		return false;
	}
	void append(const char *signature) 
	{ 
		spu_inventory_append(id(), signature); 
	}

	const std::vector<std::string> &names() const { return m_names; }
	const std::vector<int32_t> &locs() const { return m_locs; }
	const std::vector<void *> &ptrs() const { return m_ptrs; }
	const std::vector<uint32_t> &sizes() const { return m_sizes; }
	const std::vector<uint32_t> &types() const { return m_types; }

	void clearUniforms();
	bool addUniform(const char *name, void *ptr);
	int32_t addUniforms(const Attrs &unif_attrs);
	bool replaceUniform(const char *name, void *ptr);
	void *uniformPtr(const char *name) const;
	void setUniforms(const Attrs &attrs);
	void init(const Attrs &attrs) override;
	void dispose() override;
	void reset(uint32_t shader_id, bool is_owner = true) override;

	void set(const Attrs &attrs) override;
	int32_t get(const hash32_t &key, void *value) const override;
	void report(const char *str) const override;

	static void reload();

private:
	std::vector<std::string> m_names;
	std::vector<int32_t> m_locs;
	std::vector<void *> m_ptrs;
	std::vector<uint32_t> m_sizes;
	std::vector<uint32_t> m_types;

	struct Reload {
		std::string path;
		Attrs attrs;
		Reload() = default;
		Reload(const std::string &path, const Attrs &attrs) : path(path), attrs(attrs) {}
	};
	static std::map<SpuShader *, Reload> &ms_reloads()
	{
		static std::map<SpuShader *, Reload> v;
		return v;
	}
};
}  // namespace spu
