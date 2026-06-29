//
// ResourceShader :
//
#include "resource_shader.h"

namespace spu::libspu::resource {

ResourceShader::ResourceShader(const char *path, const Attrs &attrs) : ResourceObject(path, attrs)
{
	assert(path);
	assert(*path);

	Attrs uniq_attrs = m_attrs.uniq();

	m_reqBackground = false;  // foreground only
	m_loader.load(path, uniq_attrs);
	m_attrs = m_loader.attrs() + uniq_attrs;
	m_names = m_loader.names();

	// create shader
	{
		const auto *gl_vendor = glGetString(GL_VENDOR);
		const auto *vendor = reinterpret_cast<const char *>(gl_vendor);

		static char log[4096] = "";  // must clear

		m_attrs.emplace_back("log_ptr", log);
		m_attrs.emplace_back("log_size", sizeof(log));
		m_id = spu_shader_new(m_attrs);

		if (log[0] != 0) {
			auto def_attrs = attrs.select("def_", true);
			auto path_attrs = attrs.select("path.", true);
			if (!def_attrs.empty()) def_attrs.report("def(s)");
			message(vendor, log);
			m_isSuccess = false;

		}
		spu_message(2, "%02x vendor \"%s\" \n", m_id, vendor);
		spu_message(2, "%02x signature \"%s\"\n", m_id, m_loader.signature().c_str());
		spu_message(2, "%02x preface \"%s\"\n", m_id, m_loader.preface().c_str());

		auto cmdline = std::string("cpp ");
		for (const auto &arg: m_loader.args()) {
			cmdline += arg + " ";
		}
		spu_message(2, "%02x %s\n", m_id, cmdline.c_str());
	}

	// create digest
	{
		std::string text;
		auto add_text = [&](const char *name) {
			text += m_attrs.get(name, "");
		};
		add_text("vert");
		add_text("tesc");
		add_text("tese");
		add_text("geom");
		add_text("frag");
		add_text("comp");
	}
}
void ResourceShader::doBackground() {}
void ResourceShader::doEpilogue() {}

void ResourceShader::message(const char *vendor, const char *log)
{
	// not Nvidia
	if (strncmp(vendor, "NVIDIA", 6) != 0) {
		aux_printf("---- [%s] raw message bebin -----\n", vendor);
		aux_printf("%s\n", log);
		aux_printf("---- [%s] raw message end -------\n", vendor);
	}

	std::vector<std::string> messages;
	auto lines = extract_from_string(log, "\n");
	for (const auto &line: lines) {
		std::string message;
		if (line.find("error") != std::string::npos || line.find("wraning") != std::string::npos) {
			auto tokens = extract_from_string(line, ":");
			if (tokens.size() > 2 && isdigit(tokens[0][0]) != 0) {
				auto lineno_tokens = extract_from_string(tokens[0], "()");
				auto index = stoi(lineno_tokens[0]);
				auto lineno = stoi(lineno_tokens[1]);

				message = m_names.at(index) + ":" + std::to_string(lineno + 1);
				for (auto i = 1u; i < tokens.size(); i++) {
					message += std::string(": ") + tokens[i];
				}
			}
			else {
				message = m_names.at(0);
				for (auto i = 1u; i < tokens.size(); i++) {
					message += std::string(": ") + tokens[i];
				}
			}
		}
		if (!message.empty() && std::find(begin(messages), end(messages), message) == end(messages)) {
			messages.push_back(message);
		}
	}
	aux_printf("error(s):\n");
	for (auto &message: messages) {
		aux_printf("    %s\n", message.c_str());
	}
}

}  // namespace spu::libspu::resource
