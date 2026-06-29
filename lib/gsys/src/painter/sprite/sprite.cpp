//
// Sprite :
//
#include <gsys/canvas.h>
#include <gsys/painter/sprite.h>

namespace spu::gs_painter {

void Sprite::init(const Attrs &attrs)
{
	setIsKeepInHost(true);

	// parent & decorator
	{
		const char *path = "painter/sprite/radiance.us";
		Attrs def_attrs = {
		        {"path",         path},
                        {"divisor",      1   },
                        {"a.a_position", 4   },
		        {"a.a_color",    4   },
                        {"a.a_texcoord", 4   },
                        {"a.a_span",     4   },
		};
		GsPainter::init(def_attrs + attrs);
	}

	// shader
	{
		Attrs unif_attrs = {
		        {"u_nodeview",   &u_nodeview  },
                        {"u_viewscreen", &u_viewscreen},
		        {"u_min_alpha",  &u_min_alpha },
                        {"u_albedo",     &u_albedo    },
		        {"u_albedomap",  &u_albedomap },
		};
		addUniforms(unif_attrs);
	}
}

void Sprite::doUse(uint32_t id)
{
	assert(instanceCount() == 1);  // not instancing
	auto *current = GsCanvas::getCurrent();
	auto &drawcall = getDrawcalls().at(id);
	auto &coms = drawcall.coms;
	auto vertices = getVerticesView();
	auto vertices_count = uint32_t(vertices.size());

	for (auto &com: coms) {
		if (com.first == 0 && com.count == 0) com.count = vertices_count;

		com.mode = GL_TRIANGLE_STRIP;
		com.target = GL_ARRAY_BUFFER;
		com.base_instance = std::min(com.first, vertices_count);
		com.instance_count = std::min(com.count, vertices_count - com.first);
		com.first = 0;
		com.count = 4;
	}

	auto nodeworld = *static_cast<const Mat4f *>(instancePtr());
	u_nodeview = current->worldview(0) * nodeworld;
	u_viewscreen = current->viewscreen();
	u_albedo = drawcall.ub_material.albedo;
	u_min_alpha = drawcall.ub_material.min_alpha;
	u_albedomap = drawcall.albedomap.id();

	GsPainter::doUse(id);
}

void Sprite::update()
{
	auto vertices = getVerticesView();
	if (!vertices.empty()) {
		auto &range = getRange();
		range.invalidate();
		for (auto &vertex: vertices) {
			auto &p = vertex.p;
			auto dx0 = vertex.s.x0;
			auto dy0 = vertex.s.y0;
			auto dx1 = vertex.s.x1;
			auto dy1 = vertex.s.y1;
			range.expand(Vec3f(p.x + dx0, p.y + dy0, p.z));
			range.expand(Vec3f(p.x + dx1, p.y + dy1, p.z));
		}
		SpuArray::send(vertices.data(), vertices.size(), 0);
	}
}
}  // namespace spu::gs_painter
