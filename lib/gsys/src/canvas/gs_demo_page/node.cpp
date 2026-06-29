//
// GsDemoPage :
//
#include <gsys/canvas/gs_demo_page.h>
#include <gsys/util/node_mixer.h>
#include <gsys/painter/stdout.h>
#include <gsys/painter/dummy.h>
#include <gsys/node/manifold_2d.h>
#include <gsys/node/wavefront_obj.h>

namespace spu {

void gs_canvas::GsDemoPage::setSyncCallback(const Attrs &attrs, GsNode *node)
{
	struct CallbackArg {
		gs_canvas::GsDemoPage *app;
		GsNode *node;
		Attrs drawcall_attrs;
		bool use_inspector;
		bool use_guizmo;
	};

	auto sync_callback = [](void *arg) {
		auto callback_arg = (CallbackArg *)arg;
		auto *app = callback_arg->app;
		auto *node = callback_arg->node;
		auto use_inspector = callback_arg->use_inspector;
		auto use_guizmo = callback_arg->use_guizmo;
		auto &drawcall_attrs = callback_arg->drawcall_attrs;

		if (use_inspector) {
			node->startInspector();
		}
		if (use_guizmo) {
			app->getGuizmo()->setRelatedNodes({node});
		}

		auto *painter = node->getPainter();
		for (auto &drawcall: painter->getDrawcalls()) {
			drawcall.set(drawcall_attrs);
		}
		// painter->sortDrawcalls();
		GsDrawcall::sort(painter->getDrawcalls());
		delete callback_arg;
		return true;
	};
	auto *callback_arg = new CallbackArg();
	callback_arg->app = this;
	callback_arg->node = node;

	auto *painter = node->getPainter();
	callback_arg->drawcall_attrs = attrs.select("painter.drawcall.");
	callback_arg->drawcall_attrs.preserve();
	callback_arg->use_inspector = attrs.get("use_inspector", true);
	callback_arg->use_guizmo = attrs.get("use_guizmo", true);
	painter->syncCallback(sync_callback, callback_arg);
}

GsNode *gs_canvas::GsDemoPage::createManifold2D(const Attrs &attrs, GsPainter *painter)
{
	auto mesh_grid = Vec4i(c_ndiv, c_ndiv, 1, 1);
	auto mapnode = Mat4f().rot("x", pi() / 2).scale(2.0);
	auto maptexc = Mat4f().scale({0.5, 0.5, 1.0}).trans({0.5, 0.5, 0.0});
	auto min_notch = attrs.get("min_notch", c_min_notch);

	Attrs def_attrs = {
	        {"mesh_grid", mesh_grid},
	        {"mapnode",   &mapnode },
	        {"maptexc",   &maptexc },
	};
	auto node = new gs_node::Manifold2D(def_attrs + attrs);
	node->setProperty("lod", 1);
	node->replacePainter(painter);
	node->setMinNotch(min_notch);
	setSyncCallback(attrs, node);
	return node;
}

GsNode *gs_canvas::GsDemoPage::createWavefrontObj(const Attrs &attrs, GsPainter *painter)
{
	auto node = new gs_node::WavefrontObj(attrs);
	node->replacePainter(painter);
	setSyncCallback(attrs, node);
	return node;
}

GsNode *gs_canvas::GsDemoPage::createDividedWavefrontObj(const Attrs &attrs, GsPainter *painter)
{
	auto node = createWavefrontObj(attrs, painter);
	auto divide_grid = attrs.get("divide_grid", vec4i_t(1, 1, 1, 1));
	GsNodeMixer mixer;
	mixer.init(node);
	mixer.divide(divide_grid);
	return node;
}

GsNode *gs_canvas::GsDemoPage::createLodWavefrontObj(const Attrs &attrs, GsPainter *painter)
{
	auto node = createWavefrontObj(attrs, painter);

	// need organzie
	std::vector<int32_t> remesh_grids = {
	        64, 32, 16, 8,
	        // 16, 8, 4, // debug NEED PARAMETERIZE
	};
	node->setProperty("lod", 1);

	GsNodeMixer mixer;
	mixer.init(node);  // implies sync

	auto sub_node_attrs = attrs;
	sub_node_attrs.emplace_back("remesh.grid", remesh_grids[0]);  // place holder

	for (auto &remesh_grid: remesh_grids) {
		sub_node_attrs.replace("remesh.grid", remesh_grid);
		gs_node::WavefrontObj sub_node(sub_node_attrs);
		auto dummy_painter = new gs_painter::Dummy(Attrs());
		sub_node.replacePainter(dummy_painter);
		sub_node.sync(0);
		mixer.add(&sub_node);
	}
	mixer.send();

	auto span = node->getRanges()[0].span();
	auto scale = std::max({span.x, span.y, span.z});
	auto &notches = node->getNotches();
	auto min_notch = attrs.get("min_notch", c_min_notch);

	notches.clear();
	for (auto &remesh_grid: remesh_grids) {
		notches.push_back(scale / remesh_grid);
	}
	node->setMinNotch(min_notch);
	return node;
}
GsNode *gs_canvas::GsDemoPage::createNode(const Attrs &attrs, GsPainter *painter)
{
	attrs.subpeek({"drawcall."}, "use 'painter.drawcall.*' instead");

	auto lod = attrs.get("prop.lod", 0);
	auto divide_grid = attrs.get("divide_grid", vec4i_t(1, 1, 1, 1));

	auto *shape = attrs.get<const char *>("shape", nullptr);
	auto *path = attrs.get<const char *>("path", nullptr);

	GsNode *node;
	if (shape && *shape) {
		node = createManifold2D(attrs, painter);
	}
	else if (lod) {
		node = createLodWavefrontObj(attrs, painter);
	}
	else if (divide_grid.x > 1 || divide_grid.y > 1 || divide_grid.z > 1) {
		node = createDividedWavefrontObj(attrs, painter);
	}
	else {
		node = createWavefrontObj(attrs, painter);
	}
	if (node->name().empty()) {
		if (shape && *shape) {
			node->setName(shape);
		}
		else if (path && *path) {
			auto stem = std::filesystem::path(path).stem();
			node->setName(stem.string());
		}
	}
	auto prop_attrs = attrs.select("prop.");
	for (auto &key: {"render", "debug_render", "lazy", "lod"}) {
		auto value = prop_attrs.getf<int32_t>(key);
		if (value.hit) {
			node->setProperty(key, value.value);
		}
	}
	return node;
}
}  // namespace spu
