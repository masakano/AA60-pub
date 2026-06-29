//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>
#include <images/filtered.hpp>
#include <shapes/torus.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec3 a_tangent;                                                                     \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_tangent;                                                                    \n"
    "out vec2 g_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       g_normal = mat3(u_nodeworld) * a_normal;                                        \n"
    "       g_tangent = mat3(u_nodeworld) * a_tangent;                                      \n"
    "       g_texcoord =                                                                    \n"
    "vec2(4.0*a_texcoord.x,2.0*a_texcoord.y+a_texcoord.x);                                  \n"
    "}                                                                                      \n"
};

const char *c_geom =  {
    "#version 330                                                                           \n"
    "#extension GL_ARB_gpu_shader5 : enable                                                 \n"
    "layout(triangles, invocations = 28) in;                                                \n"
    "layout(triangle_strip, max_vertices = 6) out;                                          \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "mat4 u_matrix = u_viewsceen * u_worldview;                                             \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "const float winding_direction = -1;                                                    \n"
    "const float shell_height = 0.1;                                                        \n"
    "in vec3 g_normal[3];                                                                   \n"
    "in vec3 g_tangent[3];                                                                  \n"
    "in vec2 g_texcoord[3];                                                                 \n"
    "out gl_PerVertex {                                                                     \n"
    "       vec4 gl_Position;                                                               \n"
    "       float gl_ClipDistance[3];                                                       \n"
    "};                                                                                     \n"
    "flat out mat3 f_position_front;                                                        \n"
    "flat out mat3 f_normal_front;                                                          \n"
    "flat out mat3 f_tangent_front;                                                         \n"
    "flat out mat3 f_texcoord_front;                                                        \n"
    "flat out vec3 f_Wfront;                                                                \n"
    "noperspective out vec3 f_barycentric;                                                  \n"
    "out vec3 f_position;                                                                   \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_tangent;                                                                    \n"
    "out vec3 f_texcoord;                                                                   \n"
    "mat3x4 get_world_pos(int id)                                                           \n"
    "{                                                                                      \n"
    "       mat3x4 result;                                                                  \n"
    "       id -= 2;                                                                        \n"
    "       if (id == -2)                                                                   \n"
    "       {                                                                               \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       result[v] = gl_in[2-v].gl_Position;                             \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       else if (id == -1)                                                              \n"
    "       {                                                                               \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       result[v] = gl_in[v].gl_Position+                               \n"
    "                               vec4(shell_height*g_normal[v], 0.0);                    \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       else                                                                            \n"
    "       {                                                                               \n"
    "               int v = id/2;                                                           \n"
    "               int w = (v+1)%3;                                                        \n"
    "               if (id % 2 == 0)                                                        \n"
    "               {                                                                       \n"
    "                       result[0] = gl_in[v].gl_Position;                               \n"
    "                       result[1] = gl_in[w].gl_Position;                               \n"
    "                       result[2] = gl_in[v].gl_Position+                               \n"
    "                           vec4(shell_height*g_normal[v], 0.0);                        \n"
    "               }                                                                       \n"
    "               else                                                                    \n"
    "               {                                                                       \n"
    "                       result[0] = gl_in[w].gl_Position;                               \n"
    "                       result[1] = gl_in[w].gl_Position+                               \n"
    "                               vec4(shell_height*g_normal[w], 0.0);                    \n"
    "                       result[2] = gl_in[v].gl_Position+                               \n"
    "                               vec4(shell_height*g_normal[v], 0.0);                    \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       return result;                                                                  \n"
    "}                                                                                      \n"
    "mat3x4 get_view_pos(mat3x4 pos)                                                        \n"
    "{                                                                                      \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "               pos[v] = u_matrix * pos[v];                                             \n"
    "       return pos;                                                                     \n"
    "}                                                                                      \n"
    "mat3 get_screen_pos(mat3x4 pos)                                                        \n"
    "{                                                                                      \n"
    "       mat3 res;                                                                       \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "               res[v] = pos[v].xyz/pos[v].w;                                           \n"
    "       return res;                                                                     \n"
    "}                                                                                      \n"
    "bool is_front_facing(mat3x4 view_pos)                                                  \n"
    "{                                                                                      \n"
    "       mat3 screen_pos = get_screen_pos(view_pos);                                     \n"
    "       return winding_direction * cross(                                               \n"
    "               screen_pos[1]-screen_pos[0],                                            \n"
    "               screen_pos[2]-screen_pos[0]                                             \n"
    "       ).z < 0.0;                                                                      \n"
    "}                                                                                      \n"
    "mat3 get_tex_coords(int id)                                                            \n"
    "{                                                                                      \n"
    "       mat3 result;                                                                    \n"
    "       id -= 2;                                                                        \n"
    "       if (id == -2)                                                                   \n"
    "       {                                                                               \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       result[v] = vec3(g_texcoord[2-v], 0.0);                         \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       else if (id == -1)                                                              \n"
    "       {                                                                               \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       result[v] = vec3(g_texcoord[v], 1.0);                           \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       else                                                                            \n"
    "       {                                                                               \n"
    "               int v = id/2;                                                           \n"
    "               int w = (v+1)%3;                                                        \n"
    "               if (id % 2 == 0)                                                        \n"
    "               {                                                                       \n"
    "                       result[0] = vec3(g_texcoord[v], 0.0);                           \n"
    "                       result[1] = vec3(g_texcoord[w], 0.0);                           \n"
    "                       result[2] = vec3(g_texcoord[v], 1.0);                           \n"
    "               }                                                                       \n"
    "               else                                                                    \n"
    "               {                                                                       \n"
    "                       result[0] = vec3(g_texcoord[w], 0.0);                           \n"
    "                       result[1] = vec3(g_texcoord[w], 1.0);                           \n"
    "                       result[2] = vec3(g_texcoord[v], 1.0);                           \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       return result;                                                                  \n"
    "}                                                                                      \n"

    "mat3 get_vectors(int id, vec3 attrib[3])                                               \n"
    "{                                                                                      \n"
    "       mat3 result;                                                                    \n"
    "       id -= 2;                                                                        \n"
    "       if (id == -2)                                                                   \n"
    "       {                                                                               \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       result[v] = attrib[2-v];                                        \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       else if (id == -1)                                                              \n"
    "       {                                                                               \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       result[v] = attrib[v];                                          \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       else                                                                            \n"
    "       {                                                                               \n"
    "               int v = id/2;                                                           \n"
    "               int w = (v+1)%3;                                                        \n"
    "               if (id % 2 == 0)                                                        \n"
    "               {                                                                       \n"
    "                       result[0] = attrib[v];                                          \n"
    "                       result[1] = attrib[w];                                          \n"
    "                       result[2] = attrib[v];                                          \n"
    "               }                                                                       \n"
    "               else                                                                    \n"
    "               {                                                                       \n"
    "                       result[0] = attrib[w];                                          \n"
    "                       result[1] = attrib[w];                                          \n"
    "                       result[2] = attrib[v];                                          \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       return result;                                                                  \n"
    "}                                                                                      \n"

    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       int ft = gl_InvocationID/4+1;                                                   \n"
    "       mat3x4 world_pos_f = get_world_pos(ft);                                         \n"
    "       mat3x4 view_pos_f = get_view_pos(world_pos_f);                                  \n"
    "       if (!is_front_facing(view_pos_f)) return;                                       \n"
    "       int bt[2];                                                                      \n"
    "       bt[0] = (gl_InvocationID%4)*2;                                                  \n"
    "       bt[1] = bt[0]+1;                                                                \n"
    "       mat3x4 world_pos[2];                                                            \n"
    "       mat3x4 view_pos[2];                                                             \n"
    "       bool front_facing[2];                                                           \n"
    "       for (int b=0; b!=2; ++b)                                                        \n"
    "       {                                                                               \n"
    "               if (ft == bt[b])                                                        \n"
    "               {                                                                       \n"
    "                       front_facing[b] = true;                                         \n"
    "               }                                                                       \n"
    "               else                                                                    \n"
    "               {                                                                       \n"
    "                       world_pos[b] = get_world_pos(bt[b]);                            \n"
    "                       view_pos[b] = get_view_pos(world_pos[b]);                       \n"
    "                       front_facing[b] = is_front_facing(view_pos[b]);                 \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       if (front_facing[0] && front_facing[1]) return;                                 \n"
    "       vec4 clip_plane[3];                                                             \n"
    "       for (int v=0; v!=3; ++v)                                                        \n"
    "       {                                                                               \n"
    "               int w = (v+1)%3;                                                        \n"
    "               vec3 p0 = world_pos_f[v].xyz;                                           \n"
    "               vec3 p1 = world_pos_f[w].xyz;                                           \n"
    "               vec3 p2 = u_eye_position;                                               \n"
    "               vec3 pv = winding_direction*normalize(cross(p1-p0, p2-p0));             \n"
    "               clip_plane[v] = vec4(pv, -dot(pv, p0));                                 \n"
    "       }                                                                               \n"
    "       vec3 lo = u_eye_position;                                                       \n"
    "       vec3 p0 = world_pos_f[0].xyz;                                                   \n"
    "       vec3 pu = world_pos_f[1].xyz-p0;                                                \n"
    "       vec3 pv = world_pos_f[2].xyz-p0;                                                \n"
    "       vec3 lp = lo-p0;                                                                \n"
    "       float w0 = view_pos_f[0].w;                                                     \n"
    "       float wu = view_pos_f[1].w-w0;                                                  \n"
    "       float wv = view_pos_f[2].w-w0;                                                  \n"
    "       mat3 normal_f = get_vectors(ft, g_normal);                                      \n"
    "       vec3 n0 = normal_f[0];                                                          \n"
    "       vec3 nu = normal_f[1]-n0;                                                       \n"
    "       vec3 nv = normal_f[2]-n0;                                                       \n"
    "       mat3 tangent_f = get_vectors(ft, g_tangent);                                    \n"
    "       vec3 t0 = tangent_f[0];                                                         \n"
    "       vec3 tu = tangent_f[1]-t0;                                                      \n"
    "       vec3 tv = tangent_f[2]-t0;                                                      \n"
    "       mat3 tex_coord_f = get_tex_coords(ft);                                          \n"
    "       vec3 tc0 = tex_coord_f[0];                                                      \n"
    "       vec3 tcu = tex_coord_f[1]-tc0;                                                  \n"
    "       vec3 tcv = tex_coord_f[2]-tc0;                                                  \n"
    "       for (int b=0; b!=2; ++b)                                                        \n"
    "       {                                                                               \n"
    "               if (front_facing[b]) continue;                                          \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       vec3 lt = world_pos[b][v].xyz;                                  \n"
    "                       mat3 im = mat3(lo-lt, pu, pv);                                  \n"
    "                       vec3 ic = inverse(im)*lp;                                       \n"
    "                       float s = ic.y;                                                 \n"
    "                       float t = ic.z;                                                 \n"
    "                       f_position_front[v] = p0+pu*s+pv*t;                             \n"
    "                       f_normal_front[v] = n0+nu*s+nv*t;                               \n"
    "                       f_tangent_front[v] = t0+tu*s+tv*t;                              \n"
    "                       f_texcoord_front[v] = tc0+tcu*s+tcv*t;                          \n"
    "                       f_Wfront[v] = w0+wu*s+wv*t;                                     \n"
    "               }                                                                       \n"
    "               mat3 normal = get_vectors(bt[b], g_normal);                             \n"
    "               mat3 tangent = get_vectors(bt[b], g_tangent);                           \n"
    "               mat3 tex_coord = get_tex_coords(bt[b]);                                 \n"
    "               for (int v=0; v!=3; ++v)                                                \n"
    "               {                                                                       \n"
    "                       gl_Position = view_pos[b][v];                                   \n"
    "                       for (int c=0; c!=3; ++c)                                        \n"
    "                       {                                                               \n"
    "                               gl_ClipDistance[c] = dot(                               \n"
    "                                       clip_plane[c],                                  \n"
    "                                       world_pos[b][v]                                 \n"
    "                               );                                                      \n"
    "                       }                                                               \n"
    "                       f_position = world_pos[b][v].xyz;                               \n"
    "                       f_normal = normal[v];                                           \n"
    "                       f_tangent = tangent[v];                                         \n"
    "                       f_texcoord = tex_coord[v];                                      \n"
    "                       f_barycentric = vec3(0.0);                                      \n"
    "                       f_barycentric[v] = 1.0;                                         \n"
    "                       EmitVertex();                                                   \n"
    "               }                                                                       \n"
    "               EndPrimitive();                                                         \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                             \n"
    "#extension GL_EXT_gpu_shader4_1 : enable                                                 \n"
    "uniform float u_time;                                                                    \n"
    "uniform sampler2D u_color_texture;                                                       \n"
    "uniform sampler2D u_bump_texture;                                                        \n"
    "uniform vec3 u_light_position;                                                           \n"
    "flat in mat3 f_position_front;                                                           \n"
    "flat in mat3 f_normal_front;                                                             \n"
    "flat in mat3 f_tangent_front;                                                            \n"
    "flat in mat3 f_texcoord_front;                                                           \n"
    "flat in vec3 f_Wfront;                                                                   \n"
    "noperspective in vec3 f_barycentric;                                                     \n"
    "in vec3 f_position;                                                                      \n"
    "in vec3 f_normal;                                                                        \n"
    "in vec3 f_tangent;                                                                       \n"
    "in vec3 f_texcoord;                                                                      \n"
    "out vec3 final_color;                                                                    \n"
    "vec3 vcdiv(vec3 a, vec3 b)                                                               \n"
    "{                                                                                        \n"
    "       return vec3(a.x/b.x, a.y/b.y, a.z/b.z);                                           \n"
    "}                                                                                        \n"
    "void main()                                                                              \n"
    "{                                                                                        \n"
    "       const vec3 one = vec3(1.0, 1.0, 1.0);                                             \n"
    "       vec3 bzfv = vcdiv(f_barycentric,f_Wfront);                                        \n"
    "       float idobzfv = 1.0/dot(one,bzfv);                                                \n"
    "       vec3 p0 = f_position;                                                             \n"
    "       vec3 p1 = (f_position_front*bzfv)*idobzfv;                                        \n"
    "       vec3 n0 = f_normal;                                                               \n"
    "       vec3 n1 = (f_normal_front*bzfv)*idobzfv;                                          \n"
    "       vec3 t0 = f_tangent;                                                              \n"
    "       vec3 t1 = (f_tangent_front*bzfv)*idobzfv;                                         \n"
    "       vec3 tc0 = f_texcoord;                                                            \n"
    "       vec3 tc1 = (f_texcoord_front*bzfv)*idobzfv;                                       \n"
    "       float tl = textureQueryLod(u_bump_texture, tc1.xy).x;                             \n"
    "       ivec2 ts = textureSize(u_bump_texture, int(tl));                                  \n"
    "       int mts = max(ts.x, ts.y);                                                        \n"
    "       vec2 dtc = tc1.xy - tc0.xy;                                                       \n"
    "       float mdtc = max(abs(dtc.x), abs(dtc.y));                                         \n"
    "       int nsam = max(min(int(mdtc*mts), mts), 1);                                       \n"
    "       float step = 1.0 / nsam;                                                          \n"
    "       for (int s=0; s<=nsam; ++s)                                                       \n"
    "       {                                                                                 \n"
    "               vec3 tc = mix(tc1, tc0, s*step);                                          \n"
    "               vec4 bm = texture(u_bump_texture, tc.xy);                                 \n"
    "               if (tc.z <= bm.w+0.01)                                                    \n"
    "               {                                                                         \n"
    "                       vec3 p = mix(p1, p0, s*step);                                     \n"
    "                       vec3 n = mix(n1, n0, s*step);                                     \n"
    "                       vec3 t = mix(t1, t0, s*step);                                     \n"
    "                       vec3 b = cross(n, t);                                             \n"
    "                       vec3 ldir = normalize(u_light_position - p);                      \n"
    "                       vec3 nml = normalize(t*bm.x+b*bm.y+n*bm.z);                       \n"
    "                       float l = max(dot(ldir, nml), 0.0)*max(dot(ldir, n)+0.3, 0.0)+0.2;\n"
    "                       final_color = texture(u_color_texture, tc.xy).rgb*l;              \n"
    "                       return;                                                           \n"
    "               }                                                                         \n"
    "       }                                                                                 \n"
    "       discard;                                                                          \n"
    "}                                                                                        \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	uint32_t u_color_texture;
	uint32_t u_bump_texture;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen",      &u_viewsceen     },
                        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
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

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"geom", c_geom},
		        {"vert", c_vert},
		};

		m_array.initShader(shader_attrs, Attrs(m_unifs));
		m_array.initArray(
		        shapes::Torus(1.0, 0.5, 72, 48), {"position", "normal", "tangent", "texcoord"});

		auto tex_image = images::LoadTexture("bricks_color_hmap");
		auto bump_map_image = images::NormalMap(tex_image, images::NormalMap::FromAlpha());

		{
			// tex_image.data(), 0,
			//};
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
			        {"iformat",     GL_RGBA8               },
			        {"width",       tex_image.width()      },
			        {"height",      tex_image.height()     },
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
			        {"wrap_s",      GL_REPEAT              },
			        {"wrap_t",      GL_REPEAT              },
			        {"data",        tex_image.data()       },
			        {"auto_mipmap", 1                      },
			};
			m_colorTexture.init(attrs);
			m_unifs.u_color_texture = m_colorTexture.id();
		}

		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
			        {"iformat",     GL_RGBA32F             },
			        {"width",       bump_map_image.width() },
			        {"height",      bump_map_image.height()},
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
			        {"wrap_s",      GL_REPEAT              },
			        {"wrap_t",      GL_REPEAT              },
			        {"data",        bump_map_image.data()  },
			        {"auto_mipmap", 1                      },
			};
			m_bumpTexture.init(attrs);
			m_unifs.u_bump_texture = m_bumpTexture.id();
		}

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = false;
		renderstate.flags.clip_distance0 = true;
		renderstate.flags.clip_distance1 = true;
		renderstate.flags.clip_distance2 = true;
	}

	void render() override
	{
		auto esec = getSeconds().current();

		// reshape
		m_unifs.u_viewsceen = math::perspective(viewport(0), 70, 1, 20);

		// Angle angle = fullCircles(esec / 23.0) * 2.0 * M_PI;
		// auto angle = esec / 23.0 * math::two_pi();
		auto angle = esec / 23.0 * math::two_pi();

		m_unifs.u_light_position
		        = Vec3f(cos(angle) * 20.0, (1.2 + sin(angle)) * 15.0, sin(angle) * 20.0);

		// auto x = sin(esec / 13.0 * math::two_pi());;
		auto x = sin(esec / 13.0 * math::two_pi());
		;
		if (x + 0.93 < 0.0) {
			// setPace(0.2);
			getSeconds().setPace(0.2);
		}
		else {
			getSeconds().setPace(1.0);
			// setPace(1.0);
		}

		auto worldview = Mat4f::orbiting(ezero(), esec, 9.5 + x * 5.1, 0, 0, 0, 17, 0, 89, 20);

		m_unifs.u_worldview = worldview;
		m_unifs.u_eye_position = worldview.unitary_inverse().c[3];

		m_unifs.u_nodeworld = math::unit().trans({+2.0, 0, 0})
		                    * math::unit().rot("x", esec / 13.0 * math::two_pi());

		m_array.draw(nullptr);
		m_unifs.u_nodeworld = math::unit().trans({-2.0, 0, 0})
		                    * math::unit().rot("z", -esec / 11.0 * math::two_pi());

		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("031_brick_torus");
}  // namespace
}  // namespace spu::oglplus
