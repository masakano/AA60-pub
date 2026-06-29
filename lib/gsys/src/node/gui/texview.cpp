//
// Texview :
//
#include <gsys/canvas/copy.h>
#include <gsys/node/gui/texview.h>

namespace spu::gs_node::gui {
namespace {
float round(float x, float y) { return floor((x + y - 1) / y) * y; }
}  // namespace

void Texview::init(const Attrs &attrs)
{
	Base::init(attrs);

	auto *name = attrs.get<const char *>("name", nullptr);
	auto *parent = attrs.get<Base *>("parent", nullptr);
	auto columns = getColumns(name, parent);

	auto *desc = attrs.get<Desc *>("desc", nullptr);
	assert(desc);
	m_desc = *desc;

	auto target = 0u;
	auto width = 0;
	auto height = 0;
	for (auto &texture_id: m_desc.ids) {
		auto new_target = texture_id >> 16;
		auto new_width = 0;
		auto new_height = 0;

		spu_inventory_sync(texture_id, false);
		spu_texture_get(texture_id, "width", &new_width);
		spu_texture_get(texture_id, "height", &new_height);
		if (width == 0) {
			width = new_width;
			height = new_height;
			target = new_target;
		}
	}

	// range
	{
		if (target == GL_TEXTURE_CUBE_MAP || target == GL_TEXTURE_CUBE_MAP_ARRAY) {
			if (m_desc.column == 0) m_desc.column = 1;
			m_aspect = 1.0 / 6.0;
		}
		else if (width >= height) {
			if (m_desc.column == 0) m_desc.column = 2;
			m_aspect = float(height) / float(width);
		}
		else {
			if (m_desc.column == 0) m_desc.column = 4;
			m_aspect = float(height) / float(width);
		}
		auto &range = getARange();
		auto nx = m_desc.column;
		auto ny = int32_t(m_desc.ids.size() + nx - 1) / nx;
		auto sx = float(columns) / nx;
		auto sy = sx * m_aspect;
		range.p0.y = range.p1.y - round(sy * ny, c_charheight);  // span + title
	}
	// canvas
	{
		Attrs attrs = {
		        {"target",            target},
		        {"def_use_bias_gain", 1     },
		};
		m_canvas.init(attrs);
	}
}

void Texview::update()
{
	auto cursor = getCursor(getGesture()->curr().cursor);
	if (getARange().inside(cursor)) {
		const auto c_rate = 1.5f;
		auto wheel = getGesture()->wheel();
		if (wheel > 0) m_canvas.ub_connect.sources[0].gain *= c_rate;
		if (wheel < 0) m_canvas.ub_connect.sources[0].gain /= c_rate;
		Base::update();
	}
}

void Texview::doRender()
{
	if (getWindowModifier().rate() > 0) {
		const auto glue = 2.0f;
		auto master_viewport = Rectf(getNodefrag() * getARange());
		auto scissor = Rectf(scissorRange());

		auto nx = m_desc.column;

		auto sx = master_viewport.sx / nx;
		auto sy = sx * m_aspect;

		auto ox = master_viewport.ox;
		auto oy = master_viewport.oy + master_viewport.sy - sy;

		for (auto i = 0u; i < m_desc.ids.size(); i++) {
			Rectf viewport = {
			        ox + glue,
			        oy + glue,
			        sx - glue,
			        sy - glue,
			};
			m_canvas.u_color0 = m_desc.ids[i];
			m_canvas.ub_connect.sources[0].layer = vector_at(m_desc.layers, i);
			m_canvas.ub_connect.sources[0].level = vector_at(m_desc.levels, i);

			m_canvas.getViewports().at(0) = viewport;
			m_canvas.getScissors().at(0) = scissor;

			m_canvas.begin();
			m_canvas.render();
			m_canvas.end();

			ox += sx;
			if (ox >= master_viewport.sx + master_viewport.ox) {
				ox = master_viewport.ox;
				oy -= sy;
			}
		}
	}
	Base::doRender();
}
}  // namespace spu::gs_node::gui
