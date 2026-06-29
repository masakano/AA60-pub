//
// SpuRenderstate :
//
#pragma once
#include <spu/spu.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#endif

namespace spu {

struct SpuRenderstate {
public:
	static constexpr hash32_t e_none = "none";
	static constexpr hash32_t e_add = "add";
	static constexpr hash32_t e_screen = "screen";
	static constexpr hash32_t e_lighten = "lighten";
	static constexpr hash32_t e_multiply = "multiply";
	static constexpr hash32_t e_darken = "darken";
	static constexpr hash32_t e_linear_burn = "linear_burn";
	static constexpr hash32_t e_alpha = "alpha";
	static constexpr hash32_t e_constant_alpha = "constant_alpha";
	static constexpr hash32_t e_constant_add = "constant_add";

	union Flags {
		struct {
			int fill              : 1 = true;   // GL_POLYGON_MODE
			int blend             : 1 = false;  // GL_BLEND
			int cull_face         : 1 = false;  // GL_CULL_FACE
			int ccw               : 1 = true;   // GL_FRONT_FACE
			int depth_test        : 1 = false;  // GL_DEPTH_TEST
			int stencil_test      : 1 = false;  // GL_STENCIL_TEST
			int scissor_test      : 1 = true;   // GL_SCISSOR_TEST
			int conservative      : 1 = false;  // GL_CONSERVATIVE_RASTERIZATION
			int point_sprite      : 1 = false;  // GL_POINT_SPRITE
			int program_point_size: 1 = false;  // GL_PROGRAM_POINT_SIZE
			int fill_offset       : 1 = false;  // GL_POLYGON_OFFSET_FILL
			int line_offset       : 1 = false;  // GL_POLYGON_OFFSET_LINE
			int point_offset      : 1 = false;  // GL_POLYGON_OFFSET_POINT
			int line_stipple      : 1 = false;  // GL_LINE_STIPPLE
			int multisample       : 1 = true;   // GL_MULTISAMPLE
			int sample_shading    : 1 = false;  // GL_SAMPLE_SHADING
			int cube_map_seamless : 1 = true;   // GL_TEXTURE_CUBE_MAP_SEAMLESS
			int depth_clamp       : 1 = true;   // GL_DEPTH_CLAMP
			int clip_distance0    : 1 = false;  // GL_CLIP_DISTANCE0
			int clip_distance1    : 1 = false;  // GL_CLIP_DISTANCE1
			int clip_distance2    : 1 = false;  // GL_CLIP_DISTANCE2
			int clip_distance3    : 1 = false;  // GL_CLIP_DISTANCE3
			int clip_distance4    : 1 = false;  // GL_CLIP_DISTANCE4
			int clip_distance5    : 1 = false;  // GL_CLIP_DISTANCE5
			int srgb_encode       : 1 = true;   // GL_FRAMEBUFFER_SRGB
			int _pad              : 7 = 0;
		};
		uint32_t bits;
	} flags;

	struct BlendFunc {
		uint32_t sc = GL_SRC_ALPHA;            // function for source color
		uint32_t dc = GL_ONE_MINUS_SRC_ALPHA;  // function for destination color
		uint32_t sa = GL_SRC_ALPHA;            // function for source color
		uint32_t da = GL_ONE_MINUS_SRC_ALPHA;  // function for destination alpha
	} blend_func;                                  // function for blend function

	struct BlendEq {
		uint32_t c = GL_FUNC_ADD;  // equation for color
		uint32_t a = GL_FUNC_ADD;  // equation for alpha
	} blend_eq;                        // blend equation

	struct WriteMask {
		uint32_t r   : 1 = true;  // glColorMask
		uint32_t g   : 1 = true;  // glColorMask
		uint32_t b   : 1 = true;  // glColorMask
		uint32_t a   : 1 = true;  // glColorMask
		uint32_t z   : 1 = true;  // glDepthMask
		uint32_t _pad: 27 = 0;
	} write_mask;  // write mask

	struct StencilFunc {
		uint32_t f_func = GL_ALWAYS;  // GL_STENCIL_FUNC
		uint32_t f_ref = 0;           // GL_STENCIL_REF
		uint32_t f_mask = 0x1f;       // GL_STENCIL_VALUE_MASK
		uint32_t f_sfail = GL_KEEP;   // GL_STENCIL_FAIL
		uint32_t f_dfail = GL_KEEP;   // GL_STENCIL_PASS_DEPTH_FAIL
		uint32_t f_pass = GL_KEEP;    // GL_STENCIL_PASS_DEPTH_PASS

		uint32_t b_func = GL_ALWAYS;  // GL_STENCIL_BACK_FUNC
		uint32_t b_ref = 0;           // GL_STENCIL_BACK_REF
		uint32_t b_mask = 0x1f;       // GL_STENCIL_BACK_VALUE_MASK
		uint32_t b_sfail = GL_KEEP;   // GL_STENCIL_BACK_FAIL
		uint32_t b_dfail = GL_KEEP;   // GL_STENCIL_BACK_PASS_DEPTH_FAIL
		uint32_t b_pass = GL_KEEP;    // GL_STENCIL_BACK_PASS_DEPTH_PASS
	} stencil_func;                       // stencil function

	struct PolyOffset {
		float factor = 0;  // GL_POLYGON_OFFSET_FACTOR
		float units = 0;   // GL_POLYGON_OFFSET_UNITS
	} poly_offset;             // polygon offsets

	struct LineStipple {
		int32_t factor = 0;   // GL_LINE_STRIP_REPEAT
		int32_t pattern = 0;  // GL_LINE_STRIP_PATTERN
	} line_stipple;               // line stipple pattern

	uint32_t depth_func = GL_LESS;               // glDepthFunc
	vec4f_t blend_color = {0.5, 0.5, 0.5, 0.5};  // glBlendColor

	uint32_t cull_face = GL_BACK;     // glCullFace
	float line_width = 1.0;           // glLineWidth
	float point_size = 1.0;           // glPointSize
	float min_sample_shading = 1.0;   // glMinSampleShading
	int32_t blend_func_channel = -1;  // glBlendFuncSeparatei (-1: all cancel)

	SpuRenderstate(bool use_current = false);

	bool use() const;  // instance -> cache -> device
	void get();
	void set(const Attrs &attrs);
	void setBlendType(const hash32_t &blend_type);
	friend bool operator==(const SpuRenderstate &rs0, const SpuRenderstate &rs1);

	static void report(const char *str);
	static void sync();
	static void startup(const Attrs &graphics_attrs);
	static void shutdown();
};

class SpuScopedRenderstate : public SpuRenderstate {
public:
	SpuScopedRenderstate(bool use_current = false) : SpuRenderstate(use_current), m_prev(true) {}
	SpuScopedRenderstate(const SpuRenderstate &renderstate) : SpuRenderstate(renderstate), m_prev(true) {}
	~SpuScopedRenderstate() { m_prev.use(); /* restore */ }

private:
	SpuRenderstate m_prev;
};

}  // namespace spu

#ifdef __clang__
#pragma clang diagnostic pop
#endif
