//
// App :
//
#include "base_app.h"
namespace spu::gsquads {
class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void menu() override;
	void render() override;
	void init(const Attrs &attrs) override;
	SpuShader m_linesadjacencyShader;
	SpuShader m_fansShader;
	SpuArray m_array;
	Mat4f u_modelscreen;
	int32_t u_vid_offset = 0;
	int32_t m_mode = 0;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	m_array.init({
	        {"nelem", 4}
        });

	// shader
	{
		Attrs unif_attrs = {
		        {"u_modelscreen", &u_modelscreen},
		        {"u_vid_offset",  &u_vid_offset },
		};
		loadShader(m_fansShader, "gsquads/quadsasfans.us", Attrs(), unif_attrs);
		loadShader(m_linesadjacencyShader, "gsquads/quadsaslinesadj.us", Attrs(), unif_attrs);
	}
	spu_frame_set(-1, "bgcolor0", c_blue);
}

void App::menu()
{
	ImGui::Combo("draw mode", &m_mode, "mode1\0mode2\0\0");
	ImGui::Combo("vid offset", &u_vid_offset, "offset0\0offset1\0offset2\0offset3\0\0");
}

void App::render()
{
	auto t = getSeconds().current();

	auto modelview = c_unit.rot("XZ", -t * 30.0, -t * 5.0).trans({0.0, 0.0, -2.0});

	sb6::Composition composition;
	composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
	auto viewscreen = composition.viewscreen();

	u_modelscreen = viewscreen * modelview;
	switch (m_mode) {
	case 0:
		m_fansShader.use();
		m_array.draw(GL_TRIANGLE_FAN, 0, 4);
		break;
	case 1:
		m_linesadjacencyShader.use();
		m_array.draw(GL_LINES_ADJACENCY, 0, 4);
		break;
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gsquads");
}  // namespace spu::gsquads
