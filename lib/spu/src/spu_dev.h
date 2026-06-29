//
// DeviceConfig :
//
#pragma once

#include <ssys/attrs.h>
#include <spu/spu_pad.h>

namespace spu::libspu {

struct DeviceConfig {
	hash32_t name = "unknown";
	uint32_t interval = 1;
	bool use_pad = true;
	SpuPad pads[2];
	void sync_pad(SpuPad &pad)
	{
		if (use_pad) {
			if (pads[0].reset) {
				pads[0].reset = 0;
				pad = pads[0];
			}
			else {
				pads[1] = pads[0];
				pads[0] = pad;
			}
		}
	}
};

class Device {
public:
	virtual ~Device() = default;
	virtual void init(DeviceConfig *dev_config) = 0;
	virtual void terminate() = 0;
	virtual void open(const Attrs &attrs) = 0;
	virtual void close() = 0;
	virtual void poll() = 0;
	virtual bool swap() = 0;
	virtual int32_t get(const hash32_t &key, void *value) = 0;
};
}  // namespace spu::libspu
