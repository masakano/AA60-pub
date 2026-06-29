//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/load.hpp>

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "mat4 u_matrix = u_viewsceen*u_worldview;                                               \n"
    "in vec3 a_corner;                                                                      \n"
    "out vec3 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_matrix * vec4(a_corner * 10.0, 1.0);                            \n"
    "       f_texcoord = a_corner;                                                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform samplerCube u_texture;                                                         \n"
    "in vec3 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(texture(u_texture, normalize(f_texcoord)).rgb, 1.0);         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;
	SpuTexture m_texture;

	Mat4f u_viewsceen;
	Mat4f u_worldview;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		{
			Attrs shader_attrs = {
			        {"frag", c_frag},
			        {"vert", c_vert},
			};

			Attrs unif_attrs = {
			        {"u_viewsceen", &u_viewsceen},
			        {"u_worldview", &u_worldview},
			        {"u_texture",   &u_texture  },
			};
			shapes::loadShader(m_shader, shader_attrs, unif_attrs);
		}
		{
			const std::vector<vec3f_t> sky_box_corners = {
			        -eone(),
			        {+1.0, -1.0, -1.0},
			        {-1.0, +1.0, -1.0},
			        {+1.0, +1.0, -1.0},
			        {-1.0, -1.0, +1.0},
			        {+1.0, -1.0, +1.0},
			        {-1.0, +1.0, +1.0},
			        {+1.0, +1.0, +1. },
			};

			const std::vector<uint32_t> sky_box_indices
			        = {1, 3, 5, 7, 9, 4, 6, 0, 2, 9, 2, 6, 3, 7, 9,
			           4, 0, 5, 1, 9, 5, 7, 4, 6, 9, 0, 2, 1, 3, 9};

			Attrs vert_attrs = {
			        {"shader_id",  m_shader.id()         },
			        {"data",       sky_box_corners.data()},
			        {"nelem",      sky_box_corners.size()},
			        {"a.a_corner", 3                     },
			};

			m_array.init(vert_attrs);
			m_array.send(sky_box_indices, -1);

			Attrs array_attrs = {
			        {"restart", 9},
			};
			m_array.set(array_attrs);
		}
		{
			images::Image image0 = images::LoadTexture("cloudy_day-cm_0", false, false);
			images::Image image1 = images::LoadTexture("cloudy_day-cm_1", false, false);
			images::Image image2 = images::LoadTexture("cloudy_day-cm_2", false, false);
			images::Image image3 = images::LoadTexture("cloudy_day-cm_3", false, false);
			images::Image image4 = images::LoadTexture("cloudy_day-cm_4", false, false);
			images::Image image5 = images::LoadTexture("cloudy_day-cm_5", false, false);

			auto unit = image0.dataSize();
			std::vector<uint8_t> pix(unit * 6);

			memcpy(&pix[unit * 0], image0.data(), image0.dataSize());
			memcpy(&pix[unit * 1], image1.data(), image1.dataSize());
			memcpy(&pix[unit * 2], image2.data(), image2.dataSize());
			memcpy(&pix[unit * 3], image3.data(), image3.dataSize());
			memcpy(&pix[unit * 4], image4.data(), image4.dataSize());
			memcpy(&pix[unit * 5], image5.data(), image5.dataSize());

			int32_t cube_target[6] = {
			        GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
			};

			Attrs tex_attrs = {
			        {"target",      GL_TEXTURE_CUBE_MAP},
			        {"iformat",     GL_RGBA8           },
			        {"width",       image0.width()     },
			        {"height",      image0.height()    },
			        {"min_filter",  GL_LINEAR          },
			        {"mag_filter",  GL_LINEAR          },
			        {"wrap_s",      GL_CLAMP_TO_EDGE   },
			        {"wrap_t",      GL_CLAMP_TO_EDGE   },
			        {"data",        pix.data()         },
			        {"cube_target", cube_target        },
			};
			m_texture.init(tex_attrs);
			u_texture = m_texture.id();
		}
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_viewsceen = math::perspective(viewport(0), 48, 1, 100);
		u_worldview = Mat4f::orbiting(ezero(), esec, 3.5, 0, 0, 0, 13.0, 0, -85, 19);
		m_shader.use();
		m_array.draw(GL_TRIANGLE_STRIP);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("017_sky_box");
}  // namespace
}  // namespace spu::oglplus
