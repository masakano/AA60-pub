//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cube.hpp>
#include <shapes/sphere.hpp>
#include <math/curve.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_sphere_vert =  {
    "#version 330                                                                           \n"
    "uniform float u_time;                                                                  \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform vec4 u_light_pos;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal, a_tangent;                                                           \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "out vec3 f_refl;                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float amps = 0.02, ampt = 0.05/(1.0+u_time*0.1);                                \n"
    "       float pers = 6.0, pert = 3.0+u_time*0.1;                                        \n"
    "       float vels = 4.0, velt = 3.0;                                                   \n"
    "       float as = (a_texcoord.s + u_time/vels)*3.1415*2.0;                             \n"
    "       float at = (a_texcoord.t + u_time/velt)*3.1415*2.0;                             \n"
    "       vec3 bitangent = cross(a_normal, a_tangent);                                    \n"
    "       f_normal = normalize(                                                           \n"
    "               a_normal - a_tangent * cos(as*pers)*amps*5.0+                           \n"
    "               a_normal - bitangent* cos(at*pert)*ampt*5.0                             \n"
    "       );                                                                              \n"
    "       gl_Position = vec4(                                                             \n"
    "               a_position.xyz + a_normal * (                                           \n"
    "                       amps * sin(as*pers)+                                            \n"
    "                       ampt * sin(at*pert)                                             \n"
    "               ), 1.0                                                                  \n"
    "       );                                                                              \n"
    "       f_light = u_light_pos.xyz - gl_Position.xyz;                                    \n"
    "       f_refl = reflect(                                                               \n"
    "                       -vec3(                                                          \n"
    "                               u_worldview[0][2],                                      \n"
    "                               u_worldview[1][2],                                      \n"
    "                               u_worldview[2][2]                                       \n"
    "                       ),                                                              \n"
    "                       normalize(f_normal)                                             \n"
    "       );                                                                              \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               gl_Position;                                                            \n"
    "}                                                                                      \n"
};

const char *c_sphere_frag =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform samplerCube u_cube_tex;                                                        \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "in vec3 f_refl;                                                                        \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = length(f_light);                                                      \n"
    "       float d = l != 0? dot(                                                          \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light)                                                      \n"
    "       ) / l : 0.0;                                                                    \n"
    "       float e = pow(dot(                                                              \n"
    "               normalize(f_refl),                                                      \n"
    "               normalize(f_light)                                                      \n"
    "       ), 32.0);                                                                       \n"
    "       vec3 color = texture(                                                           \n"
    "               u_cube_tex,                                                             \n"
    "               normalize(f_refl)                                                       \n"
    "       ).rgb;                                                                          \n"
    "       final_color = vec4(                                                             \n"
    "               (0.3  + max(d,0.0)*0.5) * color +                                       \n"
    "               (0.05 + max(e,0.0))*vec3(1.0, 1.0, 1.0),                                \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_cube_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform vec4 u_light_pos;                                                              \n"
    "uniform vec3 u_offset;                                                                 \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_color;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(a_position.xyz+u_offset, 1.0);                               \n"
    "       f_normal = a_normal;                                                            \n"
    "       f_color = normalize(                                                            \n"
    "               vec3(1.0, 1.0, 1.0) -                                                   \n"
    "               normalize(a_normal + u_offset)                                          \n"
    "       );                                                                              \n"

    "       f_light = u_light_pos.xyz - gl_Position.xyz;                                    \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               gl_Position;                                                            \n"
    "}                                                                                      \n"
};

const char *c_cube_frag =  {
    "#version 330                                                                           \n"
    "in vec3 f_color;                                                                       \n"
    "in vec3 f_normal;                                                                      \n"
    "in vec3 f_light;                                                                       \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float l = length(f_light);                                                      \n"
    "       float d = l != 0? dot(                                                          \n"
    "               normalize(f_normal),                                                    \n"
    "               normalize(f_light)                                                      \n"
    "       ) / l : 0.0;                                                                    \n"
    "       float i = 0.1 + 4.2*max(d, 0.0);                                                \n"
    "       final_color = vec4(f_color*i, 1.0);                                             \n"
    "}                                                                                      \n"
};

const char *c_cmap_vert =  {
    "#version 330                                                                           \n"
    "uniform vec4 u_light_pos;                                                              \n"
    "uniform vec3 u_offset;                                                                 \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 g_color;                                                                      \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_light;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(a_position.xyz+u_offset, 1.0);                               \n"
    "       g_color = normalize(                                                            \n"
    "               vec3(1.0, 1.0, 1.0) -                                                   \n"
    "               normalize(a_normal + u_offset)                                          \n"
    "       );                                                                              \n"
    "       g_normal = a_normal;                                                            \n"
    "       g_light = u_light_pos.xyz - gl_Position.xyz;                                    \n"
    "}                                                                                      \n"
};

const char *c_cmap_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 18) out;                                         \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "const mat4 cube_face_matrix[6] = mat4[6](                                              \n"
    "       mat4(                                                                           \n"
    "                0.0,  0.0, -1.0,  0.0,                                                 \n"
    "                0.0, -1.0,  0.0,  0.0,                                                 \n"
    "               -1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       ), mat4(                                                                        \n"
    "                0.0,  0.0,  1.0,  0.0,                                                 \n"
    "                0.0, -1.0,  0.0,  0.0,                                                 \n"
    "                1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       ), mat4(                                                                        \n"
    "                1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0, -1.0,  0.0,                                                 \n"
    "                0.0,  1.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       ), mat4(                                                                        \n"
    "                1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  1.0,  0.0,                                                 \n"
    "                0.0, -1.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       ), mat4(                                                                        \n"
    "                1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0, -1.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0, -1.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       ), mat4(                                                                        \n"
    "               -1.0,  0.0,  0.0,  0.0,                                                 \n"
    "                0.0, -1.0,  0.0,  0.0,                                                 \n"
    "                0.0,  0.0,  1.0,  0.0,                                                 \n"
    "                0.0,  0.0,  0.0,  1.0                                                  \n"
    "       )                                                                               \n"
    ");                                                                                     \n"
    "in vec3 g_color[];                                                                     \n"
    "in vec3 g_normal[];                                                                    \n"
    "in vec3 g_light[];                                                                     \n"
    "out vec3 f_color;                                                                      \n"
    "out vec3 f_normal;                                                                     \n"
    "out vec3 f_light;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       for (gl_Layer=0; gl_Layer!=6; ++gl_Layer)                                       \n"
    "       {                                                                               \n"
    "               for (int i=0; i!=3; ++i)                                                \n"
    "               {                                                                       \n"
    "                       gl_Position =                                                   \n"
    "                               u_viewsceen *                                           \n"
    "                               cube_face_matrix[gl_Layer]*                             \n"
    "                               gl_in[i].gl_Position;                                   \n"
    "                       f_color = g_color[i];                                           \n"
    "                       f_normal = g_normal[i];                                         \n"
    "                       f_light = g_light[i];                                           \n"
    "                       EmitVertex();                                                   \n"
    "               }                                                                       \n"
    "               EndPrimitive();                                                         \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_worldview;
	Mat4f u_viewsceen;
	Vec3f u_offset = ezero();
	Vec4f u_light_pos = ezero<Vec4f>();
	float u_time = 0.0;
	uint32_t u_cube_tex = 0u;

	Uniforms()
	{
		m_attrs = {
		        {"u_worldview", &u_worldview},
                        {"u_viewsceen", &u_viewsceen},
		        {"u_offset",    &u_offset   },
                        {"u_light_pos", &u_light_pos},
		        {"u_time",      &u_time     },
                        {"u_cube_tex",  &u_cube_tex },
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class CubeArray : public shapes::Array {
public:
	enum {
		e_shape = 0,
		e_cmap,
	};

	explicit CubeArray(const Uniforms &unifs)
	{
		Attrs shape_shader_attrs = {
		        {"frag", c_cube_frag},
		        {"vert", c_cube_vert},
		};

		Attrs cmap_shader_attrs = {
		        {"frag", c_cube_frag},
		        {"vert", c_cmap_vert},
		        {"geom", c_cmap_geom},
		};

		setMaxShaderType(2);  // 0:shape, 1:cmap
		initShader(shape_shader_attrs, Attrs(unifs), 0);
		initShader(cmap_shader_attrs, Attrs(unifs), 1);

		shapes::Cube cube_shape;
		initArray(cube_shape, {"position", "normal"});
	}
};

class SphereArray : public shapes::Array {
public:
	explicit SphereArray(const Uniforms &unifs)
	{
		Attrs shader_attrs = {
		        {"frag", c_sphere_frag},
		        {"vert", c_sphere_vert},
		};
		Array::initShader(shader_attrs, Attrs(unifs));
		shapes::Sphere make_sphere(1.0, 72, 48);
		Array::initArray(make_sphere, {"position", "normal", "tangent", "texcoord"});
	}
};

class CubeMapFrame : public SpuFrame {
public:
	uint32_t m_colorId;

	explicit CubeMapFrame(int32_t tex_side)
	{
		int32_t cube_target[6] = {
		        cubemapFace(0), cubemapFace(1), cubemapFace(2),
		        cubemapFace(3), cubemapFace(4), cubemapFace(5),
		};
		auto viewport = Rectf(0, 0, tex_side, tex_side);

		Attrs attrs = {
		        {"viewport0",          viewport             },
		        {"depth.target",       GL_TEXTURE_CUBE_MAP  },
		        {"depth.min_filter",   GL_NEAREST           },
		        {"depth.mag_filter",   GL_NEAREST           },
		        {"depth.iformat",      GL_DEPTH_COMPONENT32F},
		        {"depth.cube_target",  cube_target          },

		        {"color0.target",      GL_TEXTURE_CUBE_MAP  },
		        {"color0.min_filter",  GL_LINEAR            },
		        {"color0.mag_filter",  GL_LINEAR            },
		        {"color0.wrap_s",      GL_CLAMP_TO_EDGE     },
		        {"color0.wrap_t",      GL_CLAMP_TO_EDGE     },
		        {"color0.wrap_r",      GL_CLAMP_TO_EDGE     },
		        {"color0.iformat",     GL_RGB8              },
		        {"color0.cube_target", cube_target          },
		};
		SpuFrame::init(attrs);
		m_colorId = getBuffer("color0").id();
	}

	int32_t cubemapFace(uint32_t face)
	{
		assert(face <= 5);
		return GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
	}
};

class App : public SpuPage {
public:
	std::vector<Vec3f> m_cubeOffsets;

	static std::vector<Vec3f> makeCubeOffsets(float distance, int32_t cubes_per_side)
	{
		int32_t instance_count = 2 * cubes_per_side * cubes_per_side
		                       + 2 * cubes_per_side * (cubes_per_side - 2)
		                       + 2 * (cubes_per_side - 2) * (cubes_per_side - 2);

		std::vector<Vec3f> offsets(instance_count, ezero());

		int32_t cpf = cubes_per_side * cubes_per_side;
		int32_t cpe = cubes_per_side - 2;
		int32_t cpl = 2 * cubes_per_side + 2 * cpe;
		int32_t cpi = cpl * cpe;

		for (auto i = 0; i != instance_count; ++i) {
			int32_t id = i;
			float omax = (cubes_per_side - 1) * 0.5;
			float imax = (cpe - 1) * 0.5;
			float dx;
			float dy;
			float dz;
			if (id < cpf) {
				dx = -omax;
				dy = (id / cubes_per_side) - omax;
				dz = (id % cubes_per_side) - omax;
			}
			else if (id < cpf + cpi) {
				id -= cpf;
				dx = (id / cpl) - imax;
				id = id % cpl;
				if (id < cubes_per_side) {
					dy = -omax;
					dz = id - omax;
				}
				else if (id < cubes_per_side + cpe * 2) {
					id -= cubes_per_side;
					dy = -imax + id / 2;
					dz = (id % 2 == 1) ? omax : -omax;
				}
				else {
					id -= cubes_per_side + cpe * 2;
					dy = omax;
					dz = id - omax;
				}
			}
			else {
				id -= cpf + cpi;
				dx = omax;
				dy = (id / cubes_per_side) - omax;
				dz = (id % cubes_per_side) - omax;
			}
			offsets[i] = {dx * distance, dy * distance, dz * distance};
		}
		return offsets;
	}

	Uniforms m_unifs;
	CubeArray m_cube;
	SphereArray m_sphere;
	CubeMapFrame m_frame;
	CubicBezierLoop<Vec3f, double> m_lightPath;

	App(const char *name)
	        : SpuPage(name, true, {0.0, 0.0, 0.0, 0.0}), m_cube(m_unifs), m_sphere(m_unifs), m_frame(128)
	{
	}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_cubeOffsets = makeCubeOffsets(2.5, 6);
		m_lightPath.init({
		        {+0.0, +6.0, +0.0},
                        {-3.0, -4.0, +3.5},
                        {+0.0, -3.0, -4.0},
                        {+3.5, -4.0, +3.0}
                });

		m_unifs.u_viewsceen = math::perspective(viewport(0), 90, 1, 20);
		m_unifs.u_cube_tex = m_frame.m_colorId;

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto light_pos = Vec4f(m_lightPath.position(esec / 10.0), 1.0);
		auto worldview = Mat4f::orbiting(ezero(), esec, 4, 0, 0, 0, 16, 0, 30, 20);

		// cubemaps
		{
			{
				std::vector<Vec4f> colors(6, ezero<Vec4f>());
				m_frame.getBuffer("color0").send(
				        colors.data(), GL_RGBA32F, nullptr, nullptr, true);
			}
			{
				std::vector<float> depths(6, 1.0);
				m_frame.getBuffer("depth").send(
				        depths.data(), GL_DEPTH_COMPONENT32F, nullptr, nullptr, true);
			}

			m_frame.begin();
			m_unifs.u_light_pos = light_pos;
			m_unifs.u_viewsceen = math::perspective(viewport(0), 90, 1, 20);

			auto b = std::begin(m_cubeOffsets);
			auto e = std::end(m_cubeOffsets);

			m_cube.setShaderType(CubeArray::e_cmap);
			for (auto i = b; i != e; ++i) {
				m_unifs.u_offset = *i;
				m_cube.draw(nullptr);
			}
			m_frame.end();
		}

		// objects
		{
			m_unifs.u_viewsceen = math::perspective(viewport(0), 70, 1, 20);
			m_unifs.u_worldview = worldview;
			m_unifs.u_light_pos = light_pos;
			m_unifs.u_time = esec;

			auto b = std::begin(m_cubeOffsets);
			auto e = std::end(m_cubeOffsets);

			// cubes
			m_cube.setShaderType(CubeArray::e_shape);
			for (auto i = b; i != e; ++i) {
				m_unifs.u_offset = *i;
				m_cube.draw(nullptr);
			}

			// sphere
			m_sphere.draw(nullptr);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("030_cube_mapping");
}  // namespace
}  // namespace spu::oglplus
