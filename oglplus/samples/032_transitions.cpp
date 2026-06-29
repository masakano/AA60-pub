//
// DrawArray :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/brushed_metal.hpp>
#include <images/random.hpp>
#include <shapes/wicker_torus.hpp>

#include <cstdlib>
#include <ctime>

#include <spu/spu.h>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec3 a_tangent;                                                                     \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_tangent;                                                                    \n"
    "out vec3 g_bitangent;                                                                  \n"
    "out vec3 g_light_dir;                                                                  \n"
    "out vec2 g_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_nodeworld *                                                           \n"
    "               a_position;                                                             \n"
    "       g_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       g_normal = mat3(u_nodeworld) * a_normal;                                        \n"
    "       g_tangent = mat3(u_nodeworld)* a_tangent;                                       \n"
    "       g_bitangent = cross(g_normal, g_tangent);                                       \n"
    "       g_texcoord = a_texcoord;                                                        \n"
    "}                                                                                      \n"
};

const char *c_shape_geom =  {
    "#version 330                                                                             \n"
    "layout(triangles) in;                                                                    \n"
    "layout(triangle_strip, max_vertices = 6) out;                                            \n"
    "uniform mat4 u_worldscreen[2];                                                           \n"
    "uniform vec3 u_eye_position[2];                                                          \n"
    "uniform vec3 u_color[2];                                                                 \n"
    "in vec3 g_normal[];                                                                      \n"
    "in vec3 g_tangent[];                                                                     \n"
    "in vec3 g_bitangent[];                                                                   \n"
    "in vec3 g_light_dir[];                                                                   \n"
    "in vec2 g_texcoord[];                                                                    \n"
    "out vec3 f_normal;                                                                       \n"
    "out vec3 f_tangent;                                                                      \n"
    "out vec3 f_bitangent;                                                                    \n"
    "out vec3 f_light_dir;                                                                    \n"
    "out vec3 f_view_dir;                                                                     \n"
    "out vec3 f_color;                                                                        \n"
    "out vec2 f_texcoord;                                                                     \n"
    "void main()                                                                              \n"
    "{                                                                                        \n"
    "       for (int l=0; l!=2; ++l)                                                          \n"
    "       {                                                                                 \n"
    "               gl_Layer = l;                                                             \n"
    "               f_color = u_color[l];                                                     \n"
    "               for (int v=0; v!=3; ++v)                                                  \n"
    "               {                                                                         \n"
    "                       f_normal = g_normal[v];                                           \n"
    "                       f_tangent = g_tangent[v];                                         \n"
    "                       f_bitangent = g_bitangent[v];                                     \n"
    "                       f_texcoord = g_texcoord[v];                                       \n"
    "                       f_light_dir = g_light_dir[v];                                     \n"
    "                       f_view_dir =                                                      \n"
    "                               u_eye_position[l] -                                       \n"
    "                               gl_in[v].gl_Position.xyz;                                 \n"
    "                       gl_Position =                                                     \n"
    "                               u_worldscreen[l] *                                        \n"
    "                               gl_in[v].gl_Position;                                     \n"
    "                       EmitVertex();                                                     \n"
    "               }                                                                         \n"
    "               EndPrimitive();                                                           \n"
    "       }                                                                                 \n"
    "}                                                                                        \n"
};

const char *c_shape_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_metal_texture;                                                     \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_tangent;                                                                     \n"
    "in vec3 f_bitangent;                                                                   \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec3 f_color;                                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec2 tex_coord = vec2(f_texcoord.s*16, f_texcoord.t*4);                         \n"
    "       vec3 t = texture(u_metal_texture, tex_coord).rgb;                               \n"
    "       vec3 normal = normalize(                                                        \n"
    "               2*f_normal +                                                            \n"
    "               (t.r - 0.5)*f_tangent +                                                 \n"
    "               (t.g - 0.5)*f_bitangent                                                 \n"
    "       );                                                                              \n"
    "       vec3 light_refl = reflect(                                                      \n"
    "               -normalize(f_light_dir),                                                \n"
    "               normal                                                                  \n"
    "       );                                                                              \n"
    "       float diffuse = max(dot(                                                        \n"
    "               normalize(normal),                                                      \n"
    "               normalize(f_light_dir)                                                  \n"
    "       ), 0.0);                                                                        \n"
    "       float specular = pow(max(dot(                                                   \n"
    "               normalize(light_refl),                                                  \n"
    "               normalize(f_view_dir)                                                   \n"
    "       )+0.1, 0.0), 8+t.b*24);                                                         \n"
    "       vec3 light_color = vec3(1.0, 1.0, 1.0);                                         \n"
    "       vec3 shape_color = mix(f_color, light_color, t.b);                              \n"
    "       final_color =                                                                   \n"
    "               shape_color*(diffuse+0.4) +                                             \n"
    "               light_color*specular*pow(0.6+t.b*0.8, 4.0);                             \n"
    "}                                                                                      \n"
};

const char *c_clear_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "}                                                                                      \n"
};

const char *c_clear_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 6) out;                                          \n"
    "uniform vec3 u_color1[2];                                                              \n"
    "uniform vec3 u_color2[2];                                                              \n"
    "uniform vec2 u_origin[2];                                                              \n"
    "out vec3 f_color1, f_color2;                                                           \n"
    "out vec2 f_position, f_origin;                                                         \n"
    "flat out int f_layer;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       for (gl_Layer=0; gl_Layer!=2; ++gl_Layer)                                       \n"
    "       {                                                                               \n"
    "               f_color1 = u_color1[gl_Layer];                                          \n"
    "               f_color2 = u_color2[gl_Layer];                                          \n"
    "               f_origin = u_origin[gl_Layer];                                          \n"
    "               f_layer = gl_Layer;                                                     \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       gl_Position = gl_in[v].gl_Position;                             \n"
    "                       f_position = gl_Position.xy;                                    \n"
    "                       EmitVertex();                                                   \n"
    "               }                                                                       \n"
    "               EndPrimitive();                                                         \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_clear_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_color1, f_color2;                                                            \n"
    "in vec2 f_position, f_origin;                                                          \n"
    "flat in int f_layer;                                                                   \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec2 v = f_position - f_origin;                                                 \n"
    "       float l = length(v);                                                            \n"
    "       float a = atan(v.y, v.x)/3.1415;                                                \n"
    "       float d = 32-f_layer*15;                                                        \n"
    "       if (l < 0.01)                                                                   \n"
    "               final_color = f_color1;                                                 \n"
    "       else if (int(d*(1.0 + a)) % 2 == 0)                                             \n"
    "               final_color = f_color1 * 0.9 + l*0.1;                                   \n"
    "       else                                                                            \n"
    "               final_color = f_color2 * 0.8 + pow(l,1.4)*0.2;                          \n"
    "}                                                                                      \n"
};

const char *c_trans_vert =  {
    "#version 400                                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "out vec2 g_position, g_texcoord;                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       g_position = a_position.xy;                                                     \n"
    "       g_texcoord = vec2(                                                              \n"
    "               (a_position.x*0.5 + 0.5),                                               \n"
    "               (a_position.y*0.5 + 0.5)                                                \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_trans_frag = {
    "#version 400                                                                           \n"
    "uniform sampler2DArray u_frame_texture;                                                \n"
    "uniform sampler2D u_noise_texture;                                                     \n"
    "uniform float u_mix_factor;                                                            \n"
    "uniform bool u_direction;                                                              \n"
    "subroutine vec4 u_transition_func();                                                   \n"
    "subroutine uniform u_transition_func u_transition;                                     \n"
    "in vec2 g_position, g_texcoord;                                                        \n"
    "out vec4 final_color;                                                                  \n"
    "int frame0() { return u_direction?1:0; }                                               \n"
    "int frame1() { return u_direction?0:1; }                                               \n"
    "float factor() { return u_direction?1.0-u_mix_factor:u_mix_factor; }                   \n"
    "vec4 texel0()                                                                          \n"
    "{                                                                                      \n"
    "       return texelFetch(u_frame_texture, ivec3(gl_FragCoord.xy, frame0()), 0);        \n"
    "}                                                                                      \n"
    "vec4 texel1()                                                                          \n"
    "{                                                                                      \n"
    "       return texelFetch(u_frame_texture, ivec3(gl_FragCoord.xy, frame1()), 0);        \n"
    "}                                                                                      \n"
    "vec4 mix_texels(float factor)                                                          \n"
    "{                                                                                      \n"
    "       return mix(texel0(), texel1(), factor);                                         \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 vertical()                                                                        \n"
    "{                                                                                      \n"
    "       return mix_texels((g_texcoord.y > factor())?0.0:1.0);                           \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 horizontal()                                                                      \n"
    "{                                                                                      \n"
    "       return mix_texels((g_texcoord.x > factor())?0.0:1.0);                           \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 spiral()                                                                          \n"
    "{                                                                                      \n"
    "       vec2 v = g_position.xy*31.0;                                                    \n"
    "       float l = sqrt(length(v));                                                      \n"
    "       float t = atan(v.y, v.x)/3.1415;                                                \n"
    "       return mix_texels((fract(l+t) > factor())?0.0:1.0);                             \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 rings()                                                                           \n"
    "{                                                                                      \n"
    "       vec2 v = g_position.xy*12.0;                                                    \n"
    "       float l = length(v);                                                            \n"
    "       return mix_texels((fract(l) > factor())?0.0:1.0);                               \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 sweep_fade()                                                                      \n"
    "{                                                                                      \n"
    "       vec2 v = g_texcoord;                                                            \n"
    "       float l = length(v);                                                            \n"
    "       float a = atan(v.y, v.x)/3.1415;                                                \n"
    "       return mix_texels(clamp(2*factor() - a, 0.0, 1.0));                             \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 center_fan()                                                                      \n"
    "{                                                                                      \n"
    "       vec2 v = g_position;                                                            \n"
    "       float l = length(v);                                                            \n"
    "       float a = atan(-v.y, v.x)/3.1415;                                               \n"
    "       return mix_texels((fract(9*a) > factor())?0.0:1.0);                             \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 corner_fan()                                                                      \n"
    "{                                                                                      \n"
    "       vec2 v = g_texcoord;                                                            \n"
    "       float l = length(v);                                                            \n"
    "       float a = atan(-v.y, v.x)/3.1415;                                               \n"
    "       return mix_texels((fract(15*a) > factor())?0.0:1.0);                            \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 rectangle()                                                                       \n"
    "{                                                                                      \n"
    "       float x = (abs(g_position.x)<factor())?1.0:0.0;                                 \n"
    "       float y = (abs(g_position.y)<factor())?1.0:0.0;                                 \n"
    "       return mix_texels(x*y);                                                         \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 ellipse_fade()                                                                    \n"
    "{                                                                                      \n"
    "       float t = length(g_position)/sqrt(2.0);                                         \n"
    "       return mix_texels(clamp(2*factor()-t, 0.0, 1.0));                               \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 ellipse()                                                                         \n"
    "{                                                                                      \n"
    "       float t = length(g_position)/sqrt(2.0);                                         \n"
    "       return mix_texels((factor()>t)?1.0:0.0);                                        \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 circles()                                                                         \n"
    "{                                                                                      \n"
    "       int a = 16;                                                                     \n"
    "       int s = 2*a;                                                                    \n"
    "       float r = sqrt(2.0)*a;                                                          \n"
    "       vec2 fc = gl_FragCoord.xy;                                                      \n"
    "       float cv = length(vec2(int(fc.x) % s - a, int(fc.y) % s - a)/r);                \n"
    "       float tc = g_texcoord.x * g_texcoord.y;                                         \n"
    "       return mix_texels(clamp(3*factor()-tc-cv, 0.0, 1.0));                           \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 blend()                                                                           \n"
    "{                                                                                      \n"
    "       return mix_texels(factor());                                                    \n"
    "}                                                                                      \n"
    "subroutine(u_transition_func)                                                          \n"
    "vec4 diag_dissolve()                                                                   \n"
    "{                                                                                      \n"
    "       float t = texture(u_noise_texture, g_texcoord).r;                               \n"
    "       float tc = g_texcoord.x*g_texcoord.y;                                           \n"
    "       return mix_texels(clamp(3*factor()-tc-t, 0.0, 1.0));                            \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = u_transition();                                                   \n"
    "}                                                                                      \n"
};

/* clang-format on */
class DrawArray : public shapes::Array {
public:
	Mat4f u_worldscreen[2];
	Mat4f u_nodeworld;
	Vec3f u_eye_position[2];
	Vec3f u_color[2];
	Vec3f u_light_position;
	uint32_t u_metal_texture;

	DrawArray()
	{
		Attrs shader_attrs = {
		        {"frag", c_shape_frag},
		        {"vert", c_shape_vert},
		        {"geom", c_shape_geom},
		};

		Attrs unif_attrs = {
		        {"u_worldscreen",    &u_worldscreen[0] },
		        {"u_eye_position",   &u_eye_position[0]},
		        {"u_nodeworld",      &u_nodeworld      },
		        {"u_light_position", &u_light_position },
		        {"u_color",          &u_color[0]       },
		        {"u_metal_texture",  &u_metal_texture  },
		};
		Array::initShader(shader_attrs, unif_attrs);
	}
};

class ClearShader : public SpuShader {
public:
	Vec3f u_color1[2];
	Vec3f u_color2[2];
	Vec2f u_origin[2];

	ClearShader()
	{
		Attrs shader_attrs = {
		        {"frag", c_clear_frag},
		        {"vert", c_clear_vert},
		        {"geom", c_clear_geom},
		};

		Attrs unif_attrs = {
		        {"u_color1", &u_color1[0]},
		        {"u_color2", &u_color2[0]},
		        {"u_origin", &u_origin[0]},
		};
		shapes::loadShader(*this, shader_attrs, unif_attrs);
	}
};

class TransitionShader : public SpuShader {
public:
	uint32_t u_frame_texture;
	uint32_t u_noise_texture;
	float u_mix_factor;
	uint32_t u_direction;
	int32_t u_transition;

	float m_prevMixFactor;
	int32_t m_transitionLocs[16];
	int32_t m_transitionCount;

	TransitionShader()
	{
		Attrs shader_attrs = {
		        {"frag", c_trans_frag},
		        {"vert", c_trans_vert},
		};

		Attrs unif_attrs = {
		        {"u_frame_texture", &u_frame_texture},
                        {"u_noise_texture", &u_noise_texture},
		        {"u_mix_factor",    &u_mix_factor   },
                        {"u_direction",     &u_direction    },
		        {"u_transition",    &u_transition   },
		};
		shapes::loadShader(*this, shader_attrs, unif_attrs);

		m_prevMixFactor = 0;
		setMixFactor(0.0);

		std::vector<const char *> transitions = {
		        "vertical",   "horizontal", "spiral",        "rings",        "sweep_fade",
		        "center_fan", "corner_fan", "rectangle",     "ellipse_fade", "ellipse",
		        "circles",    "blend",      "diag_dissolve",
		};

		u_transition = 0;
		m_transitionCount = 13;
		spu_shader_loc(
		        id(), transitions.data(), m_transitionLocs, nullptr, nullptr, transitions.size());
	}

	void nextTransition()
	{
		auto next = 0u;
		u_transition = m_transitionLocs[next];
		u_direction = rand() % 2;
	}

	void setMixFactor(float factor)
	{
		const float margin = 0.2;

		factor -= margin;
		factor /= (1.0 - 2.0 * margin);

		if (factor <= 0.0) {
			if (m_prevMixFactor > 0.0) {
				nextTransition();
			}
			factor = 0.0;
		}
		else if (factor >= 1.0) {
			if (m_prevMixFactor < 1.0) {
				nextTransition();
			}
			factor = 1.0;
		}
		u_mix_factor = factor;
		m_prevMixFactor = factor;
	}
};

class Screen : public SpuArray {
public:
	explicit Screen(const SpuShader &shader)
	{
		std::vector<vec2f_t> vertices = {
		        {-1.0, -1.0},
		        {-1.0, +1.0},
		        {+1.0, -1.0},
		        {+1.0, +1.0},
		};
		Attrs attrs = {
		        {"shader_id",    shader.id()    },
		        {"a.a_position", 2              },
		        {"nelem",        vertices.size()},
		        {"data",         vertices.data()},
		};
		SpuArray::init(attrs);
	}

	void draw() { SpuArray::draw(GL_TRIANGLE_STRIP); }
};

class Frame : public SpuFrame {
public:
	Frame(float width, float height)
	{
		auto viewport = Rectf(0, 0, width, height);
		Attrs attrs = {
		        {"viewport0",         viewport             },
		        {"depth.target",      GL_TEXTURE_2D_ARRAY  },
		        {"depth.iformat",     GL_DEPTH_COMPONENT32F},
		        {"depth.depth",       2                    },
		        {"depth.min_filter",  GL_LINEAR            },
		        {"depth.mag_filter",  GL_LINEAR            },
		        {"depth.wrap_s",      GL_CLAMP_TO_EDGE     },
		        {"depth.wrap_t",      GL_CLAMP_TO_EDGE     },
		        {"color0.target",     GL_TEXTURE_2D_ARRAY  },
		        {"color0.iformat",    GL_RGB8              },
		        {"color0.depth",      2                    },
		        {"color0.min_filter", GL_LINEAR            },
		        {"color0.mag_filter", GL_LINEAR            },
		        {"color0.wrap_s",     GL_CLAMP_TO_EDGE     },
		        {"color0.wrap_t",     GL_CLAMP_TO_EDGE     },
		};
		SpuFrame::init(attrs);
	}
};

class App : public SpuPage {
public:
	ClearShader m_clearShader;
	TransitionShader m_transitionShader;
	Screen m_screen;

	DrawArray m_torus;
	Frame m_frame;
	Mat4f m_viewscreen0;
	Mat4f m_viewscreen1;

	SpuTexture m_metalTexture;
	SpuTexture m_noiseTexture;

	App(const char *name) : SpuPage(name, true), m_screen(m_clearShader), m_frame(256, 256) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		auto torus_shape = shapes::WickerTorus(1.5, 1.0, 0.03, 18, 36);
		m_torus.initArray(torus_shape, {"position", "normal", "tangent", "texcoord"});

		generateMetalTexture();
		generateNoiseTexture();

		m_torus.u_metal_texture = m_metalTexture.id();

		m_torus.u_light_position = {10.0, 30.0, 20.0};
		m_torus.u_color[0] = {0.7, 0.1, 0.2};
		m_torus.u_color[1] = {0.2, 0.1, 0.7};

		m_clearShader.u_color1[0] = {0.9, 0.4, 0.5};
		m_clearShader.u_color2[0] = {1.0, 0.5, 0.6};
		m_clearShader.u_color1[1] = {0.5, 0.4, 0.9};
		m_clearShader.u_color2[1] = {0.6, 0.5, 1.0};

		m_transitionShader.u_noise_texture = m_noiseTexture.id();

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.cull_face = false /*1*/;
		renderstate.cull_face = GL_BACK;
		// renderstate.use();

		{
			m_frame = Frame(int32_t(viewport(0).sx), int32_t(viewport(0).sy));
			m_transitionShader.u_frame_texture = m_frame.getBuffer("color0").id();
			m_viewscreen0 = math::perspective(viewport(0), 70, 1, 20);
			m_viewscreen1 = math::perspective(viewport(0), 24, 1, 40);
		}
	}

	void renderFrames()
	{
		auto origin = Vec3f(0.0, 0.0, 0.0);
		auto target0 = Vec3f(0.0, 1.5, 0.0);

		auto esec = getSeconds().current();
		auto worldview0 = Mat4f::orbiting(target0, esec, 4, 2, 11, 0, 19, 35, 30, 20);
		auto worldscreen0 = m_viewscreen0 * worldview0;

		m_torus.u_worldscreen[0] = worldscreen0;
		m_torus.u_eye_position[0] = worldview0.unitary_inverse().c[3];

		m_clearShader.u_origin[0] = worldscreen0 * Vec4f(origin, 1.0);

		auto target1 = Vec3f(0.0, -0.1, 0.1);
		auto worldview1 = Mat4f::orbiting(target1, esec, 12.5, 0, 0, 0, 37, 0, 85, 11);
		auto worldscreen1 = m_viewscreen1 * worldview1;

		m_torus.u_worldscreen[1] = worldscreen1;
		m_torus.u_eye_position[1] = worldview1.unitary_inverse().c[3];

		m_clearShader.u_origin[1] = worldscreen1 * Vec4f(origin, 1.0);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = false;
		renderstate.use();

		m_clearShader.use();
		m_screen.draw();

		renderstate.flags.depth_test = true;
		renderstate.use();

		m_frame.set("bgcolor0", Vec4f(-1));
		m_frame.clear();

		m_torus.u_nodeworld = math::unit().rot("Xy", 25.0, -esec / 7.0 * math::two_pi())
		                    * math::unit().trans(-ez());

		m_torus.draw(nullptr);
	}

	void mergeFrames()
	{
		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.cull_face = false;
		renderstate.flags.depth_test = false;
		renderstate.use();

		auto bgcolor = Vec4f(1, 0, 0, 1);
		Attrs frame_attrs = {
		        {"bgcolor0", bgcolor},
		        {"bgdepth",  -1.0   },
		};

		renderstate.use();
		SpuPage::set(frame_attrs);

		auto esec = getSeconds().current();
		m_transitionShader.setMixFactor(-cos(esec / 6.0 * math::two_pi()) * 0.5 + 0.5);
		m_transitionShader.use();
		m_screen.draw();
	}

	void render() override
	{
		m_frame.begin();
		renderFrames();
		m_frame.end();
		mergeFrames();
	}

	void generateMetalTexture()
	{
		auto image = images::BrushedMetalUByte(512, 512, 5120, -32, 32, 16, 32);

		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D          },
		        {"iformat",    GL_RGB8                }, // need functional
		        {"width",      image.width()          },
		        {"height",     image.height()         },
		        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter", GL_LINEAR              },
		        {"wrap_s",     GL_REPEAT              },
		        {"wrap_t",     GL_REPEAT              },
		        {"data",       image.data()           },
		};
		m_metalTexture.init(attrs);
	}

	void generateNoiseTexture()
	{
		auto image = images::RandomRedUByte(512, 512);

		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D },
                        {"iformat",    GL_R8         }, // need functional
		        {"width",      image.width() },
                        {"height",     image.height()},
                        {"min_filter", GL_LINEAR     },
		        {"mag_filter", GL_LINEAR     },
                        {"wrap_s",     GL_REPEAT     },
                        {"wrap_t",     GL_REPEAT     },
		        {"data",       image.data()  },
		};
		m_noiseTexture.init(attrs);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("032_transitions");
}  // namespace
}  // namespace spu::oglplus
