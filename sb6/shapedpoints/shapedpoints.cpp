//
// App :
//
#include <cmath>
#include "base_app.h"
namespace spu::shapedpoints {
enum { e_num_stars = 2000 };
/* clang-format off */
const char *fs_source = {
    "#version 410 core                                                                      \n"
    "layout (location = 0) out vec4 color;                                                  \n"
    "flat in int shape;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = vec4(1.0);                                                                 \n"
    "    vec2 p = gl_PointCoord * 2.0 - vec2(1.0);                                          \n"
    "    if (shape == 0)                                                                    \n"
    "    {                                                                                  \n"
    "        if (dot(p, p) > 1.0)                                                           \n"
    "            discard;                                                                   \n"
    "    }                                                                                  \n"
    "    else if (shape == 1)                                                               \n"
    "    {                                                                                  \n"
    "        if (dot(p, p) > sin(atan(p.y, p.x) * 5.0))                                     \n"
    "            discard;                                                                   \n"
    "    }                                                                                  \n"
    "    else if (shape == 2)                                                               \n"
    "    {                                                                                  \n"
    "        if (abs(0.8 - dot(p, p)) > 0.2)                                                \n"
    "            discard;                                                                   \n"
    "    }                                                                                  \n"
    "    else if (shape == 3)                                                               \n"
    "    {                                                                                  \n"
    "        if (abs(p.x) < abs(p.y))                                                       \n"
    "            discard;                                                                   \n"
    "    }                                                                                  \n"
    "}                                                                                      \n"
};

const char *vs_source = {
    "#version 410 core                                                                      \n"
    "flat out int shape;                                                                    \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    const vec4[4] position = vec4[4](vec4(-0.4, -0.4, 0.5, 1.0),                       \n"
    "                                     vec4( 0.4, -0.4, 0.5, 1.0),                       \n"
    "                                     vec4(-0.4,  0.4, 0.5, 1.0),                       \n"
    "                                     vec4( 0.4,  0.4, 0.5, 1.0));                      \n"
    "    gl_Position = position[gl_VertexID];                                               \n"
    "    shape = gl_VertexID;                                                               \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	SpuShader m_shader;
	SpuArray m_array;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	m_shader.init({
	        {"frag", fs_source},
                {"vert", vs_source}
        });
	m_array.init({
	        {"nelem", 4}
        });
	auto &renderstate = getRenderstate();
	renderstate.flags.point_sprite = true;
	renderstate.point_size = 200.0;
	// renderstate.use();
}

void App::render()
{
	m_shader.use();
	m_array.draw(GL_POINTS, 0, 4);
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("shapedpoints");
}  // namespace spu::shapedpoints
