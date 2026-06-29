//
// ReceiverTweakbar :
//
#include "base_app.h"
#include <gsys/canvas/copy.h>
#include <ssys/shared_memory.h>

namespace spu {

class ReceiverApp : public BaseApp {
public:
	explicit ReceiverApp(const char *name) : BaseApp(name) {}
	void init(const Attrs &attrs) override;
	void render() override;

private:
	SharedMemory m_shm;
	gs_canvas::Copy m_copy;
	SpuTexture m_texture;
};

void ReceiverApp::init(const Attrs &attrs)
{
	GsPage::init(attrs);

	auto receiver_attrs = attrs.select("receiver.", true);

	Attrs texture_attrs = {
	        {"iformat",     GL_SRGB8_ALPHA8},
                {"width",       c_width        },
                {"height",      c_height       },
                {"max_level",   0              },
	        {"auto_mipmap", 0              },
	};
	m_texture.init(texture_attrs + receiver_attrs);

	Attrs canvas_attrs = {
	        {"use_unif_block", false         },
	        {"def_vflip",      0             },
	        {"path",           "flip_copy.us"},
	};
	m_copy.init(canvas_attrs);

	m_copy.u_color0 = m_texture.id();
	m_shm.init(c_shm_path, sizeof(SharedBuffer) + c_width * c_height * sizeof(RGBA8));
	auto *ptr = (SharedBuffer *)m_shm.ptr();
	ptr->control = StreamControl{};
	setReceiverControl(ptr->control);
}

void ReceiverApp::render()
{
	auto *ptr = (SharedBuffer *)m_shm.ptr();

	setReceiverControl(ptr->control);
	uint32_t count = 0;
	m_texture.decode(&count);
	m_texture.recv(ptr->pixels, GL_SRGB8_ALPHA8);
	ptr->count = count;
	m_copy.render();
}
static ObjectRegistry<GsCanvas>::Creator<ReceiverApp> page_creator("receiver");
}  // namespace spu
