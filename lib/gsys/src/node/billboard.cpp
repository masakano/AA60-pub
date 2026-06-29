//
// Billboard :
//
#include <gsys/node/billboard.h>
#include <gsys/painter/billboard.h>

namespace spu::gs_node {

void Billboard::init(const Attrs &attrs)
{
	// capture
	{
		Attrs def_attrs = {
		        {"color0.target",      GL_TEXTURE_2D          },
		        {"color0.iformat",     GL_RGBA8               },
		        {"color0.mag_filter",  GL_LINEAR              },
		        {"color0.min_filter",  GL_LINEAR_MIPMAP_LINEAR},
		        {"color0.auto_mipmap", 1                      },
		        {"color0.wrap_s",      GL_CLAMP_TO_EDGE       },
		        {"color0.wrap_t",      GL_CLAMP_TO_EDGE       },

		        {"depth.target",       GL_TEXTURE_2D          },
		        {"depth.iformat",      GL_DEPTH_COMPONENT32F  },
		        {"depth.mag_filter",   GL_LINEAR              },
		        {"depth.min_filter",   GL_LINEAR              },
		        {"depth.auto_mipmap",  0                      },
		        {"depth.max_level",    0                      },
		};
		m_atlas.init(def_attrs + attrs);
	}
	// painter
	{
		auto *painter = new gs_painter::Billboard(attrs.select("painter."));
		painter->getADrawcall().albedomap = m_atlas.getBuffer("color0");
		replacePainter(painter);
	}
	getARange() = {Vec3f(-1.0), Vec3f(+1.0)};
}

void Billboard::setMobs(const std::vector<Mob> &mobs)
{
	std::vector<gs_painter::Billboard::Substance> substances;
	for (const auto &mob: mobs) {
		const auto &local_composition = m_atlas.getLocalCompositions().at(mob.id);
		auto nodeworld = Mat4f().trans(mob.position);

		gs_painter::Billboard::Substance substance = {
		        nodeworld,
		        local_composition.worldscreen(0).inverse(),
		        m_atlas.getScreentexc(mob.id),
		};
		substances.push_back(substance);
	}
	getSubstances<gs_painter::Billboard::Substance>(0) = substances;
}
}  // namespace spu::gs_node
