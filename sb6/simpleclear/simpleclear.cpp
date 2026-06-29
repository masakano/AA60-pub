//
// App :
//
#include "base_app.h"
namespace spu::simpleclear {
class App : public BaseApp {
public:
	void render() override;
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	const Vec4f red = {1.0, 0.0, 0.0, 1.0};
	spu_frame_set(-1, "bgcolor0", red);
}

void App::render() { BaseApp::clear(); }
static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("simpleclear");
}  // namespace spu::simpleclear
