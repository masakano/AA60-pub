//
// SpuPage :
//
#include <spu++/spu_site.h>

namespace spu {

template<class base_t>
std::map<const std::string, typename ObjectRegistry<base_t>::creator_t> &ObjectRegistry<base_t>::ms_creators()
{
	static std::map<const std::string, typename ObjectRegistry<base_t>::creator_t> v;
	return v;
}

template class ObjectRegistry<SpuPage>;

SpuPage::SpuPage(const char *name, bool depth_test, const Vec4f &bgcolor0, float bgdepth, int32_t stencil)
        : m_name(name), m_bgcolor0(bgcolor0), m_bgdepth(bgdepth), m_bgstencil(stencil)
{
	m_renderstate.flags.depth_test = depth_test;
}

SpuPage::~SpuPage() { delete m_gesture; }

void SpuPage::init(const Attrs &)
{
	m_seconds.reset();
	m_gesture = new SpuGesture(nullptr);

	spu_frame_get(-1, "viewport0", m_viewport0.f);  //

	Attrs frame_attrs{
	        {"bgcolor0",  m_bgcolor0 },
	        {"bgdepth",   m_bgdepth  },
	        {"bgstencil", m_bgstencil},
	};
	spu_frame_set(-1, frame_attrs);
}

void SpuPage::begin()
{
	m_renderstate.use();
	spu_frame_clear(-1);  // must be here

	m_seconds.update();
	m_gesture->update();

	const uint32_t c_escape_code = 0x1b;
	if (m_gesture->keyPressed(c_escape_code)) {
		m_isAlive = false;
	}
}

void SpuPage::end()
{
	spu_frame_set(-1, "viewport0", m_viewport0);
	spu_frame_set(-1, "scissor0", m_viewport0);
}

template<class manager_t> void runSpuSite(const Attrs &attrs)
{
	auto backoffice = new SpuBackoffice(attrs, SpuRenderstate::startup, SpuRenderstate::shutdown);
	auto app_attrs = backoffice->getAttrs();
	auto manager = new manager_t(app_attrs);

	for (auto &app_name: manager_t::selectedAppNames(attrs)) {
		aux_printf(" ------ [%s] ------\n", app_name.c_str());
		auto alives_list = spu_graphics_get_alives_list();

		auto app = manager->create(app_name.c_str(), app_attrs);
		manager->exec(app);
		delete app;

		attrs.traceCheck(4);
		spu_graphics_prune(alives_list);
	}

	delete manager;
	delete backoffice;
}

void SpuSite::main(const Attrs &attrs) { runSpuSite<SpuSite>(attrs); }
}  // namespace spu
