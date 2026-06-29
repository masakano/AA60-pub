//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cube.hpp>
#include <math/curve.hpp>
#include "replace_text.h"

namespace spu::oglplus {
namespace {
/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldtexcs[c_max_lights];                                             \n"
    "uniform vec3 u_light_positions[c_max_lights];                                            \n"
    "uniform vec3 u_offsets[c_cube_count];                                                    \n"
    "uniform int u_use_offset;                                                              \n"
    "uniform int u_light_count;                                                             \n"
    "in vec3 a_position, a_normal;                                                          \n"
    "out gl_PerVertex {                                                                     \n"
    "       vec4 gl_Position;                                                               \n"
    "};                                                                                     \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_lights[c_max_lights];                                                         \n"
    "out vec4 f_texcoords[c_max_lights];                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 new_pos = a_position;                                                      \n"
    "       if (u_use_offset != 0)                                                          \n"
    "               new_pos += u_offsets[gl_InstanceID];                                    \n"
    "       gl_Position = vec4(new_pos, 1.0);                                               \n"
    "       f_normal = a_normal;                                                            \n"
    "       for (int i=0; i!=u_light_count; ++i)                                            \n"
    "       {                                                                               \n"
    "               f_lights[i] = (                                                         \n"
    "                       u_light_positions[i] -                                          \n"
    "                       gl_Position.xyz                                                 \n"
    "               );                                                                      \n"
    "               f_texcoords[i] =                                                        \n"
    "                       u_worldtexcs[i] *                                           \n"
    "                       gl_Position;                                                    \n"
    "       }                                                                               \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               gl_Position;                                                            \n"

    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_light_colors[c_max_lights];                                               \n"
    "uniform sampler2DShadow u_shadow_texs[c_max_lights];                                     \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_lights[c_max_lights];                                                          \n"
    "in vec4 f_texcoords[c_max_lights];                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 color = vec3(0.1, 0.1, 0.1);                                               \n"
    "       for (int i=0; i!=c_max_lights; ++i)                                               \n"
    "       {                                                                               \n"
    "               float d = dot(                                                          \n"
    "                       normalize(f_normal),                                            \n"
    "                       normalize(f_lights[i])                                          \n"
    "               );                                                                      \n"
    "               if (d > 0.0)                                                            \n"
    "               {                                                                       \n"
    "                       float l = sqrt(length(f_lights[i]));                            \n"
    "                       d /= l;                                                         \n"
    "                       vec3 coord = (                                                  \n"
    "                               f_texcoords[i].xyz /                                    \n"
    "                               f_texcoords[i].w                                        \n"
    "                       ) * 0.5 + 0.5;                                                  \n"
    "                       if (                                                            \n"
    "                               coord.x >= 0.0 &&                                       \n"
    "                               coord.x <= 1.0 &&                                       \n"
    "                               coord.y >= 0.0 &&                                       \n"
    "                               coord.y <= 1.0 &&                                       \n"
    "                               coord.z <= 1.0                                          \n"
    "                       )                                                               \n"
    "                       {                                                               \n"
    "                               d *= texture(                                           \n"
    "                                       u_shadow_texs[i],                               \n"
    "                                       coord                                           \n"
    "                               );                                                      \n"
    "                       }                                                               \n"
    "               }                                                                       \n"
    "               color += u_light_colors[i] *                                            \n"
    "                       3.2 * max(d, 0.0);                                              \n"
    "       }                                                                               \n"
    "       final_color = vec4(color, 1.0);                                                 \n"
    "}                                                                                      \n"
};

const char *c_shadow_frag =  {
    "#version 330                                                                           \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       final_color = vec4(0.0, 0.0, 0.0, 1.0);                                         \n"
    "}                                                                                      \n"
};

static constexpr int32_t c_max_lights = 3;
static constexpr int32_t c_cube_count = 49;
const Attrs replace_attrs = {
	{"c_max_lights", c_max_lights},
	{"c_cube_count", c_cube_count},
};


/* clang-format on */

class Uniforms {
public:
	Mat4f u_worldview;
	Mat4f u_viewsceen;
	Mat4f u_worldtexcs[c_max_lights];

	vec3f_t u_light_positions[c_max_lights];
	vec3f_t u_offsets[c_cube_count];
	vec3f_t u_light_colors[c_max_lights];

	uint32_t u_shadow_texs[c_max_lights];
	int32_t u_use_offset;
	int32_t u_light_count;

	Uniforms()
	{
		m_attrs = {
		        {"u_worldview",       &u_worldview      },
                        {"u_viewsceen",       &u_viewsceen      },
		        {"u_worldtexcs",      &u_worldtexcs     },
                        {"u_light_positions", &u_light_positions},
		        {"u_offsets",         &u_offsets        },
                        {"u_light_colors",    &u_light_colors   },
		        {"u_shadow_texs",     &u_shadow_texs    },
                        {"u_use_offset",      &u_use_offset     },
		        {"u_light_count",     &u_light_count    },
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class Cubes {
public:
	enum {
		e_shape = 0,
		e_shadow,
	};

	explicit Cubes(const Uniforms &unifs)
	{
		m_shapeArray.setMaxShaderType(2);  // 0:shape, 1:shadow

		{
			Attrs shader_attrs = {
			        {"vert", replaceText(c_vert, replace_attrs)      },
			        {"frag", replaceText(c_shape_frag, replace_attrs)},
			};
			m_shapeArray.initShader(shader_attrs, Attrs(unifs), 0);
		}
		{
			Attrs shader_attrs = {
			        {"vert", replaceText(c_vert, replace_attrs)       },
			        {"frag", replaceText(c_shadow_frag, replace_attrs) },
			};
			m_shapeArray.initShader(shader_attrs, Attrs(unifs), 1);
		}
		m_shapeArray.initArray(shapes::Cube(), {"position", "normal"});
	}

	SpuShader &getShader(int32_t type) { return m_shapeArray.getShader(type); }

	void useShader(int32_t type)
	{
		m_shapeArray.setShaderType(type);
		m_shapeArray.useShader();
	}

	void drawShape()
	{
		m_shapeArray.setShaderType(e_shape);
		m_shapeArray.draw(nullptr);
	}

	void drawShadow()
	{
		m_shapeArray.setShaderType(e_shadow);
		m_shapeArray.draw(nullptr);
	}

	void setOffsets(const std::vector<Vec3f> &offsets)
	{
		m_offsets = offsets;
		m_shapeArray.setInstanceCount(m_offsets.size());
	}

	const std::vector<Vec3f> &offsets() const { return m_offsets; }

private:
	shapes::Array m_shapeArray;
	std::vector<Vec3f> m_offsets;
};

class Plate : public SpuArray {
public:
	explicit Plate(Cubes &cubes) : m_cubes(cubes)
	{
		auto shader_id = m_cubes.getShader(m_cubes.e_shape).id();

		float positions[4 * 3]
		        = {-100.0, 0.0, 100.0, -100.0, 0.0, -100.0, +100.0, 0.0, 100.0, +100.0, 0.0, -100.0};

		float normals[4 * 3] = {
		        -0.1, 1.0, 0.1, -0.1, 1.0, -0.1, +0.1, 1.0, 0.1, +0.1, 1.0, -0.1,
		};

		Attrs position_attrs = {
		        {"shader_id",    shader_id},
		        {"a.a_position", 3        },
		        {"nelem",        4        },
		        {"data",         positions},
		};

		Attrs normal_attrs = {
		        {"a.a_normal", 3      },
		        {"nelem",      4      },
		        {"data",       normals},
		};

		m_array.init(position_attrs);
		m_array.aux(normal_attrs, 1);
	}

	void drawShape()
	{
		m_cubes.useShader(m_cubes.e_shape);  // borrow shader
		m_array.draw(GL_TRIANGLE_STRIP);
	}

	void drawShadow()
	{
		m_cubes.useShader(m_cubes.e_shadow);  // borrow shader
		m_array.draw(GL_TRIANGLE_STRIP);
	}

private:
	Cubes &m_cubes;
	SpuArray m_array;
};

class ShadowmapFrames : public std::vector<SpuFrame> {
public:
	ShadowmapFrames(int32_t tex_side, int32_t N)
	{
		for (auto i = 0; i < N; i++) {
			auto viewport = Rectf(0, 0, tex_side, tex_side);
			Attrs attrs = {
			        {"viewport0",          viewport                 },
			        {"color0.target",      GL_RENDERBUFFER          },
			        {"depth.target",       GL_TEXTURE_2D            },
			        {"depth.iformat",      GL_DEPTH_COMPONENT32F    },
			        {"depth.min_filter",   GL_LINEAR                },
			        {"depth.mag_filter",   GL_LINEAR                },
			        {"depth.wrap_s",       GL_CLAMP_TO_EDGE         },
			        {"depth.wrap_t",       GL_CLAMP_TO_EDGE         },
			        {"depth.compare_mode", GL_COMPARE_REF_TO_TEXTURE},
			        {"bgcolor0",           Vec4f(-1)                }, // no clear
			};
			push_back(SpuFrame(attrs));
		}
	}
};

class App : public SpuPage {
public:
	Uniforms m_unifs;
	Cubes m_cubes;
	Plate m_plate;

	ShadowmapFrames m_frames;

	std::vector<CubicBezierLoop<Vec3f, double>> m_lightPaths;
	std::vector<Vec3f> m_lightPositions;
	std::vector<Mat4f> m_lightProjMatrices;
	std::vector<Vec3f> m_lightColors;

	static std::vector<Vec3f> makeCubeOffsets(float distance, int32_t cubes_per_side)
	{
		int32_t instance_count = cubes_per_side * cubes_per_side;
		std::vector<Vec3f> offsets(instance_count);

		auto i = std::begin(offsets);
		auto e = std::end(offsets);

		float d = distance * cubes_per_side;

		for (auto z = 0; z != cubes_per_side; ++z) {
			for (auto x = 0; x != cubes_per_side; ++x) {
				assert(i != e);
				*i
				        = Vec3f((float(x) / (cubes_per_side - 1) - 0.5) * d, 0.5,
				                (float(z) / (cubes_per_side - 1) - 0.5) * d);
				++i;
			}
		}
		assert(i == e);
		return offsets;
	}

	std::vector<CubicBezierLoop<Vec3f, double>> makeLightPaths()
	{
		std::vector<CubicBezierLoop<Vec3f, double>> light_paths(3);

		light_paths[0].init({
		        {-9.0, 5.0,  8.0 },
                        {0.0,  6.0,  -9.0},
                        {9.0,  5.0,  9.0 },
                        {0.0,  15.0, 0.0 }
                });

		light_paths[1].init({
		        {-8.0, 5.0, 9.0 },
                        {-9.0, 9.0, -9.0},
                        {9.0,  8.0, -9.0},
                        {9.0,  4.0, 9.0 }
                });

		light_paths[2].init({
		        {-9.0, 7.0, 9.0 },
                        {9.0,  5.0, 9.0 },
                        {9.0,  5.0, -9.0},
                        {-9.0, 8.0, -9.0}
                });

		return light_paths;
	}

	App(const char *name)
	        : SpuPage(name, true, {0.1, 0.1, 0.1, 0.0}), m_cubes(m_unifs), m_plate(m_cubes),
	          m_frames(512, c_max_lights)
	{
	}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_cubes.setOffsets(makeCubeOffsets(1.8, 7));
		m_lightPaths = makeLightPaths();

		m_lightPositions.resize(m_lightPaths.size());
		m_lightProjMatrices.resize(m_lightPaths.size());
		m_lightColors = {
		        {1.0, 0.1, 0.01},
		        {0.1, 1.0, 0.1 },
		        {0.1, 0.1, 1.0 },
		};

		assert(m_lightPaths.size() == m_lightPositions.size());
		assert(m_lightPaths.size() == m_lightColors.size());

		for (auto i = 0u; i < m_lightColors.size(); i++) {
			m_unifs.u_light_colors[i] = m_lightColors[i];
		}

		for (uint32_t i = 0, n = m_lightPaths.size(); i != n; ++i) {
			m_unifs.u_shadow_texs[i] = m_frames[i].getBuffer("depth").id();
		}

		for (auto i = 0u; i < m_cubes.offsets().size(); i++) {
			m_unifs.u_offsets[i] = m_cubes.offsets()[i];
		}

		// renderstate & bg
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.cull_face = GL_BACK;
			renderstate.poly_offset = {1.0, 1.0};
		}
	}

	void render() override
	{
		assert(m_lightPaths.size() == m_lightPositions.size());

		m_unifs.u_light_count = 0;

		auto light_persp_matrix = math::perspective(viewport(0), 78, 1, 80);

		m_unifs.u_viewsceen = light_persp_matrix;

		for (uint32_t i = 0, n = m_lightPositions.size(); i != n; ++i) {
			SpuScopedRenderstate renderstate(true);

			auto esec = getSeconds().current();
			m_lightPositions[i] = m_lightPaths[i].position((i + 1) * esec / 12.0);

			const auto light_matrix = math::lookat(m_lightPositions[i], ezero(), ey());

			m_unifs.u_light_positions[i] = m_lightPositions[i];

			m_lightProjMatrices[i] = light_persp_matrix * light_matrix;

			// Bind the off-screen FBO
			m_frames[i].begin();
			m_frames[i].clear();

			renderstate.flags.fill_offset = true;
			renderstate.use();

			m_unifs.u_worldview = light_matrix;
			m_unifs.u_use_offset = 1;
			m_cubes.drawShadow();

			renderstate.flags.ccw = false;
			renderstate.use();

			m_unifs.u_use_offset = 0;

			m_plate.drawShadow();
			m_frames[i].end();
		}
		// Now we're going to draw into the default framebuffer
		{
			auto esec = getSeconds().current();
			for (auto i = 0u; i < m_lightPositions.size(); i++) {
				m_unifs.u_light_positions[i] = m_lightPositions[i];
			}

			for (auto i = 0u; i < m_lightProjMatrices.size(); i++) {
				m_unifs.u_worldtexcs[i] = m_lightProjMatrices[i];
			}

			m_unifs.u_light_count = m_lightPositions.size();
			m_unifs.u_viewsceen = math::perspective(viewport(0), 70, 1, 80);
			m_unifs.u_worldview = Mat4f::orbiting(ezero(), esec, 25, 10, 20, 0, 8, 50, 30, 15);

			// clear it
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.fill = false;
			renderstate.use();

			m_unifs.u_use_offset = 1;
			m_cubes.drawShape();

			renderstate.flags.fill = true;
			renderstate.use();

			m_unifs.u_use_offset = 1;
			m_cubes.drawShape();

			renderstate.flags.ccw = false;
			renderstate.use();

			m_unifs.u_use_offset = 0;
			m_plate.drawShape();
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("029_shadow_mapping");
}  // namespace
}  // namespace spu::oglplus
