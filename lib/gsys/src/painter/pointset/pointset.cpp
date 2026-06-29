//
// Pointset :
//
#include "pointset_inspector.h"

#include <gsys/canvas/shadowmap.h>
#include <gsys/decorator/instance.h>
#include <gsys/decorator/material.h>
#include <gsys/decorator/shadowmap.h>
#include <gsys/painter/pointset.h>
#include <bit>

// debug
#include <algorithm>  // std::shuffle
#include <random>     // std::mt19937, std::random_device

namespace spu::gs_painter {

void Pointset::init(const Attrs &attrs)
{
	// mode
	{
		m_isParticleset = attrs.get("particleset", 0);
		m_isAutoSort = attrs.get("auto_sort", 0);
	}

	// parent
	{
		const char *radiance_path = "painter/pointset/radiance.us";
		const char *depth_path = "painter/pointset/depth.us";

		Attrs def_attrs = {
		        {"path.radiance",       radiance_path  },
		        {"path.depth",          depth_path     },
		        {"a.a_position",        4              },
		        {"def_use_particleset", m_isParticleset},
		};

		GsPainter::init(def_attrs + attrs);
		getDecorators().push_back(new gs_decorator::Instance(this, attrs));
		getDecorators().push_back(new gs_decorator::Material(this, attrs));
		getDecorators().push_back(new gs_decorator::Shadowmap(this, attrs));

		Attrs unif_attrs = {
		        {"u_viewfrag",  &u_viewfrag },
		        {"u_viewworld", &u_viewworld},
		};
		addUniforms(unif_attrs);
	}

	// sort shader
	{
		Attrs shader_attrs = {
		        {"def_local_size_x", def_local_size_x},
		};

		Attrs unif_attrs = {
		        {"u_rayorg", &u_rayorg},
		        {"u_raydir", &u_raydir},
		        {"u_j",      &u_j     },
		        {"u_k",      &u_k     },
		};

		m_sortArray.getShader().init("compute/bitonic_sort.us", shader_attrs);
		m_sortArray.getShader().addUniforms(unif_attrs);
	}

	// sort buffer (#1 linked to main array:#0 later)
	{
		Attrs sort_array_attrs1 = {
		        {"shader_id",  m_sortArray.getShader().id()},
		        {"a.a_buffer", 0                           },
		};
		m_sortArray.aux(sort_array_attrs1, 1);
	}
}

void Pointset::gpuSort()  // back-to-front sort
{
	auto *current = GsCanvas::getCurrent();
	current->viewworld(0).get_orientation(&u_rayorg, &u_raydir, nullptr);

	// roundup
	{
		uint32_t nelem;
		SpuArray::get("0.nelem", &nelem);
		if (nelem != m_nelem2) {                    // tricky
			auto c_dummy = Vec4f(0, 0, 0, -1);  // w < 0
			auto nelem2 = std::bit_ceil(nelem);
			aux_message(0, "sort buffer resized %d -> %d\n", nelem, nelem2);
			std::vector<Vec4f> points(nelem);
			SpuArray::recv(points.data(), points.size());
			points.resize(nelem2, c_dummy);
			SpuArray::send(points.data(), points.size());
			m_nelem = nelem;
			m_nelem2 = nelem2;
		}
	}

	// patch drawcall
	{
		auto &drawcalls = getDrawcalls();
		drawcalls.resize(1);
		drawcalls[0].coms[0].first = 0;
		drawcalls[0].coms[0].count = m_nelem;
		drawcalls[0].coms[0].target = GL_ARRAY_BUFFER;
	}

	// bitonic sort
	{
		m_sortArray.link(1, *this, 0);
		for (u_k = 2; u_k <= m_nelem2; u_k = 2 * u_k) {
			for (u_j = u_k >> 1; u_j > 0; u_j = u_j >> 1) {
				m_sortArray.getDim().x = m_nelem2 / def_local_size_x;
				m_sortArray.compute();
				spu_graphics_memory_barrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}
		}
	}
}

void Pointset::startInspector()
{
	if (m_inspector == nullptr) {
		m_inspector = new PointsetInspector(this);
	}
}

void Pointset::doRender()
{
	auto *current = GsCanvas::getCurrent();
	if (m_isAutoSort && current->getShaderType() == e_radiance) {
		gpuSort();
	}
	GsPainter::doRender();
}

void Pointset::doUse(uint32_t id)
{
	if (id == 0) {
		auto *current = GsCanvas::getCurrent();
		u_viewfrag = current->screenfrag(0) * current->viewscreen();
		u_viewworld = current->viewworld(0);
	}
	auto &drawcall = getDrawcalls().at(id);

	drawcall.flags.point_sprite = true;
	drawcall.flags.program_point_size = true;
	drawcall.coms[0].mode = GL_POINTS;
	if (m_isParticleset) {
		drawcall.flags.blend = true;
	}
	GsPainter::doUse(id);
}
}  // namespace spu::gs_painter
