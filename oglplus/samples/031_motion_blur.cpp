//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/checker.hpp>
#include <shapes/cube.hpp>
#include <shapes/obj_mesh.hpp>
#include <shapes/screen.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_single_nodeworld;                                                       \n"
    "uniform uint u_single_model;                                                           \n"
    "layout (std140) uniform u_model_block {                                                \n"
    "       mat4 model_matrices[512];                                                       \n"
    "};                                                                                     \n"
    "const vec3 u_light_pos = vec3(0.0, 0.0, 0.0);                                          \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_color;                                                                      \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       mat4 u_nodeworld =                                                              \n"
    "               u_single_model!=0u?                                                     \n"
    "               u_single_nodeworld:                                                     \n"
    "               model_matrices[gl_InstanceID];                                          \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_light_dir = normalize(u_light_pos - gl_Position.xyz);                         \n"
    "       f_normal = mat3(u_nodeworld) * a_normal;                                        \n"
    "       f_texcoord = u_single_model!=0u ? a_position.xz : a_texcoord;                   \n"
    "       f_color = abs(normalize((u_nodeworld * a_position).yxz));                       \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2D u_checker_tex;                                                       \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_color;                                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float c = texture(u_checker_tex, f_texcoord).r;                                 \n"
    "       float l = pow(max(dot(f_light_dir, f_normal)+0.1,0.0)*1.6,2.0);                 \n"
    "       final_color = mix(                                                              \n"
    "               vec3(0, 0, 0),                                                          \n"
    "               f_color,                                                                \n"
    "               mix(c, 1.0, 0.2)*(l+0.3)                                                \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_blur_vert =  {
    "#version 330                                                                           \n"
    "uniform vec2 u_screen_size;                                                            \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       f_texcoord = u_screen_size * a_texcoord;                                        \n"
    "}                                                                                      \n"
};

const char *c_blur_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2DRect u_current_frame;                                                 \n"
    "uniform sampler2DRect u_previous_frames;                                               \n"
    "uniform float u_splitter;                                                              \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "vec3 sharp()                                                                           \n"
    "{                                                                                      \n"
    "       return texture(u_current_frame, f_texcoord).rgb;                                \n"
    "}                                                                                      \n"
    "vec3 blurred()                                                                         \n"
    "{                                                                                      \n"
    "       vec3 prev = vec3(0, 0, 0);                                                      \n"
    "       const vec2 otc[9] = vec2[9](                                                    \n"
    "               vec2(-1,-1),                                                            \n"
    "               vec2( 0,-1),                                                            \n"
    "               vec2( 1,-1),                                                            \n"
    "               vec2(-1, 0),                                                            \n"
    "               vec2( 0, 0),                                                            \n"
    "               vec2( 1, 0),                                                            \n"
    "               vec2(-1, 1),                                                            \n"
    "               vec2( 0, 1),                                                            \n"
    "               vec2( 1, 1)                                                             \n"
    "       );                                                                              \n"
    "       const float is = 1.0/9.0;                                                       \n"
    "       for (int s=0; s!=9; ++s)                                                        \n"
    "       {                                                                               \n"
    "               vec2 tc = f_texcoord+otc[s]*2;                                          \n"
    "               prev += texture(u_previous_frames, tc).rgb * is;                        \n"
    "       }                                                                               \n"
    "       vec3 curr = texture(u_current_frame, f_texcoord).rgb;                           \n"
    "       float a = curr.x+curr.y+curr.z;                                                 \n"
    "       return mix(curr, prev, 0.95-0.4*a);                                             \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       if (gl_FragCoord.x < u_splitter-1)                                              \n"
    "               final_color = sharp();                                                  \n"
    "       else if (gl_FragCoord.x > u_splitter+1)                                         \n"
    "               final_color = blurred();                                                \n"
    "       else final_color = vec3(1.0, 1.0, 1.0);                                         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_model_block[512];
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_single_nodeworld;
	Vec2f u_screen_size;
	float u_splitter;
	uint32_t u_single_model;
	uint32_t u_checker_tex;
	uint32_t u_current_frame;
	uint32_t u_previous_frames;
	uint32_t m_blurTex[2];  // future use

	Uniforms()
	{
		m_attrs = {
		        {"u_model_block",      &u_model_block     },
                        {"u_viewsceen",        &u_viewsceen       },
		        {"u_worldview",        &u_worldview       },
                        {"u_single_nodeworld", &u_single_nodeworld},
		        {"u_screen_size",      &u_screen_size     },
                        {"u_splitter",         &u_splitter        },
		        {"u_single_model",     &u_single_model    },
                        {"u_checker_tex",      &u_checker_tex     },
		        {"u_current_frame",    &u_current_frame   },
                        {"u_previous_frames",  &u_previous_frames },
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class Instances {
public:
	Instances(Uniforms &unifs, uint32_t n = 256)
	{
		m_positions.init(makePositions(), 0.25);
		m_normals.init(makeNormals(), 0.25);

		m_count = 2 * n;
		std::vector<Mat4f> matrix_data(m_count);
		auto p = begin(matrix_data);

		auto step = 1.0 / n;

		for (auto i = 0u; i != n; ++i) {
			auto pos = m_positions.position(i * step);
			auto tgt = normalize(
			        (m_positions.position((i + 1) * step) - m_positions.position((i - 1) * step)));

			auto tmp = m_normals.position(i * step);
			auto nml = normalize((dot(tmp, tgt) != 0.0 ? tmp - tgt * dot(tmp, tgt) : tmp));
			auto btg = cross(nml, tgt);

			for (auto j = 0u; j != 2; ++j) {
				const float s[2] = {-3.0, 3.0};
				auto p3 = pos + btg * s[j];

				auto matrix = Mat4f(Vec4f(btg, 0), Vec4f(nml, 0), Vec4f(tgt, 0), Vec4f(p3, 1));

				*p = matrix;
				p++;
			}
		}
		assert(p == end(matrix_data));
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

		return {Vec4f(btg, 0), Vec4f(nml, 0), Vec4f(tgt, 0), Vec4f(pos, 1)};
	}

	Vec3f position(double t) const { return m_positions.position(t); }
	Vec3f normal(double t) const { return m_normals.position(t); }
	uint32_t getCount() const { return m_count; }

private:
	std::vector<Vec3f> makePositions()
	{
		return {
		        {10.0,  0.0,   70.0 },
                        {60.0,  0.0,   60.0 },
                        {95.0,  0.0,   0.0  },
                        {60.0,  0.0,   -45.0},
		        {10.0,  0.0,   -60.0},
                        {-25.0, 20.0,  -60.0},
                        {-60.0, 40.0,  -60.0},
                        {-80.0, 40.0,  -30.0},
		        {-25.0, 40.0,  10.0 },
                        {35.0,  40.0,  10.0 },
                        {80.0,  0.0,   0.0  },
                        {45.0,  -40.0, -10.0},
		        {0.0,   -40.0, -10.0},
                        {-50.0, -40.0, -10.0},
                        {-90.0, -40.0, 30.0 },
                        {-70.0, -40.0, 70.0 },
		        {-30.0, -20.0, 70.0 },
		};
	}

	std::vector<Vec3f> makeNormals()
	{
		return {
		        {0.0,  1.0,  0.0 },
                        {0.0,  1.0,  -1.0},
                        {-1.0, 0.0,  0.0 },
                        {0.0,  1.0,  2.0 },
                        {0.0,  1.0,  1.0 },
		        {1.0,  1.0,  0.5 },
                        {0.0,  1.0,  1.0 },
                        {1.0,  0.0,  0.0 },
                        {0.0,  1.0,  -1.0},
                        {0.0,  1.0,  0.0 },
		        {1.0,  0.0,  0.0 },
                        {0.0,  -1.0, 1.0 },
                        {0.0,  0.0,  1.0 },
                        {0.0,  1.0,  1.0 },
                        {1.0,  0.0,  0.0 },
		        {0.0,  1.0,  -1.0},
                        {-1.0, 1.0,  0.0 },
		};
	}
	CubicBezierLoop<Vec3f, double> m_positions;
	CubicBezierLoop<Vec3f, double> m_normals;
	uint32_t m_count;
};

class MotionBlurBuffers : public SpuFrame {
public:
	void resize(int32_t width, int32_t height)
	{
		// tex
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_RECTANGLE},
			        {"min_filter",  GL_NEAREST          },
			        {"mag_filter",  GL_NEAREST          },
			        {"width",       width               },
			        {"height",      height              },
			        {"wrap_s",      GL_CLAMP_TO_EDGE    },
			        {"wrap_t",      GL_CLAMP_TO_EDGE    },
			        {"iformat",     GL_RGB8             },
			        {"auto_mipmap", 0                   },
			};
			m_tex[0].init(attrs);
			m_tex[1].init(attrs);

			auto bgcolor = ezero<Vec4f>();
			m_tex[0].send(bgcolor.f, GL_RGBA32F, nullptr, nullptr, true);
			m_tex[1].send(bgcolor.f, GL_RGBA32F, nullptr, nullptr, true);
		}

		// frame
		{
			auto bgcolor = ezero<Vec4f>();
			auto viewport = Rectf(0, 0, width, height);
			Attrs attrs = {
			        {"viewport0",         viewport             },
			        {"color0.texture_id", m_tex[0].id()        },
			        {"depth.target",      GL_RENDERBUFFER      },
			        {"depth.iformat",     GL_DEPTH_COMPONENT32F},
			        {"bgcolor0",          bgcolor              },
			};
			SpuFrame::init(attrs);
		}
	}

	void begin()
	{
		SpuFrame::begin();
		SpuFrame::clear();
	}
	SpuTexture &getCurrent() { return m_tex[0]; }
	SpuTexture &getPrevious() { return m_tex[1]; }

private:
	SpuTexture m_tex[2];
};

class CheckerTexture : public SpuTexture {
public:
	CheckerTexture()
	{
		auto image = images::CheckerRedBlack(256, 256, 8, 8);

		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D          },
		        {"iformat",    GL_R8                  },
		        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter", GL_LINEAR              },
		        {"width",      image.width()          },
		        {"height",     image.height()         },
		        {"wrap_s",     GL_REPEAT              },
		        {"wrap_t",     GL_REPEAT              },
		        {"data",       image.data()           },
		};
		SpuTexture::init(attrs);
	}
};

class App : public SpuPage {
public:
	Uniforms m_unifs;
	Instances m_instances;
	MotionBlurBuffers m_blurBuffers;
	CheckerTexture m_checkerTexture;
	uint32_t m_screenWidth;

	shapes::Array m_cube;
	shapes::Array m_arrow;

	shapes::Array m_screen;

	App(const char *name) : SpuPage(name, true), m_instances(m_unifs) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shape_shader_attrs = {
		        {"frag", c_shape_frag},
		        {"vert", c_shape_vert},
		};
		Attrs blur_shader_attrs = {
		        {"frag", c_blur_frag},
		        {"vert", c_blur_vert},
		};

		m_cube.initShader(shape_shader_attrs, Attrs(m_unifs));
		m_arrow.initShader(shape_shader_attrs, Attrs(m_unifs));
		m_screen.initShader(blur_shader_attrs, Attrs(m_unifs));

		m_cube.initArray(shapes::Cube(), {"position", "normal", "tangent", "texcoord"});

		File input_file("assets/models/arrow_z.obj", "rb");

		shapes::ObjMesh mesh(input_file, shapes::ObjMesh::LoadingOptions(false).normals().materials());
		m_arrow.initArray(mesh, {"position", "normal"});
		m_screen.initArray(shapes::Screen(), {"position", "texcoord"});
		m_unifs.u_checker_tex = m_checkerTexture.id();

		{
			m_screenWidth = int32_t(viewport(0).sx);
			m_blurBuffers.resize(int32_t(viewport(0).sx), int32_t(viewport(0).sy));

			auto viewscreen = math::perspective(viewport(0), 60, 1, 300);

			m_unifs.u_viewsceen = viewscreen;
			m_unifs.u_screen_size = Vec2f(viewport(0).sx, viewport(0).sy);
			m_unifs.u_current_frame = m_blurBuffers.getCurrent().id();
			m_unifs.u_previous_frames = m_blurBuffers.getPrevious().id();
		}
	}

	void render() override
	{
		auto samples = 8;
		auto esec = getSeconds().current();

		for (auto s = 0; s != samples; ++s) {
			// draw objects off-screen
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.use();

			m_blurBuffers.begin();

			auto pos = esec / 20.0;

			auto e = m_instances.position(pos - 0.03 + sin(esec / 7.0 * math::two_pi()) * 0.01);
			auto c = m_instances.position(pos + 0.02 + sin(esec / 11.0 * math::two_pi()) * 0.01);
			auto u = m_instances.normal(pos - 0.02 + sin(esec / 9.0 * math::two_pi()) * 0.02);

			m_unifs.u_worldview = math::lookat(e, c, u);
			m_unifs.u_single_model = 0;

			m_cube.setInstanceCount(m_instances.getCount());
			m_cube.draw(nullptr);
			m_cube.setInstanceCount(1);

			m_unifs.u_single_model = 1;
			m_unifs.u_single_nodeworld = m_instances.makeMatrix(
			        pos - 0.007 + sin(esec / 13.0 * math::two_pi()) * 0.007, 0.001);

			m_arrow.draw(nullptr);

			m_unifs.u_single_nodeworld = m_instances.makeMatrix(
			        pos + 0.013 + sin(esec / 7.0 * math::two_pi()) * 0.007, 0.001);
			m_arrow.draw(nullptr);

			// motion blur
			m_blurBuffers.end();

			renderstate.flags.depth_test = false;
			renderstate.use();

			m_screen.draw(nullptr);

			m_blurBuffers.getCurrent().copy(m_blurBuffers.getPrevious().id());
		}
		m_blurBuffers.getCurrent().copy(0);
		m_unifs.u_splitter = (sin(esec / 20.0 * math::two_pi()) * 0.5 + 0.5) * m_screenWidth;
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("031_motion_blur");
}  // namespace
}  // namespace spu::oglplus
