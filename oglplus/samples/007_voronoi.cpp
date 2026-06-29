//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>

#include <images/random.hpp>
#include <shapes/screen.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform vec2 u_offset;                                                                 \n"
    "uniform float u_scale;                                                                 \n"
    "in vec4 a_position;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       f_texcoord = u_scale * (0.5*a_position.xy + u_offset);                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform float u_scale;                                                                 \n"
    "uniform sampler2D u_texture;                                                           \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "const vec2 offs[9] = vec2[9](                                                          \n"
    "       vec2(-1,-1),                                                                    \n"
    "       vec2(-1, 0),                                                                    \n"
    "       vec2(-1, 1),                                                                    \n"
    "       vec2( 0,-1),                                                                    \n"
    "       vec2( 0, 0),                                                                    \n"
    "       vec2( 0, 1),                                                                    \n"
    "       vec2( 1,-1),                                                                    \n"
    "       vec2( 1, 0),                                                                    \n"
    "       vec2( 1, 1)                                                                     \n"
    ");                                                                                     \n"

    "float dist(vec2 tc, vec2 ofs)                                                          \n"
    "{                                                                                      \n"
    "       vec2 cc = floor(tc+ofs);                                                        \n"
    "       vec2 cp = texture(u_texture, cc/textureSize(u_texture, 0)).xy;                  \n"
    "       return distance(tc, cc+cp);                                                     \n"
    "}                                                                                      \n"

    "vec3 point_color(vec2 tc, vec2 ofs)                                                    \n"
    "{                                                                                      \n"
    "       vec2 cc = floor(tc+ofs);                                                        \n"
    "       return texture(u_texture, cc/textureSize(u_texture, 0)).rgb;                    \n"
    "}                                                                                      \n"

    "vec3 voronoi(vec2 tc)                                                                  \n"
    "{                                                                                      \n"
    "       float md = 2.0;                                                                 \n"
    "       int mc = 9;                                                                     \n"
    "       for (int c=0; c<9; ++c)                                                         \n"
    "       {                                                                               \n"
    "               float d = dist(tc, offs[c]);                                            \n"
    "               if (md > d)                                                             \n"
    "               {                                                                       \n"
    "                       md = d;                                                         \n"
    "                       mc = c;                                                         \n"
    "               }                                                                       \n"
    "       }                                                                               \n"
    "       return mix(                                                                     \n"
    "               point_color(tc, offs[mc])*mix(1.4, 0.5, md),                            \n"
    "               vec3(0, 0, 0),                                                          \n"
    "               pow(exp(1-md*512/u_scale), 2.0)                                         \n"
    "       );                                                                              \n"
    "}                                                                                      \n"

    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = voronoi(f_texcoord);                                              \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	shapes::Array m_screen;
	SpuTexture m_texture;

	Vec2f u_offset;
	float u_scale;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_offset",  &u_offset },
		        {"u_scale",   &u_scale  },
		        {"u_texture", &u_texture},
		};

		m_screen.initShader(shader_attrs, unif_attrs);
		m_screen.initArray(shapes::Screen(), {"position"});

		srand(0);  // not use rand()..
		auto image = images::RandomRGBUByte(256, 256);

		Attrs texture_attrs = {
		        {"target",      GL_TEXTURE_2D             },
		        {"iformat",     GL_RGB8                   },
		        {"width",       image.width()             },
		        {"height",      image.height()            },
		        {"data",        image.data()},
		        {"min_filter",  GL_NEAREST                },
		        {"mag_filter",  GL_NEAREST                },
		        {"wrap_s",      GL_REPEAT                 },
		        {"wrap_t",      GL_REPEAT                 },
		        {"auto_mipmap", 0                         },
		};
		m_texture.init(texture_attrs);
		u_texture = m_texture.id();
	}

	void render() override
	{
		auto esec = getSeconds().current();
		u_offset = {
		        float(cos(esec * 2 / 59 * math::two_pi())),
		        float(sin(esec * 2 / 61 * math::two_pi())),
		};
		u_scale = 25 + sin(esec / 19 * math::two_pi()) * 24;
		m_screen.draw(nullptr);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("007_voronoi");
}  // namespace
}  // namespace spu::oglplus
