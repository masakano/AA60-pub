//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/plane.hpp>
#include <shapes/spiral_sphere.hpp>

namespace spu::oglplus {
namespace {

class App : public SpuPage {
public:
	shapes::Plane m_planeShape;

	int32_t m_texWidth;
	int32_t m_texHeight;
	int32_t m_texSizeDiv;

	shapes::Array m_planeArray;
	shapes::Array m_shapeArray;
	SpuFrame m_frame;

	Vec3f u_light_position;
	Mat4f u_viewscreen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_normal;
	uint32_t u_lightmap;

	void buildPlaneShader()
	{
		Attrs unif_attrs = {
		        {"u_light_position", &u_light_position},
		        {"u_viewscreen",     &u_viewscreen    },
		        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
		        {"u_normal",         &u_normal        },
		        {"u_lightmap",       &u_lightmap      },
		};
		shapes::loadShader(
		        m_planeArray.getAShader(), "shaders/027_reflected_shape/plane.us", Attrs(), unif_attrs);
	}

	void buildPlaneArray() { m_planeArray.initArray(m_planeShape, {"position"}); }

	void buildShapeShader()
	{
		Attrs unif_attrs = {
		        {"u_light_position", &u_light_position},
		        {"u_viewscreen",     &u_viewscreen    },
		        {"u_worldview",      &u_worldview     },
		        {"u_nodeworld",      &u_nodeworld     },
		};
		shapes::loadShader(
		        m_shapeArray.getAShader(), "shaders/027_reflected_shape/shape.us", Attrs(), unif_attrs);
	}

	void buildShapeArray() { m_shapeArray.initArray(shapes::SpiralSphere(), {"position", "normal"}); }

	void buildFrame()
	{
		auto viewport = Rectf(0, 0, m_texWidth / m_texSizeDiv, m_texHeight / m_texSizeDiv);

		Attrs attrs = {
		        {"viewport0",      viewport             },
		        {"color0.target",  GL_TEXTURE_RECTANGLE },
		        {"color0.iformat", GL_RGBA8             },
		        {"depth.target",   GL_TEXTURE_RECTANGLE },
		        {"depth.iformat",  GL_DEPTH_COMPONENT32F},
		};
		m_frame = SpuFrame(attrs);
		u_lightmap = m_frame.getBuffer("color0").id();
	}

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_planeShape = shapes::Plane(ezero(), Vec3f(3, 0, 0), Vec3f(0, 0, -3), 15, 15);

		m_texWidth = 800;
		m_texHeight = 600;
		m_texSizeDiv = 2;

		buildPlaneShader();
		buildPlaneArray();

		u_light_position = {3.0, 0.5, 2.0};
		u_normal = m_planeShape.normal();

		buildShapeShader();
		buildShapeArray();
		buildFrame();

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.flags.depth_test = true;
		renderstate.flags.cull_face = true;

		// glDisable(GL_FRAMEBUFFER_SRGB);  // temporary
		{
			u_viewscreen = math::perspective(viewport(0), 60, 1, 20);
			buildFrame();
		}
	}

	void render() override
	{
		auto bgcolor = Vec4f(0.5, 0.5, 0.4, 0.0);
		Attrs frame_attrs = {
		        {"bgcolor0", bgcolor},
		};

		SpuPage::set(frame_attrs);
		m_frame.set(frame_attrs);

		Mat4f reflection_y;
		reflection_y.c[1] = -reflection_y.c[1];

		auto reflection = math::unit().trans({0, -1, 0}) * reflection_y;

		auto esec = getSeconds().current();
		auto worldview = Mat4f::orbiting(ezero(), esec, 4.5, 0, 0, 0, 40, 45, -35, 7);

		// render into the off-screen framebuffer
		m_frame.begin();
		{
			u_nodeworld = math::unit().trans({0.0, 0.6, 0.0})
			            * math::unit().rot("x", -esec / 12.0 * math::two_pi());
			u_worldview = worldview * reflection;

			// shape_prog.use();
			m_frame.clear();
			m_shapeArray.draw(nullptr);
		}
		m_frame.end();

		// render into the on-screen framebuffer
		{
			u_worldview = worldview;

			SpuPage::clear();
			m_shapeArray.draw(nullptr);
		}
		{
			u_worldview = worldview;
			u_nodeworld = math::unit().trans({0.0, -0.5, 0.0});

			m_planeArray.draw(nullptr);
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("027_reflected_shape");

}  // namespace
}  // namespace spu::oglplus
