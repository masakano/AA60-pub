//
// App :
//
#include "base_app.h"
namespace spu::fragmentlist {

class App : public BaseApp {
public:
	App(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;
	SpuFrame m_frame;  // no msaa on fram buffer
	SpuArray m_array;
	SpuArray m_appendArray;
	SpuShader m_clearShader;
	SpuShader m_appendShader;
	SpuShader m_resolveShader;
	uint32_t m_slot;
	sb6::Object m_object;
	Mat4f u_modelscreen;
	uint32_t u_head_pointer;
};

void App::init(const Attrs &attrs)
{
	BaseApp::init(attrs);

	// shaders
	{
		Attrs unif_attrs = {
		        {"u_modelscreen",  &u_modelscreen },
		        {"u_head_pointer", &u_head_pointer},
		        {"u_head_pointer", &u_head_pointer},
		};
		loadShader(m_clearShader, "fragmentlist/clear.us", Attrs(), unif_attrs);
		loadShader(m_appendShader, "fragmentlist/append.us", Attrs(), unif_attrs);
		loadShader(m_resolveShader, "fragmentlist/resolve.us", Attrs(), unif_attrs);
	}
	// array (clear, resolve)
	{
		Attrs attrs0 = {
		        {"a.0",   4},
		        {"nelem", 4},
		};
		Attrs attrs1 = {
		        {"a.+0",  0           }, // shader storage
		        {"nelem", 0x80000 * 32},
		};
		m_array.init(attrs0);
		m_array.aux(attrs1, 1);
	}
	// array (object)
	{
		const char *sym[] = {
		        "a.a_position",
		        nullptr,
		};
		m_object.load("dragon.sbm", Attrs(), sym, m_appendShader.id());
		m_appendArray.reset(m_object.getArray().id());

		auto slot_count = m_object.slotCount();
		Attrs attrs0 = {
		        {"a.+0", 0}, // shader storage
		};
		Attrs attrs1 = {
		        {"a.+1",  0}, // shader storage
		        {"nelem", 4},
		};
		m_appendArray.aux(attrs0, slot_count + 0);
		m_appendArray.aux(attrs1, slot_count + 1);
		m_appendArray.link(slot_count, m_array, 1);
		m_slot = slot_count + 1;
	}
	// image texture
	{
		Attrs attrs = {
		        {"target",      GL_TEXTURE_2D},
                        {"iformat",     GL_R32UI     },
                        {"width",       1280         },
		        {"height",      720          },
                        {"max_level",   0            },
                        {"auto_mipmap", 0            },
		};
		u_head_pointer = spu_texture_new(attrs);
	}
	// frame
	{
		Attrs attrs = {
		        {"viewport0",      viewport(0)          },
		        {"color0.target",  GL_RENDERBUFFER      },
		        {"color0.iformat", GL_RGBA8             },
		        {"depth.target",   GL_RENDERBUFFER      },
		        {"depth.iformat",  GL_DEPTH_COMPONENT32F},
		};
		m_frame = SpuFrame(attrs);
	}
}

void App::render()
{
	const auto t = getSeconds().current();
	const uint32_t barrier_bits = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT /*| GL_ATOMIC_COUNTER_BARRIER_BIT*/
	                            | GL_SHADER_STORAGE_BARRIER_BIT;
	m_clearShader.use();
	{
		const auto zero = 0u;
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
		m_appendArray.send(&zero, 4, m_slot);
		spu_graphics_memory_barrier(barrier_bits);
	}
	// m_frame.use(0);  // depth only
	m_appendShader.use();
	{
		SpuScopedRenderstate renderstate(1);
		auto modelworld = c_unit.scale(7.0);
		auto eye = Vec3f(cosf(t * 0.35) * 120.0f, cosf(t * 0.40) * 30.0f, sinf(t * 0.35) * 120.0f);

		sb6::Composition composition;
		composition.lookat(eye, Vec3f(0.0, 30.0, 0.0), ey());
		composition.perspective(viewport(0), 50.0, 0.1, 1000.0);
		auto worldview = composition.worldview();
		auto viewscreen = composition.viewscreen();
		auto modelview = worldview * modelworld;

		u_modelscreen = viewscreen * modelview;
		renderstate.flags.depth_test = false /*1*/;
		renderstate.use();
		// m_object.drawSingle();
		m_object.draw();
		spu_graphics_memory_barrier(barrier_bits);
	}
	m_resolveShader.use();
	{
		m_array.draw(GL_TRIANGLE_STRIP, 0, 4);
	}
}

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("fragmentlist");
}  // namespace spu::fragmentlist
