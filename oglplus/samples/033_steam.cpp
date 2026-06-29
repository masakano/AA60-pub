//
// Uniforms :
//
#include <spu++/spu_page.h>
#include <images/cloud.hpp>
#include <images/random.hpp>
#include <shapes/obj_mesh.hpp>
#include <shapes/screen.hpp>
#include <shapes/array.hpp>
#include <math/matrix.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_mesh_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "const vec3 back_light = vec3(0.0, 0.0,-20.0);                                          \n"
    "const vec3 ambi_light = vec3(0.0, 0.0, 7.0);                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_backlight_dir;                                                              \n"
    "out vec3 g_ambilight_dir;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       g_normal = mat3(u_nodeworld) * a_normal;                                        \n"
    "       g_backlight_dir = back_light - gl_Position.xyz;                                 \n"
    "       g_ambilight_dir = ambi_light - gl_Position.xyz;                                 \n"
    "       gl_Position = u_viewscreen * u_worldview * gl_Position;                         \n"
    "}                                                                                      \n"
};

const char *c_mesh_frag =  {
    "#version 330                                                                           \n"
    "in vec3 g_normal;                                                                      \n"
    "in vec3 g_backlight_dir;                                                               \n"
    "in vec3 g_ambilight_dir;                                                               \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float bl = dot(normalize(g_normal),normalize(g_backlight_dir));                 \n"
    "       float al = dot(normalize(g_normal),normalize(g_ambilight_dir));                 \n"
    "       bl = max(bl+0.1, 0.0);                                                          \n"
    "       al = max(al+0.1, 0.0)/length(g_ambilight_dir);                                  \n"
    "       final_color = vec3(0.7, 0.6, 1.0)*(bl*0.2+al);                                  \n"
    "}                                                                                      \n"
};
	
const char *c_phys_comp =  {
    "#version 440									\n"
    "layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;			\n"
    "uniform vec4 u_emit_pos_and_coef;                                                  \n"
    "uniform vec4 u_emit_dir_and_coef;                                                  \n"
    "uniform float u_age_mult;                                                          \n"
    "uniform float u_delta_t;                                                           \n"
    "uniform sampler2D u_noise_tex;                                                     \n"
    "											\n"
    "readonly buffer a_position_and_id { vec4 v[]; } b_position_and_id;			\n"
    "readonly buffer a_velocity_and_age { vec4 v[]; } b_velocity_and_age;		\n"
    "writeonly buffer a_xfb_position_and_id { vec4 v[]; } b_xfb_position_and_id;	\n"
    "writeonly buffer a_xfb_velocity_and_age { vec4 v[]; } b_xfb_velocity_and_age;	\n"
    "											\n"
    "vec4 in_position_and_id()								\n"
    "{											\n"
    "       uint index = gl_GlobalInvocationID.x;					\n"
    "       return b_position_and_id.v[index];					        \n"
    "}											\n"
    "											\n"
    "vec4 in_velocity_and_age()								\n"
    "{											\n"
    "       uint index = gl_GlobalInvocationID.x;					\n"
    "       return b_velocity_and_age.v[index];					        \n"
    "}											\n"
    "											\n"
    "void main()                                                                        \n"
    "{                                                                                  \n"
    "       uint index = gl_GlobalInvocationID.x;					\n"
    "       vec4 position_and_id = in_position_and_id();				\n"
    "       vec4 velocity_and_age = in_velocity_and_age();				\n"
    "       int id1 = int(index)%256;                                                   \n"
    "       int id2 = int(position_and_id.w)%256;					\n"
    "       int id3 = id1 ^ id2;                                                        \n"
    "       float age = velocity_and_age.w;						\n"
    "       age += u_delta_t * u_age_mult;                                              \n"
    "       if (age > 1.0)                                                              \n"
    "       {                                                                           \n"
    "               vec3 rand1 = texelFetch(u_noise_tex, ivec2(id1, id2), 0).rgb;       \n"
    "               vec3 rand2 = texelFetch(u_noise_tex, ivec2(id1, id3), 0).rgb;       \n"
    "               b_xfb_position_and_id.v[index] = vec4(                              \n"
    "                       u_emit_pos_and_coef.xyz+                                    \n"
    "                       u_emit_pos_and_coef.w*(rand1-0.5),                          \n"
    "                       id2+1                                                       \n"
    "               );                                                                  \n"
    "               b_xfb_velocity_and_age.v[index] = vec4(                             \n"
    "                       u_emit_dir_and_coef.xyz+                                    \n"
    "                       u_emit_dir_and_coef.w*(rand2-0.5),                          \n"
    "                       0.0                                                         \n"
    "               );                                                                  \n"
    "       }                                                                           \n"
    "       else                                                                        \n"
    "       {                                                                           \n"
    "               float drag = mix(0.0, 0.4, pow(age, 2.0))*u_delta_t;                \n"
    "               b_xfb_position_and_id.v[index] = vec4(                              \n"
    "                       velocity_and_age.xyz*u_delta_t+				\n"
    "                       position_and_id.xyz,					\n"
    "                       position_and_id.w						\n"
    "               );                                                                  \n"
    "               b_xfb_velocity_and_age.v[index] = vec4(                             \n"
    "                       velocity_and_age.xyz*(1.0-drag),				\n"
    "                       age                                                         \n"
    "               );                                                                  \n"
    "       }                                                                           \n"
    "}                                                                                  \n"
};
	

const char *c_volume_vert =  {
    "#version 330                                                                           \n"
    "in vec4 a_position_and_id;                                                             \n"
    "in vec4 a_velocity_and_age;                                                            \n"
    "out int g_Id1, g_Id2;                                                                  \n"
    "out float g_age;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(a_position_and_id.xyz, 1.0);                                 \n"
    "       g_Id1 = int(gl_VertexID)%256;                                                   \n"
    "       g_Id2 = int(a_position_and_id.w)%256;                                           \n"
    "       g_age = a_velocity_and_age.w;                                                   \n"
    "}                                                                                      \n"
};

const char *c_volume_geom =  {
    "#version 330                                                                           \n"
    "layout(points) in;                                                                     \n"
    "layout(triangle_strip, max_vertices = 16) out;                                         \n"
    "uniform mat4 u_viewscreen;                                                             \n"
    "uniform mat4 u_worldview;                                                              \n"
    "mat4 inv_camera_matrix = inverse(u_worldview);                                         \n"
    "uniform mat4 u_occl_worldscreen;                                                       \n"
    "uniform sampler2D u_noise_tex;                                                         \n"
    "uniform int u_frame_i_d;                                                               \n"
    "in int g_Id1[1];                                                                       \n"
    "in int g_Id2[1];                                                                       \n"
    "in float g_age[1];                                                                     \n"
    "out vec4 f_GFTcoord;                                                                   \n"
    "out vec2 f_texcoord;                                                                   \n"
    "out float f_age;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float s0 = 4;                                                                   \n"
    "       float s1 = 20;                                                                  \n"
    "       vec3 rand = texelFetch(                                                         \n"
    "               u_noise_tex,                                                            \n"
    "               ivec2(g_Id1[0],g_Id2[0]),                                               \n"
    "               0                                                                       \n"
    "       ).rgb;                                                                          \n"
    "       vec2 oc = vec2(-1.0, 1.0);                                                      \n"
    "       float angle = rand.r*4.0*3.1415+(rand.g-0.5)*g_age[0]*11;                       \n"
    "       float cx = cos(angle), sx = sin(angle);                                         \n"
    "       mat2 rot = mat2(cx, sx, -sx, cx);                                               \n"
    "       for (int j=0;j!=2;++j)                                                          \n"
    "       for (int i=0;i!=2;++i)                                                          \n"
    "       {                                                                               \n"
    "               vec2 offs = vec2(oc[i], oc[j]);                                         \n"
    "               offs *= mix(s0, s1, pow(g_age[0], 2.0));                                \n"
    "               offs = rot * offs;                                                      \n"
    "               gl_Position = u_worldview * gl_in[0].gl_Position;                       \n"
    "               gl_Position += vec4(offs, 0, 0);                                        \n"
    "               f_GFTcoord = u_occl_worldscreen * inv_camera_matrix * gl_Position;      \n"
    "               gl_Position = u_viewscreen * gl_Position;                               \n"
    "               f_texcoord = vec2(float(i), float(j));                                  \n"
    "               f_age = g_age[0];                                                       \n"
    "               EmitVertex();                                                           \n"
    "       }                                                                               \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_volume_frag =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform sampler2D u_occlude_tex;                                                       \n"
    "uniform sampler2D u_cloud_tex;                                                         \n"
    "in vec4 f_GFTcoord;                                                                    \n"
    "in vec2 f_texcoord;                                                                    \n"
    "in float f_age;                                                                        \n"
    "out vec2 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 GFTcoord = (f_GFTcoord.xyz/f_GFTcoord.w)*0.5 + 0.5;                        \n"
    "       float gf = texture(u_occlude_tex, GFTcoord.xy).r - GFTcoord.z;                  \n"
    "       float gf1 = (gf>0.0)?1.0:min(-gf*5, 1.0);                                       \n"
    "       float gf2 = (gf>0.0)?1.0:min(-gf*4, 1.0);                                       \n"
    "       gf = max(gf, 0);                                                                \n"
    "       float a0 = 1.0-pow(f_age, 0.125);                                               \n"
    "       float a1 = max((f_age-0.3)*pow(0.99-f_age, 0.25), 0.0);                         \n"
    "       float d = texture(u_cloud_tex, f_texcoord).r*0.5;                               \n"
    "       final_color.r = d*(a0*(gf1*0.3+gf2*0.1)+a1*0.05);                               \n"
    "       final_color.g = d*a0*a1*gf*2;                                                   \n"
    "}                                                                                      \n"
};

const char *c_composite_vert =  {
    "#version 330                                                                           \n"
    "uniform vec2 u_screen_size;                                                            \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 g_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       g_texcoord = a_texcoord*u_screen_size;                                          \n"
    "}                                                                                      \n"
};

const char *c_composite_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2DRect u_geom_tex;                                                      \n"
    "uniform sampler2DRect u_volm_tex;                                                      \n"
    "in vec2 g_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 geom = texture(u_geom_tex, g_texcoord).rgb;                                \n"
    "       vec2 volm = texture(u_volm_tex, g_texcoord).rg;                                 \n"
    "       float d = min(pow(volm.r*0.5, 2.00), 2.0);                                      \n"
    "       float l = volm.g;                                                               \n"
    "       vec3 volc = vec3(1.0, 1.0, 1.0) * mix(0.5, 0.8, l);                             \n"
    "       final_color = mix(geom, volc, d*0.7);                                           \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_viewscreen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Mat4f u_occl_worldscreen;
	Vec4f u_emit_pos_and_coef;
	Vec4f u_emit_dir_and_coef;
	Vec2f u_screen_size;
	float u_age_mult;
	float u_delta_t;

	uint32_t u_noise_tex = 0;
	uint32_t u_occlude_tex = 0;
	uint32_t u_cloud_tex = 0u;
	uint32_t u_geom_tex = 0;
	uint32_t u_volm_tex = 0;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewscreen",        &u_viewscreen       },
		        {"u_worldview",         &u_worldview        },
		        {"u_nodeworld",         &u_nodeworld        },
		        {"u_occl_worldscreen",  &u_occl_worldscreen },
		        {"u_emit_pos_and_coef", &u_emit_pos_and_coef},
		        {"u_emit_dir_and_coef", &u_emit_dir_and_coef},
		        {"u_screen_size",       &u_screen_size      },
		        {"u_age_mult",          &u_age_mult         },
		        {"u_delta_t",           &u_delta_t          },
		        {"u_noise_tex",         &u_noise_tex        },
		        {"u_occlude_tex",       &u_occlude_tex      },
		        {"u_noise_tex",         &u_noise_tex        },
		        {"u_cloud_tex",         &u_cloud_tex        },
		        {"u_geom_tex",          &u_geom_tex         },
		        {"u_volm_tex",          &u_volm_tex         },
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class NoiseTexture : public SpuTexture {
public:
	NoiseTexture()
	{
		auto image = images::RandomRGBUByte(256, 256);

		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D},
                        {"iformat",     GL_RGB8      },
                        {"width",       256          },
		        {"height",      256          },
                        {"data",        image.data() },
                        {"mag_filter",  GL_LINEAR    },
		        {"min_filter",  GL_LINEAR    },
                        {"wrap_s",      GL_REPEAT    },
                        {"wrap_t",      GL_REPEAT    },
		        {"auto_mipmap", 0            },
		};
		init(attrs);
	}
};

class CloudTexture : public SpuTexture {
public:
	CloudTexture()
	{
		auto image = images::Cloud2D(images::Cloud(128, 128, 128, ezero(), 0.5));

		auto border = Vec4f(0, 0, 0, 0);
		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D          },
		        {"iformat",    GL_RGB8                },
		        {"width",      128                    },
		        {"height",     128                    },
		        {"data",       image.data()           },
		        {"mag_filter", GL_LINEAR              },
		        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
		        {"wrap_s",     GL_CLAMP_TO_BORDER     },
		        {"wrap_t",     GL_CLAMP_TO_BORDER     },
		        {"border",     border                 },
		};
		init(attrs);
	}
};

class ObjectArray : public shapes::Array {
public:
	explicit ObjectArray(Uniforms &unifs) : m_unifs(unifs)
	{
		File file("assets/models/large_fan.obj", "rb");

		shapes::ObjMesh m_mesh_loader(file, shapes::ObjMesh::LoadingOptions(false).normals());

		m_fanIndex = m_mesh_loader.getMeshIndex("Fan");

		Attrs shader_attrs = {
		        {"frag", c_mesh_frag},
		        {"vert", c_mesh_vert},
		};

		Array::initShader(shader_attrs, Attrs(unifs));
		Array::initArray(m_mesh_loader, {"position", "normal"});
	}

	void animate(double time)
	{
		const auto driver = [&](uint32_t phase) {
			Mat4f unit;
			if (phase == m_fanIndex) {
				m_unifs.u_nodeworld = unit.rot("z", -time / 2.0 * math::two_pi());
			}
			else {
				m_unifs.u_nodeworld = unit;
			}
			useShader();
			return true;
		};
		Array::setDriver(driver);
	}

private:
	uint32_t m_fanIndex;
	Uniforms &m_unifs;
};

class OcclusionFrame : public SpuFrame {
public:
	Mat4f m_viewscreen;
	Mat4f m_worldview;
	Uniforms &m_unifs;

	OcclusionFrame(Uniforms &unifs, int32_t side) : m_unifs(unifs)
	{
		auto viewport = Rectf(0, 0, side, side);
		Attrs attrs = {
		        {"viewport0",          viewport             },
		        {"depth.target",       GL_TEXTURE_2D        },
		        {"depth.iformat",      GL_DEPTH_COMPONENT32F},
		        {"depth.min_filter",   GL_NEAREST           },
		        {"depth.mag_filter",   GL_NEAREST           },
		        {"depth.wrap_s",       GL_CLAMP_TO_EDGE     },
		        {"depth.wrap_t",       GL_CLAMP_TO_EDGE     },
		        {"depth.auto_mipmap",  0                    },
		        {"color0.target",      GL_RENDERBUFFER      },
		        {"color0.iformat",     GL_R8                },
		        {"color0.auto_mipmap", 0                    },
		};
		SpuFrame::init(attrs);
		m_unifs.u_occlude_tex = getBuffer("depth").id();

		m_viewscreen = Mat4f::projection(-7, +7, -7, +7, 5, 30, false);
		m_worldview = math::lookat(Vec3f(0, 0, -7), Vec3f(0, 0, 0), ey());
	}

	void animate(ObjectArray &object, double time)
	{
		auto viewscreen_save = m_unifs.u_viewscreen;
		auto worldview_save = m_unifs.u_worldview;

		SpuFrame::begin();
		{
			clear();
			m_unifs.u_viewscreen = m_viewscreen;
			m_unifs.u_worldview = m_worldview;
			object.animate(time);
			object.draw(nullptr);
		}
		SpuFrame::end();
		m_unifs.u_viewscreen = viewscreen_save;
		m_unifs.u_worldview = worldview_save;
	}
};

class SteamArray : public SpuArray {
public:
	SpuComputeArray &getArray() { return m_compArray; }

	explicit SteamArray(Uniforms &unifs) : m_unifs(unifs)
	{
		m_unifs.u_noise_tex = m_noiseTex.id();
		m_unifs.u_cloud_tex = m_cloudTex.id();
		m_unifs.u_emit_pos_and_coef = {0.0, 0.0, -9.0, 1.5};
		m_unifs.u_emit_dir_and_coef = {0.0, 0.0, 4.5, 1.5};
		m_unifs.u_age_mult = m_ageMult;

		// compute
		{
			auto &array = m_compArray;
			auto &shader = array.getShader();

			Attrs attrs = {
			        {"comp", c_phys_comp},
			};
			shapes::loadShader(shader, attrs, Attrs(unifs));

			std::vector<Vec4f> v(c_max_particle_count, Vec4f(2));

			// input (position)
			Attrs attrs0 = {
			        {"shader_id",           shader.id()},
			        {"a.a_position_and_id", 4          },
			};

			// input (velocity)
			Attrs attrs1 = {
			        {"a.a_velocity_and_age", 4},
			};

			// output (position)
			Attrs attrs2 = {
			        {"a.a_xfb_position_and_id", 4                   },
			        {"nelem",                   c_max_particle_count},
			        {"data",                    v.data()            },
			};

			// output (velocity)
			Attrs attrs3 = {
			        {"a.a_xfb_velocity_and_age", 4                   },
			        {"nelem",                    c_max_particle_count},
			        {"data",                     v.data()            },
			};
			array.aux(attrs0, 0);
			array.aux(attrs1, 1);
			array.aux(attrs2, 2);
			array.aux(attrs3, 3);
		}

		// volume program
		{
			Attrs attrs = {
			        {"vert", c_volume_vert},
			        {"geom", c_volume_geom},
			        {"frag", c_volume_frag},
			};
			shapes::loadShader(m_volmShader, attrs, Attrs(unifs));
		}

		// volume array
		{
			// input (position)
			Attrs attrs0 = {
			        {"shader_id",           m_volmShader.id()},
			        {"a.a_position_and_id", 4                },
			};

			// input (velocity)
			Attrs attrs1 = {
			        {"a.a_velocity_and_age", 4},
			};

			SpuArray::aux(attrs0, 0);
			SpuArray::aux(attrs1, 1);

			SpuArray::link(0, m_compArray, 2);
			SpuArray::link(1, m_compArray, 3);
		}
		spawn();
	}

	void animate(double time_diff, uint32_t /*frame_no*/)
	{
		m_unifs.u_delta_t = time_diff;
		m_compArray.getDim().x = (m_particleCount + 63) / 64;  // local_size_x = 64
		m_compArray.compute();
		m_compArray.copy(0, m_compArray, 2);
		m_compArray.copy(1, m_compArray, 3);
	}

	void draw()
	{
		m_volmShader.use();
		SpuArray::draw(GL_POINTS, 0, m_particleCount);
	}

private:
	static constexpr uint32_t c_max_particle_count = 1024;
	const float m_ageMult = 0.15;

	uint32_t m_particleCount = 0;

	NoiseTexture m_noiseTex;
	CloudTexture m_cloudTex;

	// SpuShader m_physShader;
	SpuShader m_volmShader;

	SpuComputeArray m_compArray;
	// SpuArray m_physArray[2];  // double buffer

	Uniforms &m_unifs;

	void spawn()
	{
		auto prev_spawn = 0.0;
		auto time = 0.0;
		auto time_diff = 1.0 / 25.0;
		auto spawn_interval = 1.0 / (m_ageMult * c_max_particle_count);
		auto frame_no = 0u;

		while (m_particleCount < c_max_particle_count) {
			if (prev_spawn + spawn_interval < time) {
				++m_particleCount;
				prev_spawn = time;
			}
			animate(time_diff, frame_no);
			time += time_diff;
			frame_no++;
		}
	}
};

class App : public SpuPage {
public:
	Uniforms m_unifs;

	ObjectArray m_object;
	OcclusionFrame m_occlFrame;
	SteamArray m_steamArray;

	uint32_t m_frameNo = 0;

	shapes::Array m_screenArray;
	SpuFrame m_geomFrame;
	SpuFrame m_volmFrame;

	App(const char *name)
	        : SpuPage(name, true), m_object(m_unifs), m_occlFrame(m_unifs, 256), m_steamArray(m_unifs)
	{
	}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_composite_frag},
		        {"vert", c_composite_vert},
		};

		m_screenArray.initShader(shader_attrs, Attrs(m_unifs));
		m_screenArray.initArray(shapes::Screen(), {"position", "texcoord"});

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.blend_func = {GL_ONE, GL_ONE, GL_ONE, GL_ONE};
		// renderstate.use();

		{
			auto h_width = viewport(0).sx / 2;
			auto h_height = viewport(0).sy / 2;

			m_unifs.u_screen_size = Vec2f(h_width, h_height);
			m_unifs.u_viewscreen = math::perspective(viewport(0), 60, 1, 100);

			{
				auto viewport = Rectf(0, 0, h_width, h_height);
				auto bgcolor = ezero<Vec4f>();
				auto bgdepth = 1.0f;
				Attrs attrs = {
				        {"viewport0",          viewport             },
				        {"color0.target",      GL_TEXTURE_RECTANGLE },
				        {"color0.iformat",     GL_RGB8              },
				        {"color0.wrap_s",      GL_CLAMP_TO_EDGE     },
				        {"color0.wrap_t",      GL_CLAMP_TO_EDGE     },
				        {"color0.min_filter",  GL_LINEAR            },
				        {"color0.mag_filter",  GL_LINEAR            },
				        {"color0.auto_mipmap", 0                    },
				        {"depth.target",       GL_RENDERBUFFER      },
				        {"depth.iformat",      GL_DEPTH_COMPONENT32F},
				        {"bgcolor0",           bgcolor              },
				        {"bgdepth",            bgdepth              },
				};
				m_geomFrame.init(attrs);
				m_unifs.u_geom_tex = m_geomFrame.getBuffer("color0").id();
			}
			{
				auto viewport = Rectf(0, 0, h_width, h_height);
				auto bgcolor = ezero<Vec4f>();
				Attrs attrs = {
				        {"viewport0",          viewport                           },
				        {"color0.target",      GL_TEXTURE_RECTANGLE               },
				        {"color0.iformat",     GL_RG16F                           },
				        {"color0.wrap_s",      GL_CLAMP_TO_EDGE                   },
				        {"color0.wrap_t",      GL_CLAMP_TO_EDGE                   },
				        {"color0.min_filter",  GL_LINEAR                          },
				        {"color0.mag_filter",  GL_LINEAR                          },
				        {"color0.auto_mipmap", 0                                  },
				        {"depth.texture_id",   m_geomFrame.getBuffer("depth").id()}, // share
				        {"bgcolor0",           bgcolor                            },
				        {"bgdepth",            -1.0                               },
				};
				m_volmFrame.init(attrs);
				m_unifs.u_volm_tex = m_volmFrame.getBuffer("color0").id();
			}
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto dsec = getSeconds().delta();
		auto &renderstate = SpuPage::getRenderstate();

		// camera

		{
			auto worldview0 = Mat4f().rot(
			        "z", radians(sin(esec / 11.0 * math::two_pi()) * 9
			                     + sin(esec / 13.0 * math::two_pi()) * 7));

			auto worldview1 = Mat4f::orbiting(
			        ezero(), esec, 40, 0, 0,
			        radians(sin(esec / 23.0 * math::two_pi()) * 35
			                + cos(esec / 31.0 * math::two_pi()) * 45 - 90),
			        0,
			        sin(esec / 19.0 * math::two_pi()) * 35 + sin(esec / 20.0 * math::two_pi()) * 45,
			        0, 0);

			m_unifs.u_worldview = worldview0 * worldview1;
		}

		// occlusion frame
		{
			renderstate.flags.depth_test = true;
			renderstate.write_mask.z = 1;
			renderstate.use();
			m_occlFrame.animate(m_object, esec);
		}

		// draw obect
		{
			m_geomFrame.begin();
			m_geomFrame.clear();

			renderstate.flags.depth_test = true;
			renderstate.write_mask.z = 1;
			renderstate.flags.blend = false;
			renderstate.use();

			m_object.animate(esec);
			m_object.draw(nullptr);
			m_geomFrame.end();
		}

		// draw steam
		{
			m_steamArray.animate(dsec, m_frameNo);
			m_volmFrame.begin();
			m_volmFrame.clear();

			renderstate.write_mask.z = 0;
			renderstate.flags.blend = true;
			renderstate.use();

			m_unifs.u_occl_worldscreen = m_occlFrame.m_viewscreen * m_occlFrame.m_worldview;

			m_steamArray.draw();
			m_volmFrame.end();
		}

		// composite
		{
			renderstate.flags.depth_test = false;
			renderstate.flags.blend = false;
			renderstate.use();

			m_screenArray.draw(nullptr);
		}
		{
			// m_prevTime = esec;
			m_frameNo++;
		}
	}
};
static ObjectRegistry<SpuPage>::Creator<App> page_creator("033_steam");
}  // namespace
}  // namespace spu::oglplus
