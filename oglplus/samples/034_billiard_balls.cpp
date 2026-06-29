//
// Uniforms :
//
#include <spu++/spu_page.h>
#include <images/brushed_metal.hpp>
#include <shapes/plane.hpp>
#include <shapes/sphere.hpp>
#include <shapes/array.hpp>
#include <math/matrix.hpp>

#define e_ball_count 15
#define e_ball_count_txt "15"

namespace spu::oglplus {
namespace {

const int32_t c_cube_target[6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5,
};

class Uniforms {
public:
	Mat4f u_worldview;
	Mat4f u_viewscreen;
	Mat4f u_nodeworld;
	Mat4f u_nodescreen;
	Mat4f u_texture_matrix;

	Vec3f u_ball_positions[e_ball_count];
	Vec3f u_eye_position;
	Vec3f u_light_position;
	Vec3f u_color1;
	Vec3f u_color2;

	uint32_t u_ball_index;
	uint32_t u_cloth_texture;
	uint32_t u_lightmap_texture;
	uint32_t u_number_texture;
	uint32_t u_lightmap;

	Uniforms()
	{
		m_attrs = {
		        {"u_worldview",        &u_worldview        },
		        {"u_viewscreen",       &u_viewscreen       },
		        {"u_nodeworld",        &u_nodeworld        },
		        {"u_nodescreen",       &u_nodescreen       },
		        {"u_texture_matrix",   &u_texture_matrix   },
		        {"u_ball_positions",   &u_ball_positions[0]},
		        {"u_eye_position",     &u_eye_position     },
		        {"u_light_position",   &u_light_position   },
		        {"u_color1",           &u_color1           },
		        {"u_color2",           &u_color2           },
		        {"u_ball_index",       &u_ball_index       },
		        {"u_cloth_texture",    &u_cloth_texture    },
		        {"u_lightmap_texture", &u_lightmap_texture },
		        {"u_number_texture",   &u_number_texture   },
		        {"u_lightmap",         &u_lightmap         },
		};
	}
	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class App : public SpuPage {
public:
	Uniforms m_unifs;

	Vec3f m_planeU = Vec3f(16, 0, 0);
	Vec3f m_planeV = Vec3f(0, 0, -16);

	shapes::Array m_plane;
	shapes::Array m_sphere;

	const uint32_t c_ball_count = e_ball_count;

	std::vector<Vec3f> m_ballColors;
	std::vector<Vec3f> m_ballOffsets;
	std::vector<Vec3f> m_ballRotations;

	SpuTexture m_clothTexture;
	SpuTexture m_lightmapTexture;
	SpuTexture m_numberTexture;
	std::vector<SpuTexture> m_reflectTextures;

	enum {
		e_normal = 0,
		e_colormap,
	};

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_reflectTextures.resize(c_ball_count);

		Attrs normal_shader_attrs = {
		        {"def_color_map",  "0"             },
		        {"def_ball_count", e_ball_count_txt},
		};
		Attrs colormap_shader_attrs = {
		        {"def_color_map",  "1"             },
		        {"def_ball_count", e_ball_count_txt},
		};

		// shader
		m_sphere.setMaxShaderType(2);  // 0:normal 1:colormap
		m_sphere.initShader(
		        "shaders/034_billiard_balls/ball.us", normal_shader_attrs, Attrs(m_unifs), 0);
		m_sphere.initShader(
		        "shaders/034_billiard_balls/ball.us", colormap_shader_attrs, Attrs(m_unifs), 1);

		m_plane.setMaxShaderType(2);  // 0:normal 1:colormap
		m_plane.initShader(
		        "shaders/034_billiard_balls/cloth.us", normal_shader_attrs, Attrs(m_unifs), 0);
		m_plane.initShader(
		        "shaders/034_billiard_balls/cloth.us", colormap_shader_attrs, Attrs(m_unifs), 1);

		// array
		auto plane_shape = shapes::Plane(m_planeU, m_planeV);
		m_plane.initArray(plane_shape, {"position", "normal", "tangent", "texcoord"});

		auto sphere_shape = shapes::Sphere(1.0, 36, 24);
		m_sphere.initArray(sphere_shape, {"position", "normal", "tangent", "texcoord"});

		// clang-format off
		m_ballColors = {
			{ 0.8, 0.5, 0.2 }, { 0.2, 0.2, 0.5 }, { 0.6, 0.2, 0.4 }, { 0.1, 0.1, 0.3 },
			{ 0.0, 0.0, 0.0 }, { 0.3, 0.1, 0.2 }, { 0.2, 0.5, 0.2 }, { 0.6, 0.3, 0.2 }
		};

		m_ballOffsets = {
			{ 3.0, 1.0, 6.5 },  { 5.0, 1.0, 5.0 },  { 3.0, 1.0, -1.0 }, { -0.1, 1.0, -1.1 },
			{ -3.0, 1.0, 3.0 }, { -2.8, 1.0, 7.0 }, { -1.1, 1.0, 9.0 }, { 3.0, 1.0, 2.0 },
			{ -7.0, 1.0, 3.0 }, { -9.5, 1.0, 4.5 }, { 1.0, 1.0, 5.2 },  { -8.0, 1.0, 8.0 },
			{ -5.0, 1.0, 1.0 }, { 2.0, 1.0, 9.0 },  { 8.0, 1.0, 7.5 }
		};

		m_ballRotations = {
			{ 0.3, -0.2, -0.1 }, { 0.2, 0.3, 0.4 },   { -0.4, -0.4, 0.2 },  { 0.2, 0.3, -0.4 },
			{ -0.7, -0.2, 0.6 }, { 0.3, 0.3, 0.2 },   { 0.5, 0.2, 0.3 },    { -0.4, 0.4, -0.4 },
			{ 0.3, -0.3, 0.1 },  { 0.1, -0.2, -0.2 }, { -0.2, -0.3, -0.0 }, { -0.3, 0.5, 0.3 },
			{ -0.4, 0.1, 0.1 },  { 0.3, 0.3, -0.2 },  { -0.2, -0.2, 0.4 }
		};
		// clang-format on

		assert(m_ballOffsets.size() == c_ball_count);
		assert(m_ballRotations.size() == c_ball_count);

		preRenderLightmap();

		// reset environment
		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;
		renderstate.cull_face = GL_BACK;
		renderstate.use();

		initializeNumberTexture();
		initializeClothTexture();

		const uint32_t cubemap_side = 128;

		std::vector<SpuTexture> temp_cubemaps(c_ball_count);

		initializeCubemaps(m_reflectTextures, cubemap_side);
		initializeCubemaps(temp_cubemaps, cubemap_side);

		// prerender the cubemaps
		preRenderCubemaps(temp_cubemaps, m_reflectTextures, cubemap_side);
	}

	void initializeNumberTexture()
	{
		const auto width = 256;
		const auto height = 256;
		const auto depth = 15;

		const char *paths[] = {
		        "pool_ball_1", "pool_ball_2", "pool_ball_3", "pool_ball_4", "pool_ball_5",
		        "pool_ball_6", "pool_ball_7", "pool_ball_8", "pool_ball_9", "pool_ball10",
		        "pool_ball11", "pool_ball12", "pool_ball13", "pool_ball14", "pool_ball15",
		};

		auto border = Vec4f(0, 0, 0, 0);
		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D_ARRAY    },
		        {"iformat",    GL_RGBA8               },
		        {"width",      width                  },
		        {"height",     height                 },
		        {"depth",      depth                  },
		        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter", GL_LINEAR              },
		        {"wrap_s",     GL_CLAMP_TO_BORDER     },
		        {"wrap_t",     GL_CLAMP_TO_BORDER     },
		        {"border",     border                 },
		};
		m_numberTexture.init(attrs);

		for (auto i = 0; i < 15; i++) {
			std::string full_path = std::string("assets/textures/") + paths[i] + ".png";
			SpuTexture single_texture(full_path.c_str(), Attrs());
			int32_t dst_loc[4] = {0, 0, i, 0};

			single_texture.sync(false);
			m_numberTexture.copy(single_texture.id(), dst_loc, nullptr, nullptr);
		}
		m_numberTexture.update();
		m_unifs.u_number_texture = m_numberTexture.id();
	}

	void initializeClothTexture()
	{
		const auto image = images::BrushedMetalUByte(512, 512, 10240, -16, +16, 8, 32);

		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D             },
		        {"iformat",    GL_RGB8                   },
		        {"width",      image.width()             },
		        {"height",     image.height()            },
		        {"min_filter", GL_LINEAR_MIPMAP_LINEAR   },
		        {"mag_filter", GL_LINEAR                 },
		        {"wrap_s",     GL_REPEAT                 },
		        {"wrap_t",     GL_REPEAT                 },
		        {"data",       image.data()},
		};
		m_clothTexture.init(attrs);
		m_unifs.u_cloth_texture = m_clothTexture.id();
	}

	void initializeCubemaps(std::vector<SpuTexture> &cubemaps, size_t cubemap_side) const
	{
		const std::vector<GLubyte> black(cubemap_side * cubemap_side * 4 * 6, 0x00);  // 6 faces

		for (auto b = 0u; b < c_ball_count; b++) {
			Attrs attrs = {
			        {"target",      GL_TEXTURE_CUBE_MAP       },
                                {"iformat",     GL_RGB8                   },
			        {"width",       cubemap_side              },
                                {"height",      cubemap_side              },
			        {"min_filter",  GL_LINEAR                 },
                                {"mag_filter",  GL_LINEAR                 },
			        {"wrap_s",      GL_CLAMP_TO_EDGE          },
                                {"wrap_t",      GL_CLAMP_TO_EDGE          },
			        {"wrap_r",      GL_CLAMP_TO_EDGE          },
                                {"data",        black.data()},
			        {"cube_target", c_cube_target             },
			};
			cubemaps[b].init(attrs);
		}
	}

	void preRenderLightmap()
	{
		const auto light_position = Vec3f(0.0, 20.0, -2.0);
		const auto c_tex_side = 512;

		m_unifs.u_light_position = light_position;
		m_unifs.u_color1 = {0.1, 0.3, 0.1};
		m_unifs.u_color2 = {0.3, 0.4, 0.3};

		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D   },
                                {"iformat",    GL_RGB8         },
			        {"width",      c_tex_side      },
                                {"height",     c_tex_side      },
			        {"min_filter", GL_LINEAR       },
                                {"mag_filter", GL_LINEAR       },
			        {"wrap_s",     GL_CLAMP_TO_EDGE},
                                {"wrap_t",     GL_CLAMP_TO_EDGE},
			};
			m_lightmapTexture.init(attrs);
			m_unifs.u_lightmap_texture = m_lightmapTexture.id();
		}

		SpuFrame frame;
		{
			auto viewport = Rectf(0, 0, c_tex_side, c_tex_side);
			auto bgcolor = Vec4f(1.0, 1.0, 1.0, 0.0);
			Attrs attrs = {
			        {"viewport0",         viewport                  },
			        {"color0.texture_id", m_unifs.u_lightmap_texture},
			        {"bgcolor0",          bgcolor                   },
			        {"bgdepth",           -1.0                      }
                        };
			frame.init(attrs);
		}
		frame.begin();
		frame.clear();

		shapes::Array plane_array;
		Attrs shader_attrs = {
		        {"def_color_map",  "0"             },
		        {"def_ball_count", e_ball_count_txt},
		};
		plane_array.initShader("shaders/034_billiard_balls/lightmap.us", shader_attrs, Attrs(m_unifs));

		auto plane_shape = shapes::Plane(m_planeU, m_planeV);
		plane_array.initArray(plane_shape, {"position", "normal", "tangent", "texcoord"});

		auto i_u = length(m_planeU);
		i_u = 1.0 / (i_u * i_u);

		auto i_v = length(m_planeV);
		i_v = 1.0 / (i_v * i_v);

		auto v0 = m_planeU * i_u;
		auto v1 = m_planeV * i_v;

		m_unifs.u_nodescreen = Mat4f(
		        v0.x, v0.y, v0.z, 0.0, v1.x, v1.y, v1.z, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

		m_unifs.u_light_position = light_position;
		for (auto i = 0u; i < m_ballOffsets.size(); i++) {
			m_unifs.u_ball_positions[i] = m_ballOffsets[i];
		}

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = false;
		renderstate.use();

		plane_array.draw(nullptr);

		renderstate.flags.depth_test = true;
		renderstate.use();

		frame.end();
	}

	void preRenderCubemaps(
	        std::vector<SpuTexture> &src_cubemaps, std::vector<SpuTexture> &dst_cubemaps, int32_t tex_side)
	{
		SpuTexture common_depth_texture;
		Attrs attrs = {
		        {"target",      GL_TEXTURE_CUBE_MAP  },
		        {"iformat",     GL_DEPTH_COMPONENT32F},
		        {"width",       tex_side             },
		        {"height",      tex_side             },
		        {"min_filter",  GL_NEAREST           },
		        {"mag_filter",  GL_NEAREST           },
		        {"wrap_s",      GL_CLAMP_TO_EDGE     },
		        {"wrap_t",      GL_CLAMP_TO_EDGE     },
		        {"cube_target", c_cube_target        },
		};
		common_depth_texture.init(attrs);

		auto texc_viewport = Rectf(0, 0, 1, 1);
		m_unifs.u_viewscreen = math::perspective(texc_viewport, 90, 1, 80);

		for (auto b = 0u; b != c_ball_count; ++b) {
			SpuFrame frame;
			auto viewport = Rectf(0, 0, tex_side, tex_side);

			Attrs attrs = {
			        {"viewport0",         viewport                 },
			        {"color0.texture_id", dst_cubemaps[b].id()     },
			        {"depth.texture_id",  common_depth_texture.id()},
			};
			frame.init(attrs);

			frame.begin();

			m_unifs.u_eye_position = m_ballOffsets[b];
			m_unifs.u_worldview = math::unit().trans(-m_ballOffsets[b]);

			renderScene(e_colormap, frame.id(), src_cubemaps, b);
			frame.end();
		}
	}

	// void renderScene(int32_t type, SpuFrame &frame, std::vector<SpuTexture> &cubemaps, int32_t skipped =
	// -1)
	void renderScene(
	        int32_t type, uint32_t frame_id, std::vector<SpuTexture> &cubemaps, int32_t skipped = -1)
	{
		{
			auto bgcolor = Vec4f(0.12, 0.13, 0.11, 0.0);
			auto bgdepth = 1.0f;
			Attrs frame_attrs = {
			        {"bgcolor0", bgcolor},
                                {"bgdepth",  bgdepth}
                        };
			spu_frame_set(frame_id, frame_attrs);

			for (auto i = 0; i < 6; i++) {
				spu_frame_set(frame_id, "layer", i);
				spu_frame_clear(frame_id);
				spu_frame_set(frame_id, "layer", -1);
			}
		}

		// Render the plane
		m_unifs.u_nodeworld = math::unit();
		m_unifs.u_texture_matrix = {
		        Vec4f(16.0, 0.0, 0.0, 0.0),
		        Vec4f(0.0, 16.0, 0.0, 0.0),
		        Vec4f(0.0, 0.0, 1.0, 0.0),
		        Vec4f(0.0, 0.0, 0.0, 1.0),
		};

		{
			const auto light_position = Vec3f(0.0, 20.0, -2.0);
			m_unifs.u_light_position = light_position;
			m_unifs.u_color1 = {0.1, 0.3, 0.1};
			m_unifs.u_color2 = {0.3, 0.4, 0.3};
		}

		m_plane.setShaderType(type);
		m_plane.draw(nullptr);

		// Render the balls
		m_unifs.u_texture_matrix = {
		        Vec4f(6.0, 0.0, 0.0, 0.0),
		        Vec4f(0.0, 3.0, 0.0, 0.0),
		        Vec4f(0.0, 0.0, 1.0, 0.0),
		        Vec4f(0.0, -1.0, 0.0, 1.0),

		};

		for (auto i = 0u; i != c_ball_count; ++i) {
			if (int(i) == skipped) {
				continue;
			}
			auto rot = m_ballRotations[i];
			auto ci = ((i / 4) % 2 == 0) ? i : ((i / 4) + 2) * 4 - i - 1;
			ci %= 8;
			auto col = m_ballColors[ci];

			m_unifs.u_nodeworld = math::unit().trans(m_ballOffsets[i])
			                    * math::unit().rot(
			                            "xyz", -rot.x, math::two_pi(), -rot.y * math::two_pi(),
			                            -rot.z * math::two_pi());

			if (i > 7) {
				m_unifs.u_color1 = {1.0, 0.9, 0.8};
			}
			else {
				m_unifs.u_color1 = col;
			}
			m_unifs.u_color2 = col;
			m_unifs.u_ball_index = i;
			m_unifs.u_lightmap = cubemaps[i].id();

			m_sphere.setShaderType(type);
			m_sphere.draw(nullptr);
		}
	}
	void render() override
	{
		auto esec = getSeconds().current();
		auto eye_target = Vec3f(0.0, 2.2, 5.0);
		auto worldview = Mat4f::orbiting(eye_target, esec, 16.0, 12.0, 15.0, 0, 24.0, 50.0, 35.0, 20.0);

		m_unifs.u_viewscreen = math::perspective(viewport(0), 60, 1, 80);
		m_unifs.u_worldview = worldview;
		m_unifs.u_eye_position = worldview.unitary_inverse().c[3];

		renderScene(e_normal, -1, m_reflectTextures);
	}
};
static ObjectRegistry<SpuPage>::Creator<App> page_creator("034_billiard_balls");
}  // namespace
}  // namespace spu::oglplus
