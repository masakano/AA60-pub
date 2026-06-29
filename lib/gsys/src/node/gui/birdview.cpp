//
// Birdview :
//
#include <gsys/node/gui/birdview.h>

#include <utility>

namespace spu::gs_node::gui {

void Birdview::init(const Attrs &attrs)
{
	Base::init(attrs);
	auto curr = getGesture()->curr();

	auto *name = attrs.get<const char *>("name", nullptr);
	auto *parent = attrs.get<Base *>("parent", nullptr);
	auto width = float(getColumns(name, parent));
	auto height = width * float(curr.winsize[1]) / float(curr.winsize[0]);

	m_canvas.init(Attrs());

	auto camera_name = peeloff_string(name);
	Attrs camera_attrs = {
	        {"name",    camera_name },
	        {"canvas",  &m_canvas   },
	        {"gesture", getGesture()},
	};
	m_camera.init(camera_attrs + attrs.select("camera."));

	auto &range = getARange();

	range.p1.x = std::max(range.p1.x, range.p0.x + width);
	range.p0.y = std::min(range.p0.y, range.p1.y - height);  // extend to downside

	range.p1.y += 0.5;  // for tilt
	                    // range.p0.x += 0.5; // for side bar
}

void Birdview::update()
{
	if (isFocus()) {
		m_camera.update();
	}
}

void Birdview::doRender()
{
	if (getWindowModifier().rate() > 0) {
		// auto &c = m_canvas.getComposition();
		m_canvas.getViewports().at(0) = Rectf(getNodefrag() * getARange());
		m_canvas.getScissors().at(0) = Rectf(scissorRange());
		m_canvas.begin();
		m_canvas.clear();
		coreDraw();
		m_canvas.end();
	}
	Base::doRender();
}
}  // namespace spu::gs_node::gui
