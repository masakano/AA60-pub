//
// GlfwDevice :
//
#pragma once
#include "../spu_dev.h"

namespace spu::libspu::glfw {

class GlfwDevice : public Device {
public:
	void init(DeviceConfig *dev_config) override;
	void terminate() override;
	void open(const Attrs &attrs) override;
	void close() override;
	void poll() override;
	bool swap() override;
	int32_t get(const hash32_t &key, void *value) override;
};
}  // namespace spu::libspu::glfw
