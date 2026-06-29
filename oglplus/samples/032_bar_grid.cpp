//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/brushed_metal.hpp>
#include <shapes/plane.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_metal_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal, a_tangent;                                                           \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec3 g_normal, g_tangent, g_bitangent;                                             \n"
    "out vec3 g_light_dir, g_view_dir;                                                      \n"
    "out vec2 g_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       g_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       g_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "       g_normal = a_normal;                                                            \n"
    "       g_tangent = a_tangent;                                                          \n"
    "       g_bitangent = cross(g_normal, g_tangent);                                       \n"
    "       g_texcoord = a_texcoord * 50.0;                                                 \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_metal_frag =  {
    "#version 330                                                                           \n"
    "const vec3 u_color1 = vec3(0.5, 0.5, 0.6);                                             \n"
    "const vec3 u_color2 = vec3(0.7, 0.7, 0.8);                                             \n"
    "uniform sampler2D u_texture;                                                           \n"
    "uniform float u_light_multiplier;                                                      \n"
    "in vec3 g_normal, g_tangent, g_bitangent;                                              \n"
    "in vec3 g_light_dir, g_view_dir;                                                       \n"
    "in vec2 g_texcoord;                                                                    \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       bool odd = (int(g_texcoord.x)+int(g_texcoord.y)) % 2 == 0;                      \n"
    "       vec2 tex_coord = odd?g_texcoord.yx:g_texcoord.xy;                               \n"
    "       vec3 sample = texture(u_texture, tex_coord).rgb;                                \n"
    "       vec3 light_color = vec3(1.0, 1.0, 1.0);                                         \n"
    "       vec3 normal = normalize(                                                        \n"
    "               2.0*g_normal +                                                          \n"
    "               (sample.r - 0.5)*g_tangent +                                            \n"
    "               (sample.g - 0.5)*g_bitangent                                            \n"
    "       );                                                                              \n"
    "       vec3 light_refl = reflect(                                                      \n"
    "               -normalize(g_light_dir),                                                \n"
    "               normal                                                                  \n"
    "       );                                                                              \n"
    "       float specular = u_light_multiplier * pow(max(dot(                              \n"
    "               normalize(light_refl),                                                  \n"
    "               normalize(g_view_dir)                                                   \n"
    "       )+0.04, 0.0), 16+sample.b*48)*pow(0.4+sample.b*1.6, 4.0);                       \n"
    "       normal = normalize(g_normal*3.0 + normal);                                      \n"
    "       float diffuse = u_light_multiplier * pow(max(dot(                               \n"
    "               normalize(normal),                                                      \n"
    "               normalize(g_light_dir)                                                  \n"
    "       ), 0.0), 2.0);                                                                  \n"
    "       vec3 u_color = mix(u_color1, u_color2, sample.b)*(odd?0.95:1.05);               \n"
    "       final_color =                                                                   \n"
    "               light_color * u_color * diffuse +                                       \n"
    "               light_color * specular;                                                 \n"
    "}                                                                                      \n"
};

const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform vec3 u_light_position;                                                         \n"
    "uniform vec3 u_eye_position;                                                           \n"
    "layout(std140) uniform u_bar_offset_block {                                            \n"
    "       vec4 bar_offsets[4096];                                                         \n"
    "};                                                                                     \n"
    "in vec4 a_pos_and_offs;                                                                \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec4 g_color;                                                                      \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_light_dir, g_view_dir;                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(a_pos_and_offs.xyz, 1.0);                                    \n"
    "       vec4 offs = bar_offsets[gl_InstanceID];                                         \n"
    "       gl_Position.x += offs.x;                                                        \n"
    "       gl_Position.y += a_pos_and_offs.w * offs.y;                                     \n"
    "       gl_Position.z += offs.z;                                                        \n"
    "       g_color = vec4(                                                                 \n"
    "               abs(offs.yxz)/offs.w,                                                   \n"
    "               a_pos_and_offs.w*sqrt(offs.y)                                           \n"
    "       );                                                                              \n"
    "       g_normal = a_normal;                                                            \n"
    "       g_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "       g_view_dir = u_eye_position - gl_Position.xyz;                                  \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "uniform float u_light_multiplier;                                                      \n"
    "in vec4 g_color;                                                                       \n"
    "in vec3 g_normal;                                                                      \n"
    "in vec3 g_light_dir, g_view_dir;                                                       \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 u_color = g_color.rgb;                                                     \n"
    "       vec3 light_color = vec3(1.0, 1.0, 1.0);                                         \n"
    "       vec3 light_refl = reflect(                                                      \n"
    "               -normalize(g_light_dir),                                                \n"
    "               normalize(g_normal)                                                     \n"
    "       );                                                                              \n"
    "       float ambient = 0.2;                                                            \n"
    "       float incandescent = 0.6 * g_color.a;                                           \n"
    "       float diffuse = u_light_multiplier * max(dot(                                   \n"
    "               normalize(g_normal),                                                    \n"
    "               normalize(g_light_dir)                                                  \n"
    "       )+0.1, 0.0);                                                                    \n"
    "       float specular = pow(u_light_multiplier,2.0)*pow(max(dot(                       \n"
    "               normalize(light_refl),                                                  \n"
    "               normalize(g_view_dir)                                                   \n"
    "       )-0.04, 0.0), 2.0);                                                             \n"
    "       final_color =                                                                   \n"
    "               u_color * incandescent+                                                 \n"
    "               light_color * u_color * (ambient+diffuse)+                              \n"
    "               light_color * specular;                                                 \n"
    "}                                                                                      \n"
};

const char *c_shadow_vert =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_light_position;                                                         \n"
    "layout(std140) uniform u_bar_offset_block {                                            \n"
    "       vec4 bar_offsets[4096];                                                         \n"
    "};                                                                                     \n"
    "in vec4 a_pos_and_offs;                                                                \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 g_normal, g_light_dir;                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = vec4(a_pos_and_offs.xyz, 1.0);                                    \n"
    "       vec4 offs = bar_offsets[gl_InstanceID];                                         \n"
    "       gl_Position.x += offs.x;                                                        \n"
    "       gl_Position.y += a_pos_and_offs.w * offs.y;                                     \n"
    "       gl_Position.z += offs.z;                                                        \n"
    "       g_normal = a_normal;                                                            \n"
    "       g_light_dir = u_light_position - gl_Position.xyz;                               \n"
    "}                                                                                      \n"
};

const char *c_shadow_geom =  {
    "#version 330                                                                           \n"
    "layout(triangles) in;                                                                  \n"
    "layout(triangle_strip, max_vertices = 12) out;                                         \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "in vec3 g_normal[3], g_light_dir[3];                                                   \n"
    "void make_near_vertex(int index)                                                       \n"
    "{                                                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen*                                                            \n"
    "               u_worldview*                                                            \n"
    "               gl_in[index].gl_Position;                                               \n"
    "       EmitVertex();                                                                   \n"
    "}                                                                                      \n"
    "void make_far_vertex(int index)                                                        \n"
    "{                                                                                      \n"
    "       vec3 pos = gl_in[index].gl_Position.xyz;                                        \n"
    "       pos -= g_light_dir[index];                                                      \n"
    "       gl_Position =                                                                   \n"
    "               u_viewsceen*                                                            \n"
    "               u_worldview*                                                            \n"
    "               vec4(pos, 1.0);                                                         \n"
    "       EmitVertex();                                                                   \n"
    "}                                                                                      \n"
    "void make_plane(int a, int b)                                                          \n"
    "{                                                                                      \n"
    "       make_near_vertex(a);                                                            \n"
    "       make_near_vertex(b);                                                            \n"
    "       make_far_vertex(a);                                                             \n"
    "       make_far_vertex(b);                                                             \n"
    "       EndPrimitive();                                                                 \n"
    "}                                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 ld = (                                                                     \n"
    "               g_light_dir[0]+                                                         \n"
    "               g_light_dir[1]+                                                         \n"
    "               g_light_dir[2]                                                          \n"
    "       );                                                                              \n"
    "       vec3 fn = (                                                                     \n"
    "               g_normal[0]+                                                            \n"
    "               g_normal[1]+                                                            \n"
    "               g_normal[2]                                                             \n"
    "       );                                                                              \n"
    "       if (dot(fn, ld) >= 0.0)                                                         \n"
    "       {                                                                               \n"
    "               make_plane(1, 0);                                                       \n"
    "               make_plane(2, 1);                                                       \n"
    "               make_plane(0, 2);                                                       \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};
/* clang-format on */

class Uniforms {
public:
	Mat4f u_worldview;
	Mat4f u_viewsceen;
	Vec3f u_eye_position;
	Vec3f u_light_position;
	float u_light_multiplier;
	uint32_t u_texture;
	std::vector<float> u_bar_offset_block;

	Uniforms()
	{
		u_bar_offset_block.resize(4 * size_t(4096));

		m_attrs = {
		        {"u_worldview",        &u_worldview             },
		        {"u_viewsceen",        &u_viewsceen             },
		        {"u_eye_position",     &u_eye_position          },
		        {"u_light_position",   &u_light_position        },
		        {"u_light_multiplier", &u_light_multiplier      },
		        {"u_texture",          &u_texture               },
		        {"u_bar_offset_block", u_bar_offset_block.data()},
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class FloorArray : public shapes::Array {
public:
	SpuTexture m_texture;
	uint32_t u_texture;

	explicit FloorArray(const Uniforms &unifs)
	{
		Attrs shader_attrs = {
		        {"frag", c_metal_frag},
		        {"vert", c_metal_vert},
		};

		Array::initShader(shader_attrs, Attrs(unifs));
		auto plane_shape = shapes::Plane(Vec3f(100, 0, 0), Vec3f(0, 0, -100));

		Array::initArray(plane_shape, {"position", "normal", "tangent", "texcoord"});
		auto image = images::BrushedMetalUByte(512, 512, 5120, -3, +3, 32, 128);

		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D          },
		        {"iformat",     GL_RGB8                },
		        {"width",       image.width()          },
		        {"height",      image.height()         },
		        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter",  GL_LINEAR              },
		        {"wrap_s",      GL_REPEAT              },
		        {"wrap_t",      GL_REPEAT              },
		        {"data",        image.data()           },
		        {"auto_mipmap", 1                      },
		};
		m_texture.init(attrs);
		u_texture = m_texture.id();
	}
};

class GridsArray : private shapes::Array {
public:
	GridsArray(
	        const Uniforms &unifs, std::vector<float> &uniform_buffer, uint32_t side, float size,
	        float chamfer, float space)
	{
		m_barCount = side * side;
		m_barOffsetBlock = uniform_buffer.data();

		assert(space > 0.0);

		initShaders(unifs);  // must be first
		initArray(size, chamfer);

		const float h1 = 0.5 * (size * side + space * (side - 1));
		const float h2 = 0.5 * size;

		auto pi = begin(uniform_buffer);
		auto pe = uniform_buffer.end();

		for (auto z = 0u; z != side; ++z) {
			const float zoffs = -h1 + h2 + z * (size + space);
			for (auto x = 0u; x != side; ++x) {
				const float xoffs = -h1 + h2 + x * (size + space);
				assert(pi != pe);
				*pi++ = xoffs;
				*pi++ = 4.0 * m_frand();
				assert(pi != pe);
				*pi++ = zoffs;
				*pi++ = h1;
			}
		}
		m_barOffsetBlockSize = pi - begin(uniform_buffer);
	}

	void update(float interval, const std::vector<uint32_t> &triggered)
	{
		const auto c_max = 5.0f;
		auto falloff = interval * 0.2f;

		if (falloff > 0.0) {
			falloff = 1.0 - falloff;
		}

		for (uint32_t i = 0u, n = m_barOffsetBlockSize; i != n; i += 4) {
			auto incr = m_frand();
			incr *= incr * incr;
			if (incr > 0.95) {
				m_barOffsetBlock[i + 1] += interval * incr * 10.0;
				if (m_barOffsetBlock[i + 1] > c_max) {
					m_barOffsetBlock[i + 1] = c_max;
				}
			}
			m_barOffsetBlock[i + 1] *= falloff;
		}

		for (auto &i: triggered) {
			auto idx = 4 * i + 1;
			m_barOffsetBlock[idx] += 1.0;
			if (m_barOffsetBlock[idx] > c_max) {
				m_barOffsetBlock[idx] = c_max;
			}
		}
	}

	void drawShape()
	{
		m_shapeShader.use();
		SpuArray::draw(GL_TRIANGLE_STRIP, 0, 0, m_barCount, 0, 0);
	}

	void drawShadow()
	{
		m_shadowShader.use();
		SpuArray::draw(GL_TRIANGLE_STRIP, 0, 0, m_barCount, 0, 0);
	}

private:
	SpuShader m_shapeShader;
	SpuShader m_shadowShader;
	RandomGenerator<float> m_frand;

	uint32_t m_barCount;
	float *m_barOffsetBlock;
	int32_t m_barOffsetBlockSize;

	void initShaders(const Uniforms &unifs)
	{
		{
			Attrs attrs = {
			        {"vert", c_shape_vert},
			        {"frag", c_shape_frag},
			};
			shapes::loadShader(m_shapeShader, attrs, Attrs(unifs));
		}
		{
			Attrs attrs = {
			        {"vert", c_shadow_vert},
			        {"geom", c_shadow_geom},
			};
			shapes::loadShader(m_shadowShader, attrs, Attrs(unifs));
		}
	}

	void initArray(float size, float chamfer)
	{
		const auto a = size * 0.5f;
		const auto b = a * chamfer;
		const auto z = 0.0f;
		const auto o = 1.0f;

		/*   (10/11)   (8/9)
		 *      ^        ^
		 *      |        |
		 *      o--------o
		 *     /          \
		 * <--o (12/13)    o--> (6/7)
		 *    |            |
		 * <--o (14/15)    o--> (4/5)
		 *     \          /
		 *      o--------o
		 *      |        |
		 *      v        v
		 *    (0/1)    (2/3)
		 */
		struct Vertex {
			float m_px, m_py, m_pz, m_pw, m_nx, m_ny, m_nz;  // pw: offset
		};

		/* clang-format off */
		std::vector<Vertex> vertices = {
			//  0/1
			{ -a + b, z, a, z, z, z, o,},
			{ -a + b, b, a, o, z, z, o,},
			//  2/3
			{ a - b, z, a, z, z, z, o,},
			{ a - b, b, a, o, z, z, o,},
			//  4/5
			{ a, z, a - b, z, o, z, z,},
			{ a, b, a - b, o, o, z, z,},
			//  6/7
			{ a, z, -a + b, z, o, z, z,},
			{ a, b, -a + b, o, o, z, z,},
			//  8/9
			{ a - b, z, -a, z, z, z, -o,},
			{ a - b, b, -a, o, z, z, -o,},
			//  10/11
			{ -a + b, z, -a, z, z, z, -o,},
			{ -a + b, b, -a, o, z, z, -o,},
			// 12/13
			{ -a, z, -a + b, z, -o, z, z,},
			{ -a, b, -a + b, o, -o, z, z,},
			//  14/15
			{ -a, z, a - b, z, -o, z, z,},
			{ -a, b, a - b, o, -o, z, z,},

			/*
			 *       (11)     (9)
			 *        o--------o
			 *       /|(19)(18)|\
			 * (13) o-o--------o-o (7)
			 *      | |        | |
			 *      | |(16)(17)| |
			 * (15) o-o--------o-o (5)
			 *       \|        |/
			 *        o--------o
			 *       (1)      (3)
			 */

			//  16
			{ -a + b, b + b, a - b, o, z, o, z,},
			//  17
			{ a - b, b + b, a - b, o, z, o, z,},
			//  18
			{ a - b, b + b, -a + b, o, z, o, z,},
			//  19
			{ -a + b, b + b, -a + b, o, z, o, z,},
		};

		auto pri = uint16_t(vertices.size());

		std::vector<uint16_t> indices = {
			// sides
			0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 0, pri,

			// chamfer
			1, 16, 3, 17, pri,
			3, 17, 5, pri,
			5, 17, 7, 18, pri,
			7, 18, 9, pri,
			9, 18, 11, 19, pri,
			11, 19, 13, pri,
			13, 19, 15, 16, pri,
			15, 16, 1, pri,
			
			// top
			16, 19, 17, 18,
		};
		/* clang-format on */

		Attrs attrs0 = {
		        {"shader_id",        m_shapeShader.id()},
                        {"a.a_pos_and_offs", 4                 },
                        {"a.a_normal",       3                 },
		        {"data",             vertices.data()   },
                        {"nelem",            vertices.size()   },
		};

		Attrs attrs1 = {
		        {"restart", vertices.size()},
		};

		SpuArray::init(attrs0);
		SpuArray::send(indices, -1, 2);
		SpuArray::set(attrs1);
	}
};

class App : public SpuPage {
public:
	static constexpr auto c_side = 16u;
	Uniforms m_unifs;
	GridsArray m_grids;
	FloorArray m_floor;
	std::vector<uint32_t> m_triggered;

	App(const char *name)
	        : SpuPage(name, true), m_grids(m_unifs, m_unifs.u_bar_offset_block, c_side, 0.8, 0.2, 0.2),
	          m_floor(m_unifs)
	{
	}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_triggered.reserve(10);
		m_unifs.u_texture = m_floor.u_texture;
	}

	void render() override
	{
		auto renderstate_save = getRenderstate();
		auto esec = getSeconds().current();
		auto dsec = getSeconds().delta();

		m_unifs.u_viewsceen = math::perspective(viewport(0), 65, 1.0, 100.0);
		m_unifs.u_light_position
		        = Vec3f(20.0 + sin(esec / 37.0 * math::two_pi()) * 10.0,
		                25.0 + cos(esec / 37.0 * math::two_pi()) * 15.0, 10.0);

		auto worldview = Mat4f::orbiting(ezero(), esec, 18, 0, 0, 0, 17, 45, 40, 21);
		auto viewworld = worldview.unitary_inverse();
		auto eye_position = viewworld.c[3];

		m_grids.update(dsec, m_triggered);
		m_triggered.clear();

		m_unifs.u_worldview = worldview;
		m_unifs.u_eye_position = eye_position;

		{
			Attrs attrs = {
			        {"bgstencil", 128},
			};
			set(attrs);
			clear();

			// Draw objects with low light and the depth
			auto &renderstate = getRenderstate();
			renderstate.write_mask = {1, 1, 1, 1, 1};
			renderstate.flags.ccw = false;  // need check
			renderstate.flags.cull_face = true;
			renderstate.flags.stencil_test = false;
			renderstate.flags.depth_test = true;
			renderstate.depth_func = GL_LEQUAL;

			renderstate.use();

			m_unifs.u_light_multiplier = 0.4;
			m_floor.draw(nullptr);

			m_unifs.u_light_multiplier = 0.2;
			m_grids.drawShape();

			// Draw the shadow "volume" into the stencil buffer
			renderstate.write_mask = {0, 0, 0, 0, 0};
			renderstate.flags.stencil_test = true;

			renderstate.stencil_func = {
			        GL_ALWAYS, 0, ~0u, GL_KEEP, GL_KEEP, GL_INCR,
			        GL_ALWAYS, 0, ~0u, GL_KEEP, GL_KEEP, GL_DECR,
			};
			renderstate.use();
			m_grids.drawShadow();
		}

		{
			// Draw stencilled parts of objects with full light
			m_unifs.u_light_multiplier = 1.0;

			auto &renderstate = getRenderstate();
			renderstate.write_mask = {1, 1, 1, 1, 0};
			renderstate.flags.stencil_test = true;

			renderstate.stencil_func = {
			        GL_EQUAL, 128, ~0u, GL_KEEP, GL_KEEP, GL_KEEP,
			        GL_EQUAL, 128, ~0u, GL_KEEP, GL_KEEP, GL_KEEP,
			};
			renderstate.use();
			m_floor.draw(nullptr);
			m_grids.drawShape();
		}
		getRenderstate() = renderstate_save;
	}

	void mouseMoveNormalized(float x, float y, float)
	{
		auto ix = uint32_t(c_side * (0.5f * (1.0f + x)));
		auto iy = uint32_t(c_side * (0.5f * (1.0f + y)));
		if (ix < c_side && iy < c_side) {
			m_triggered.push_back(iy * c_side + ix);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("032_bar_grid");
}  // namespace
}  // namespace spu::oglplus
