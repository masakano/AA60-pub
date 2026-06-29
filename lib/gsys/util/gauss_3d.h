//
// GsGauss3D :
//
#pragma once
#include <spu++/spu++.h>

namespace spu {

/// three dementional gaussian
class GsGauss3D {
public:
	static constexpr int32_t def_local_size = 8;
	GsGauss3D() = default;
	GsGauss3D(uint32_t dst_texture, uint32_t src_texture) { init(dst_texture, src_texture); }
	void init(uint32_t dst_texture, uint32_t src_texture);
	void setVariance(float variance);
	void draw();

protected:
	SpuComputeArray m_array;
	SpuTexture m_texture;
	uint32_t m_srcTexture;
	uint32_t m_dstTexture;

	float u_weight[15];
	Vec4i u_delta = Vec4i(1, 0, 0, 0);
	uint32_t u_volume0 = 0;
	uint32_t u_volume1 = 0;
	Vec4i u_size = Vec4i(0);
};
}  // namespace spu
