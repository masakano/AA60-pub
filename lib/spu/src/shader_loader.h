//
// Loader :
//
#pragma once

#include <spu/GL/gl.h>
#include <ssys/attrs.h>
#include <ssys/serializer.h>

namespace spu::libspu::shader {

class Loader {
public:
	void load(const std::filesystem::path &path, Attrs &attrs);

	const std::string &preface() const { return m_preface; }
	const std::string &signature() const { return m_signature; }
	const std::vector<std::string> &names() const { return m_names; }
	const std::vector<std::string> &args() const { return m_args; }
	const Attrs &attrs() const { return m_attrs; }

private:
	enum {
		e_vert = 0x01,
		e_tesc = 0x02,
		e_tese = 0x04,
		e_geom = 0x08,
		e_frag = 0x10,
		e_comp = 0x20,
		e_common = 0x3f,
		e_directive = 0x100,
		e_eof = -1,
	};

	const std::map<const std::string, int32_t> m_masks = {
	        {"vert",   e_vert  },
                {"tesc",   e_tesc  },
                {"tese",   e_tese  },
                {"geom",   e_geom  },
	        {"frag",   e_frag  },
                {"comp",   e_comp  },
                {"common", e_common},
                {"eof",    e_eof   },
	};

	std::map<const std::string, std::string> m_sources = {
	        {"vert", ""},
                {"tesc", ""},
                {"tese", ""},
                {"geom", ""},
                {"frag", ""},
                {"comp", ""},
	};

	std::vector<char> m_text;
	std::string m_signature;
	std::string m_preface;
	std::vector<std::string> m_names;
	std::vector<std::string> m_args;

	std::vector<uint8_t> m_heap;  // for attrs
	Attrs m_attrs;

	uint32_t m_current = 0;
	int32_t m_index = 0;
	int32_t m_lineno = 0;
	int32_t m_type = e_common;

	int32_t getline(std::string &line);
	void readSource(const std::string &cmdbuf);
	void parseSources();
	void postprocessSources(const std::filesystem::path &full_path);
	bool loadFromCache(const std::filesystem::path &path);
	void saveToCache(const std::filesystem::path &path);

	SPU_SERIALIZER_FRIENDS
};
}  // namespace spu::libspu::shader
