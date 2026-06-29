//
// GsPage :
//
#include "timer_decorator.h"
#include "gesture_decorator.h"
#include "camera_decorator.h"

#include <gsys/canvas/gs_site.h>
#include <gsys/canvas/copy.h>
#include <gsys/node/gui/tweakbar.h>
#include <ranges>

namespace spu::gs_canvas {

namespace {
class DefaultPostproc : public GsPage::IPostproc {
public:
	Copy m_capture;

	DefaultPostproc() { m_capture.init(Attrs()); }

	void postproc(GsPage *page) override
	{
		m_capture.getViewports().at(0) = page->viewport(0);
		m_capture.u_color0 = page->getBuffer("color0").id();
		m_capture.render();
	}
};
class DefaultTweakbar : public GsPage::ITweakbar {
public:
	void draw(GsPage *) override { gs_node::gui::Tweakbar::sortAndDrawAll(); }
};
}  // namespace

GsPage::~GsPage()
{
	disposeDecorators();
	delete m_tweakbar;
	delete m_camera;
	delete m_postproc;
	delete m_gesture;
	GsDrawcall::resetDefault();
}

void GsPage::init(const Attrs &attrs)
{
	m_backoffice = attrs.get<SpuBackoffice *>("backoffice", nullptr);
	if (m_backoffice == nullptr) {
		aux_message(0, "no backoffice found\n");
	}

	setProperty(e_show_tweakbar, true);
	initMultisample(attrs);

	addDecorator(new gs_page::TimerDecorator(*this));
	addDecorator(new gs_page::GestureDecorator(*this));
	addDecorator(new gs_page::CameraDecorator(*this));

	replacePostproc(new DefaultPostproc());
	replaceTweakbar(new DefaultTweakbar());

	set(attrs);
}

void GsPage::set(const Attrs &attrs)
{
	attrs.peek("camera", "deprecated. use 'replaceCamera()' instead\n");
	GsCanvas::set(attrs);
	setDecorators(attrs);
}

void GsPage::begin()
{
	GsCanvas::begin();
	clear();
	beginDecorators();
	getRenderstate().use();
}

void GsPage::end()
{
	endDecorators();
	GsCanvas::end();
	m_postproc->postproc(this);

	if (getProperty(e_show_tweakbar)) {
		m_tweakbar->draw(this);
	}
}

void GsPage::disposeDecorators()
{
	for (auto &decorator: m_decorators) {
		delete decorator;
	}
	m_decorators.clear();
}

void GsPage::beginDecorators()
{
	for (auto &decorator: m_decorators) {
		decorator->begin();
	}
}

void GsPage::endDecorators()
{
	for (auto &decorator: m_decorators | std::views::reverse) {
		decorator->end();
	}
}

void GsPage::setDecorators(const Attrs &attrs)
{
	for (auto &decorator: m_decorators) {
		decorator->set(attrs);
	}
}

void GsPage::initMultisample(const Attrs &attrs)
{
	m_multisample = attrs.get("multisample", m_multisample);

	if (m_multisample == 0) {
		Attrs def_attrs = {
		        {"color0.target",             GL_TEXTURE_2D       },
		        {"color0.iformat",            GL_RGBA32F          },
		        {"color0.wrap_s",             GL_CLAMP_TO_EDGE    },
		        {"color0.wrap_t",             GL_CLAMP_TO_EDGE    },
		        {"color0.min_filter",         GL_LINEAR           },
		        {"color0.mag_filter",         GL_LINEAR           },
		        {"color0.max_level",          0                   },
		        {"color0.auto_mipmap",        0                   },
		        {"depth_stencil.target",      GL_TEXTURE_2D       },
		        {"depth_stencil.iformat",     GL_DEPTH32F_STENCIL8},
		        {"depth_stencil.wrap_s",      GL_CLAMP_TO_EDGE    },
		        {"depth_stencil.wrap_t",      GL_CLAMP_TO_EDGE    },
		        {"depth_stencil.max_level",   0                   },
		        {"depth_stencil.auto_mipmap", 0                   },
		};
		GsCanvas::init(def_attrs + attrs);
	}
	else {
		Attrs def_attrs = {
		        {"color0.target",              GL_TEXTURE_2D_MULTISAMPLE},
		        {"color0.iformat",             GL_RGBA32F               },
		        {"color0.multisample",         m_multisample            },
		        {"color0.resolve.target",      GL_TEXTURE_2D            },
		        {"color0.resolve.iformat",     GL_RGBA32F               },
		        {"color0.resolve.max_level",   0                        },
		        {"color0.resolve.auto_mipmap", 0                        },
		        {"color0.resolve.min_filter",  GL_NEAREST               },
		        {"color0.resolve.mag_filter",  GL_NEAREST               },

		        {"depth.target",               GL_TEXTURE_2D_MULTISAMPLE},
		        {"depth.iformat",              GL_DEPTH_COMPONENT32F    },
		        {"depth.multisample",          m_multisample            },
		        {"depth.resolve.target",       GL_TEXTURE_2D            },
		        {"depth.resolve.iformat",      GL_R32F                  },
		        {"depth.resolve.max_level",    0                        },
		        {"depth.resolve.auto_mipmap",  0                        },
		        {"depth.resolve.min_filter",   GL_NEAREST               },
		        {"depth.resolve.mag_filter",   GL_NEAREST               },
		};
		GsCanvas::init(def_attrs + attrs);
	}
}

const SpuTexture &GsPage::getBuffer(const hash32_t &slot) const
{
	if (m_multisample > 0 && slot == "color0"_h32) {
		return GsCanvas::getBuffer("color0.resolve");
	}
	if (m_multisample > 0 && slot == "depth"_h32) {
		return GsCanvas::getBuffer("depth.resolve");
	}
	if (m_multisample == 0 && slot == "depth"_h32) {
		return GsCanvas::getBuffer("depth_stencil");
	}
	return GsCanvas::getBuffer(slot);
}

SpuTexture &GsPage::getBuffer(const hash32_t &slot)
{
	return const_cast<SpuTexture &>(const_cast<const GsPage *>(this)->getBuffer(slot));
}

void GsPage::setGlobalSrgb(bool is_srgb)
{
	for (auto &object: GsObject::aliveObjects()) {
		auto canvas = dynamic_cast<GsCanvas *>(object);
		if (canvas) {
			canvas->getRenderstate().flags.srgb_encode = is_srgb;
			if (canvas->id()) {
				auto &buffer = canvas->getBuffer("color0");
				if (buffer.id()) {
					buffer.set("srgb_decode", is_srgb);
				}
			}
		}
	}
	if (m_backoffice) {
		m_backoffice->getRenderstate().flags.srgb_encode = is_srgb;
	}
}

GsPage::IPostproc *GsPage::movePostproc()
{
	auto *postproc = m_postproc;
	m_postproc = nullptr;
	return postproc;
}

GsPage::ITweakbar *GsPage::moveTweakbar()
{
	auto *tweakbar = m_tweakbar;
	m_tweakbar = nullptr;
	return tweakbar;
}

}  // namespace spu::gs_canvas
