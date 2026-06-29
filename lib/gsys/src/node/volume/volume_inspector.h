//
// VolumeInspector :
//
#pragma once
#include <gsys/node/volume.h>
#include <gsys/node/gui/tweakbar.h>

namespace spu::gs_node {
class VolumeInspector : public gui::Tweakbar {
public:
	using base_t = gui::Tweakbar;

	float m_volumeScale = 1.0;
	float m_volumeThreshold = 0.5;
	Volume *m_volume = nullptr;

	VolumeInspector(Volume *volume) : m_volume(volume)
	{
		auto &drawcall = m_volume->getPainter()->getADrawcall();
		auto &ub_material = drawcall.ub_material;
		drawcall.flags.blend = true;

		base_t::setName(padstr(m_volume->prettyName(), 28));

		base_t::addStdSlider("alpha", 0.0, 1.0, &ub_material.albedo.a, 2.0);
		base_t::addStdSlider("metallic", 0.0, 1.0, &ub_material.metallic);
		base_t::addStdSlider("roughness", 0.0, 1.0, &ub_material.roughness);
		base_t::addStdSlider("point size", 1.0 / 128.0, 1.0 / 2.0, &ub_material.point_size, 2.0);
		base_t::addStdSlider("scale", 0.001, 2.0, &m_volumeScale, 2.0);
		base_t::addStdSlider("threshold", 0.001, 2.0, &m_volumeThreshold, 2.0);

		base_t::bake();
	}

	void update() override
	{
		Attrs set_attrs = {
		        {"painter.shader.u_volume_scale",     m_volumeScale    },
		        {"painter.shader.u_volume_threshold", m_volumeThreshold},
		};
		m_volume->set(set_attrs);
		base_t::update();
	}
};
}  // namespace spu::gs_node
