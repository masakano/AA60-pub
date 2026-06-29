//
// GsObject :
//
#include <gsys/canvas/gs_site.h>
#include <gsys/drawcall.h>

namespace spu {

template<class base_t>
std::map<const std::string, typename ObjectRegistry<base_t>::creator_t> &ObjectRegistry<base_t>::ms_creators()
{
	static std::map<const std::string, typename ObjectRegistry<base_t>::creator_t> v;
	return v;
}

template class ObjectRegistry<GsCanvas>;
}  // namespace spu

namespace spu::gs_canvas {
void GsSite::main(const Attrs &attrs)
{
	auto backoffice = new SpuBackoffice(attrs, GsObject::startup, GsObject::shutdown);
	auto app_attrs = backoffice->getAttrs();
	auto manager = new gs_canvas::GsSite(app_attrs);

	for (auto &app_name: selectedAppNames(attrs)) {
		aux_printf(" ------ [%s] ------\n", app_name.c_str());
		GsDrawcall::resetDefault();
		auto alives_list = spu_graphics_get_alives_list();
		{
			auto alive_objects = GsObject::aliveObjects();  // copy
			auto app = manager->create(app_name.c_str(), app_attrs);
			manager->exec(app);
			delete app;
			app_attrs.traceCheck(4);
			GsObject::pruneObjects(alive_objects);
		}
		spu_graphics_prune(alives_list);
	}
	delete manager;
	delete backoffice;
}
}  // namespace spu::gs_canvas
