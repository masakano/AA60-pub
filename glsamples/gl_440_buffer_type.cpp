//
// App :
//
#include "base_app.h"
#include <ssys/half_float.h>

namespace spu {
namespace {

// not used ...orz
#if 0	
uint32_t floatTo11bit(uint32_t f)
{
	// 10 bits    =>                         EE EEEFFFFF
	// 11 bits    =>                        EEE EEFFFFFF
	// Half bits  =>                   SEEEEEFF FFFFFFFF
	// Float bits => SEEEEEEE EFFFFFFF FFFFFFFF FFFFFFFF

	// 0x000007c0 => 00000000 00000000 00000111 11000000
	// 0x00007c00 => 00000000 00000000 01111100 00000000
	// 0x000003ff => 00000000 00000000 00000011 11111111
	// 0x38000000 => 00111000 00000000 00000000 00000000
	// 0x7f800000 => 01111111 10000000 00000000 00000000
	// 0x00008000 => 00000000 00000000 10000000 00000000
	return ((((f & 0x7f800000) - 0x38000000) >> 17) & 0x07c0) |  // exponential
	       ((f >> 17) & 0x003f);                                 // Mantissa
}

uint32_t floatTo10bit(uint32_t f)
{
	// 10 bits    =>                         EE EEEFFFFF
	// 11 bits    =>                        EEE EEFFFFFF
	// Half bits  =>                   SEEEEEFF FFFFFFFF
	// Float bits => SEEEEEEE EFFFFFFF FFFFFFFF FFFFFFFF

	// 0x00000.1 => 00000000 00000000 00000000 00011111
	// 0x0000003F => 00000000 00000000 00000000 00111111
	// 0x000003E0 => 00000000 00000000 00000011 11100000
	// 0x000007C0 => 00000000 00000000 00000111 11000000
	// 0x00007C00 => 00000000 00000000 01111100 00000000
	// 0x000003FF => 00000000 00000000 00000011 11111111
	// 0x38000000 => 00111000 00000000 00000000 00000000
	// 0x7f800000 => 01111111 10000000 00000000 00000000
	// 0x00008000 => 00000000 00000000 10000000 00000000
	return ((((f & 0x7f800000) - 0x38000000) >> 18) & 0x03E0) |  // exponential
	       ((f >> 18) & 0x001f);                                 // Mantissa
}

uint32_t packF2x111x10(Vec3f const &v)
{
	return ((floatTo11bit(v.ui[0]) & ((1 << 11) - 1)) << 0) |
	       ((floatTo11bit(v.ui[1]) & ((1 << 11) - 1)) << 11) |
	       ((floatTo10bit(v.ui[2]) & ((1 << 10) - 1)) << 22);
}
#endif

const uint32_t c_vertex_count = 6;

const std::vector<vec2sf_t> c_vertices_f32 = {
        {+0.0, +0.0},
        {+1.0, +0.0},
        {+1.0, +1.0},
        {+1.0, +1.0},
        {+0.0, +1.0},
        {+0.0, +0.0}
};

const std::vector<vec2ub_t> c_vertices_i8 = {
        {0, 0},
        {1, 0},
        {1, 1},
        {1, 1},
        {0, 1},
        {0, 0}
};

const std::vector<vec2i_t> c_vertices_i32 = {
        {0, 0},
        {1, 0},
        {1, 1},
        {1, 1},
        {0, 1},
        {0, 0}
};

const std::vector<uint16_t> c_vertices_f16 = {
        uint16_t(HalfFloat(0.0F)), uint16_t(HalfFloat(0.0F)), uint16_t(HalfFloat(1.0F)),
        uint16_t(HalfFloat(0.0F)), uint16_t(HalfFloat(1.0F)), uint16_t(HalfFloat(1.0F)),
        uint16_t(HalfFloat(1.0F)), uint16_t(HalfFloat(1.0F)), uint16_t(HalfFloat(0.0F)),
        uint16_t(HalfFloat(1.0F)), uint16_t(HalfFloat(0.0F)), uint16_t(HalfFloat(0.0F)),
};

const std::vector<uint32_t> c_vertices_rg_b10_a2
        = {packSnorm3x10_1x2(Vec4f(0.0, 0.0, 0.0, +1.0)), packSnorm3x10_1x2(Vec4f(1.0, 0.0, 0.0, +1.0)),
           packSnorm3x10_1x2(Vec4f(1.0, 1.0, 0.0, +1.0)), packSnorm3x10_1x2(Vec4f(1.0, 1.0, 0.0, +1.0)),
           packSnorm3x10_1x2(Vec4f(0.0, 1.0, 0.0, +1.0)), packSnorm3x10_1x2(Vec4f(0.0, 0.0, 0.0, +1.0))};

/* clang-format off */
const char *c_vert = {
    "#version 330                                                                           \n"
    "uniform mat4 u_worldscreen;                                                            \n"
    "in vec4 a_position;                                                                    \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * a_position;                                          \n"
    "}                                                                                      \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0, 0.5, 0.0, 1.0);                                                  \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	std::vector<Rectf> m_viewports;
	std::vector<uint32_t> m_vtxFormats;
	uint32_t m_arrayIds[6];  // 0:F32 1:I8 2:I32 3:RGB10A2 4:F16 5:RG11B10F
	SpuShader m_shader;
	Mat4f u_worldscreen;

	App(const char *name) : BaseApp(name, true, c_white) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		m_viewports = makeViewports(3, 2);
		// program
		{
			Attrs shader_attrs = {
			        {"vert", c_vert},
			        {"frag", c_frag},
			};
			Attrs unif_attrs = {
			        {"u_worldscreen", &u_worldscreen}, // UBO
			};
			loadShader(m_shader, shader_attrs, unif_attrs);
		}
		// array
		{
			Attrs model_attrs = {
			        {"shader_id",    m_shader.id() },
			        {"nelem",        c_vertex_count},
			        {"format",       0             }, // filled later
			        {"normalize",    0             }, // filled later
			        {"a.a_position", 0             }, // filled later
			        {"data",         nullptr       }, // filled later
			};

			memset(m_arrayIds, 0, sizeof(m_arrayIds));

			model_attrs.replace("format", GL_FLOAT);
			model_attrs.replace("normalize", 0);
			model_attrs.replace("data", c_vertices_f32.data());
			model_attrs.replace("a.a_position", 2);
			m_arrayIds[0] = spu_array_new(model_attrs);

			model_attrs.replace("format", GL_BYTE);
			model_attrs.replace("normalize", 0);
			model_attrs.replace("data", c_vertices_i8.data());
			model_attrs.replace("a.a_position", 2);
			m_arrayIds[1] = spu_array_new(model_attrs);

			model_attrs.replace("format", GL_INT);
			model_attrs.replace("normalize", 0);
			model_attrs.replace("data", c_vertices_i32.data());
			model_attrs.replace("a.a_position", 2);
			m_arrayIds[2] = spu_array_new(model_attrs);

			model_attrs.replace("format", GL_INT_2_10_10_10_REV);
			model_attrs.replace("normalize", 1);
			model_attrs.replace("data", c_vertices_rg_b10_a2.data());
			model_attrs.replace("a.a_position", 4);
			m_arrayIds[3] = spu_array_new(model_attrs);

			model_attrs.replace("format", GL_HALF_FLOAT);
			model_attrs.replace("normalize", 0);
			model_attrs.replace("data", c_vertices_f16.data());
			model_attrs.replace("a.a_position", 2);
			m_arrayIds[4] = spu_array_new(model_attrs);

#if 0
			// TODO: byte per element (3/4) is not integer!!
			model_attrs.replace("format", GL_UNSIGNED_INT_10F_11F_11F_REV);
			model_attrs.replace("normalize", 0);
			model_attrs.replace("data", c_verticesRG11FB10F);
			m_arrayIds[5] = spu_array_new(model_attrs);
#endif
		}
	}

	void render() override
	{
		for (auto i = 0; i < 6; i++) {
			spu_frame_set(-1, "viewport0", m_viewports[i]);

			getCamera().setViewscreen(viewport(0), 45, 0.1, 100.0);
			u_worldscreen = getCamera().worldscreen() * Mat4f();
			m_shader.use();

			// reject unsupported array
			if (m_arrayIds[i] != 0) {
				spu_array_draw(m_arrayIds[i], GL_TRIANGLES);
			}
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_440_buffer_type");
}  // namespace
}  // namespace spu
