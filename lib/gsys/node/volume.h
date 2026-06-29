//
// Volume :
//
#pragma once

#include <gsys/node/gui/tweakbar.h>
#include <gsys/node.h>

namespace spu::gs_node {
namespace volume {
class ParticlesToVolume;
class Shadowcast;
}  // namespace volume

class Volume : public GsNode {
public:
	explicit Volume(const char *name = nullptr) : GsNode(name) {}
	explicit Volume(const Attrs &attrs) : Volume() { init(attrs); }
	~Volume();

	void init(const Attrs &attrs) override;
	void update() override;
	void startInspector() override;

	void sendParticles(const std::vector<Vec3f> &particles);
	void sendDensity(const void *density, uint32_t format = GL_R32F);
	Vec4i size() const { return m_size; }

protected:
	Vec4i m_size = {64, 64, 64, 1};

	volume::ParticlesToVolume *m_particlesToVolume = nullptr;
	volume::Shadowcast *m_shadowcast = nullptr;

	SpuTexture m_volumeTexture;
	SpuTexture m_clutTexture;
	SpuTexture m_occlusionTexture;

	gui::Tweakbar m_volumeView;
	gui::Tweakbar m_occlusionView;
};
}  // namespace spu::gs_node
