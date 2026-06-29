//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cube.hpp>
#include <shapes/screen.hpp>
#include <shapes/twisted_torus.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_sky_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewtexc;                                                               \n"
    "uniform mat4 u_worldview;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_viewtexc * u_worldview * a_position;                            \n"
    "       f_texcoord = a_texcoord;                                                        \n"
    "}                                                                                      \n"
};

const char *c_sky_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_sky_noise_texture;                                                 \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out float frag_value;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float n = 1.0;                                                                  \n"
    "       for (int l=0; l!=7; ++l)                                                        \n"
    "       {                                                                               \n"
    "               float t = textureLod(u_sky_noise_texture, f_texcoord, l).r;             \n"
    "               n *= (0.05*(18-l) + t*0.05*(2+l));                                      \n"
    "       }                                                                               \n"
    "       n = pow(min(2.1*n, 1.0), 4.0);                                                  \n"
    "       frag_value = n;                                                                 \n"
    "}                                                                                      \n"
};

const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "layout (std140) uniform u_model_block {                                                \n"
    "       mat4 model_matrices[128];                                                       \n"
    "};                                                                                     \n"
    "const vec3 u_light_pos = vec3(0.0, 0.0, 0.0);                                          \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       mat4 u_nodeworld = model_matrices[gl_InstanceID];                               \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_light_dir = normalize(u_light_pos - gl_Position.xyz);                         \n"
    "       f_normal = mat3(u_nodeworld)*a_normal;                                          \n"
    "       f_color = abs(a_normal-normalize((u_nodeworld*a_position).yxz));                \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_color;                                                                       \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = 0.5 + sqrt(max(dot(f_light_dir,f_normal)+0.1,0.0));                   \n"
    "       final_color = f_color * min(l, 0.8);                                            \n"
    "}                                                                                      \n"
};

const char *c_fog_vert =  {
    "#version 330                                                                           \n"
    "uniform vec2 u_screen_size;                                                            \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       f_texcoord = u_screen_size*a_texcoord;                                          \n"
    "}                                                                                      \n"
};

const char *c_fog_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2DRect u_noise_texture;                                                 \n"
    "uniform sampler2DRect u_color_texture;                                                 \n"
    "uniform sampler2DRect u_depth_texture;                                                 \n"
    "const int u_samples = 64;                                                              \n"
    "uniform vec2 u_sample_offs[64];                                                          \n"
    "const float inv_samples = 1.0 / u_samples;                                             \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float n = texture(u_noise_texture, f_texcoord).r;                               \n"
    "       n -= 0.5;                                                                       \n"
    "       float z = pow(texture(u_depth_texture, f_texcoord).r, 4);                       \n"
    "       z = pow(clamp((z-0.35-0.1*n)*(2.5+0.5*n), 0.0, 1.0), 8);                        \n"
    "       int nsam = int(1+z*(u_samples-1));                                              \n"
    "       float wsum = 0.0;                                                               \n"
    "       final_color = vec3(0);\n" // fix (tesla)
    "       for (int i=0; i!=nsam; ++i)                                                     \n"
    "       {                                                                               \n"
    "               vec2 so = u_sample_offs[i]*8.0*z;                                         \n"
    "               vec2 sam_tex_coord = f_texcoord+vec2(so.x, so.y);                       \n"
    "               float sz = pow(texture(u_depth_texture, sam_tex_coord).r, 2);           \n"
    "               float n = texture(u_noise_texture, sam_tex_coord).r;                    \n"
    "               float d = mix(0.85, 1.0, n);\n" // enhance
    "               vec3 t = texture(u_color_texture, sam_tex_coord).rgb;                   \n"
    "               vec3 fd = vec3(d, d, d);                                                \n"
    "               float s = (0.3*t.r + 0.59*t.g + 0.11*t.b);                              \n"
    "               vec3 fs = vec3(s, s, s);                                                \n"
    "               float w = 1.0-abs(z-sz);                                                \n"
    "               final_color += w*mix(t, mix(fd, fs, sz*(1.0-sz)), z*sz);                \n"
    "               wsum += w;                                                              \n"
    "       }                                                                               \n"
    "       final_color /= wsum;                                                            \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_model_block[128];
	Mat4f u_viewtexc;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Vec2f u_screen_size;
	Vec2f u_sample_offs[64];

	uint32_t u_sky_noise_texture;
	uint32_t u_noise_texture = 0u;
	uint32_t u_color_texture = 0u;
	uint32_t u_depth_texture = 0u;

	Uniforms()
	{
		m_attrs = {
		        {"u_model_block",       &u_model_block      },
		        {"u_viewtexc",          &u_viewtexc         },
		        {"u_viewsceen",         &u_viewsceen        },
		        {"u_worldview",         &u_worldview        },
		        {"u_screen_size",       &u_screen_size      },
		        {"u_sample_offs",       &u_sample_offs      },
		        {"u_sky_noise_texture", &u_sky_noise_texture},
		        {"u_noise_texture",     &u_noise_texture    },
		        {"u_color_texture",     &u_color_texture    },
		        {"u_depth_texture",     &u_depth_texture    },
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class Instances {
public:
	Instances(Uniforms &unifs, uint32_t n = 64)
	{
		m_pathPos.init(makePositions(), 0.25);
		m_pathNml.init(makeNormals(), 0.25);

		RandomGenerator<float> frand = {0.0, 1.0};

		m_count = 2 * n;
		std::vector<Mat4f> matrix_data(m_count);
		auto p = begin(matrix_data);

		auto step = 1.0 / n;

		for (auto i = 0u; i != n; ++i) {
			auto pos = m_pathPos.position(i * step);
			auto tgt = normalize(
			        (m_pathPos.position((i + 1) * step) - m_pathPos.position((i - 1) * step)));

			auto tmp = m_pathNml.position(i * step);
			auto nml = normalize((dot(tmp, tgt) != 0.0 ? tmp - tgt * dot(tmp, tgt) : tmp));

			auto btg = cross(nml, tgt);

			for (auto j = 0u; j != 2; ++j) {
				const float s[2] = {-1.0, 1.0};

				auto offs = pos + s[j] * (3 + 4 * frand()) * btg + 2 * frand() * tgt
				          + 6 * frand() * nml;

				auto matrix = math::unit().trans(offs)
				            * math::unit().rot("x", -frand() * math::two_pi())
				            * math::unit().rot("y", -frand() * math::two_pi())
				            * math::unit().rot("z", -frand() * math::two_pi());

				*p = matrix;
				p++;
			}
		}
		assert(p == matrix_data.end());
		memcpy(unifs.u_model_block, matrix_data.data(), matrix_data.size() * sizeof(Mat4f));
	}

	Mat4f makeMatrix(double t, double dt) const
	{
		auto pos = position(t);
		auto tgt = normalize((position(t + dt) - position(t - dt)));
		auto tmp = normal(t + dt);
		auto dtt = dot(tmp, tgt);
		auto nml = normalize((dtt != 0.0 ? tmp - tgt * dtt : tmp));
		auto btg = cross(nml, tgt);

		return Mat4f(Vec4f(btg, 0), Vec4f(nml, 0), Vec4f(tgt, 0), Vec4f(pos, 1)).transpose4();
	}

	Vec3f position(double t) const { return m_pathPos.position(t); }
	Vec3f normal(double t) const { return m_pathNml.position(t); }
	uint32_t count() const { return m_count; }

private:
	CubicBezierLoop<Vec3f, double> m_pathPos;
	CubicBezierLoop<Vec3f, double> m_pathNml;
	uint32_t m_count;

	std::vector<Vec3f> makePositions()
	{
		/* clang-format off */
		return {
			{ 10.0,   0.0,  70.0}, { 60.0,   0.0,  60.0}, { 95.0,   0.0,   0.0},
			{ 60.0,   0.0, -45.0}, { 10.0,   0.0, -60.0}, {-25.0,  20.0, -60.0},
			{-60.0,  40.0, -60.0}, {-80.0,  40.0, -30.0}, {-25.0,  40.0,  10.0},
			{ 35.0,  40.0,  10.0}, { 80.0,   0.0,   0.0}, { 45.0, -40.0, -10.0},
			{  0.0, -40.0, -10.0}, {-50.0, -40.0, -10.0}, {-90.0, -40.0,  30.0},
			{-70.0, -40.0,  70.0}, {-30.0, -20.0,  70.0}, 
		};
		/* clang-format on */
	}

	std::vector<Vec3f> makeNormals()
	{
		/* clang-format off */
		return {
			{ 0.0,  1.0,  0.0}, { 0.0,  1.0, -1.0}, {-1.0,  0.0,  0.0},
			{ 0.0,  1.0,  2.0}, { 0.0,  1.0,  1.0}, { 1.0,  1.0,  0.5},
			{ 0.0,  1.0,  1.0}, { 1.0,  0.0,  0.0}, { 0.0,  1.0, -1.0},
			{ 0.0,  1.0,  0.0}, { 1.0,  0.0,  0.0}, { 0.0, -1.0,  1.0},
			{ 0.0,  0.0,  1.0}, { 0.0,  1.0,  1.0}, { 1.0,  0.0,  0.0},
			{ 0.0,  1.0, -1.0}, {-1.0,  1.0,  0.0}, 
		};
		/* clang-format on */
	}
};

class SkyFrame : public SpuFrame {
public:
	void resize(Uniforms &unifs, float width, float height)
	{
		auto viewport = Rectf(0, 0, width, height);
		Attrs attrs = {
		        {"viewport0",          viewport            },
		        {"color0.target",      GL_TEXTURE_RECTANGLE},
		        {"color0.iformat",     GL_R8               },
		        {"color0.min_filter",  GL_LINEAR           },
		        {"color0.mag_filter",  GL_LINEAR           },
		        {"color0.wrap_s",      GL_CLAMP_TO_EDGE    },
		        {"color0.wrap_t",      GL_CLAMP_TO_EDGE    },
		        {"color0.auto_mipmap", 0                   },
		};
		SpuFrame::init(attrs);
		unifs.u_noise_texture = getBuffer("color0").id();
	}
};

class FogFrame : public SpuFrame {
public:
	void resize(Uniforms &unifs, float width, float height)
	{
		auto bgcolor = Vec4f(0.95, 0.95, 0.95, 0.0);
		auto viewport = Rectf(0, 0, width, height);

		Attrs attrs = {
		        {"viewport0",          viewport             },
		        {"color0.target",      GL_TEXTURE_RECTANGLE },
		        {"color0.iformat",     GL_RGB8              },
		        {"color0.min_filter",  GL_LINEAR            },
		        {"color0.mag_filter",  GL_LINEAR            },
		        {"color0.wrap_s",      GL_CLAMP_TO_EDGE     },
		        {"color0.wrap_t",      GL_CLAMP_TO_EDGE     },
		        {"color0.auto_mipmap", 0                    },
		        {"depth.target",       GL_TEXTURE_RECTANGLE },
		        {"depth.iformat",      GL_DEPTH_COMPONENT32F},
		        {"depth.min_filter",   GL_LINEAR            },
		        {"depth.mag_filter",   GL_LINEAR            },
		        {"depth.wrap_s",       GL_CLAMP_TO_EDGE     },
		        {"depth.wrap_t",       GL_CLAMP_TO_EDGE     },
		        {"depth.auto_mipmap",  0                    },
		        {"bgcolor",            bgcolor.f            },
		};
		SpuFrame::init(attrs);
		unifs.u_color_texture = getBuffer("color0").id();
		unifs.u_depth_texture = getBuffer("depth").id();
	}
};

class App : public SpuPage {
public:
	Uniforms m_unifs;
	Instances m_instances;

	SkyFrame m_skyFrame;
	FogFrame m_fogFrame;

	uint32_t m_screenWidth;

	shapes::Array m_skybox;
	shapes::Array m_torus;
	shapes::Array m_screen;
	SpuTexture m_skyNoiseTexture;

	App(const char *name) : SpuPage(name, true), m_instances(m_unifs) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs sky_shader_attrs = {
		        {"frag", c_sky_frag},
		        {"vert", c_sky_vert},
		};

		Attrs shape_shader_attrs = {
		        {"frag", c_shape_frag},
		        {"vert", c_shape_vert},
		};

		Attrs fog_shader_attrs = {
		        {"frag", c_fog_frag},
		        {"vert", c_fog_vert},
		};

		m_skybox.initShader(sky_shader_attrs, Attrs(m_unifs));
		m_torus.initShader(shape_shader_attrs, Attrs(m_unifs));
		m_screen.initShader(fog_shader_attrs, Attrs(m_unifs));

		m_skybox.initArray(shapes::Cube(1000, 1000, 1000), {"position", "texcoord"});

		m_torus.initArray(shapes::TwistedTorus(1.0, 0.2, 0.01, 6, 36, 4), {"position", "normal"});
		m_screen.initArray(shapes::Screen(), {"position", "texcoord"});

		// patch

		// make shadow offset
		{
			for (auto &u_sample_off: m_unifs.u_sample_offs) {
				auto u = frand();
				auto v = frand();

				u_sample_off
				        = {sqrtf(v) * cosf(2.0f * pi() * u), sqrtf(v) * sinf(2.0f * pi() * u)};
			}
		}

		// make sky noise
		{
			const auto c_tex_side = size_t(64);
			std::vector<std::vector<uint32_t>> pix_array;  // plance holder

			// for (auto side = tex_side; side; side >>= 1) { // not work in AMD !
			for (auto side = c_tex_side; side > 1; side >>= 1) {
				std::vector<uint32_t> pix;
				pix.reserve(side * side);
				for (auto i = 0u; i < side * side; i++) {
					pix.push_back(rand());
				}
				pix_array.push_back(pix);
			}

			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
			        {"iformat",     GL_R8                  },
			        {"width",       c_tex_side             },
			        {"height",      c_tex_side             },
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
			        {"mag_filter",  GL_LINEAR              },
			        {"wrap_s",      GL_REPEAT              },
			        {"wrap_t",      GL_REPEAT              },
			        {"auto_mipmap", 0                      },
			};
			m_skyNoiseTexture.init(attrs);

			for (auto &pix: pix_array) {
				int32_t i = &pix - &pix_array[0];
				int32_t loc[4] = {0, 0, 0, i};
				m_skyNoiseTexture.send(pix.data(), GL_R8, loc);
			}

			m_unifs.u_sky_noise_texture = m_skyNoiseTexture.id();
		}

		{
			m_screenWidth = int32_t(viewport(0).sx);
			m_skyFrame.resize(m_unifs, int32_t(viewport(0).sx), int32_t(viewport(0).sy));
			m_fogFrame.resize(m_unifs, int32_t(viewport(0).sx), int32_t(viewport(0).sy));

			m_unifs.u_viewtexc = math::perspective(viewport(0), 75, 1, 2000);
			m_unifs.u_viewsceen = math::perspective(viewport(0), 75, 1, 200);
			m_unifs.u_screen_size = Vec2f(viewport(0).sx, viewport(0).sy);
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto pos = esec / 90.0;

		// 57 only
		auto e = m_instances.position(pos - 0.03 + sin(esec / 7.0 * math::two_pi()) * 0.01);
		auto c = m_instances.position(pos + 0.02 + sin(esec / 11.0 * math::two_pi()) * 0.01);
		auto u = m_instances.normal(pos - 0.02 + sin(esec / 9.0 * math::two_pi()) * 0.02);

		auto worldview = math::lookat(e, c, u);

		// sky
		auto &renderstate = SpuPage::getRenderstate();
		renderstate.write_mask.z = 0;
		renderstate.flags.depth_test = false;
		renderstate.flags.cull_face = false;
		renderstate.use();

		m_skyFrame.begin();
		m_unifs.u_worldview = worldview;
		m_skybox.draw(nullptr);
		m_skyFrame.end();

		// torus on fog frame
		renderstate.write_mask.z = 1;
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.use();

		m_fogFrame.begin();
		m_fogFrame.clear();
		m_unifs.u_worldview = worldview;
		m_torus.setInstanceCount(m_instances.count());
		m_torus.draw(nullptr);
		m_fogFrame.end();

		// fog
		renderstate.write_mask.z = 0;
		renderstate.flags.depth_test = false;
		renderstate.write_mask = {1, 1, 1, 1, 0};
		renderstate.use();

		m_screen.draw(nullptr);
		renderstate.write_mask.z = 1;
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("031_fog");
}  // namespace
}  // namespace spu::oglplus
