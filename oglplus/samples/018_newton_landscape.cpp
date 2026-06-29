//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>
#include <images/newton.hpp>
#include <shapes/plane.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform sampler2D u_texture;                                                           \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_light;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       float o = 0.0;                                                                  \n"
    "       float s[9];                                                                     \n"
    "       int k=0;                                                                        \n"
    "       for (int y=-1; y!=2; ++y)                                                       \n"
    "       for (int x=-1; x!=2; ++x)                                                       \n"
    "       {                                                                               \n"
    "               s[k] = sqrt(texture(                                                    \n"
    "                       u_texture,                                                      \n"
    "                       a_texcoord*3.0+                                                 \n"
    "                       vec2(x, y)/128.0                                                \n"
    "               ).r);                                                                   \n"
    "               o += s[k++];                                                            \n"
    "       }                                                                               \n"
    "       gl_Position.y += o*0.5;                                                         \n"
    "       vec3 c = vec3( 0.0, s[4], 0.0);                                                 \n"
    "       float d = 1.0/32.0;                                                             \n"
    "       f_normal = normalize(                                                           \n"
    "               cross(                                                                  \n"
    "                       vec3( 0.0, s[1],  -d) - c,                                      \n"
    "                       vec3(  -d, s[3], 0.0) - c                                       \n"
    "               )+                                                                      \n"
    "               cross(                                                                  \n"
    "                       vec3(   d, s[5], 0.0) - c,                                      \n"
    "                       vec3( 0.0, s[1],  -d) - c                                       \n"
    "               )+                                                                      \n"
    "               cross(                                                                  \n"
    "                       vec3( 0.0, s[7],   d) - c,                                      \n"
    "                       vec3(   d, s[5], 0.0) - c                                       \n"
    "               )+                                                                      \n"
    "               cross(                                                                  \n"
    "                       vec3(  -d, s[3], 0.0) - c,                                      \n"
    "                       vec3( 0.0, s[7],   d) - c                                       \n"
    "               )                                                                       \n"
    "       );                                                                              \n"
    "       f_light = u_light_pos - gl_Position.xyz;                                        \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               gl_Position;                                                            \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = length(f_light);                                                      \n"
    "       float d = l > 0? dot(                                                           \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light)                                                      \n"
    "       ) / l : 0.0;                                                                    \n"
    "       float i = 0.1 + 1.2*max(d, 0.0) + 4.2*pow(d, 2.0);                              \n"
    "       final_color = vec4(i*0.7, i*0.7, i*0.3, 1.0);                                   \n"
    "}                                                                                      \n"
};

uint32_t get_format(const images::Image &image)
{
	auto format = uint32_t(image.format());
	auto type   = uint32_t(image.type());
	
	switch (format) {
	case GL_RED:
		switch (type) {
		case GL_UNSIGNED_BYTE: return GL_R8;
		case GL_FLOAT: return GL_R32F;
		default: assert(0);
		}
	case GL_RGB:
		switch (type) {
		case GL_UNSIGNED_BYTE: return GL_RGB8;
		case GL_FLOAT: return GL_RGB32F;
		default: assert(0);
		}
	case GL_RGBA:
		switch (type) {
		case GL_UNSIGNED_BYTE: return GL_RGBA8;
		case GL_FLOAT: return GL_RGBA32F;
		default: assert(0);
		}
	}
	assert(0);
}

/* clang-format on */
class App : public SpuPage {
public:
	static constexpr auto c_grid_side = 128u;

	shapes::Array m_array;
	SpuTexture m_texture;

	CubicBezierLoop<Vec3f, double> m_lightPath;

	uint32_t u_texture;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;

	App(const char *name) : SpuPage(name, true, {0.8, 0.8, 0.7, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_lightPath.init(std::vector<Vec3f>{
		        {-3.0, 2.0, -3.5},
		        {+0.0, 5.0, +0.5},
		        {+3.0, 3.0, +3.0},
		        {+3.0, 3.0, -3.0},
		        {+0.0, 5.0, +0.5},
		        {-3.2, 2.0, +3.0},
		});

		{
			Attrs shader_attrs = {
			        {"frag", c_frag},
			        {"vert", c_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
                                {"u_worldview", &u_worldview},
			        {"u_nodeworld", &u_nodeworld},
                                {"u_light_pos", &u_light_pos},
			        {"u_texture",   &u_texture  },
			};

			m_array.initShader(shader_attrs, unif_attrs);
		}
		{
			shapes::Plane plane_shape(
			        {0, 0, +0}, {9, 0, +0}, {0, 0, -9}, c_grid_side * 3, c_grid_side * 3);

			m_array.initArray(plane_shape, {"position", "texcoord"});

			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.cull_face = GL_BACK;
		}

		{
			const auto image = images::NewtonFractal(
			        c_grid_side, c_grid_side, Vec3f(0.0, 0.1, 0.2), Vec3f(1.0, 0.8, 0.9),
			        Vec2f(-1, -1), Vec2f(1, 1), images::NewtonFractal::X3Minus1(),
			        images::NewtonFractal::DefaultMixer());

			Attrs tex_attrs = {
			        {"target",     GL_TEXTURE_2D     },
                                {"iformat",    get_format(image) },
			        {"width",      image.width()     },
                                {"height",     image.height()    },
			        {"min_filter", GL_LINEAR         },
                                {"mag_filter", GL_LINEAR         },
			        {"wrap_s",     GL_MIRRORED_REPEAT},
                                {"wrap_t",     GL_MIRRORED_REPEAT},
			        {"data",       image.data()      },
			};
			m_texture.init(tex_attrs);
			u_texture = m_texture.id();
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 60, 1, 100);
		u_worldview = Mat4f::orbiting(Vec3f(0.0, 0.5, 0.0), esec, 6.8, 0, 0, 0, 10.85, 55, -30, 20);
		u_light_pos = m_lightPath.position(esec / 10.0);
		m_array.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("018_newton_landscape");
}  // namespace
}  // namespace spu::oglplus
