//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>

#include <shapes/cube.hpp>
#include <shapes/obj_mesh.hpp>
#include <shapes/screen.hpp>
#include <shapes/spiral_sphere.hpp>
#include <shapes/torus.hpp>
#include <shapes/vector.hpp>
#include <text/font2d.hpp>
#include <math/curve.hpp>

namespace {

}  // namespace

namespace spu::oglplus {
namespace {

constexpr int32_t c_instance_count = 64;

/* clang-format off */
const char *c_track_comp =  {
    "#version 440                                                                           \n"
    "layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;                      \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "layout (std140) uniform u_model_block {                                                \n"
    "       mat4 model_matrices[64];                                                        \n"
    "};                                                                                     \n"
    "writeonly buffer a_xfb_data { vec4 v[]; } b_xfb_data;                                  \n"
    "                                                                                       \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       uint index = gl_GlobalInvocationID.x;                                           \n"
    "       vec4 position_screen =                                                          \n"
    "               u_viewsceen *                                                           \n"
    "               u_worldview *                                                           \n"
    "               model_matrices[index] *                                                 \n"
    "               vec4(0,0,0,1);                                                          \n"
    "       float org_z = position_screen.z;                                                \n"
    "       position_screen.xyz /= position_screen.w;                                       \n"
    "       bool visible =                                                                  \n"
    "               (position_screen.x >-1.2) &&                                            \n"
    "               (position_screen.x < 1.0) &&                                            \n"
    "               (position_screen.y >-1.1) &&                                            \n"
    "               (position_screen.y < 1.1) &&                                            \n"
    "               (position_screen.w > 0.0);                                              \n"
    "       if (visible) {                                                                  \n"
    "               position_screen.z += float(index);                                      \n"
    "       }                                                                               \n"
    "       else {                                                                          \n"
    "               position_screen.z = float(0xffff);                                      \n"
    "       }                                                                               \n"
    "       position_screen.w = org_z;                                                      \n"
    "       b_xfb_data.v[index] = position_screen;                                          \n"
    "}                                                                                      \n"
};


const char *c_shape_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "layout (std140) uniform u_model_block {                                                \n"
    "       mat4 model_matrices[64];                                                        \n"
    "};                                                                                     \n"
    "const vec3 u_light_pos = vec3(0.0, 0.0, 0.0);                                          \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 g_light_dir;                                                                  \n"
    "out vec3 g_normal;                                                                     \n"
    "out vec3 g_color;                                                                      \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       mat4 u_nodeworld = model_matrices[gl_InstanceID];                               \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       g_light_dir = normalize(u_light_pos - gl_Position.xyz);                         \n"
    "       g_normal = mat3(u_nodeworld) * a_normal;                                        \n"
    "       g_color = abs(normalize((u_nodeworld * a_position).yxz));                       \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_shape_frag =  {
    "#version 330                                                                           \n"
    "in vec3 g_light_dir;                                                                   \n"
    "in vec3 g_normal;                                                                      \n"
    "in vec3 g_color;                                                                       \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float d = max(dot(g_light_dir, g_normal)+0.1,0.0);                              \n"
    "       final_color = g_color * (0.1 + d);                                              \n"
    "}                                                                                      \n"
};

const char *c_hud_vert =  {
    "#version 330                                                                           \n"
    "uniform vec2 u_screen_size;                                                            \n"
    "in vec4 a_position;                                                                    \n"
    "in vec2 a_texcoord;                                                                    \n"
    "out vec2 g_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       g_texcoord = u_screen_size*vec2(a_texcoord.x,1.0-a_texcoord.y);                 \n"
    "}                                                                                      \n"
};

const char *c_hud_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2DRect u_texture;                                                       \n"
    "in vec2 g_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float fa = texture(u_texture, g_texcoord).r;                                    \n"
    "       float la = 0.0;                                                                 \n"
    "       la += texture(u_texture, g_texcoord+vec2(-1,-1)).r;                             \n"
    "       la += texture(u_texture, g_texcoord+vec2( 0,-1)).r;                             \n"
    "       la += texture(u_texture, g_texcoord+vec2( 1,-1)).r;                             \n"
    "       la += texture(u_texture, g_texcoord+vec2(-1, 0)).r;                             \n"
    "       la += texture(u_texture, g_texcoord+vec2( 1, 0)).r;                             \n"
    "       la += texture(u_texture, g_texcoord+vec2(-1, 1)).r;                             \n"
    "       la += texture(u_texture, g_texcoord+vec2( 0, 1)).r;                             \n"
    "       la += texture(u_texture, g_texcoord+vec2( 1, 1)).r;                             \n"
    "       if (la+fa <= 0.0) discard;                                                      \n"
    "       else                                                                            \n"
    "       {                                                                               \n"
    "               la = min(la, 1.0);                                                      \n"
    "               vec3 fill_color = vec3(1.0, 1.0, 1.0);                                  \n"
    "               vec3 line_color = vec3(0.0, 0.0, 0.0);                                  \n"
    "               final_color =                                                           \n"
    "                       vec4(line_color*la, la)+                                        \n"
    "                       vec4(fill_color*fa, fa);                                        \n"
    "       }                                                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class Uniforms {
public:
	std::vector<Mat4f> u_model_block;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Vec2f u_screen_size;
	uint32_t u_texture = 0;

	Uniforms()
	{
		u_model_block.resize(c_instance_count);

		m_attrs = {
		        {"u_model_block", u_model_block.data()},
		        {"u_viewsceen",   &u_viewsceen        },
		        {"u_worldview",   &u_worldview        },
		        {"u_screen_size", &u_screen_size      },
		        {"u_texture",     &u_texture          },
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class Path {
public:
	Path()
	{
		m_positions.init(makePositions(), 0.25);
		m_normals.init(makeNormals(), 0.25);
	}

	Vec3f position(double t) const { return m_positions.position(t); }
	Vec3f normal(double t) const { return m_normals.position(t); }

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
		        {-30.0, -20.0, 70.0 }
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
                        {-1.0, 1.0,  0.0 }
                };
	}

	CubicBezierLoop<Vec3f, double> m_positions;
	CubicBezierLoop<Vec3f, double> m_normals;
};

class Hud : shapes::Array {
public:
	explicit Hud(const Uniforms &unifs)
	{
#ifdef _WIN32
		m_conv.init("UTF-32LE", "UTF-8");
#else
		m_conv.init("UTF-32", "UTF-8");
#endif
		m_font.init("assets/fonts/FreeSans.ttf");

		Attrs shader_attrs = {
		        {"frag", c_hud_frag},
		        {"vert", c_hud_vert},
		};

		Array::initShader(shader_attrs, Attrs(unifs));
		Array::initArray(shapes::Screen(), {"position", "texcoord"});
	}

	void resize(float width, float height)
	{
		m_width = width;
		m_height = height;
		m_pixels.resize(m_width * m_height, 0);

		Attrs attrs = {
		        {"target",      GL_TEXTURE_RECTANGLE},
		        {"iformat",     GL_R8               },
		        {"min_filter",  GL_NEAREST          },
		        {"mag_filter",  GL_NEAREST          },
		        {"wrap_s",      GL_CLAMP_TO_EDGE    },
		        {"wrap_t",      GL_CLAMP_TO_EDGE    },
		        {"auto_mipmap", 0                   },
		};
		m_texture.init(attrs);  // zero size at first
		getAShader().setUniforms({
		        {"u_texture", m_texture.id()}
                });
	}

	Vec4f m_position;  // screen
	std::string m_name;

	void setPosition(const Vec4f &position) { m_position = position; }

	void setName(const std::string &name) { m_name = name; }

	void draw()
	{
		auto inst_id = int32_t(m_position.z);

		if (inst_id == 0xffff) return;

		m_position.z = m_position.z - floor(m_position.z);
		auto dist = m_position.w;

		auto render_text
		        = [&](const text::Font2D::Layout &layout, int32_t x, int32_t y, uint32_t size) {
			          m_font.render(size, layout, m_pixels.data(), m_width, m_height, x, y);
		          };

		auto sc_x = (+m_position.x * 0.5f + 0.5f) * m_width;
		auto sc_y = (-m_position.y * 0.5f + 0.5f) * m_height;

		auto s = std::string("⊗");

		auto dst = m_conv.get(s.c_str(), s.size(), 4);

		auto lt = m_font.MakeLayout(dst);
		auto px = 30 * pow(m_position.z, 8.0);

		render_text(lt, sc_x - m_font.Width(px, lt) / 2, sc_y - m_font.Height(px, lt) / 2, px);

		std::stringstream ss;

		ss << m_name << " #" << inst_id;
		s = ss.str();

		dst = m_conv.get(s.c_str(), s.size(), 4);

		render_text(
		        m_font.MakeLayout(dst), sc_x + 30 - m_position.z * 20, sc_y - 40 + m_position.z * 20,
		        40 - 20 * m_position.z);

		ss.str(std::string());
		ss.clear();

		ss << std::setw(5) << std::setprecision(1) << std::fixed << dist << " [m]";

		s = ss.str();

		dst = m_conv.get(s.c_str(), s.size(), 4);

		render_text(
		        m_font.MakeLayout(dst), sc_x + 30 - m_position.z * 20, sc_y + 10 - m_position.z * 5,
		        12);
	}

	void flush()
	{
		// upload
		{
			Attrs set_attrs = {
			        {"width",  m_width },
			        {"height", m_height},
			};
			m_texture.set(set_attrs);
			m_texture.send(m_pixels.data(), GL_R8);
		}

		// draw
		{
			SpuScopedRenderstate renderstate(true);
			renderstate.flags.depth_test = false;
			renderstate.flags.blend = true;
			renderstate.use();
			Array::draw(nullptr);
		}

		// cleanup
		fill(begin(m_pixels), end(m_pixels), 0x00);
	}

private:
	uint32_t m_width = 0;
	uint32_t m_height = 0;
	std::vector<uint8_t> m_pixels;
	SpuTexture m_texture;
	text::Font2D m_font;
	CharsetConverter m_conv;
};

class HudPoint {
public:
	explicit HudPoint(const Uniforms &unifs)
	{
		auto &array = m_compArray;
		auto &shader = array.getShader();

		Attrs shader_attrs = {
		        {"comp", c_track_comp},
		};
		shapes::loadShader(shader, shader_attrs, Attrs(unifs));

		Attrs attrs0 = {
		        {"shader_id",    shader.id()     },
		        {"a.a_xfb_data", 4               },
		        {"nelem",        c_instance_count},
		};
		array.aux(attrs0, 0);
	}
	void compute() { m_compArray.compute(); }

	void *extract(void *dst_ptr)
	{
		const void *src_ptr = m_compArray.map("r", 0);
		memcpy(dst_ptr, src_ptr, c_instance_count * sizeof(Vec4f));
		m_compArray.unmap(0);
		return dst_ptr;
	}

private:
	SpuComputeArray m_compArray;
};

class Object : public shapes::Array {
public:
	std::vector<Mat4f> m_matrixBuffer;

	template<class ShapeBuilder>
	Object(const std::string &name, const ShapeBuilder &builder, const Uniforms &unifs) : m_hudPoint(unifs)
	{
		Attrs shader_attrs = {
		        {"vert", c_shape_vert},
		        {"frag", c_shape_frag},
		};
		initShader(shader_attrs, Attrs(unifs));

		m_name = name;

		Array::initArray(builder, {"position", "normal"});
		Array::setInstanceCount(c_instance_count);

		RandomGenerator<float> frand = {-1.0, +1.0};

		for (auto i = 0; i != c_instance_count; ++i) {
			const float s = 120.0;
			auto matrix = Mat4f().scale(3.0)
			                      .rot("zyx", -frand() * math::two_pi(), -frand() * math::two_pi(),
			                           -frand() * math::two_pi())
			                      .trans({frand() * s, frand() * s, frand() * s});

			m_matrixBuffer.emplace_back(matrix);
		}
	}
	void track(Uniforms &, Hud &hud_screen)
	{
		m_hudPoint.extract(m_xfbMap);
		m_hudPoint.compute();

		for (auto i = 0u; i < c_instance_count; i++) {
			Vec4f xfb_data = m_xfbMap[i];
			hud_screen.setName(m_name);
			hud_screen.setPosition(xfb_data);
			hud_screen.draw();
		}
	}

	Object(Object &&temp) = delete;  // no copy
private:
	HudPoint m_hudPoint;
	std::string m_name;
	Vec4f m_xfbMap[c_instance_count];
};

class App : public SpuPage {
public:
	Uniforms m_unifs;
	Path m_path;
	std::vector<Object *> m_objects;
	Hud m_hud;

	App(const char *name) : SpuPage(name, true, {0.3, 0.3, 0.3, 0.0}), m_hud(m_unifs) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		auto create_obj_mesh = [](const char *name) {
			auto path = std::string("assets/models/") + name + ".obj";
			File resource_file(path, "rb");

			return shapes::ObjMesh(resource_file, shapes::ObjMesh::LoadingOptions(false).normals());
		};

		m_objects.resize(5);

		m_objects[0] = new Object("Arrow", create_obj_mesh("arrow_z"), m_unifs);
		m_objects[1] = new Object("Monkey", create_obj_mesh("suzanne"), m_unifs);
		m_objects[2] = new Object("Cube", shapes::Cube(), m_unifs);
		m_objects[3] = new Object("Torus", shapes::Torus(), m_unifs);
		m_objects[4] = new Object("Sphere", shapes::SpiralSphere(), m_unifs);

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
		renderstate.use();

		{
			m_hud.resize(int32_t(viewport(0).sx), int32_t(viewport(0).sy));
			m_unifs.u_viewsceen = math::perspective(viewport(0), 75, 1, 1000);
			m_unifs.u_screen_size = Vec2f(viewport(0).sx, viewport(0).sy);
		}
	}

	~App()
	{
		for (auto &object: m_objects) {
			delete object;
		}
	}

	void render() override
	{
		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.use();

		auto esec = getSeconds().current();
		auto pos = esec / 60.0;

		auto e = m_path.position(pos - 0.03 + sin(esec / 7.0 * math::two_pi()) * 0.01);
		auto c = m_path.position(pos + 0.02 + sin(esec / 11.0 * math::two_pi()) * 0.01);
		auto u = m_path.normal(pos - 0.02 + sin(esec / 9.0 * math::two_pi()) * 0.02);
		m_unifs.u_worldview = math::lookat(e, c, u);

		for (auto &object: m_objects) {
			m_unifs.u_model_block = object->m_matrixBuffer;
			object->draw(nullptr);
			object->track(m_unifs, m_hud);
		}

		m_hud.flush();
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("032_object_tracking");
}  // namespace
}  // namespace spu::oglplus
