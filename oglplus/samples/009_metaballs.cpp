//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "in vec2 a_position;                                                                    \n"
    "out vec3 f_position;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       f_position = vec3(a_position, 0.0);                                             \n"
    "       gl_Position = vec4(f_position, 1.0);                                            \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler1D u_texture;                                                           \n"
    "in vec3 f_position;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "                                                                                       \n"
    "const vec3 u_ambient_color = vec3(0.3, 0.4, 0.9);                                      \n"
    "const vec3 u_albedo_color = vec3(0.5, 0.6, 1.0);                                       \n"
    "const vec3 light_dir = normalize(vec3(1.0, 1.0, 1.0));                                 \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       int i = 0, n = textureSize(u_texture, 0);                                       \n"
    "                                                                                       \n"
    "       float inv_n = 1.0/n;                                                            \n"
    "       float value = 0.0;                                                              \n"
    "       vec3 normal = vec3(0.0, 0.0, 0.0);                                              \n"
    "       while (i != n)                                                                  \n"
    "       {                                                                               \n"
    "               vec4 metaball = texelFetch(u_texture, i, 0);                            \n"
    "               float radius = metaball.w;                                              \n"
    "               vec3 vect = f_position - metaball.xyz;                                  \n"
    "               float tmp = pow(radius,2.0)/dot(vect, vect)-0.25;                       \n"
    "               value += tmp;                                                           \n"
    "               float mul = max(tmp, 0.0);                                              \n"
    "               normal += mul*vec3(vect.xy, mul*inv_n/radius);                          \n"
    "               ++i;                                                                    \n"
    "       }                                                                               \n"
    "       if (value > 0.0)                                                                \n"
    "       {                                                                               \n"
    "               float diffuse = 1.4*max(dot(                                            \n"
    "                       light_dir,                                                      \n"
    "                       normalize(normal)                                               \n"
    "               ), 0.0);                                                                \n"
    "               float ambient = 0.3;                                                    \n"
    "               final_color =                                                           \n"
    "                       ambient*u_ambient_color+                                        \n"
    "                       diffuse*u_albedo_color;                                         \n"
    "       }                                                                               \n"
    "       else final_color = vec3(0.4, 0.4, 0.4);                                         \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	SpuShader m_shader;
	SpuArray m_array;
	SpuTexture m_texture;

	std::vector<CubicBezierLoop<Vec4f, double>> m_ballPaths;
	uint32_t u_texture;

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		for (auto i = 0u; i != 64; ++i) {
			auto j = 0;
			auto n = 3 + rand() % 3;
			std::vector<Vec4f> points(n);
			while (j != n) {
				points[j]
				        = {1.4f * frand() - 0.7f, 1.4f * frand() - 0.7f, 0.0f,
				           0.1f * frand() + 0.1f};
				++j;
			}
			CubicBezierLoop<Vec4f, double> ball_path;
			ball_path.init(points);
			m_ballPaths.emplace_back(ball_path);
		}

		Attrs shader_attrs = {
		        {"frag", c_frag},
		        {"vert", c_vert},
		};

		Attrs unif_attrs = {
		        {"u_texture", &u_texture},
		};

		shapes::loadShader(m_shader, shader_attrs, unif_attrs);

		std::vector<vec2f_t> vertices = {
		        {-1.0, -1.0},
		        {-1.0, +1.0},
		        {+1.0, -1.0},
		        {+1.0, +1.0},
		};

		Attrs vert_attrs = {
		        {"shader_id",    m_shader.id()  },
		        {"data",         vertices.data()},
		        {"nelem",        vertices.size()},
		        {"a.a_position", 2              },
		};

		m_array.init(vert_attrs);

		Attrs tex_attrs = {
		        {"target",     GL_TEXTURE_1D     },
                        {"iformat",    GL_RGBA32F        },
                        {"width",      m_ballPaths.size()},
		        {"min_filter", GL_NEAREST        },
                        {"mag_filter", GL_NEAREST        },
                        {"wrap_s",     GL_MIRRORED_REPEAT},
		};

		m_texture.init(tex_attrs);
		u_texture = m_texture.id();
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto metaball_count = m_ballPaths.size();

		std::vector<Vec4f> metaballs(metaball_count);
		for (auto ball = 0u; ball != metaball_count; ++ball) {
			auto pos = m_ballPaths[ball].position(esec / 10.0);
			metaballs[ball] = pos;
		}

		spu_texture_send(u_texture, metaballs.data(), GL_RGBA32F);
		m_shader.use();
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("009_metaballs");
}  // namespace
}  // namespace spu::oglplus
