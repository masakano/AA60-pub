//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/gradient.hpp>
#include <shapes/cube.hpp>
#include <shapes/screen.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "layout (std140) uniform u_offset_block {vec3 u_offset[16*16*16];};                     \n"
    "in vec3 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               vec4(a_position+u_offset[gl_InstanceID],                                \n"
    "1.0);                                                                                  \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "out float frag_value;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       frag_value = 1.0/16.0;                                                          \n"
    "}                                                                                      \n"
};

const char *c_screen_vert =  {
    "#version 330                                                                           \n"
    "uniform vec2 u_screen_size;                                                            \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       f_texcoord = a_texcoord*u_screen_size;                                          \n"
    "}                                                                                      \n"
};

const char *c_screen_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler1D u_palette;                                                           \n"
    "uniform sampler2DRect u_texture;                                                       \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float overdraw = texture(u_texture, f_texcoord).r;                              \n"
    "       final_color = texture(u_palette, overdraw);                                     \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Vec2f u_screen_size;
	Vec4f u_offset_block[16 * 16 * 16];
	uint32_t u_palette;
	uint32_t u_texture;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen",    &u_viewsceen   },
                        {"u_worldview",    &u_worldview   },
		        {"u_screen_size",  &u_screen_size },
                        {"u_offset_block", &u_offset_block},
		        {"u_palette",      &u_palette     },
                        {"u_texture",      &u_texture     },
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

struct UOffsetData : public std::vector<Vec4f> {
	explicit UOffsetData(size_t n) : std::vector<Vec4f>(n * n * n)
	{
		float d = 1.414;
		auto p = begin();
		for (auto k = 0u; k != n; ++k) {
			float z = (k - (n + 1) * 0.5) * d;
			for (auto j = 0u; j != n; ++j) {
				float y = (j - (n + 1) * 0.5) * d;
				for (auto i = 0u; i != n; ++i) {
					float x = (i - (n + 1) * 0.5) * d;
					*p++ = {x, y, z, 0};
				}
			}
		}
	}
};

class App : public SpuPage {
public:
	static constexpr auto c_n = 16u;

	Uniforms m_unifs;

	shapes::Array m_cube;
	shapes::Array m_screen;

	SpuFrame m_frame;
	SpuTexture m_palette;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Vec2f u_screen_size;
	Vec4f u_offset_block[16 * 16 * 16];

	explicit App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shape_shader_attrs = {
		        {"frag", c_shape_frag},
		        {"vert", c_shape_vert},
		};
		Attrs screen_shader_attrs = {
		        {"frag", c_screen_frag},
		        {"vert", c_screen_vert},
		};

		m_cube.initShader(shape_shader_attrs, Attrs(m_unifs));
		m_cube.initArray(shapes::Cube(), {"position"});
		m_cube.setInstanceCount(c_n * c_n * c_n);

		m_screen.initShader(screen_shader_attrs, Attrs(m_unifs));
		m_screen.initArray(shapes::Screen(), {"position", "texcoord"});

		{
			auto data = UOffsetData(c_n);
			memcpy(m_unifs.u_offset_block, data.data(), data.size() * sizeof(data[0]));
		}

		{
			auto image = images::LinearGradient(
			        16, ezero(),
			        std::map<float, Vec3f>({
			                {0.0 / 16.0,  {0.0, 0.0, 0.0}},
			                {1.0 / 16.0,  {0.5, 0.0, 1.0}},
			                {3.0 / 16.0,  ez()           },
			                {6.0 / 16.0,  {0.0, 0.6, 0.6}},
			                {8.0 / 16.0,  ey()           },
			                {11.0 / 16.0, {0.6, 0.6, 0.0}},
			                {13.0 / 16.0, {1.0, 0.1, 0.0}},
			                {16.0 / 16.0, {0.7, 0.0, 0.0}}
                        }));

			Attrs tex_attrs = {
			        {"target",      GL_TEXTURE_1D   },
                                {"iformat",     GL_RGB8         },
			        {"data",        image.data()    },
                                {"width",       image.width()   },
			        {"min_filter",  GL_NEAREST      },
                                {"mag_filter",  GL_NEAREST      },
			        {"wrap_s",      GL_CLAMP_TO_EDGE},
                                {"auto_mipmap", 0               },
			};
			m_palette.init(tex_attrs);
			m_unifs.u_palette = m_palette.id();
		}

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.cull_face = true;
		renderstate.flags.ccw = true;  // correct?
		                               // renderstate.use();

		{
			m_unifs.u_viewsceen = math::perspective(viewport(0), 70, 1, 100);
			m_unifs.u_screen_size = Vec2f(viewport(0).sx, viewport(0).sy);

			Attrs attrs = {
			        {"viewport0",          viewport(0)          },
                                {"color0.target",      GL_TEXTURE_RECTANGLE },
			        {"color0.iformat",     GL_R8                },
                                {"color0.auto_mipmap", 0                    },
			        {"depth.target",       GL_RENDERBUFFER      },
                                {"depth.iformat",      GL_DEPTH_COMPONENT32F},
			        {"depth.auto_mipmap",  0                    },
			};
			m_frame.init(attrs);
			m_unifs.u_texture = m_frame.getBuffer("color0").id();
		}
	}

	void renderOffscreen()
	{
		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.blend = true;
		renderstate.blend_func = {
		        GL_ONE,
		        GL_ONE,
		        GL_ONE,
		        GL_ONE,
		};
		renderstate.use();

		m_frame.begin();
		m_frame.clear();

		auto esec = getSeconds().current();
		m_unifs.u_worldview = Mat4f::orbiting(ezero(), esec, 27, 0, 0, 0, 15.6, 0, 80, 23);

		m_cube.draw(nullptr);
		m_frame.end();
	}

	void renderOnscreen(double)
	{
		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = false;
		renderstate.flags.blend = false;
		renderstate.use();
		m_screen.draw(nullptr);
	}

	void render() override
	{
		auto esec = getSeconds().current();
		renderOffscreen();
		renderOnscreen(esec);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("021_overdraw");
}  // namespace
}  // namespace spu::oglplus
