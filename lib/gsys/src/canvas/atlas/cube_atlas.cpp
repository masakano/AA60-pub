//
// CubeAtlas :
//
#include <gsys/canvas/atlas.h>

namespace spu::gs_canvas {

void CubeAtlas::init(const Attrs &attrs)
{
	attrs.peek("slot", "use 'cube_count' instead");
	attrs.peek("unit", "use 'cube_size' instead");

	m_cubeSize = attrs.get("cube_size", 1);
	m_cubeCount = attrs.get("cube_count", 1);
	m_iformat = attrs.get("iformat", GL_RGBA16F);

	// atlas
	{
		auto depth_target = attrs.get("depth_target", GL_RENDERBUFFER);

		if (m_iformat == GL_DEPTH_COMPONENT32F) {
			m_iformat = 0;
			depth_target = GL_TEXTURE_2D;
		}

		auto viewport = Rectf(0, 0, m_cubeSize * 6, m_cubeSize * m_cubeCount);
		Attrs atlas_attrs = {
		        {"nx",            6                    },
		        {"ny",            m_cubeCount          },
		        {"viewport0",     viewport             },
		        {"depth.target",  depth_target         },
		        {"depth.iformat", GL_DEPTH_COMPONENT32F},
		};
		if (m_iformat) {
			Attrs color_attrs = {
			        {"color0.target",      GL_TEXTURE_2D},
			        {"color0.iformat",     m_iformat    },
			        {"color0.auto_mipmap", 0            },
			};
			atlas_attrs += color_attrs;
		}
		Atlas::init(atlas_attrs + attrs);
	}
}

uint32_t CubeAtlas::iformat(const hash32_t &slot) const
{
	if (slot == "depth"_h32) {
		return GL_R32F;
	}
	return m_iformat;
}

void CubeAtlas::update()
{
	if (m_drawfunc.func == nullptr) return;

	auto cubeComposition = Composition::cubeComposition(m_drawfunc.center, m_drawfunc.near, m_drawfunc.far);
	for (auto n = 0; n < 6; n++) {
		if ((m_drawfunc.face_mask & (1 << n)) != 0) {
			auto atlas_index = m_drawfunc.cube_index * 6 + n;
			m_localCompositions.at(atlas_index).takeover(cubeComposition, n, 0);
			subBegin(atlas_index);
			m_drawfunc.func();
			subEnd();
		}
	}
	GsCanvas::update();
}

void CubeAtlas::cubeToAtlas(
        uint32_t cube_index, const SpuTexture &cube_texture, uint32_t layer, const hash32_t &slot)
{
	if (slot == "color0"_h32) {
		checkFormat(cube_texture);
	}

	for (auto n = 0; n < 6; n++) {
		if ((m_drawfunc.face_mask & (1 << n)) != 0) {
			auto atlas_index = cube_index * 6 + n;
			auto &lc = getLocalCompositions().at(atlas_index);
			auto u = m_cubeSize;
			uint32_t size[4] = {u, u, 1, 1};
			int32_t src_loc[4] = {0, 0, int32_t(layer) * 6 + n, 0};
			int32_t dst_loc[4] = {int32_t(lc.viewport(0).ox), int32_t(lc.viewport(0).oy), 0, 0};

			getBuffer(slot).copy(cube_texture.id(), dst_loc, src_loc, size);
		}
	}
}

void CubeAtlas::atlasToCube(uint32_t cube_index, SpuTexture &cube_texture, uint32_t layer, const hash32_t &slot)
{
	if (slot == "color0"_h32) {
		checkFormat(cube_texture);
	}

	for (auto n = 0; n < 6; n++) {
		if ((m_drawfunc.face_mask & (1 << n)) != 0) {
			auto atlas_index = cube_index * 6 + n;
			auto &lc = getLocalCompositions().at(atlas_index);
			auto u = m_cubeSize;
			uint32_t size[4] = {u, u, 1, 1};
			int32_t src_loc[4] = {int32_t(lc.viewport(0).ox), int32_t(lc.viewport(0).oy), 0, 0};
			int32_t dst_loc[4] = {0, 0, int32_t(layer) * 6 + n, 0};

			cube_texture.copy(getBuffer(slot).id(), dst_loc, src_loc, size);
		}
	}
}

void CubeAtlas::checkFormat(const SpuTexture &cube_texture) const
{
	uint32_t iformat;
	cube_texture.get("iformat", &iformat);
	aux_error(
	        iformat != m_iformat, "upload texture must be same format of atlas's (%s)\n",
	        opengl_const(m_iformat));
}

void CubeAtlas::initCubeArrayTexture(
        SpuTexture &texture, uint32_t layer_count, const Attrs &aux_attrs, const hash32_t &slot)
{
	if (layer_count == 0) {
		layer_count = cubeCount();  // 6 faces x nslot layers
	}

	Attrs attrs = {
	        {"target",      GL_TEXTURE_CUBE_MAP_ARRAY},
	        {"iformat",     iformat(slot)            },
	        {"width",       cubeSize()               },
	        {"height",      cubeSize()               },
	        {"depth",       layer_count * 6          },
	        {"max_level",   0                        },
	        {"auto_mipmap", 0                        },
	};
	texture.init(attrs + aux_attrs);
}
}  // namespace spu::gs_canvas
