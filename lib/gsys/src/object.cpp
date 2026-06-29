//
// GsObject :
//
#include <filesystem>
#include <gsys/object.h>
#include <spu++/spu++.h>
#include "c_ball_png.h"

namespace spu {

GsObject::GsObject(const char *name) : m_hash(name), m_name(name ? name : "")
{
	setProperty(e_render, 1);
	setProperty(e_debug_render, 0);
	setProperty(e_lazy, 0);
	setProperty(e_alive, 1);

	ms_state->objects.push_back(this);
}

GsObject::~GsObject()
{
	dispose();
	auto op = vector_find(ms_state->objects, this);
	if (op != end(ms_state->objects)) {
		ms_state->objects.erase(op);
	}
}

void GsObject::dispose()
{
	if (vector_is_find(ms_state->objects, m_inspector)) {
		delete m_inspector;
		m_inspector = nullptr;
	}
}

void GsObject::init(const Attrs &attrs) { setName(attrs.get("name", m_name)); }

void GsObject::set(const Attrs &attrs) { setName(attrs.get("name", m_name)); }

void GsObject::setName(const std::string &name)
{
	m_name = name;
	m_hash = hash32_t(name.c_str());
}

std::string GsObject::typeName() const
{
	auto str = demangle_string(typeid(*this).name());
	auto pos = str.rfind("::");
	return pos != std::string::npos ? str.substr(pos + 2) : str;
}

std::string GsObject::prettyName() const
{
	auto type_name = typeName();
	if (m_name.empty() || m_name == type_name) {
		return type_name;
	}
	else {
		return type_name + " '" + peeloff_string(m_name) + "'";
	}
}

bool GsObject::sync(bool is_nonblock)
{
	const auto c_sleep_usec = 16000;
	while (doSync(is_nonblock)) {
		if (is_nonblock) {
			return true;
		}
		sleep_microsec(c_sleep_usec);
	}
	if (!m_isInCallback && !m_callbacks.empty()) {
		m_isInCallback = true;
		for (auto &callback: m_callbacks) {
			callback.func(callback.arg);
		}
		m_callbacks.clear();
		m_isInCallback = false;
	}
	return false;
}

void GsObject::syncCallback(const std::function<void(void *)> &callback, void *arg)
{
	m_callbacks.push_back({callback, arg});
}

bool GsObject::isDefaultTexture(uint32_t texture_id)
{
	return (texture_id == 0 || texture_id == ms_state->white_texture.id()
	        || texture_id == ms_state->black_texture.id() || texture_id == ms_state->light_texture.id()
	        || texture_id == ms_state->brdf_texture.id());
}

void GsObject::startup(const Attrs &attrs)
{
	SpuRenderstate::startup(attrs);
	ms_state = new State();

	// white/black texture
	{
		Vec4f pixel;
		Attrs texture_attrs = {
		        {"target",      GL_TEXTURE_2D},
		        {"iformat",     GL_RGBA32F   },
		        {"width",       1            },
		        {"height",      1            },
		        {"min_filter",  GL_NEAREST   },
		        {"mag_filter",  GL_NEAREST   },
		        {"data",        pixel.f      },
		        {"auto_mipmap", 0            },
		        {"max_level",   0            },
		};

		pixel = eone<Vec4f>();
		ms_state->white_texture.init(texture_attrs);
		ms_state->white_texture.append("[default_white]");

		pixel = ezero<Vec4f>();
		ms_state->black_texture.init(texture_attrs);
		ms_state->black_texture.append("[default_black]");
	}
	// lightmap
	{
		Attrs texture_attrs = {
		        {"target",      GL_TEXTURE_2D          },
		        {"iformat",     GL_RGBA16F             },
		        {"clamp",       64.0                   }, // need parameterize
		        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter",  GL_LINEAR              },
		        {"wrap_s",      GL_REPEAT              },
		        {"wrap_t",      GL_CLAMP_TO_EDGE       },
		        {"auto_mipmpa", 1                      },
		};
		const char *path = "default/lightmap.hdr";
		ms_state->light_texture.init(path, texture_attrs);
	}

	// default brdf
	{
		const uint32_t c_brdf_size = 512;
		Attrs texture_attrs = {
		        {"target",      GL_TEXTURE_2D          },
		        {"iformat",     GL_RG32F               },
		        {"width",       c_brdf_size            },
		        {"height",      c_brdf_size            },
		        {"wrap_s",      GL_REPEAT              },
		        {"wrap_t",      GL_CLAMP_TO_EDGE       },
		        {"min_filter",  GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter",  GL_LINEAR              },
		        {"auto_mipmap", 1                      },
		};
		ms_state->brdf_texture.init(texture_attrs);
		ms_state->brdf_texture.append("[default_brdf]");

		Rectf viewport = {0, 0, c_brdf_size, c_brdf_size};
		Attrs frame_attrs = {
		        {"color0",    ms_state->brdf_texture.id()},
		        {"viewport0", viewport                   },
		};
		SpuFrame frame(frame_attrs);

		const auto *path = "decorator/material/pbr_integrate_brdf.us";
		SpuShader shader(path);

		SpuScopedRenderstate renderstate(1);
		renderstate.flags.depth_test = 0;
		renderstate.flags.blend = 0;
		renderstate.use();  // for safetya

		frame.begin();
		shader.use();
		spu_array_draw(0, GL_TRIANGLE_STRIP);
		frame.end();
	}
	// special path
	File::embed(c_ball_png_name, c_ball_png_data, sizeof(c_ball_png_data));

	// entry startup
	auto compar = [](const Startup &s0, const Startup &s1) { return s0.level < s1.level; };
	vector_sort(startups(), compar);
	for (auto &startup: startups()) {
		startup.startup(attrs);
	}
}

void GsObject::pruneObjects(const std::vector<GsObject *> &alives)
{
	auto objects = ms_state->objects;  // copy
	for (auto &object: objects) {
		if (!vector_is_find(alives, object)) {
			if (vector_is_find(ms_state->objects, object)) {
				aux_message(0, "prune: %p '%s' \n", object, object->prettyName().c_str());
				delete object;
			}
		}
	}
}

void GsObject::shutdown()
{
	reverse(begin(startups()), end(startups()));
	for (auto &startup: startups()) {
		startup.shutdown();
	}
	pruneObjects(std::vector<GsObject *>());  // prune all
	delete ms_state;
	ms_state = nullptr;
	SpuRenderstate::shutdown();
}

void GsObject::report(const char *str) const
{
	if (str && *str) {
		auto line = std::string(strlen(str) + 1, '-');
		aux_printf("%s\n", line.c_str());
		aux_printf("%s:\n", str);
		aux_printf("%s\n", line.c_str());
	}
	aux_printf("    name:\n\t'%s'\n", prettyName().c_str());
	aux_printf("    property:\n\t");

	for (auto &prop: m_properties) {
		aux_printf("'%s'=%d ", prop.first.c_str(), prop.second);
	}
	// aux_printf("\n");
}
}  // namespace spu
