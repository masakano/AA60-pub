//
// SpuFrame :
//
#include <spu++/spu_frame.h>

namespace spu {
namespace {

const std::vector<hash32_t> s_hashes = {
        "color0.resolve",  // must be 1st (don't use std::map)
        "depth.resolve",  "color0", "color1", "color2", "color3",  "color4",
        "color5",         "color6", "color7", "depth",  "stencil", "depth_stencil",
};
}  // namespace

namespace spu_object {
FrameBody::FrameBody(uint32_t handle, bool is_owner) : SpuBody(handle, is_owner), textures(s_hashes.size()) {}

FrameBody::FrameBody(const Attrs &attrs) : FrameBody(0, true)
{
	Rectf viewport0;
	spu_frame_get(-1, "viewport0", viewport0.f);
	viewport0 = attrs.get<vec4f_t>("viewport0", viewport0);

	auto frame_attrs = attrs;
	frame_attrs.emplace_back("viewport0", viewport0);  // force to set

	for (auto &hash: s_hashes) {
		auto &texture = textures[&hash - &s_hashes[0]];
		auto prefix = std::string(hash.c_str()) + ".";
		auto attach_attrs = attrs.select(prefix.c_str());

		if (!attach_attrs.empty()) {
			auto texture_id = attach_attrs.get("texture_id", 0);

			if (texture_id) {
				texture.reset(texture_id, false);  // no ownership
				int32_t width = 0;
				int32_t height = 0;

				texture.get("width", &width);
				texture.get("height", &height);

				aux_error(
				        viewport0.ox + viewport0.sx != width
				                || viewport0.oy + viewport0.sy != height,
				        "prefix texture(%dx%d) is mismatch width viewport (%g,%g,%g,%g)\n",
				        width, height, viewport0.ox, viewport0.oy, viewport0.sx, viewport0.sy);
			}
			else {
				Attrs tex_attrs = {
				        {"width",  int32_t(viewport0.ox + viewport0.sx)},
				        {"height", int32_t(viewport0.oy + viewport0.sy)},
				};
				texture.init(attach_attrs + tex_attrs);
			}
			frame_attrs.emplace_back(hash.c_str(), texture.id());
		}
	}
	handle = spu_frame_new(frame_attrs);
}

FrameBody::~FrameBody()
{
	assert(use_count == 0);
	if ((handle != 0u) && is_owner) {
		spu_frame_delete(handle);
	}
}
}  // namespace spu_object

const SpuTexture &SpuFrame::getBuffer(const hash32_t &slot) const
{
	aux_error(!id(), "getBuffer() to invalid frame_id(%u)\n", id());

	for (auto &hash: s_hashes) {
		if (slot == hash) {
			return get_body()->textures[&hash - &s_hashes[0]];
		}
	}

	for (auto &hash: s_hashes) {
		auto &texture = get_body()->textures[&hash - &s_hashes[0]];
		aux_printf("    %08x %s\n", texture.id(), hash.c_str());
	}
	aux_error(true, "unknown slot '%08x'\n", slot);
}

SpuTexture &SpuFrame::getBuffer(const hash32_t &slot)
{
	return const_cast<SpuTexture &>(const_cast<const SpuFrame *>(this)->getBuffer(slot));
}

void SpuFrame::begin() { spu_frame_begin(id()); }
void SpuFrame::clear() { spu_frame_clear(id()); }
void SpuFrame::end() { spu_frame_end(); }

}  // namespace spu
