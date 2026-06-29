//
// RippleTexShader :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>
#include <shapes/plane.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_ripple_vert =  {
    "#version 330                                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(0.0, 0.0, 0.0, 1.0);                                         \n"
    "}                                                                                      \n"
};

const char *c_ripple_geom =  {
    "#version 330                                                                           \n"
    "layout (points) in;                                                                    \n"
    "layout (triangle_strip, max_vertices = 4) out;                                         \n"
    "uniform int u_texture_size;                                                             \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void make_vertex(vec2 pos, vec2 tc)                                                    \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(pos, 0.0, 1.0);                                              \n"
    "       f_texcoord = tc;                                                                \n"
    "       EmitVertex();                                                                   \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float rts = u_texture_size;                                                      \n"
    "       make_vertex(vec2(-1.0,-1.0), vec2(  0,   0));                                   \n"
    "       make_vertex(vec2(-1.0, 1.0), vec2(  0, rts));                                   \n"
    "       make_vertex(vec2( 1.0,-1.0), vec2(rts,   0));                                   \n"
    "       make_vertex(vec2( 1.0, 1.0), vec2(rts, rts));                                   \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
};

const char *c_ripple_frag =  {
    "#version 330                                                                           \n"
    "uniform ivec2 u_new_drop;                                                              \n"
    "uniform sampler2D u_texture1;                                                          \n"
    "uniform sampler2D u_texture2;                                                          \n"
    "uniform int u_texture_size;                                                             \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout (location = 0) out vec4 final_bump;                                             \n"
    "layout (location = 1) out float final_height;                                          \n"
    "ivec2 wrap_tc(ivec2 tc)                                                                \n"
    "{                                                                                      \n"
    "       if (tc.x < 0) tc.x = u_texture_size-1;                                           \n"
    "       if (tc.x >= u_texture_size) tc.x = 0;                                            \n"
    "       if (tc.y < 0) tc.y = u_texture_size-1;                                           \n"
    "       if (tc.y >= u_texture_size) tc.y = 0;                                            \n"
    "       return tc;                                                                      \n"
    "}                                                                                      \n"

    "float height_at(sampler2D tex, ivec2 tc, float factor)                                 \n"
    "{                                                                                      \n"
    "       return texelFetch(tex, wrap_tc(tc), 0).r * factor *                             \n"
        "0.98;" // !NEED p_a_r_a_m_eterized!
    "}                                                                                      \n"

    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       ivec2 tc = ivec2(f_texcoord);                                                   \n"
    "       float  ch = height_at(u_texture2, tc, 1.0);                                     \n"
    "       float xp1 = height_at(u_texture2, tc+ivec2( 1, 0), 0.25);                       \n"
    "       float xm1 = height_at(u_texture2, tc+ivec2(-1, 0), 0.25);                       \n"
    "       float yp1 = height_at(u_texture2, tc+ivec2( 0, 1), 0.25);                       \n"
    "       float ym1 = height_at(u_texture2, tc+ivec2( 0,-1), 0.25);                       \n"
    "       final_height = xp1 + xm1 + yp1 + ym1;                                           \n"
    "       final_height += height_at(u_texture2, tc+ivec2( 1,-1), 0.25);                   \n"
    "       final_height += height_at(u_texture2, tc+ivec2( 1, 1), 0.25);                   \n"
    "       final_height += height_at(u_texture2, tc+ivec2(-1,-1), 0.25);                   \n"
    "       final_height += height_at(u_texture2, tc+ivec2(-1, 1), 0.25);                   \n"
    "       final_height -= height_at(u_texture1, tc, 1.0);                                 \n"
    "       vec2 d = u_new_drop - tc;                                                       \n"
    "       final_height += length(d)<8?1.0:0.0;                                            \n"
    "       vec3 frag_normal = vec3(                                                        \n"
    "               (xm1 - ch) + (ch - xp1),                                                \n"
    "               (ym1 - ch) + (ch - yp1),                                                \n"
    "               0.1                                                                     \n"
    "       );                                                                              \n"
    "       final_bump = vec4(                                                              \n"
    "               normalize(frag_normal),                                                 \n"
    "               final_height                                                            \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_water_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "mat4 u_matrix = u_viewsceen*u_worldview;                                               \n"
    "in vec3 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec3 a_tangent;                                                                     \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_normal, f_tangent, f_bitangent;                                             \n"
    "out vec3 f_light_dir, f_view_dir;                                                      \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_matrix * vec4(a_position, 1.0);                                 \n"

    "       f_tangent = a_tangent;                                                          \n"
    "       f_bitangent = cross(a_normal, a_tangent);                                       \n"
    "       f_light_dir = u_light_position - a_position;                                    \n"
    "       f_view_dir = u_eye_position - a_position;                                       \n"
    "       f_texcoord = a_texcoord * 16.0;                                                 \n"
    "}                                                                                      \n"
};

const char *c_water_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_ripple_texture;                                                    \n"
    "uniform samplerCube u_env_texture;                                                     \n"
    "in vec3 f_normal, f_tangent, f_bitangent;                                              \n"
    "in vec3 f_light_dir, f_view_dir;                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "layout (location = 0) out vec4 final_color;                                            \n"
    "const vec3 light_color = vec3(1.0, 1.0, 0.95);                                         \n"
    "const vec3 bg_color = vec3(0.4, 0.4, 0.4);                                             \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec4 tex_bump = texture(u_ripple_texture, f_texcoord);                          \n"
    "       vec3 frag_normal = normalize(                                                   \n"
    "               tex_bump.x * f_tangent+                                                 \n"
    "               tex_bump.y * f_bitangent+                                               \n"
    "               tex_bump.z * f_normal                                                   \n"
    "       );                                                                              \n"
    "       vec3 frag_light_dir = normalize(f_light_dir);                                   \n"
    "       vec3 frag_light_refl = reflect(-frag_light_dir, frag_normal);                   \n"
    "       vec3 frag_view_dir = normalize(f_view_dir);                                     \n"
    "       vec3 frag_view_refl = reflect(-frag_view_dir, frag_normal);                     \n"
    "       vec3 frag_view_refr = refract(                                                  \n"
    "               -normalize(frag_view_dir+vec3(0.0, 1.0, 0.0)),                          \n"
    "               frag_normal,                                                            \n"
    "               1.2                                                                     \n"
    "       );                                                                              \n"
    "       float diffuse = max(dot(frag_normal, frag_light_dir), 0.0);                     \n"
    "       float specular = clamp(pow(dot(frag_view_dir, frag_light_refl)+0.1, 64.0),      \n"
    "                              0.0, 2.0);                                               \n"
    "       float transparency = max(dot(frag_normal, frag_view_dir)+0.3, 0.0);             \n"
    "       float visibility = min(16.0 / dot(f_view_dir, f_view_dir), 1.0);                \n"
    "       vec3 sky_color = texture(u_env_texture, frag_view_refl).rgb;                    \n"
    "       vec3 ground_color = texture(u_env_texture, frag_view_refr).rgb;                 \n"
    "       vec3 water_color =                                                              \n"
    "               mix(                                                                    \n"
    "                       0.7*sky_color,                                                  \n"
    "                       0.8*ground_color*light_color*diffuse,                           \n"
    "                       transparency                                                    \n"
    "               )+                                                                      \n"
    "               0.2*light_color*diffuse+                                                \n"
    "               0.05*light_color*specular;                                              \n"
    "       final_color = vec4(                                                             \n"
    "               mix(bg_color, water_color, visibility),                                 \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class RippleTexShader : public SpuShader {
public:
	int32_t u_new_drop[2];
	int32_t u_texture_size;
	uint32_t u_texture1;
	uint32_t u_texture2;

	explicit RippleTexShader(uint32_t ripple_tex_size)
	{
		Attrs shader_attrs = {
		        {"frag", c_ripple_frag},
		        {"vert", c_ripple_vert},
		        {"geom", c_ripple_geom},
		};

		Attrs unif_attrs = {
		        {"u_new_drop",     &u_new_drop[0] },
		        {"u_texture_size", &u_texture_size},
		        {"u_texture1",     &u_texture1    },
		        {"u_texture2",     &u_texture2    },
		};
		shapes::loadShader(*this, shader_attrs, unif_attrs);
		u_texture_size = ripple_tex_size;
	}
};

class RippleTexHolder {
public:
	RippleTexHolder(size_t ripple_tex_size)
	{
		m_heightTextures.resize(m_nhm);

		const std::vector<float> pix(ripple_tex_size * ripple_tex_size, 0.5);

		for (auto i = 0u; i < m_nhm; i++) {
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D  },
                                {"iformat",    GL_R32F        }, // need functional
			        {"width",      ripple_tex_size},
                                {"height",     ripple_tex_size},
			        {"min_filter", GL_NEAREST     },
                                {"mag_filter", GL_NEAREST     },
			        {"wrap_s",     GL_REPEAT      },
                                {"wrap_t",     GL_REPEAT      },
			        {"data",       pix.data()     },
			};
			m_heightTextures[i].init(attrs);
		}

		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D  },
                                {"iformat",    GL_RGBA8       }, // need functional
			        {"width",      ripple_tex_size},
                                {"height",     ripple_tex_size},
			        {"min_filter", GL_LINEAR      },
                                {"mag_filter", GL_LINEAR      },
			        {"wrap_s",     GL_REPEAT      },
                                {"wrap_t",     GL_REPEAT      },
			};
			m_bumpTexture.init(attrs);
		}
	}
	void swap() { ++m_curr; }

	uint32_t heightTextures0() const { return m_heightTextures[(m_curr + 0) % m_nhm].id(); }
	uint32_t heightTextures1() const { return m_heightTextures[(m_curr + 1) % m_nhm].id(); }
	uint32_t heightTextures2() const { return m_heightTextures[(m_curr + 2) % m_nhm].id(); }
	uint32_t bumpTexture0() const { return m_bumpTexture.id(); }

private:
	const uint32_t m_nhm = 3;
	uint32_t m_curr = 0;
	std::vector<SpuTexture> m_heightTextures;
	SpuTexture m_bumpTexture;
};

class RippleGenerator {
public:
	RippleGenerator() : m_shader(m_size), m_holder(m_size)
	{
		m_array.init({
		        {"nelem", 1}
                });
	}

	void update()
	{
		auto viewport = Rectf(0, 0, m_size, m_size);
		Attrs attrs = {
		        {"viewport0",         viewport                  },
		        {"color0.texture_id", m_holder.bumpTexture0()   },
		        {"color1.texture_id", m_holder.heightTextures2()},
		};
		m_frame.init(attrs);
		m_frame.begin();
		m_shader.u_texture1 = m_holder.heightTextures0();
		m_shader.u_texture2 = m_holder.heightTextures1();
		m_shader.u_new_drop[0] = rand() % m_size;
		m_shader.u_new_drop[1] = rand() % m_size;
		m_shader.use();
		m_array.draw(GL_POINTS);
		m_holder.swap();
		m_frame.end();
	}

	uint32_t get() const { return m_holder.bumpTexture0(); }

private:
	int32_t m_size = 1024;
	RippleTexShader m_shader;
	RippleTexHolder m_holder;

	SpuArray m_array;
	SpuFrame m_frame;
};

class WaterArray : public shapes::Array {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Vec3f u_light_position;
	Vec3f u_eye_position;
	uint32_t u_ripple_texture;
	uint32_t u_env_texture;

	WaterArray()
	{
		Attrs shader_attrs = {
		        {"frag", c_water_frag},
		        {"vert", c_water_vert},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen",      &u_viewsceen     },
                        {"u_worldview",      &u_worldview     },
		        {"u_light_position", &u_light_position},
                        {"u_eye_position",   &u_eye_position  },
		        {"u_ripple_texture", &u_ripple_texture},
                        {"u_env_texture",    &u_env_texture   },
		};
		Array::initShader(shader_attrs, unif_attrs);
	}

	template<class shape_t> void initArray(const shape_t &shape)
	{
		Array::initArray(shape, {"position", "normal", "tangent", "texcoord"});
	}
};

class EnvmapTexture : public SpuTexture {
public:
	EnvmapTexture()
	{
		int32_t cube_target[6] = {
		        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};

		auto image0 = images::LoadTexture("cloudy_day-cm_0", false, false);
		auto image1 = images::LoadTexture("cloudy_day-cm_1", false, false);
		auto image2 = images::LoadTexture("cloudy_day-cm_2", false, false);
		auto image3 = images::LoadTexture("cloudy_day-cm_3", false, false);
		auto image4 = images::LoadTexture("cloudy_day-cm_4", false, false);
		auto image5 = images::LoadTexture("cloudy_day-cm_5", false, false);

		auto unit = image0.dataSize();
		std::vector<uint8_t> pix(unit * 6);

		memcpy(&pix[unit * 0], image0.data(), image0.dataSize());
		memcpy(&pix[unit * 1], image1.data(), image1.dataSize());
		memcpy(&pix[unit * 2], image2.data(), image2.dataSize());
		memcpy(&pix[unit * 3], image3.data(), image3.dataSize());
		memcpy(&pix[unit * 4], image4.data(), image4.dataSize());
		memcpy(&pix[unit * 5], image5.data(), image5.dataSize());

		Attrs attrs = {
		        {"target",      GL_TEXTURE_CUBE_MAP     },
		        {"min_filter",  GL_LINEAR               },
		        {"mag_filter",  GL_LINEAR               },
		        {"wrap_s",      GL_CLAMP_TO_EDGE        },
		        {"wrap_t",      GL_CLAMP_TO_EDGE        },
		        {"wrap_r",      GL_CLAMP_TO_EDGE        },
		        {"iformat",     GL_RGBA8                },
		        {"width",       image0.width()          },
		        {"height",      image0.height()         },
		        {"cube_target", cube_target             },
		        {"data",        (const void *)pix.data()},
		};
		init(attrs);
	}
};

class App : public SpuPage {
public:
	EnvmapTexture m_envTexture;
	RippleGenerator m_ripples;
	WaterArray m_array;

	App(const char *name) : SpuPage(name, true, {0.4, 0.4, 0.4, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_array.initArray(shapes::Plane(Vec3f(40, 0, 0), Vec3f(0, 0, -40)));
		m_array.u_light_position = {2.0, 10.0, -4.0};
		m_array.u_ripple_texture = m_ripples.get();
		m_array.u_env_texture = m_envTexture.id();
	}

	void render() override
	{
		m_ripples.update();

		auto esec = getSeconds().current();
		auto worldview = Mat4f::orbiting(ezero(), esec, 6, 1, 31, 0, 17, 50, 35, 21);

		m_array.u_viewsceen = math::perspective(viewport(0), 60, 1, 100);
		m_array.u_worldview = worldview;
		m_array.u_eye_position = worldview.unitary_inverse().c[3];
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("030_rain");
}  // namespace
}  // namespace spu::oglplus
