//
// ResourceImage :
//
#pragma once

#include "resource_object.h"
#include "image_loader.h"

namespace spu::libspu::resource {

class ResourceImage : public ResourceObject {
public:
	ResourceImage(const char *path, const Attrs &attrs);
	int32_t get(const hash32_t &key, void *value) const override;

private:
	image::Loader m_loader;
	void doBackground() override;
	void doEpilogue() override;
	inline static uint32_t ms_id = (GL_TEXTURE_HOST_IMAGE << 16);
};
}  // namespace spu::libspu::resource
