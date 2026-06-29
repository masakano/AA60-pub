//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>
#include <images/filtered.hpp>
#include <shapes/plane.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "in vec3 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 g_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(a_position, 1.0);                                            \n"
    "       g_texcoord = a_texcoord;                                                        \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "#extension GL_ARB_gpu_shader5 : enable                                                 \n"
    "layout(triangles, invocations = 7) in;                                                 \n"
    "layout(triangle_strip, max_vertices = 21) out;                                         \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "mat4 u_matrix = u_viewsceen * u_worldview;                                             \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "in vec2 g_texcoord[3];                                                                 \n"
    "out gl_PerVertex {                                                                     \n"
    "       vec4 gl_Position;                                                               \n"
    "       float gl_ClipDistance[3];                                                       \n"
    "};                                                                                     \n"
    "flat out mat3 f_position_front;                                                        \n"
    "flat out mat3 f_texcoord_front;                                                        \n"
    "flat out vec3 f_wfront;                                                                \n"
    "noperspective out vec3 f_barycentric;                                                  \n"
    "out vec3 f_position;                                                                   \n"
    "out vec3 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 world_pos[8*3];                                                            \n"
    "       vec3 tex_coord[8*3];                                                            \n"
    "       vec4 view_pos[8*3];                                                             \n"
    "       vec3 screen_pos[8*3];                                                           \n"
    "       bool front_facing[8];                                                           \n"
    "       int ft = gl_InvocationID+1;                                                     \n"
    "       for (int pass=0; pass!=2; ++pass)                                               \n"
    "       {                                                                               \n"
    "               bool first = pass == 0;                                                 \n"
    "               if (((ft == 0) && first) || (((ft != 0) && !first)))                    \n"
    "               {                                                                       \n"
    "                       for (int v=0; v!=3; ++v)                                        \n"
    "                       {                                                               \n"
    "                               int w = 2-v;                                            \n"
    "                               world_pos[0+v] = gl_in[w].gl_Position;                  \n"
    "                               tex_coord[0+v] = vec3(g_texcoord[w], 0.0);              \n"
    "                       }                                                               \n"
    "               }                                                                       \n"
    "               vec4 n = vec4(-0.15 * normalize(cross(                                  \n"
    "                       gl_in[1].gl_Position.xyz-gl_in[0].gl_Position.xyz,              \n"
    "                       gl_in[2].gl_Position.xyz-gl_in[0].gl_Position.xyz               \n"
    "               )), 0.0);                                                               \n"
    "               if (((ft == 1) && first) || (((ft != 1) && !first)))                    \n"
    "               {                                                                       \n"
    "                       for (int v=0; v!=3; ++v)                                        \n"
    "                       {                                                               \n"
    "                               world_pos[3+v] = gl_in[v].gl_Position + n;              \n"
    "                               tex_coord[3+v] = vec3(g_texcoord[v], 1.0);              \n"
    "                       }                                                               \n"
    "               }                                                                       \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       int w = (v+1)%3;                                                \n"
    "                       int k = 2+2*v;                                                  \n"
    "                       if (((ft == k) && first) || (((ft != k) && !first)))            \n"
    "                       {                                                               \n"
    "                               world_pos[6+0+v*6] = gl_in[v].gl_Position;              \n"
    "                               tex_coord[6+0+v*6] = vec3(g_texcoord[v], 0.0);          \n"
    "                               world_pos[6+1+v*6] = gl_in[w].gl_Position;              \n"
    "                               tex_coord[6+1+v*6] = vec3(g_texcoord[w], 0.0);          \n"
    "                               world_pos[6+2+v*6] = gl_in[v].gl_Position + n;          \n"
    "                               tex_coord[6+2+v*6] = vec3(g_texcoord[v], 1.0);          \n"
    "                       }                                                               \n"
    "                       k = 3+2*v;                                                      \n"
    "                       if (((ft == k) && first) || (((ft != k) && !first)))            \n"
    "                       {                                                               \n"
    "                               world_pos[6+3+v*6] = gl_in[w].gl_Position;              \n"
    "                               tex_coord[6+3+v*6] = vec3(g_texcoord[w], 0.0);          \n"
    "                               world_pos[6+4+v*6] = gl_in[w].gl_Position + n;          \n"
    "                               tex_coord[6+4+v*6] = vec3(g_texcoord[w], 1.0);          \n"
    "                               world_pos[6+5+v*6] = gl_in[v].gl_Position + n;          \n"
    "                               tex_coord[6+5+v*6] = vec3(g_texcoord[v], 1.0);          \n"
    "                       }                                                               \n"
    "               }                                                                       \n"
    "               for (int t=first?ft:0; t!=8; ++t)                                       \n"
    "               {                                                                       \n"
    "                       if (!first && (t == ft)) continue;                              \n"
    "                       int o = t*3;                                                    \n"
    "                       for (int v=0; v!=3; ++v)                                        \n"
    "                       {                                                               \n"
    "                               int w = o+v;                                            \n"
    "                               view_pos[w] = u_matrix * world_pos[w];                  \n"
    "                               screen_pos[w] = view_pos[w].xyz/view_pos[w].w;          \n"
    "                       }                                                               \n"
    "                       front_facing[t] = cross(                                        \n"
    "                                       screen_pos[o+1]-screen_pos[o+0],                \n"
    "                                       screen_pos[o+2]-screen_pos[o+0]                 \n"
    "                       ).z < 0.0;                                                      \n"
    "                       if (first) break;                                               \n"
    "               }                                                                       \n"
    "               if (first && !front_facing[ft]) return;                                 \n"
    "       }                                                                               \n"
    "       int o = ft*3;                                                                   \n"
    "       vec4 clip_plane[3];                                                             \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "       {                                                                               \n"
    "               int w = (v+1)%3;                                                        \n"
    "               vec3 p0 = world_pos[o+v].xyz;                                           \n"
    "               vec3 p1 = world_pos[o+w].xyz;                                           \n"
    "               vec3 p2 = u_eye_position;                                               \n"
    "               vec3 pv = normalize(cross(p1-p0, p2-p0));                               \n"
    "               clip_plane[v] = vec4(pv, -dot(pv, p0));                                 \n"
    "       }                                                                               \n"
    "       vec3 lo = u_eye_position;                                                       \n"
    "       vec3 p0 = world_pos[o+0].xyz;                                                   \n"
    "       vec3 pu = world_pos[o+1].xyz-p0;                                                \n"
    "       vec3 pv = world_pos[o+2].xyz-p0;                                                \n"
    "       vec3 lp = lo-p0;                                                                \n"
    "       float w0 = view_pos[o+0].w;                                                     \n"
    "       float wu = view_pos[o+1].w-w0;                                                  \n"
    "       float wv = view_pos[o+2].w-w0;                                                  \n"
    "       vec3 t0 = tex_coord[o+0];                                                       \n"
    "       vec3 tu = tex_coord[o+1]-t0;                                                    \n"
    "       vec3 tv = tex_coord[o+2]-t0;                                                    \n"
    "       for (int bt=0; bt!=8; ++bt)                                                     \n"
    "       {                                                                               \n"
    "               int k = bt*3;                                                           \n"
    "               if ((ft != bt) && !front_facing[bt])                                    \n"
    "               {                                                                       \n"
    "                       for (int v=0; v!=3; ++v)                                        \n"
    "                       {                                                               \n"
    "                               vec3 lt = world_pos[k+v].xyz;                           \n"
    "                               mat3 im = mat3(lo-lt, pu, pv);                          \n"
    "                               vec3 ic = inverse(im)*lp;                               \n"
    "                               float s = ic.y;                                         \n"
    "                               float t = ic.z;                                         \n"
    "                               f_position_front[v] = p0+pu*s+pv*t;                     \n"
    "                               f_texcoord_front[v] = t0+tu*s+tv*t;                     \n"
    "                               f_wfront[v] = w0+wu*s+wv*t;                             \n"
    "                       }                                                               \n"
    "                       for (int v=0; v!=3; ++v)                                        \n"
    "                       {                                                               \n"
    "                               int w = k+v;                                            \n"
    "                               gl_Position = view_pos[w];                              \n"
    "                               for (int c=0; c!=3; ++c)                                \n"
    "                               {                                                       \n"
    "                                       gl_ClipDistance[c] = dot(                       \n"
    "                                               clip_plane[c],                          \n"
    "                                               world_pos[w]                            \n"
    "                                       );                                              \n"
    "                               }                                                       \n"
    "                               f_position = world_pos[w].xyz;                          \n"
    "                               f_texcoord = tex_coord[w];                              \n"
    "                               f_barycentric = vec3(0.0);                              \n"
    "                               f_barycentric[v] = 1.0;                                 \n"
    "                               EmitVertex();                                           \n"
    "                       }                                                               \n"
    "                       EndPrimitive();                                                 \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_frag =       {
    "#version 330                                                                           \n"

    "uniform float u_time;                                                                  \n"
    "uniform sampler2D u_color_texture;                                                     \n"
    "uniform sampler2D u_bump_texture;                                                      \n"
    "uniform vec3 u_light_position;                                                         \n"

    "flat in mat3 f_position_front;                                                         \n"
    "flat in mat3 f_texcoord_front;                                                         \n"
    "flat in vec3 f_wfront;                                                                 \n"
    "noperspective in vec3 f_barycentric;                                                   \n"
    "in vec3 f_position;                                                                    \n"
    "in vec3 f_texcoord;                                                                    \n"

    "out vec3 final_color;                                                                  \n"

    "vec3 vcdiv(vec3 a, vec3 b)                                                             \n"
    "{                                                                                      \n"
    "       return vec3(a.x/b.x, a.y/b.y, a.z/b.z);                                         \n"
    "}                                                                                      \n"

    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       const vec3 one = vec3(1.0, 1.0, 1.0);                                           \n"
    "       vec3 bzfv = vcdiv(f_barycentric,f_wfront);                                      \n"
    "       vec3 p0 = f_position;                                                           \n"
    "       vec3 p1 = (f_position_front*bzfv)/dot(one,bzfv);                                \n"
    "       vec3 tc0 = f_texcoord;                                                          \n"
    "       vec3 tc1 = (f_texcoord_front*bzfv)/dot(one,bzfv);                               \n"
    "       ivec2 ts = textureSize(u_bump_texture, 0);                                      \n"
    "       int mts = max(ts.x, ts.y);                                                      \n"
    "       vec2 dtc = tc1.xy - tc0.xy;                                                     \n"
    "       float mdtc = max(abs(dtc.x), abs(dtc.y));                                       \n"
    "       int nsam = max(min(int(mdtc*mts), mts/2), 1);                                   \n"
    "       float step = 1.0 / nsam;                                                        \n"
    "       for (int s=0; s<=nsam; ++s)                                                     \n"
    "       {                                                                               \n"
    "               vec3 tc = mix(tc1, tc0, s*step);                                        \n"
    "               vec4 bm = texture(u_bump_texture, tc.xy);                               \n"
    "               if (tc.z <= bm.w)                                                       \n"
    "               {                                                                       \n"
    "                       vec3 p = mix(p1, p0, s*step);                                   \n"
    "                       vec3 ldir = normalize(u_light_position - p);                    \n"
    "                       float l = max(dot(ldir, bm.xzy), 0.0)*1.3;                      \n"
    "                       final_color = texture(u_color_texture, tc.xy).rgb*l;            \n"
    "                       return;                                                         \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       discard;                                                                        \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	uint32_t u_color_texture;
	uint32_t u_bump_texture;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen",      &u_viewsceen     },
                        {"u_worldview",      &u_worldview     },
		        {"u_eye_position",   &u_eye_position  },
                        {"u_light_position", &u_light_position},
		        {"u_color_texture",  &u_color_texture },
                        {"u_bump_texture",   &u_bump_texture  },
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class App : public SpuPage {
public:
	Uniforms m_unifs;
	shapes::Array m_array;
	SpuTexture m_colorTexture;
	SpuTexture m_bumpTexture;

	shapes::Plane makeShape() { return {ezero(), ex(), -ez(), 32, 32}; }

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"vert", c_vert},
                        {"geom", c_geom},
                        {"frag", c_frag}
                };

		m_array.initShader(shader_attrs, Attrs(m_unifs));
		m_array.initArray(makeShape(), {"position", "texcoord"});

		auto tex_image = images::LoadTexture("stones_color_hmap");
		auto normal_image = images::NormalMap(tex_image, images::NormalMap::FromAlpha());

		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
                                {"iformat",     GL_RGBA8               },
			        {"data",        tex_image.data()       },
                                {"width",       tex_image.width()      },
			        {"height",      tex_image.height()     },
                                {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
                                {"wrap_s",      GL_REPEAT              },
			        {"wrap_t",      GL_REPEAT              },
                                {"auto_mipmap", 1                      },
			};
			m_colorTexture.init(attrs);
			m_unifs.u_color_texture = m_colorTexture.id();
		}

		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
			        {"iformat",     GL_RGBA32F             },
			        {"data",        normal_image.data()    },
			        {"width",       normal_image.width()   },
			        {"height",      normal_image.height()  },
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
			        {"wrap_s",      GL_REPEAT              },
			        {"wrap_t",      GL_REPEAT              },
			        {"auto_mipmap", 1                      },
			};
			m_bumpTexture.init(attrs);
			m_unifs.u_bump_texture = m_bumpTexture.id();
		}

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto angle = esec / 31.0 * math::two_pi();

		m_unifs.u_viewsceen = math::perspective(viewport(0), 70, 1, 20);
		m_unifs.u_worldview = Mat4f::orbiting(ezero(), esec, 2.9, 0, 0, 0, 17, 45, 40, 20);
		m_unifs.u_eye_position = m_unifs.u_worldview.unitary_inverse().c[3];
		m_unifs.u_light_position
		        = Vec3f(cos(angle) * 20.0, (1.2 + sin(angle)) * 15.0, sin(angle) * 20.0);
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("029_muddy_stones");
}  // namespace
}  // namespace spu::oglplus
