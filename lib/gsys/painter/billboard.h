//
// Billboard :
//
#pragma once
#include <gsys/painter.h>
#include <gsys/painter/vertex.h>
#include <gsys/decorator/instance.h>

namespace spu::gs_painter {

class Billboard : public GsPainter {
public:
	struct Substance {
		Mat4f nodeworld;
		Mat4f screennode;
		Vec4f screentexc;
	};

	explicit Billboard(const char *name = nullptr) : GsPainter(name) {}
	explicit Billboard(const Attrs &attrs) { init(attrs); }

	void init(const Attrs &attrs) override
	{
		Attrs init_attrs = {
		        {"path",                      "painter/billboard/radiance.us"},
		        {"1.rewind",                  1                              },
		        {"1.divisor",                 1                              },
		        {"1.a.a_instance_nodeworld",  16                             },
		        {"1.a.a_instance_screennode", 16                             },
		        {"1.a.a_instance_screentexc", 4                              },
		        {"nelem",                     4                              },
		};
		GsPainter::init(init_attrs + attrs);
		getDecorators().push_back(new gs_decorator::Instance(this, init_attrs + attrs));

		Attrs unif_attrs = {
		        {"u_albedomap", &u_albedomap},
		};
		addUniforms(unif_attrs);
		getDrawcalls().at(0).coms[0].mode = GL_TRIANGLE_STRIP;
		set(attrs);
	}
	// PAINTER_NO_VERTEX_FUNCS;

protected:
	uint32_t u_albedomap = 0;
	void doUse(uint32_t id) override { u_albedomap = getDrawcalls().at(id).albedomap.id(); }
};
}  // namespace spu::gs_painter
