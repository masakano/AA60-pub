//
// Volume :
//
#include "particles_to_volume.h"
#include "shadowcast.h"
#include "volume_inspector.h"
#include <gsys/node/volume.h>
#include <gsys/painter/raycast.h>
#include <gsys/painter/mcube.h>

#include <gsys/node/gui/tweakbar.h>
#include <gsys/painter/pointset.h>

namespace spu::gs_node {

Volume::~Volume()
{
	delete m_particlesToVolume;
	delete m_shadowcast;
}

void Volume::init(const Attrs &attrs)
{
	m_size = attrs.get<vec4i_t>("grid.size", m_size);

	auto painter_name = attrs.get("painter.name", hash32_t("mcube"));
	auto volume_iformat = attrs.get("volume.iformat", 0);
	auto clut_iformat = attrs.get("clut.iformat", 0);
	auto occlusion_iformat = attrs.get("occlusion.iformat", 0);

	if (volume_iformat) {
		Attrs def_attrs = {
		        {"target",      GL_TEXTURE_3D   },
                        {"min_filter",  GL_LINEAR       },
		        {"mag_filter",  GL_LINEAR       },
                        {"max_level",   0               },
		        {"auto_mipmap", 0               },

		        {"wrap_s",      GL_CLAMP_TO_EDGE},
                        {"wrap_t",      GL_CLAMP_TO_EDGE},
		        {"wrap_r",      GL_CLAMP_TO_EDGE},

		        {"width",       m_size.x        },
                        {"height",      m_size.y        },
		        {"depth",       m_size.z        },
		};
		m_volumeTexture.init(def_attrs + attrs.select("volume."));
		m_particlesToVolume = new volume::ParticlesToVolume(m_volumeTexture.id());

		m_volumeView.setName("volume texture");
		m_volumeView.addStdTexview(m_volumeTexture.id(), 8);
		m_volumeView.bake();
	}
	if (clut_iformat) {
		Attrs def_attrs = {
		        {"target",      GL_TEXTURE_1D   },
		        {"min_filter",  GL_LINEAR       },
		        {"mag_filter",  GL_LINEAR       },
		        {"max_level",   0               },
		        {"auto_mipmap", 0               },
		        {"wrap_s",      GL_CLAMP_TO_EDGE},
		        {"height",      1               },
		};
		m_clutTexture.init(def_attrs + attrs.select("clut."));
	}
	if (occlusion_iformat) {
		Attrs def_attrs = {
		        {"target",      GL_TEXTURE_3D   },
                        {"min_filter",  GL_LINEAR       },
		        {"mag_filter",  GL_LINEAR       },
                        {"max_level",   0               },
		        {"auto_mipmap", 0               },

		        {"wrap_s",      GL_CLAMP_TO_EDGE},
                        {"wrap_t",      GL_CLAMP_TO_EDGE},
		        {"wrap_r",      GL_CLAMP_TO_EDGE},

		        {"width",       m_size.x        },
                        {"height",      m_size.y        },
		        {"depth",       m_size.z        },
		};
		m_occlusionTexture.init(def_attrs + attrs.select("occlusion."));
		m_shadowcast = new volume::Shadowcast(m_size);

		m_occlusionView.setName("occulusion texture");
		m_occlusionView.addStdTexview(m_occlusionTexture.id(), 8);
		m_occlusionView.bake();
	}

	// painter
	{
		std::map<hash32_t, std::function<GsPainter *(const Attrs &)>> creators = {
		        {"mcube",       [](const Attrs &attrs) { return new gs_painter::Mcube(attrs); }   },
		        {"raycast",     [](const Attrs &attrs) { return new gs_painter::Raycast(attrs); } },
		        {"pointset",    [](const Attrs &attrs) { return new gs_painter::Pointset(attrs); }},
		        {"particleset", [](const Attrs &attrs) { return new gs_painter::Pointset(attrs); }},
		};

		Attrs painter_attrs = {
		        {"grid.size",      m_size                               },
		        {"volume.iformat", volume_iformat                       },
		        {"clut.iformat",   clut_iformat                         },
		        {"auto_sort",      painter_name == "particleset" ? 1 : 0},
		        {"particleset",    painter_name == "particleset" ? 1 : 0},
		};
		auto painter = creators.at(painter_name)(painter_attrs);
		replacePainter(painter);

		Attrs shader_attrs = {
		        {"shader.u_volume",    m_volumeTexture.id()   },
		        {"shader.u_clut",      m_clutTexture.id()     },
		        {"shader.u_occlusion", m_occlusionTexture.id()},
		};
		painter->set(shader_attrs);
	}

	// default drawcall
	{
		GsDrawcall &drawcall = getPainter()->getADrawcall();
		drawcall.ub_material.metallic = 0.5;
		drawcall.ub_material.roughness = 0.2;
		drawcall.ub_material.point_size = 0.1;
	}
	getARange() = Range3f(Vec3f(-1), Vec3f(+1));
}

void Volume::sendParticles(const std::vector<Vec3f> &particles)
{
	auto pointset_painter = dynamic_cast<gs_painter::Pointset *>(getPainter());
	if (pointset_painter) {
		pointset_painter->SpuArray::send(particles.data(), particles.size());
	}
	if (m_particlesToVolume) {
		auto painter = getPainter();
		m_particlesToVolume->u_radius = painter->getADrawcall().ub_material.point_size;
		m_particlesToVolume->draw(particles);
	}
}

void Volume::sendDensity(const void *density, uint32_t format)
{
	if (m_volumeTexture.id()) {
		m_volumeTexture.send(density, format);
	}
}

void Volume::update()
{
	if (m_shadowcast) {
		auto *current = GsCanvas::getCurrent();
		m_shadowcast->ub_light = current->ub_light;
		m_shadowcast->u_worldvolume = getASubstance().inverse();
		m_shadowcast->u_volume = m_volumeTexture.id();
		m_shadowcast->u_occlusion = m_occlusionTexture.id();
		m_shadowcast->compute();
	}
}

void Volume::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new VolumeInspector(this);
	}
}

}  // namespace spu::gs_node
