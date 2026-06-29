//
// ShaderObject :
//
#pragma once
#include "spu_shader_uniform.h"
#include "spu_shader_uniform_block.h"
#include "spu_shader_gathered_uniform.h"

namespace spu::libspu::spu_shader {

class ShaderObject : public libspu::Object {
public:
	enum {
		e_uniform = 0x1,
		e_uniformBlock = 0x2,
		e_subroutine = 0x3,
		e_gatheredUniform = 0x4,
		e_array = GL_ARRAY_BUFFER,
		e_storage = GL_SHADER_STORAGE_BUFFER,
	};

	ShaderObject(const Attrs &attrs)
	{
		Attrs shader_attrs = attrs;

		shader_attrs.peek("binary", "deprecated");
		auto *log = shader_attrs.pick<char *>("log_ptr", nullptr);
		auto log_size = shader_attrs.pick("log_size", 0);

		GatheredUniformSourceResult gathered_uniform_source;
		//auto is_use_unif_block = shader_attrs.pick<bool>("use_unif_block", false);
		auto is_use_unif_block = shader_attrs.pick<bool>("use_unif_block", true);
		if (is_use_unif_block) {
			preprocessGatheredUniformSources(shader_attrs, gathered_uniform_source);
		}

		m_handle.id = glCreateProgram();
		m_handle.target = 0;
		initShader(shader_attrs);

		if (!m_log.empty()) {
			if (log) {
				strncpy(log, m_log.c_str(), log_size);
			}
			else {
				printf("log: %s\n", m_log.c_str());
			}
			return;
		}
		m_unifs.init(m_handle.id);
		m_blocks.init(m_handle.id);
		m_gatheredUnifs.init(gathered_uniform_source, m_blocks);
	}

	~ShaderObject()
	{
		auto is_program = glIsProgram(m_handle.id);
		if (is_program == 0) {
			spu_message(0, "shader#%d : already deleted...\n", m_handle.id);
		}
		else {
			F(glDeleteProgram, m_handle.id);
		}
	}

	void clear() { m_unifs.clear(); }

	void getLocs(const char *names[], int32_t locs[], uint32_t sizes[], uint32_t types[], uint32_t n) const
	{
		auto get_shader_storage_location = [](const char *name, int32_t handle_id) {
			return glGetProgramResourceIndex(handle_id, GL_SHADER_STORAGE_BLOCK, name);
		};
		auto get_attrib_location = [](const char *name, int32_t handle_id) {
			return glGetAttribLocation(handle_id, name);
		};

		for (auto i = 0u; i < n; i++) {
			auto size = 0u;
			auto type = 0u;
			Handle loc = -1;
			int32_t id;
			if ((id = get_shader_storage_location(names[i], m_handle.id)) >= 0) {
				F(glShaderStorageBlockBinding, m_handle.id, id, id);
				size = 0;
				type = 0;
				loc.id = id;
				loc.target = e_storage;
			}
			else if ((id = get_attrib_location(names[i], m_handle.id)) >= 0) {
				size = 0;
				type = 0;
				loc = Handle(id, e_array);
			}
			else if ((id = m_blocks.loc(names[i])) >= 0) {
				size = m_blocks[id].size();
				type = m_blocks[id].ubo();
				loc = Handle(id, e_uniformBlock);
			}
			else if (strchr(names[i], '[')) {  // uniform and feed only, attributes
				                           // looks OK (in NV)
				m_unifs.report();
				aux_error(true, "%s : array index not allowed. (remove \'[]\')\n", names[i]);
			}
			else if ((id = m_gatheredUnifs.loc(names[i])) >= 0) {
				size = m_gatheredUnifs[id].byteSize();
				type = m_gatheredUnifs[id].type();
				loc = Handle(id, e_gatheredUniform);
			}
			else if ((id = m_unifs.getLoc(names[i])) >= 0) {
				size = m_unifs[id].byteSize();
				type = m_unifs[id].type();
				loc = Handle(id, e_uniform);
			}
			else {
				spu_message(
				        2, "symbol \"%s\" not used in shader(%02x)\n", names[i], m_handle.id);
			}

			if (locs) {
				locs[i] = loc.i;
			}
			if (sizes) {
				sizes[i] = size;
			}
			if (types) {
				types[i] = type;
			}
		}
	}

	void setUnif(const int32_t locs[], const void *const values[], uint32_t n)
	{
		for (auto i = 0u; i < n; i++) {
			if (locs[i] >= 0 && (values[i])) {
				Handle hloc = locs[i];
				if (hloc.target == e_uniform) {
					aux_error(
					        hloc.id >= m_unifs.size(), "uniform id (%d) out of range\n",
					        hloc.id);
					m_unifs[hloc.id].setUnif(values[i]);
				}
				else if (hloc.target == e_uniformBlock) {
					aux_error(
					        hloc.id >= m_blocks.size(),
					        "uniform block id (%d) out of range\n", hloc.id);
					m_blocks[hloc.id].setUnif(values[i]);
				}
				else if (hloc.target == e_gatheredUniform) {
					m_gatheredUnifs.setUnif(hloc.id, m_blocks, values[i]);
				}
			}
		}
		m_gatheredUnifs.flushUnif(m_blocks);
	}

	bool set(const char *key, const void *value)
	{
		if (m_blocks.set(key, value)) {
			return true;
		}
		return m_unifs.set(key, value);
	}

	int32_t get(const char *key, void *value)
	{
		if (m_blocks.get(key, value)) {
			return 0;
		}
		return m_unifs.get(key, value);
	}

	void use()
	{
		F(glUseProgram, m_handle.id);
		m_blocks.use();  // for transform feedback
	}

	void report() const
	{
		aux_printf("uniform:\n");
		m_unifs.report();
		aux_printf("uniform contents:\n");
		m_unifs.reportDetail(m_handle.id);
		aux_printf("uniform block:\n");
		m_blocks.report();
		aux_printf("gathered uniform:\n");
		m_gatheredUnifs.report(m_blocks);
	}

private:
	std::string m_log;
	Uniforms m_unifs;
	UniformBlocks m_blocks;
	GatheredUniforms m_gatheredUnifs;

	void link()
	{
		char log[4096];

		auto llen = 0;
		auto stat = 0;

		F(glLinkProgram, m_handle.id);
		F(glGetProgramiv, m_handle.id, GL_LINK_STATUS, &stat);
		F(glGetProgramInfoLog, m_handle.id, sizeof(log), &llen, log);

		if (stat == GL_FALSE) {
			log[llen] = 0;
			m_log += std::string(log) + "\n";
		}
	}

	int32_t compile(const char *src, int32_t type)
	{
		char log[4096];

		auto llen = 0;
		auto stat = 0;
		auto id = glCreateShader(type);

		F(glShaderSource, id, 1, &src, nullptr);
		F(glCompileShader, id);
		F(glGetShaderiv, id, GL_COMPILE_STATUS, &stat);
		F(glGetShaderInfoLog, id, sizeof(log), &llen, log);
		F(glAttachShader, m_handle.id, id);

		if (stat == GL_FALSE) {
			log[llen] = 0;
			m_log += std::string(log) + "\n";
			return 0;
		}
		return id;
	}

	void initShader(const Attrs &attrs)
	{
		attrs.peek("separable", "deprecated");

		struct ShaderAttr {
			uint32_t id;
			const char *sym;
			uint32_t type;
			uint32_t mask;
		} shader_attrs[] = {
		        {0, "frag", GL_FRAGMENT_SHADER,        GL_FRAGMENT_SHADER_BIT       },
		        {0, "vert", GL_VERTEX_SHADER,          GL_VERTEX_SHADER_BIT         },
		        {0, "geom", GL_GEOMETRY_SHADER,        GL_GEOMETRY_SHADER_BIT       },
		        {0, "tesc", GL_TESS_CONTROL_SHADER,    GL_TESS_CONTROL_SHADER_BIT   },
		        {0, "tese", GL_TESS_EVALUATION_SHADER, GL_TESS_EVALUATION_SHADER_BIT},
		        {0, "comp", GL_COMPUTE_SHADER,         GL_COMPUTE_SHADER_BIT        },
		};

		for (auto &shader_attr: shader_attrs) {
			const char *src = attrs.get<const char *>(shader_attr.sym, nullptr);
			if (src && *src) {
				shader_attr.id = compile(src, shader_attr.type);
			}
		}
		link();
		for (auto &shader_attr: shader_attrs) {
			if (shader_attr.id != 0u) {
				F(glDeleteShader, shader_attr.id);
				shader_attr.id = 0;
			}
		}
	}
};
}  // namespace spu::libspu::spu_shader
