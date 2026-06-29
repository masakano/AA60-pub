//
// Uniforms :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cage.hpp>
#include <shapes/cube.hpp>
#include <shapes/icosahedron.hpp>
#include <shapes/spiral_sphere.hpp>
#include <shapes/twisted_torus.hpp>
#include <shapes/wicker_torus.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_shadow_vert =  {
    "#version 330                                                                           \n"
    "#define side 128                                                                       \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform float u_fade;                                                                  \n"
    "uniform sampler2D u_offset_texture;                                                    \n"
    "uniform sampler2D u_height_texture;                                                    \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       ivec2 coord = ivec2(gl_InstanceID%side, gl_InstanceID/side);                    \n"
    "       vec2 offs = texelFetch(u_offset_texture, coord, 0).xy;                          \n"
    "       float height = 1.0-texelFetch(u_height_texture, coord, 0).r;                    \n"
    "       gl_Position = a_position;                                                       \n"
    "       gl_Position.xz += offs;                                                         \n"
    "       gl_Position.y  *= max(height*u_fade*side/2, 0.5);                               \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_shadow_frag =  {
    "#version 330                                                                           \n"
    "void main() { }                                                                        \n"
};

const char *c_disp_vert =  {
    "#version 330                                                                           \n"
    "#define side 128                                                                       \n"
    "uniform mat4 u_viewsceen, u_worldview,                                                 \n"
    "u_light_proj_matrix, u_light_matrix;                                                   \n"
    "uniform vec3 u_camera_pos;                                                             \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "uniform float u_fade;                                                                  \n"
    "uniform sampler2D u_offset_texture;                                                    \n"
    "uniform sampler2D u_height_texture;                                                    \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec4 f_position_shadow;                                                            \n"
    "out vec3 f_view_dir;                                                                   \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_normal;                                                                     \n"
    "out float f_light;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       ivec2 coord = ivec2(gl_InstanceID%side, gl_InstanceID/side);                    \n"
    "       vec2 offs = texelFetch(u_offset_texture, coord, 0).xy;                          \n"
    "       float u_depth = texelFetch(u_height_texture, coord, 0).r;                       \n"
    "       float height = 1.0-u_depth;                                                     \n"
    "       gl_Position = a_position;                                                       \n"
    "       gl_Position.xz += offs;                                                         \n"
    "       float l = (1.0-dot(a_normal, vec3(0,1,0))*0.5)*0.8;                             \n"
    "       f_light = (1.0-gl_Position.y*l)*sign(height)*u_fade;                            \n"
    "       gl_Position.y  *= max(height*u_fade*side/2, 0.5);                               \n"
    "       f_view_dir = u_camera_pos - gl_Position.xyz;                                    \n"
    "       f_light_dir = u_light_pos - gl_Position.xyz;                                    \n"
    "       f_position_shadow = u_light_proj_matrix * u_light_matrix * gl_Position;         \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "       f_normal = a_normal;                                                            \n"
    "}                                                                                      \n"
};

const char *c_disp_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2DShadow u_shadows;                                                     \n"
    "const vec3 light_dir = normalize(vec3(1, 0.3, 1));                                     \n"
    "const vec3 bar_color = vec3(0.4, 0.4, 0.4);                                            \n"
    "in vec4 f_position_shadow;                                                             \n"
    "in vec3 f_view_dir;                                                                    \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_normal;                                                                      \n"
    "in float f_light;                                                                      \n"
    "out vec3 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       vec3 normal = normalize(f_normal);                                              \n"
    "       vec3 view_dir = normalize(f_view_dir);                                          \n"
    "       vec3 light_dir = normalize(f_light_dir);                                        \n"
    "       vec3 light_refl = reflect(-light_dir, normal);                                  \n"
    "       vec3 shadow_coord = (f_position_shadow.xyz/f_position_shadow.w)*0.5 + 0.5;      \n"
    "       float shdw = texture(u_shadows, shadow_coord);                                  \n"
    "       float ambi = 0.15;                                                              \n"
    "       float diff = pow(max(dot(normal, light_dir)+0.1, 0.0),2.0)*0.9;                 \n"
    "       float spec = pow(max(dot(view_dir, light_refl), 0.0), 32.0)*0.4;                \n"
    "       float emis = pow(f_light, 2.0)*0.7;                                             \n"
    "       final_color =                                                                   \n"
    "               bar_color * (diff*shdw+ambi)+                                           \n"
    "               vec3(1.0, 1.0, 1.0)*spec*shdw+                                          \n"
    "               vec3(0.1, 1.0, 0.3)*emis;                                               \n"
    "}                                                                                      \n"
};

const char *c_scene_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "mat4 u_matrix = u_viewsceen*u_worldview*u_nodeworld;                                   \n"
    "in vec4 a_position;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_matrix*a_position;                                              \n"
    "}                                                                                      \n"
};

const char *c_scene_frag =  {
    "#version 330                                                                           \n"
    "void main(){ }                                                                         \n"
};

/* clang-format on */
class Uniforms {
public:
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Mat4f u_light_proj_matrix;
	Mat4f u_light_matrix;
	Vec3f u_camera_pos;
	Vec3f u_light_pos;
	float u_fade;
	uint32_t u_offset_texture;
	uint32_t u_height_texture;
	uint32_t u_shadows;

	Uniforms()
	{
		m_attrs = {
		        {"u_viewsceen",         &u_viewsceen        },
		        {"u_worldview",         &u_worldview        },
		        {"u_nodeworld",         &u_nodeworld        },
		        {"u_light_proj_matrix", &u_light_proj_matrix},
		        {"u_light_matrix",      &u_light_matrix     },
		        {"u_camera_pos",        &u_camera_pos       },
		        {"u_light_pos",         &u_light_pos        },
		        {"u_fade",              &u_fade             },
		        {"u_offset_texture",    &u_offset_texture   },
		        {"u_height_texture",    &u_height_texture   },
		        {"u_shadows",           &u_shadows          },
		};
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class DisplayScene {
public:
	~DisplayScene()
	{
		for (auto &array: m_arrays) {
			delete array;
			array = nullptr;
		}
	}

	template<typename ShapeGenerator>
	void addShape(const Uniforms &unifs, const ShapeGenerator &shape_generator)
	{
		auto *array = new shapes::Array();

		Attrs shader_attrs = {
		        {"frag", c_scene_frag},
		        {"vert", c_scene_vert},
		};

		array->initShader(shader_attrs, Attrs(unifs));
		array->initArray(shape_generator, {"position"});
		m_arrays.push_back(array);
		m_iterator = begin(m_arrays);
	}

	double draw(Uniforms &unifs, double time)
	{
		assert(!m_arrays.empty());
		assert(m_iterator != end(m_arrays));

		const auto interval = 11.0;
		auto segment = time - m_time;
		auto fade = segment * (interval - segment);
		fade -= 1.0;
		if (fade < 0.0) {
			fade = 0.0;
		}
		fade = sqrt(fade / interval);
		if (fade > 1.0) {
			fade = 1.0;
		}

		if (segment > interval) {
			if (++m_iterator == end(m_arrays)) {
				m_iterator = begin(m_arrays);
			}
			m_time = time;
		}

		auto radius = (*m_iterator)->boundingSphere().radius;
		auto dist = float(1.0 + sin(time / 13.0 * math::two_pi()) * 2.5);

		auto fovx = 45.0f;
		auto aspect = 1.0f;
		auto near = 1.0f + dist;
		auto far = radius * 2.0f + 1.0f + dist;

		unifs.u_viewsceen.set_projection(&fovx, &aspect, &near, &far, false);  // fovx

		unifs.u_worldview = Mat4f::orbiting(ezero(), time, radius + 1.5 + dist, 0, 0, 0, 27, 0, 89, 23);

		unifs.u_nodeworld = Mat4f(Quatf(time / -37.0 * math::two_pi(), Vec3f(1, 1, 1)));

		(*m_iterator)->draw(nullptr);

		return fade;
	}

private:
	double m_time = 0;
	std::vector<shapes::Array *> m_arrays;
	std::vector<shapes::Array *>::iterator m_iterator;
};

class App : public SpuPage {
public:
	enum {
		e_display = 0,
		e_shadow,
	};

	static constexpr auto c_side = size_t(128);
	static constexpr auto c_shadow_size = size_t(512);

	Uniforms m_unifs;

	SpuFrame m_heightsFrame;
	SpuFrame m_shadowsFrame;
	shapes::Array m_cube;

	SpuTexture m_offsetTexture;
	DisplayScene m_scene;

	Mat4f m_viewtexc;  // due to uniform name conflict

	void initOffsets()
	{
		std::vector<float> offset_data(c_side * c_side * 2);
		{
			auto p = std::begin(offset_data);
			for (auto j = 0; j != c_side; ++j) {
				float v = j / float(c_side - 1) - 0.5;
				for (auto i = 0; i != c_side; ++i) {
					float u = i / float(c_side - 1) - 0.5;
					*p++ = u * c_side;
					*p++ = v * c_side;
				}
			}
		}

		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D     },
			        {"iformat",     GL_RG32F          },
			        {"width",       c_side            },
			        {"height",      c_side            },
			        {"data",        offset_data.data()},
			        {"min_filter",  GL_NEAREST        },
			        {"mag_filter",  GL_NEAREST        },
			        {"wrap_s",      GL_CLAMP_TO_BORDER},
			        {"wrap_t",      GL_CLAMP_TO_BORDER},
			        {"auto_mipmap", 0                 },
			};
			m_offsetTexture.init(attrs);
			m_unifs.u_offset_texture = m_offsetTexture.id();
		}
	}

	void initHeights()
	{
		auto border = Vec4f(1, 1, 1, 1);
		auto viewport = Rectf(0, 0, c_side, c_side);

		Attrs attrs = {
		        {"viewport0",         viewport             },
		        {"depth.target",      GL_TEXTURE_2D        },
		        {"depth.iformat",     GL_DEPTH_COMPONENT32F},
		        {"depth.min_filter",  GL_NEAREST           },
		        {"depth.mag_filter",  GL_NEAREST           },
		        {"depth.wrap_s",      GL_CLAMP_TO_BORDER   },
		        {"depth.wrap_t",      GL_CLAMP_TO_BORDER   },
		        {"depth.border",      border               },
		        {"depth.auto_mipmap", 0                    },
		};
		m_heightsFrame.init(attrs);
		m_unifs.u_height_texture = m_heightsFrame.getBuffer("depth").id();
	}

	void initShadows()
	{
		auto border = Vec4f(1, 1, 1, 1);
		auto viewport = Rectf(0, 0, c_shadow_size, c_shadow_size);

		Attrs attrs = {
		        {"viewport0",          viewport                 },
		        {"depth.target",       GL_TEXTURE_2D            },
		        {"depth.iformat",      GL_DEPTH_COMPONENT32F    },
		        {"depth.min_filter",   GL_LINEAR                },
		        {"depth.mag_filter",   GL_LINEAR                },
		        {"depth.wrap_s",       GL_CLAMP_TO_BORDER       },
		        {"depth.wrap_t",       GL_CLAMP_TO_BORDER       },
		        {"depth.border",       border                   },
		        {"depth.compare_mode", GL_COMPARE_REF_TO_TEXTURE},
		        {"depth.auto_mipmap",  0                        },
		};
		m_shadowsFrame.init(attrs);
		m_unifs.u_shadows = m_shadowsFrame.getBuffer("depth").id();
	}

	App(const char *name) : SpuPage(name, true, {0.1, 0.1, 0.1, 0.0}) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		Attrs disp_shader_attrs = {
		        {"frag", c_disp_frag},
		        {"vert", c_disp_vert},
		};

		Attrs shadow_shader_attrs = {
		        {"frag", c_shadow_frag},
		        {"vert", c_shadow_vert},
		};

		m_cube.setMaxShaderType(2);  // 0:display 1:shadow
		m_cube.initShader(disp_shader_attrs, Attrs(m_unifs), 0);
		m_cube.initShader(shadow_shader_attrs, Attrs(m_unifs), 1);

		m_cube.initArray(shapes::Cube(0.95, 1.0, 0.95, 0.0, 0.5, 0.0), {"position", "normal"});

		// model
		m_cube.setInstanceCount(c_side * c_side);

		// fobo
		initShadows();
		initHeights();
		initOffsets();

		// scenes
		m_scene.addShape(m_unifs, shapes::Cage());
		m_scene.addShape(m_unifs, shapes::WickerTorus(1.0, 0.5, 0.02, 12, 12));
		m_scene.addShape(m_unifs, shapes::SpiralSphere());
		m_scene.addShape(m_unifs, shapes::TwistedTorus(0.9, 0.5, 0.02, 8, 48, 7));
		m_scene.addShape(m_unifs, shapes::Icosahedron());

		// matrix
		auto light_viewport = Rectf(0, 0, 1, 1);
		auto light_proj = math::perspective(light_viewport, 74, 1, 3 * c_side);

		m_unifs.u_light_proj_matrix = light_proj;
		m_viewtexc = light_proj;

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.poly_offset = {4.0, 4.0};
		renderstate.flags.depth_test = true;
	}

	void render() override
	{
		auto esec = getSeconds().current();
		auto viewscreen = math::perspective(viewport(0), 65, 1, 3 * c_side);
		auto light = Mat4f::orbiting(
		        Vec3f(0.0, c_side * 0.25, 0.0), esec, c_side * 1.5, 0, 0, 0.0, -13.3, 45, 25, 19);
		auto worldview = Mat4f::orbiting(ezero(), esec, c_side * 1.1, 0, 0, 0, 19, 50, 39, 20);

		// update
		double fade;
		{
			m_heightsFrame.begin();
			m_heightsFrame.clear();
			fade = m_scene.draw(m_unifs, esec);
			m_heightsFrame.end();
		}

		// Shadow map
		{
			SpuScopedRenderstate renderstate(true);

			m_shadowsFrame.begin();
			m_shadowsFrame.clear();

			m_unifs.u_fade = fade;
			m_unifs.u_worldview = light;
			m_unifs.u_viewsceen = m_viewtexc;

			renderstate.flags.fill_offset = true;
			renderstate.use();

			m_cube.setShaderType(e_shadow);
			m_cube.draw(nullptr);
			m_shadowsFrame.end();
		}

		// model
		{
			m_unifs.u_fade = fade;
			m_unifs.u_light_pos = light.unitary_inverse().c[3];
			m_unifs.u_camera_pos = worldview.unitary_inverse().c[3];
			m_unifs.u_light_matrix = light;
			m_unifs.u_worldview = worldview;
			m_unifs.u_viewsceen = viewscreen;

			m_cube.setShaderType(e_display);
			m_cube.draw(nullptr);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("030_pin_display");
}  // namespace
}  // namespace spu::oglplus
