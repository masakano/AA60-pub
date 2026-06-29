//
// FrameBody :
//
#pragma once

#include "spu_array.h"
#include "spu_texture.h"
#include <smath/range.h>

namespace spu {

namespace spu_object {
struct FrameBody : SpuBody {
	std::vector<SpuTexture> textures;
	FrameBody(uint32_t handle, bool is_owner);
	FrameBody(const Attrs &attrs);
	~FrameBody();
};
}  // namespace spu_object

class SpuFrame : public spu_object::SpuObject<spu_object::FrameBody> {
public:
	using base_t = spu_object::SpuObject<spu_object::FrameBody>;
	using base_t::get;
	using base_t::set;

	SpuFrame() = default;
	explicit SpuFrame(const Attrs &attrs) { init(attrs); };

	SpuTexture &getBuffer(const hash32_t &slot = "color0");
	const SpuTexture &getBuffer(const hash32_t &slot = "color0") const;

	void begin();
	void clear();
	void end();
	SPU_SET_GET_REPORT(frame);
};
}  // namespace spu
