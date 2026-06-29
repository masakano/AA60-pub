//
// PackNode :
//
#pragma once
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_node::gui {

template<class T> class PackNode {
public:
	struct Rect {
		float ox = 0, oy = 0, sx = 0, sy = 0;
		T *ptr = nullptr;
		Rect() = default;
		Rect(float ox, float oy, float sx, float sy) : ox(ox), oy(oy), sx(sx), sy(sy) {}
		Rect(const Rectf &r) : ox(r.ox), oy(r.oy), sx(r.sx), sy(r.sy) {}
	};

	Rect m_rect;
	std::vector<PackNode *> m_children;

	PackNode *insert(const Rect &rect, std::function<void(const Rect &)> modifier)
	{
		if (m_children.empty()) {
			// already used
			if (m_rect.ptr) {
				return nullptr;
			}
			// too small to contain
			if (!isContain(rect)) {
				return nullptr;
			}

			if (isFitPerfect(rect)) {
				m_rect.ptr = rect.ptr;
				modifier(m_rect);
				return this;
			}

			const auto sx = rect.sx;
			const auto sy = rect.sy;
			const auto dx = m_rect.sx - sx;
			const auto dy = m_rect.sy - sy;

			// divide into two piceses
			m_children.push_back(new PackNode<Tweakbar>());  // #0
			m_children.push_back(new PackNode<Tweakbar>());  // #1

			if (dx > dy) {
				m_children[0]->m_rect = Rect(m_rect.ox, m_rect.oy, sx, m_rect.sy);
				m_children[1]->m_rect = Rect(m_rect.ox + sx, m_rect.oy, dx, m_rect.sy);
			}
			else {
				m_children[0]->m_rect = Rect(m_rect.ox, m_rect.oy, m_rect.sx, sy);
				m_children[1]->m_rect = Rect(m_rect.ox, m_rect.oy + sy, m_rect.sx, dy);
			}
		}
		for (auto &child: m_children) {
			auto new_ptr = child->insert(rect, modifier);
			if (new_ptr) return new_ptr;
		}
		return nullptr;
	}
	void dispose()
	{
		for (auto &child: m_children) {
			child->dispose();
			delete child;
			child = nullptr;
		}
	}
	bool isContain(const Rect &rect) const { return rect.sx <= m_rect.sx && rect.sy <= m_rect.sy; }
	bool isFitPerfect(const Rect &rect) const { return rect.sx == m_rect.sx && rect.sy == m_rect.sy; }
};
}  // namespace spu::gs_node::gui
