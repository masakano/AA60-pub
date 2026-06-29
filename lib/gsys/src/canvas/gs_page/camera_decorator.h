//
// CameraDecorator :
//
#pragma once
#include <gsys/canvas/gs_page.h>
#include <gsys/node/camera.h>

namespace spu::gs_canvas::gs_page {

class CameraDecorator : public GsPage::IDecorator {
public:
	CameraDecorator(GsPage &page) : m_page(page)
	{
		Attrs camera_attrs = {
		        {"name",    "[default camera]"    },
		        {"canvas",  &m_page            },
		        {"gesture", m_page.getGesture()},
		};
		m_page.replaceCamera(new gs_node::Camera(camera_attrs));
	}

	void begin() override { m_page.getCamera()->update(); }

	void end() override {}

	void set(const Attrs &attrs) override
	{
		auto camera_attrs = attrs.select("camera.");
		auto *camera = m_page.getCamera();
		camera->sync(false); 
		//if (!camera_attrs.empty()) { // need FIX
		{
			camera->set(camera_attrs);
		}
		camera->sync(false); 
	}

private:
	GsPage &m_page;
};
}  // namespace spu::gs_canvas::gs_page
