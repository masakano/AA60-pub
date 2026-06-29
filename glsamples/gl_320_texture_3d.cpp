//
// App :
//
#include "base_app.h"
#include <smath/perlin_noisef.h>

namespace spu {
namespace {

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(4*(gl_VertexID%2)-1, 4*(gl_VertexID/2)-1, 0, 1);                \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler3D u_diffuse;                                                           \n"
    "uniform mat4 u_orientation;                                                            \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    vec3 sNorm = mat3(u_orientation) * vec3(gl_FragCoord.xy / vec2(1280, 720) - 0.5, 0.0);\n"
    "    float d = texture(u_diffuse, sNorm * 0.707106781 + 0.5).r;                         \n"
    "    color = vec4(d, d, d, 1);                                                          \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shader;
	Vec3f m_orientation;
	Mat4f u_orientation;  // this is bound to "Orientation" in shader
	uint32_t u_diffuse;

	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, c_orange) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			// ImprovedNoise noise;
			PerlinNoisef noise;

			const uint32_t c_size(128);
			std::vector<float> datas(c_size * c_size * c_size);
			for (auto k = 0u; k < c_size; ++k) {
				for (auto j = 0u; j < c_size; ++j) {
					for (auto i = 0u; i < c_size; ++i) {
						datas[i + j * c_size + k * c_size * c_size] = noise.noisef(
						        Vec3f(i, j, k) / float((c_size / 8 - 1)));
					}
				}
			}

			Attrs attrs = {
			        {"target",      GL_TEXTURE_3D           },
			        {"iformat",     GL_R32F                 },
			        {"data",        datas.data()            },
			        {"width",       c_size                  },
			        {"height",      c_size                  },
			        {"depth",       c_size                  },
			        {"base_level",  0                       },
			        {"max_level",   int(log2(float(c_size)))},
			        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR },
			        {"mag_filter",  GL_LINEAR               },
			        {"wrap_s",      GL_CLAMP_TO_EDGE        },
			        {"wrap_t",      GL_CLAMP_TO_EDGE        },
			        {"wrap_r",      GL_CLAMP_TO_EDGE        },
			        {"auto_mipmap", 1                       },
			};
			u_diffuse = spu_texture_new(attrs);
		}
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_orientation", &u_orientation},
			        {"u_diffuse",     &u_diffuse    },
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			const std::vector<v2fv2f_t> c_vertices = squareTriangles<v2fv2f_t>();
			Attrs attrs = {
			        {"shader_id",    m_shader.id()    },
                                {"a.a_position", 2                },
                                {"a.a_texcoord", 2                },
			        {"data",         c_vertices.data()},
                                {"nelem",        c_vertices.size()},
			};
			m_arrayId = spu_array_new(attrs);
		}
	}

	Mat4f yawPitchRoll(float yaw, float pitch, float roll)
	{
		return Mat4f().rot("y", -yaw).rot("x", -pitch).rot("z", -roll);
	}

	void render() override
	{
		m_orientation += Vec3f(0.020, 0.013, 0.011);
		u_orientation = yawPitchRoll(m_orientation.x, m_orientation.y, m_orientation.z);
		m_shader.use();
		spu_array_draw(m_arrayId, GL_TRIANGLES);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_320_texture_3d");
}  // namespace
}  // namespace spu
