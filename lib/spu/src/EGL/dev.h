//
// EglDevice :
//
#pragma once
#include "../GLFW/dev.h"

namespace spu::libspu::egl {

#ifdef _WIN32
using EglDevice = glfw::Device;
#else
class EglDevice : public Device {
public:
	void init(DeviceConfig *dev_config) override;
	void terminate() override;
	void open(const Attrs &attrs) override;
	void close() override;
	void poll() override;
	bool swap() override;
	int32_t get(const hash32_t &key, void *value) override;
};
#endif
}  // namespace spu::libspu::egl
