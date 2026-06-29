//
// SpuBackoffice :
//
#include <spu++/spu_backoffice.h>
#include <ssys/random_generator.h>
#include <ssys/shared_memory.h>
#include <imgui.h>
#include <imgui_impl_spu.h>

namespace spu {

#include "spu_backoffice_shader.h"

void SpuBackoffice::Frame::init(const Rectf &, const char *vert, const char *frag)
{
	Attrs init_attrs = {
	        {"vert", vert},
	        {"frag", frag},
	};
	m_shader.init(init_attrs);

	Attrs unit_attrs = {
	        {"u_color0", &u_color0},
	        {"u_color1", &u_color1},
	};
	m_shader.addUniforms(unit_attrs);
}

void SpuBackoffice::Frame::render(const Rectf &viewport0)
{
	spu_frame_set(0, "viewport0", viewport0);
	if (m_frame.id()) {
		m_frame.begin();
	}
	else {
		spu_frame_begin(0);
	}

	// scoped_renderstate.use();
	m_shader.use();
	if (m_array.id()) {
		m_array.draw(GL_TRIANGLE_STRIP);
	}
	else {
		spu_array_draw(0, GL_TRIANGLE_STRIP);
	}

	if (m_frame.id()) {
		m_frame.end();
	}
	else {
		spu_frame_end();
	}
}

void SpuBackoffice::EchoFrame::init(const Rectf &viewport0)
{
	Frame::init(viewport0, c_vert, c_echo_shader_frag);  //
}

void SpuBackoffice::BistFrame::init(const Rectf &viewport0)
{
	Frame::init(viewport0, c_vert, c_bist_shader_frag);

	Attrs frame_attrs = {
	        {"color0.target",      GL_TEXTURE_2D  },
	        {"color0.iformat",     GL_SRGB8_ALPHA8},
	        {"color0.max_level",   0              },
	        {"color0.auto_mipmap", 0              },
	};
	Attrs array_attrs0 = {
	        {"shader_id", m_shader.id()},
	        {"nelem",     4            },
	};
	Attrs array_attrs1 = {
	        {"a.a_atomic", 0}, // expeimental
	        {"nelem",      4},
	};

	m_frame.init(frame_attrs);
	m_array.aux(array_attrs0, 0);
	m_array.aux(array_attrs1, 1);
}

SpuBackoffice::SpuBackoffice(const Attrs &attrs, void (*sys_startup)(const Attrs &), void (*sys_shutdown)())
{
	m_sysStartup = sys_startup;
	m_sysShutdown = sys_shutdown;

	auto startup_attrs = attrs.select("startup.");
	auto conf_path = startup_attrs.get("conf_path", "environ.conf");
	auto conf_tag = startup_attrs.get("conf_tag", "spu");

	// conf
	{
		File file;
		if (file.open(conf_path, "r")) {
			m_attrs.load(file, conf_tag);
		}
	}
	m_attrs += attrs.unselect({"startup."});
	m_attrs.append("backoffice", this);
	m_attrs = m_attrs.uniq();

	// message
	{
		g_message.level = m_attrs.get("message.level", g_message.level);
		if (m_attrs.get("message.is_output", true)) {
			if (g_message.output == nullptr) {
				aux_message(0, "g_message is already invalidated (ignored)\n");
			}
		}
		else {
			g_message.output = nullptr;
		}
	}
	// base path
	{
		m_basePath = startup_attrs.get("base_path", "");
		m_basePath += ":";
		m_basePath += m_attrs.get("base_path", "");
		if (!m_basePath.empty()) File::pushBase(m_basePath);
	}

	// system startup
	{
		(*m_sysStartup)(m_attrs);
	}

	// canvas
	{
		auto is_fixed_seed = m_attrs.get("fixed_seed", false);
		auto is_use_pad = m_attrs.get("sender.use_pad", false);
		auto sender_path = m_attrs.get("sender.path", "");
		auto reference_path = m_attrs.get("reference.path", "");
		auto shm_path = m_attrs.get("shm.path", "");
		auto window = m_attrs.get("window", vec4f_t(0, 0, 1280, 720));
		auto viewport0 = m_attrs.get("viewport0", vec4f_t(0, 0, window.sx, window.sy));

		m_width = viewport0.sx;
		m_height = viewport0.sy;

		if (is_fixed_seed) {
			const std::vector<uint32_t> fix_seeds = {12345, 23451, 34512, 45123, 0};  // ad-hoc
			RandomGenerator<float>::reset(fix_seeds);
		}

		if (*sender_path) {
			m_senderPath = sender_path;
		}

		if (*reference_path) {
			m_referencePath = reference_path;
			m_bist.file.open(m_referencePath + ".dat", "w");
		}

		if (*shm_path) {
			m_shm = new SharedMemory();
			m_shm->init(shm_path, m_width * m_height * 3);  // RGB
			                                                // initBaseCanvas(nullptr);
		}
		else if (!m_senderPath.empty()) {
			auto sender_attrs = m_attrs.select("sender", true);
			Attrs texture_attrs = {
			        {"target",  GL_TEXTURE_2D  },
			        {"iformat", GL_SRGB8_ALPHA8},
			        {"width",   m_width        },
			        {"height",  m_height       },
			};
			//sender_attrs.report("sender_attrs");
			//m_stageTexture.init(texture_attrs + m_attrs.select("sender.", true));
			m_stageTexture.init(texture_attrs + sender_attrs);
			spu_graphics_set("use_pad", is_use_pad);
		}
		else if (!m_referencePath.empty()) {
			m_bistFrame.init(viewport0);

			Attrs texture_attrs = {
			        {"target",        GL_TEXTURE_2D  },
			        {"iformat",       GL_SRGB8_ALPHA8},
			        {"width",         m_width        },
			        {"height",        m_height       },
			        {"receiver.path", m_referencePath},
			};
			m_stageTexture.init(texture_attrs);
			m_stageTexture.decode();
		}

		Attrs color0_attrs = {
		        {"target",      GL_TEXTURE_2D  },
                        {"iformat",     GL_SRGB8_ALPHA8},
                        {"max_level",   0              },
		        {"auto_mipmap", 0              },
                        {"width",       m_width        },
                        {"height",      m_height       },
		};
		m_color0.init(color0_attrs);

		Attrs depth_attrs = {
		        {"target",  GL_RENDERBUFFER    },
		        {"iformat", GL_DEPTH24_STENCIL8},
		        {"width",   m_width            },
		        {"height",  m_height           },
		};
		m_depthStencil.init(depth_attrs);
		m_echoFrame.init(viewport0);
	}
	// imgui
	{
		auto font_path = m_attrs.get<const char *>("imgui.font_path", nullptr);

		ImGui::CreateContext();  // temporary
		ImGui_ImplSpu_Init();

		if (font_path) {
			auto full_font_path = File::searchPath(font_path);
			ImGuiIO &io = ImGui::GetIO();
			io.Fonts->AddFontFromFileTTF(
			        full_font_path.string().c_str(), 16.0f, nullptr,
			        io.Fonts->GetGlyphRangesJapanese());
		}
		ImGui_ImplSpu_CreateDeviceObjects();
	}
}

void SpuBackoffice::postproc()
{
	SpuScopedRenderstate renderstate(m_renderstate);
	renderstate.use();

	const auto viewport0 = Rectf(0, 0, m_width, m_height);
	if (m_shm) {
		m_color0.recv(m_shm->ptr(), GL_RGB8);
		m_echoFrame.u_color0 = m_color0.id();
	}
	else if (!m_senderPath.empty()) {
		m_stageTexture.copy(m_color0.id());
		m_stageTexture.encode(m_senderEmbedValue);
		m_echoFrame.u_color0 = m_color0.id();
	}
	else if (!m_referencePath.empty()) {
		m_stageTexture.decode();

		auto &array = m_bistFrame.getArray();
		auto *wptr = array.map<uint32_t *>("w", 1);
		*wptr = 0;
		array.unmap(1);

		m_bistFrame.u_color0 = m_color0.id();
		m_bistFrame.u_color1 = m_stageTexture.id();
		m_bistFrame.render(viewport0);

		auto *rptr = array.map<uint32_t *>("r", 1);
		auto error_rate = float(*rptr) / (m_width * m_height);
		array.unmap(1);

		m_bist.count++;
		m_bist.file.printf("%d %f\n", m_bist.count, error_rate);
		m_echoFrame.u_color0 = m_bistFrame.getFrame().getBuffer("color0").id();
	}
	else {
		m_echoFrame.u_color0 = m_color0.id();
	}
	m_echoFrame.render(viewport0);
}

SpuBackoffice::~SpuBackoffice()
{
	// imgue
	{
		ImGui_ImplSpu_Shutdown();
	}

	// frames
	{
		m_echoFrame.dispose();
		m_bistFrame.dispose();
		m_stageTexture.dispose();
		m_color0.dispose();
		m_depthStencil.dispose();
	}

	// system
	if (m_sysShutdown) {
		(*m_sysShutdown)();
	}

	// bist
	if (m_bist.count > 0) {
		aux_message(
		        0, "done. To visualize, use gnuplot with \"plot '%s' using 1:2 w lines\"\n",
		        m_bist.file.name().c_str());
	}
	delete m_shm;
	if (!m_basePath.empty()) File::popBase();
}
}  // namespace spu
