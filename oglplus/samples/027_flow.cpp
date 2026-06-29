//
// FlowMapArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>
#include <shapes/screen.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_flow_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "}                                                                                      \n"
};

const char *c_flow_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_height_texture1;                                                   \n"
    "uniform sampler2D u_height_texture2;                                                   \n"
    "uniform sampler2D u_flow_texture;                                                      \n"
    "uniform int u_hmap_size;                                                               \n"
    "uniform float u_time;                                                                  \n"
    "float ts = 1.0/u_hmap_size;                                                            \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout (location = 0) out vec4 final_bump;                                             \n"
    "layout (location = 1) out float final_height;                                          \n"
    "ivec2 clamp_tc(ivec2 tc)                                                               \n"
    "{                                                                                      \n"
    "       return clamp(tc, ivec2(0, 0), ivec2(u_hmap_size, u_hmap_size));                 \n"
    "}                                                                                      \n"
    "float height_at(                                                                       \n"
    "       sampler2D tex,                                                                  \n"
    "       float xoffs,                                                                    \n"
    "       float yoffs,                                                                    \n"
    "       float steer,                                                                    \n"
    "       float flow,                                                                     \n"
    "       float factor                                                                    \n"
    ")                                                                                      \n"
    "{                                                                                      \n"
    "       vec2 tc = f_texcoord + vec2(xoffs, yoffs)*ts;                                   \n"
    "       return texture(tex, tc+vec2(steer, flow)*ts).r*factor;                          \n"
        //"     return clamp(texture(tex, TC+vec2(steer, flow)*TS).r*factor, 0.0, 1.0);"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec2 fc = gl_FragCoord.xy+vec2(-u_hmap_size/2, -u_hmap_size+4);                 \n"
    "       float s = 2.71*sin(1.1*u_time+0.03*length(fc));                                 \n"
    "       float f = 1.62*sin(2.0*u_time-0.05*length(fc));                                 \n"
    "       vec2 tc = f_texcoord+ts*vec2(s, f)+vec2(0.0, u_time*0.1);                       \n"
    "       vec4 fm = texture(u_flow_texture, tc);                                          \n"
    "       s = 0.1*s+(0.5-fm.r)*3.0;                                                       \n"
    "       f = 0.3*f+(fm.g-0.5)*4.0;                                                       \n"
    "       float  ch = height_at(u_height_texture2, 0, 0, s, f, 1.00);                     \n"
    "       s *= 1.5;                                                                       \n"
    "       f *= 1.618;                                                                     \n"
    "       float xp1 = height_at(u_height_texture2, 1, 0, s, f, 0.25);                     \n"
    "       float xm1 = height_at(u_height_texture2,-1, 0, s, f, 0.25);                     \n"
    "       float yp1 = height_at(u_height_texture2, 0, 1, s, f, 0.25);                     \n"
    "       float ym1 = height_at(u_height_texture2, 0,-1, s, f, 0.25);                     \n"
    "       vec3 frag_normal = vec3(                                                        \n"
    "               (xm1 - ch) + (ch - xp1),                                                \n"
    "               (ym1 - ch) + (ch - yp1),                                                \n"
    "               0.2                                                                     \n"
    "       );                                                                              \n"
    "       final_height  = xp1 + xm1 + yp1 + ym1;                                          \n"
    "       final_height += height_at(u_height_texture2, 1,-1, s, f, 0.25);                 \n"
    "       final_height += height_at(u_height_texture2, 1, 1, s, f, 0.25);                 \n"
    "       final_height += height_at(u_height_texture2,-1,-1, s, f, 0.25);                 \n"
    "       final_height += height_at(u_height_texture2,-1, 1, s, f, 0.25);                 \n"
    "       f *= 1.618;                                                                     \n"
    "       s -= frag_normal.x*3.0;                                                         \n"
    "       final_height -= height_at(u_height_texture1, 0, 0, s, f, 1.00);                 \n"
    "       final_height += (fm.b-0.5)*0.09;                                                \n"
    "       if (fc.y > 0.0)                                                                 \n"
    "               final_height += min(pow(sin(u_time+length(10*fc)), 16.0), 0.3);         \n"
    "       final_bump = vec4(normalize(frag_normal),clamp(final_height, 0.0, 1.0));        \n"
    "}                                                                                      \n"
};

const char *c_screen_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "}                                                                                      \n"
};

const char *c_screen_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_background_texture;                                                \n"
    "uniform sampler2D u_normalmap;                                                         \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 nm = texture(u_normalmap, f_texcoord);                                     \n"
    "       vec2 offs = nm.xy*0.05+vec2(0.004, 0.004)*nm.w;                                 \n"
    "       vec4 c = texture(u_background_texture, f_texcoord+offs);                        \n"
    "       float l = clamp(nm.w+pow(dot(nm.xy,vec2(0.2,0.2)), 9.0), 0.3, 1.1);             \n"
    "       final_color = c.rgb*mix(1.2, 0.8, l);                                           \n"
    "}                                                                                      \n"
};

/* clang-format on */
class FlowMapArray : public shapes::Array {
public:
	uint32_t u_height_texture1;
	uint32_t u_height_texture2;
	uint32_t u_flow_texture;
	float u_time;
	int32_t u_hmap_size;

	explicit FlowMapArray(int32_t tex_size)
	{
		Attrs shader_attrs = {
		        {"vert", c_flow_vert},
		        {"frag", c_flow_frag},
		};

		Attrs unif_attrs = {
		        {"u_height_texture1", &u_height_texture1},
		        {"u_height_texture2", &u_height_texture2},
		        {"u_flow_texture",    &u_flow_texture   },
		        {"u_time",            &u_time           },
		        {"u_hmap_size",       &u_hmap_size      },
		};
		Array::initShader(shader_attrs, unif_attrs);
		Array::initArray(shapes::Screen(), {"position", "normal", "tangent", "texcoord"});
		u_hmap_size = tex_size;
	}
};

class FlowMapHolder {
public:
	const uint32_t c_nhm = 3;
	uint32_t m_curr = 0;
	std::vector<SpuTexture> m_heightTextures;
	SpuTexture m_bumpTexture;
	SpuTexture m_flowTexture;

	FlowMapHolder(size_t flow_tex_size, const images::Image &flow_texture_image)
	{
		m_heightTextures.resize(c_nhm);

		std::vector<uint32_t> v(flow_tex_size * flow_tex_size, 0);
		for (auto i = 0u; i != c_nhm; ++i) {
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D         },
                                {"iformat",     GL_R8                 },
			        {"data",        v.data()},
                                {"width",       flow_tex_size         },
			        {"height",      flow_tex_size         },
                                {"min_filter",  GL_LINEAR             },
			        {"mag_filter",  GL_LINEAR             },
                                {"wrap_s",      GL_CLAMP_TO_BORDER    },
			        {"wrap_t",      GL_CLAMP_TO_BORDER    },
                                {"auto_mipmap", 0                     },
			};
			m_heightTextures[i].init(attrs);
		}
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D   },
                                {"iformat",     GL_RGBA8        },
			        {"width",       flow_tex_size   },
                                {"height",      flow_tex_size   },
			        {"min_filter",  GL_LINEAR       },
                                {"mag_filter",  GL_LINEAR       },
			        {"wrap_s",      GL_CLAMP_TO_EDGE},
                                {"wrap_t",      GL_CLAMP_TO_EDGE},
			        {"auto_mipmap", 0               },
			};
			m_bumpTexture.init(attrs);
		}
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D                          },
			        {"iformat",     GL_RGBA8                               },
			        {"data",        flow_texture_image.data()},
			        {"width",       flow_texture_image.width()             },
			        {"height",      flow_texture_image.height()            },
			        {"min_filter",  GL_LINEAR                              },
			        {"mag_filter",  GL_LINEAR                              },
			        {"wrap_s",      GL_REPEAT                              },
			        {"wrap_t",      GL_REPEAT                              },
			        {"auto_mipmap", 0                                      },
			};
			m_flowTexture.init(attrs);
		}
	}
	void swap() { ++m_curr; }
};

class FlowSimulator {
public:
	static constexpr auto c_size = 1024;

	FlowMapArray m_array;
	FlowMapHolder m_holder;

	explicit FlowSimulator(const images::Image &flow_texture_image)
	        : m_array(c_size), m_holder(c_size, flow_texture_image)
	{
		m_array.u_flow_texture = m_holder.m_flowTexture.id();
	}

	void update(double time)
	{
		uint32_t bump_texture = m_holder.m_bumpTexture.id();
		uint32_t height_texture0
		        = m_holder.m_heightTextures[(m_holder.m_curr + 0) % m_holder.c_nhm].id();
		uint32_t height_texture1
		        = m_holder.m_heightTextures[(m_holder.m_curr + 1) % m_holder.c_nhm].id();
		uint32_t height_texture2
		        = m_holder.m_heightTextures[(m_holder.m_curr + 2) % m_holder.c_nhm].id();

		auto viewport = Rectf(0, 0, c_size, c_size);
		Attrs attrs = {
		        {"viewport0",         viewport       },
		        {"color0.texture_id", bump_texture   },
		        {"color1.texture_id", height_texture2},
		};
		SpuFrame frame(attrs);

		m_array.u_height_texture1 = height_texture0;
		m_array.u_height_texture2 = height_texture1;

		m_array.u_time = time;

		frame.begin();

		SpuScopedRenderstate renderstate(true);
		renderstate.flags.depth_test = false;
		renderstate.use();

		m_array.draw(nullptr);

		renderstate.flags.depth_test = true;
		renderstate.use();
		frame.end();

		m_holder.swap();
	}
};

class ScreenArray : public shapes::Array {
public:
	uint32_t u_background_texture;
	uint32_t u_normalmap;

	ScreenArray()
	{
		Attrs shader_attrs = {
		        {"vert", c_screen_vert},
		        {"frag", c_screen_frag},
		};

		Attrs unif_attrs = {
		        {"u_background_texture", &u_background_texture},
		        {"u_normalmap",          &u_normalmap         },
		};
		Array::initShader(shader_attrs, unif_attrs);
		Array::initArray(shapes::Screen(), {"position", "normal", "tangent", "texcoord"});
	}
};

class App : public SpuPage {
public:
	FlowSimulator m_flow;
	SpuTexture m_background;
	ScreenArray m_screenArray;

	App(const char *name)
	        : SpuPage(name, true, {0.4, 0.4, 0.4, 0.0}), m_flow(images::LoadTexture("flow_map"))
	{
	}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		auto image = images::LoadTexture("flower_glass");
		Attrs background_attrs = {
		        {"target",      GL_TEXTURE_2D             },
		        {"iformat",     GL_RGBA8                  },
		        {"data",        image.data()},
		        {"width",       image.width()             },
		        {"height",      image.height()            },
		        {"min_filter",  GL_LINEAR                 },
		        {"mag_filter",  GL_LINEAR                 },
		        {"wrap_s",      GL_MIRRORED_REPEAT        },
		        {"wrap_t",      GL_MIRRORED_REPEAT        },
		        {"auto_mipmap", 0                         },
		};
		m_background.init(background_attrs);

		m_screenArray.u_background_texture = m_background.id();
		m_screenArray.u_normalmap = m_flow.m_holder.m_bumpTexture.id();

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.use();
	}

	void render() override
	{
		auto esec = getSeconds().current();
		m_flow.update(esec);
		m_screenArray.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("027_flow");
}  // namespace
}  // namespace spu::oglplus
