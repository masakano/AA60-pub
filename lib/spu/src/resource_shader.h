//
// ResourceShader :
//
#pragma once
#include "resource_object.h"
#include "shader_loader.h"

namespace spu::libspu::resource {

class ResourceShader : public ResourceObject {
public:
	ResourceShader(const char *path, const Attrs &attrs);
	int32_t get(const hash32_t &, void *) const override { return 0; }

private:
	shader::Loader m_loader;
	std::vector<std::string> m_names;
	void message(const char *vendor, const char *log);
	void doBackground() override;
	void doEpilogue() override;
};
}  // namespace spu::libspu::resource
