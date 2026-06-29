//
// App :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <shapes/cube.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_vert =  {
    "#version 330                                                                           \n"
    "uniform mat4 u_viewsceen;                                                              \n"
    "uniform mat4 u_worldview;                                                              \n"
    "uniform mat4 u_nodeworld;                                                              \n"
    "uniform vec3 u_light_pos;                                                              \n"
    "in vec4 a_position;                                                                    \n"
    "in vec3 a_normal;                                                                      \n"
    "out vec3 f_light_dir;                                                                  \n"
    "out vec3 f_normal;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = u_nodeworld * a_position;                                         \n"
    "       f_light_dir = normalize(u_light_pos - gl_Position.xyz);                         \n"
    "       f_normal = normalize(mat3(u_nodeworld)* a_normal);                              \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;                          \n"
    "}                                                                                      \n"
};

const char *c_frag =  {
    "#version 330                                                                           \n"
    "uniform vec3 u_ambient_color;                                                          \n"
    "uniform vec3 u_albedo_color;                                                           \n"
    "in vec3 f_light_dir;                                                                   \n"
    "in vec3 f_normal;                                                                      \n"
    "out vec4 final_color;                                                                  \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float d = max(dot(f_light_dir,f_normal),0.0);                                   \n"
    "       float e = sin(                                                                  \n"
    "               10.0*f_light_dir.x +                                                    \n"
    "               20.0*f_light_dir.y +                                                    \n"
    "               25.0*f_light_dir.z                                                      \n"
    "       )*0.9;                                                                          \n"
    "       final_color = vec4(                                                             \n"
    "               mix(u_ambient_color, u_albedo_color, d+e),                              \n"
    "               1.0                                                                     \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_dof_vert =  {
    "#version 330                                                                           \n"
    "uniform uint u_viewport_width;                                                         \n"
    "uniform uint u_viewport_height;                                                        \n"
    "in vec4 a_position;                                                                    \n"
    "out vec2 f_texcoord;                                                                   \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       gl_Position = a_position;                                                       \n"
    "       f_texcoord = vec2(                                                              \n"
    "               (a_position.x*0.5 + 0.5)*u_viewport_width,                              \n"
    "               (a_position.y*0.5 + 0.5)*u_viewport_height                              \n"
    "       );                                                                              \n"
    "}                                                                                      \n"
};

const char *c_dof_frag =  {
    "#version 330                                                                           \n"
    "uniform sampler2DRect u_color_tex;                                                     \n"
    "uniform sampler2DRect u_depth_tex;                                                     \n"
    "uniform float u_focus_depth;                                                           \n"
    "uniform uint u_sample_mult;                                                            \n"
    "in vec2 f_texcoord;                                                                    \n"
    "out vec4 final_color;                                                                  \n"
    "const float strength = 16.0;                                                           \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "       float frag_depth = texture(u_depth_tex, f_texcoord).r;                          \n"
    "       vec3 color = texture(u_color_tex, f_texcoord).rgb;                              \n"
    "       float of = abs(frag_depth - u_focus_depth);                                     \n"
    "       int nsam = int(of*u_sample_mult);                                               \n"
    "       float inv_nsam = 1.0 / (1.0 + nsam);                                            \n"
    "       float astep = (3.14151*4.0)/nsam;                                               \n"
    "       for (int i=0; i!=nsam; ++i)                                                     \n"
    "       {                                                                               \n"
    "               float a = i*astep;                                                      \n"
    "               float d = sqrt(i*inv_nsam);                                             \n"
    "               float sx = cos(a)*of*strength*d;                                        \n"
    "               float sy = sin(a)*of*strength*d;                                        \n"
    "               vec2 sam_tex_coord = f_texcoord + vec2(sx, sy) + noise2(vec2(sx, sy));  \n"
    "               color += texture(u_color_tex, sam_tex_coord).rgb;                       \n"
    "       }                                                                               \n"
    "       final_color = vec4(color * inv_nsam , 1.0);                                     \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public SpuPage {
public:
	std::vector<Mat4f> m_cubeMatrices;
	Mat4f u_viewsceen;
	Mat4f u_worldview;
	Mat4f u_nodeworld;
	Vec3f u_light_pos;
	Vec3f u_ambient_color;
	Vec3f u_albedo_color;

	uint32_t u_viewport_width;
	uint32_t u_viewport_height;
	uint32_t u_color_tex;
	uint32_t u_depth_tex;
	float u_focus_depth;
	uint32_t u_sample_mult;

	std::vector<uint16_t> m_faceIndx;
	std::vector<uint16_t> m_edgeIndx;

	shapes::Array m_shapeArray;

	SpuShader m_dofShader;
	SpuArray m_dofArray;

	SpuFrame m_frame;

	// Returns a vector of cube offsets
	std::vector<Mat4f> makeCubeMatrices(uint32_t count, float max_dist)
	{
		srand(59039);
		// srand(0);
		std::vector<Mat4f> offsets(count);
		for (auto i = 0u; i != count; ++i) {
			auto x = frand();
			auto y = frand();
			auto z = frand();
			auto sx = (rand() % 2) != 0 ? 1.0f : -1.0f;
			auto sy = (rand() % 2) != 0 ? 1.0f : -1.0f;
			auto sz = (rand() % 2) != 0 ? 1.0f : -1.0f;

			offsets[i] = math::unit().trans(
			                     {sx * (1.0f + powf(x, 0.9f) * max_dist),
			                      sy * (1.0f + powf(y, 1.5f) * max_dist),
			                      sz * (1.0f + powf(z, 0.7f) * max_dist)})
			           * math::unit().rot(
			                   "xyz", -frand() * math::two_pi() / 4.0,
			                   -frand() * math::two_pi() / 4.0, -frand() * math::two_pi() / 4.0);
		}
		return offsets;
	}

	void buildShaderMain()
	{
		Attrs shader_attrs = {
		        {"vert", c_vert},
		        {"frag", c_frag},
		};

		Attrs unif_attrs = {
		        {"u_viewsceen",     &u_viewsceen    },
                        {"u_worldview",     &u_worldview    },
		        {"u_nodeworld",     &u_nodeworld    },
                        {"u_light_pos",     &u_light_pos    },
		        {"u_ambient_color", &u_ambient_color},
                        {"u_albedo_color",  &u_albedo_color },
		};
		m_shapeArray.initShader(shader_attrs, unif_attrs);
	}

	void buildModelMain()
	{
		shapes::Cube cube_shape;
		m_shapeArray.initArray(cube_shape, {"position", "normal"});
		m_faceIndx = cube_shape.indices(shapes::Shape::DefaultTag());
		m_edgeIndx = cube_shape.indices(shapes::Shape::EdgesTag());
	}

	void buildShaderDof()
	{
		Attrs shader_attrs = {
		        {"vert", c_dof_vert},
		        {"frag", c_dof_frag},
		};

		Attrs unif_attrs = {
		        {"u_viewport_width",  &u_viewport_width },
                        {"u_viewport_height", &u_viewport_height},
		        {"u_color_tex",       &u_color_tex      },
                        {"u_depth_tex",       &u_depth_tex      },
		        {"u_focus_depth",     &u_focus_depth    },
                        {"u_sample_mult",     &u_sample_mult    },
		};
		shapes::loadShader(m_dofShader, shader_attrs, unif_attrs);
	}

	void buildModelDof()
	{
		std::vector<vec2f_t> vertices = {
		        {-1.0, -1.0},
		        {-1.0, +1.0},
		        {+1.0, -1.0},
		        {+1.0, +1.0},
		};

		Attrs vert_attrs = {
		        {"shader_id",    m_dofShader.id()},
		        {"a.a_position", 2               },
		        {"nelem",        vertices.size() },
		        {"data",         vertices.data() },
		};
		m_dofArray.init(vert_attrs);
	}

	void buildFrame(float width, float height)
	{
		auto viewport = Rectf(0, 0, width, height);
		Attrs attrs = {
		        {"viewport0",      viewport             },
		        {"color0.target",  GL_TEXTURE_RECTANGLE },
		        {"color0.iformat", GL_RGBA8             },
		        {"depth.target",   GL_TEXTURE_RECTANGLE },
		        {"depth.iformat",  GL_DEPTH_COMPONENT32F},
		};
		m_frame = SpuFrame(attrs);
	}

	App(const char *name) : SpuPage(name, true) {}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		m_cubeMatrices = makeCubeMatrices(100, 10.0);

		buildFrame(int32_t(viewport(0).sx), int32_t(viewport(0).sy));
		buildShaderMain();
		buildModelMain();
		buildShaderDof();
		buildModelDof();

		u_light_pos = {30.0, 50.0, 20.0};
		u_sample_mult = 128 /*512*/;

		auto &renderstate = SpuPage::getRenderstate();
		renderstate.depth_func = GL_LEQUAL;
		renderstate.flags.depth_test = true;
		// renderstate.flags.line_smooth = true;
		renderstate.blend_func = {
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		        GL_SRC_ALPHA,
		        GL_ONE_MINUS_SRC_ALPHA,
		};
		// renderstate.use();

		{
			u_viewport_width = uint32_t(viewport(0).sx);
			u_viewport_height = uint32_t(viewport(0).sy);
			u_viewsceen = math::perspective(viewport(0), 65, 1, 50);
			buildFrame(int32_t(viewport(0).sx), int32_t(viewport(0).sy));
		}
	}

	void render() override
	{
		auto bgcolor = Vec4f(0.9, 0.9, 0.9, 0.0);

		Attrs frame_attrs = {
		        {"bgcolor0", bgcolor},
		};

		SpuPage::set(frame_attrs);
		m_frame.set(frame_attrs);
		m_frame.begin();
		m_frame.clear();

		auto esec = getSeconds().current();
		u_worldview = Mat4f::orbiting(ezero(), esec, 20.5, 0, 0, 0, 20, 0, 30, 25);

		auto i = std::begin(m_cubeMatrices);
		auto e = std::end(m_cubeMatrices);
		while (i != e) {
			u_nodeworld = *i;
			u_ambient_color = {0.7, 0.6, 0.2};
			u_albedo_color = {1.0, 0.8, 0.3};

			m_shapeArray.send(m_faceIndx, -1, 2);
			m_shapeArray.setMode(GL_TRIANGLES);
			m_shapeArray.draw(nullptr);

			u_ambient_color = {0.1, 0.1, 0.1};
			u_albedo_color = {0.3, 0.3, 0.3};

			m_shapeArray.send(m_edgeIndx, -1, 2);
			m_shapeArray.setMode(GL_LINE_LOOP);
			m_shapeArray.draw(nullptr);

			++i;
		}
		m_frame.end();

		u_color_tex = m_frame.getBuffer("color0").id();
		u_depth_tex = m_frame.getBuffer("depth").id();
		u_focus_depth = 0.6 + sin(esec / 9.0 * math::two_pi()) * 0.3;

		auto &renderstate = SpuPage::getRenderstate();

		renderstate.flags.blend = true;
		renderstate.use();
		m_dofShader.use();
		m_dofArray.draw(GL_TRIANGLE_STRIP);

		renderstate.flags.blend = false;
		renderstate.use();
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("027_depth_of_field");
}  // namespace
}  // namespace spu::oglplus
