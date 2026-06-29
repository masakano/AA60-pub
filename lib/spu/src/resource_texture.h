//
// ResourceTexture :
//
#pragma once
#include "resource_object.h"
#include "image_loader.h"

namespace spu::libspu::resource {

class ResourceTexture : public ResourceObject {
public:
	ResourceTexture(const char *path, const Attrs &attrs);
	int32_t get(const hash32_t & /*key*/, void * /*value*/) const override { return 0; }

private:
	// std::vector<std::string> m_paths;
	image::Loader m_loader;
	void doBackground() override;
	void doEpilogue() override;
};
}  // namespace spu::libspu::resource
