//
// SpuRenderstate :
//
#include <spu++/spu_renderstate.h>

namespace spu {
static_assert(std::is_trivially_copyable<SpuRenderstate>::value, "not copyable");

namespace {
int32_t s_id = -1;
SpuRenderstate s_current;
SpuRenderstate s_initial;
}  // namespace

void SpuRenderstate::startup(const Attrs &graphics_attrs)
{
	Attrs renderstate_attrs = {
	        {"flags.fill",               0                            },
	        {"flags.blend",              0                            },
	        {"flags.cull_face",          0                            },
	        {"flags.ccw",                0                            },
	        {"flags.depth_test",         0                            },
	        {"flags.stencil_test",       0                            },
	        {"flags.scissor_test",       0                            },
	        {"flags.conservative",       0                            },
	        {"flags.point_sprite",       0                            },
	        {"flags.program_point_size", 0                            },
	        {"flags.fill_offset",        0                            },
	        {"flags.line_offset",        0                            },
	        {"flags.point_offset",       0                            },
	        {"flags.line_stipple",       0                            },
	        {"flags.multisample",        0                            },
	        {"flags.sample_shading",     0                            },
	        {"flags.cube_map_seamless",  0                            },
	        {"flags.depth_clamp",        0                            },
	        {"flags.clip_distance0",     0                            },
	        {"flags.clip_distance1",     0                            },
	        {"flags.clip_distance2",     0                            },
	        {"flags.clip_distance3",     0                            },
	        {"flags.clip_distance4",     0                            },
	        {"flags.clip_distance5",     0                            },
	        {"flags.srgb_encode",        0                            },
	        {"flags",                    &s_current.flags             },
	        {"blend_func",               &s_current.blend_func        },
	        {"stencil_func",             &s_current.stencil_func      },
	        {"blend_eq",                 &s_current.blend_eq          },
	        {"poly_offset",              &s_current.poly_offset       },
	        {"write_mask",               &s_current.write_mask        },
	        {"depth_func",               &s_current.depth_func        },
	        {"blend_color",              &s_current.blend_color       },
	        {"cull_face",                &s_current.cull_face         },
	        {"line_width",               &s_current.line_width        },
	        {"point_size",               &s_current.point_size        },
	        {"line_stipple",             &s_current.line_stipple      },
	        {"blend_func_channel",       &s_current.blend_func_channel},
	        {"min_sample_shading",       &s_current.min_sample_shading},
	};

	uint32_t device_status;
	spu_graphics_get("device_status", &device_status);

	if (device_status != 1 && s_id == -1) {
		spu_graphics_init(graphics_attrs);
		s_id = spu_renderstate_new(renderstate_attrs);
		spu_renderstate_use(s_id);
		spu_renderstate_get(s_id);
		s_initial = s_current;
		// printf("init renderstate done\n");
		return;
	}
	if (device_status == 1 && s_id != -1) {
		s_current = s_initial;
		spu_renderstate_use(s_id);
		return;
	}

	aux_message(0, "Device status mismatch\n");
	aux_message(0, "    When you use SpuRenderstate::startup()/shutdown(),\n");
	aux_message(0, "    do not call spu_graphics_init()/spu_graphics_shutdown() directly\n");
	aux_abort();
}

void SpuRenderstate::shutdown()
{
	uint32_t device_status;
	spu_graphics_get("device_status", &device_status);

	if (device_status != 1 && s_id == -1) {
		// do nothing
		return;
	}
	if (device_status == 1 && s_id != -1) {
		s_current = s_initial;
		spu_renderstate_use(s_id);
		spu_graphics_shutdown();
		s_id = -1;
		return;
	}
	aux_message(0, "Device status mismatch\n");
	aux_message(0, "    When you use SpuRenderstate::startup()/shutdown(),\n");
	aux_message(0, "    do not call spu_graphics_init()/spu_graphics_shutdown() directly\n");
	aux_abort();
}

void SpuRenderstate::sync()
{
	// device -> proxy
	spu_renderstate_get(s_id);
}

SpuRenderstate::SpuRenderstate(bool use_current)
{
	if (use_current) *this = s_current;
}

void SpuRenderstate::get()
{
	// device -> proxy -> *this
	spu_renderstate_get(s_id);
	*this = s_current;
}

bool SpuRenderstate::use() const
{
	if (*this != s_current) {
		s_current = *this;
		return spu_renderstate_use(s_id);
	}
	return false;
}

bool operator==(const SpuRenderstate &rs0, const SpuRenderstate &rs1)
{
#define CMP(mem) (memcmp(&rs0.mem, &rs1.mem, sizeof(rs0.mem)) == 0)
	return CMP(flags) && CMP(blend_func) && CMP(write_mask) && CMP(stencil_func) && CMP(poly_offset)
	    && CMP(line_stipple) && CMP(blend_color) && CMP(depth_func) && CMP(cull_face) && CMP(line_width)
	    && CMP(point_size) && CMP(min_sample_shading) && CMP(blend_func_channel);

#undef CMP
}

void SpuRenderstate::setBlendType(const hash32_t &blend_type)
{
	struct Desc {
		uint32_t src;
		uint32_t dst;
		uint32_t func;
	};

	const std::map<hash32_t, Desc> descs = {
	        {e_add,            {GL_SRC_ALPHA, GL_ONE, GL_FUNC_ADD}                          },
	        {e_screen,         {GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_FUNC_ADD}                },
	        {e_lighten,        {GL_ONE, GL_ONE, GL_MAX}                                     },
	        {e_multiply,       {GL_DST_COLOR, GL_ZERO, GL_FUNC_ADD}                         },
	        {e_darken,         {GL_ONE, GL_ONE, GL_MIN}                                     },
	        {e_linear_burn,    {GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_FUNC_SUBTRACT}     },
	        {e_alpha,          {GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD}          },
	        {e_constant_alpha, {GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA, GL_FUNC_ADD}},
	        {e_constant_add,   {GL_CONSTANT_ALPHA, GL_ONE, GL_FUNC_ADD}                     },
	};

	try {
		auto &desc = descs.at(blend_type);
		blend_func.sc = blend_func.sa = desc.src;
		blend_func.dc = blend_func.da = desc.dst;
		blend_eq.c = blend_eq.a = desc.func;
	}
	catch (std::out_of_range &) {
		aux_message(0, "unknown type '%s'. available types are:\n", blend_type.c_str());
		for (auto &p: descs) {
			aux_printf("    %08x '%s'\n", p.first.value(), p.first.c_str());
		}
		aux_abort();
	};
}

void SpuRenderstate::set(const Attrs &attrs)
{
#define APPLY(value) (value = attrs.get(#value, value))
	APPLY(flags.fill);
	APPLY(flags.blend);
	APPLY(flags.cull_face);
	APPLY(flags.ccw);
	APPLY(flags.depth_test);
	APPLY(flags.stencil_test);
	APPLY(flags.point_sprite);
	APPLY(flags.program_point_size);
	APPLY(flags.fill_offset);
	APPLY(flags.line_offset);
	APPLY(flags.point_offset);
	APPLY(flags.line_stipple);
	APPLY(flags.multisample);
	APPLY(flags.sample_shading);
	APPLY(flags.cube_map_seamless);
	APPLY(flags.depth_clamp);
	APPLY(flags.clip_distance0);
	APPLY(flags.clip_distance1);
	APPLY(flags.clip_distance2);
	APPLY(flags.clip_distance3);
	APPLY(flags.clip_distance4);
	APPLY(flags.clip_distance5);
	APPLY(flags.srgb_encode);
	APPLY(blend_func.dc);
	APPLY(blend_func.da);
	APPLY(blend_eq.a);
	APPLY(write_mask.r);
	APPLY(write_mask.g);
	APPLY(write_mask.b);
	APPLY(write_mask.a);
	APPLY(write_mask.z);
	APPLY(stencil_func.f_ref);
	APPLY(stencil_func.f_sfail);
	APPLY(stencil_func.f_pass);
	APPLY(stencil_func.b_ref);
	APPLY(stencil_func.b_sfail);
	APPLY(stencil_func.b_pass);
	APPLY(poly_offset.units);
	APPLY(line_stipple.pattern);
	APPLY(blend_color);
	APPLY(line_width);
	APPLY(min_sample_shading);
	APPLY(blend_func_channel);
#undef APPLY
}

void SpuRenderstate::report(const char *str)
{
	if (str && *str) aux_printf("%s:\n", str);
	spu_renderstate_report(s_id);
}

}  // namespace spu
