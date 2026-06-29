//
// MultisampleResolver :
//
#pragma once
#include <spu/spu.h>
#include <spu/src/uniform.h>
#include <regex>

namespace spu::libspu {

class MultisampleResolver {
public:
	const uint32_t def_local_size = 16;

	uint32_t m_arrayId = 0;
	uint32_t m_shaderId = 0;
	Uniform m_unif;

	uint32_t m_dimX = 1;
	uint32_t m_dimY = 1;

	uint32_t u_multisample = 0;
	uint32_t u_textures_ms[8] = {0};
	uint32_t u_textures[8] = {0};

	~MultisampleResolver()
	{
		if (m_arrayId) spu_array_delete(m_arrayId);
		if (m_shaderId) spu_shader_delete(m_shaderId);
	}

	void init(
	        const std::vector<uint32_t> &color_textures, const std::vector<uint32_t> &color_textures_ms,
	        const std::vector<uint32_t> &depth_textures, const std::vector<uint32_t> &depth_textures_ms)
	{
		assert(color_textures.size() == color_textures_ms.size());
		assert(depth_textures.size() == depth_textures_ms.size());
		assert(color_textures.size() < 8);
		assert(depth_textures.size() < 2);

		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mask = 0;
		for (auto i = 0u; i < color_textures.size(); i++) {
			check_texture_iformat(color_textures[i], color_textures_ms[i]);
			check_texture_multisample(color_textures_ms[i]);
			check_texture_size(color_textures_ms[i], width, height);
			u_textures_ms[i] = color_textures_ms[i];
			u_textures[i] = color_textures[i];
			mask |= (1 << i);
		}
		if (depth_textures.size() == 1) {
			check_texture_iformat(depth_textures[0], depth_textures_ms[0]);
			check_texture_size(depth_textures_ms[0], width, height);
			u_textures_ms[7] = depth_textures_ms[0];
			u_textures[7] = depth_textures[0];
			mask |= (1 << 7);
		}
		Attrs array_attrs = {
		        {"nelem", 1}
                };
		m_arrayId = spu_array_new(array_attrs);

		auto comp = std::string(c_comp);
		comp = std::regex_replace(comp, std::regex("def_local_size"), std::to_string(def_local_size));
		comp = std::regex_replace(comp, std::regex("def_mask"), std::to_string(mask));

		Attrs shader_attrs = {
		        {"comp", comp.c_str()},
		};
		m_shaderId = spu_shader_new(shader_attrs);

		m_unif.init(m_shaderId);
		m_unif.addUniform("u_multisample", &u_multisample);
		m_unif.addUniform("u_color0_ms", &u_textures_ms[0]);
		m_unif.addUniform("u_color1_ms", &u_textures_ms[1]);
		m_unif.addUniform("u_color2_ms", &u_textures_ms[2]);
		m_unif.addUniform("u_color3_ms", &u_textures_ms[3]);
		m_unif.addUniform("u_color4_ms", &u_textures_ms[4]);
		m_unif.addUniform("u_color5_ms", &u_textures_ms[5]);
		m_unif.addUniform("u_color6_ms", &u_textures_ms[6]);
		m_unif.addUniform("u_depth_ms", &u_textures_ms[7]);
		m_unif.addUniform("u_color0", &u_textures[0]);
		m_unif.addUniform("u_color1", &u_textures[1]);
		m_unif.addUniform("u_color2", &u_textures[2]);
		m_unif.addUniform("u_color3", &u_textures[3]);
		m_unif.addUniform("u_color4", &u_textures[4]);
		m_unif.addUniform("u_color5", &u_textures[5]);
		m_unif.addUniform("u_color6", &u_textures[6]);
		m_unif.addUniform("u_depth", &u_textures[7]);

		m_dimX = (width + def_local_size - 1) / def_local_size;
		m_dimY = (height + def_local_size - 1) / def_local_size;

		aux_error(u_multisample == 0, "invalid multisample(%d)\n", u_multisample);
	}

	void compute()
	{
		if (m_arrayId) {
			m_unif.use();
			spu_array_draw(m_arrayId, 0xffff, m_dimX, m_dimY, 1);
		}
	}

private:
	void check_texture_multisample(uint32_t texture_id)
	{
		auto prev_multisample = u_multisample;
		spu_texture_get(texture_id, "multisample", &u_multisample);
		aux_error(
		        prev_multisample && prev_multisample != u_multisample,
		        "texture multisample (%d) mismatch\n", prev_multisample);
	}

	void check_texture_size(uint32_t texture_id, uint32_t &width, uint32_t &height)
	{
		auto prev_width = width;
		auto prev_height = height;
		spu_texture_get(texture_id, "width", &width);
		spu_texture_get(texture_id, "height", &height);
		aux_error(
		        (prev_width && prev_width != width) || (prev_height && prev_height != height),
		        "texture size (%dx%d) mismatch\n", prev_width, prev_height);
	}

	void check_texture_iformat(uint32_t dst_texture_id, uint32_t src_texture_id)
	{
		if ((src_texture_id >> 16) != GL_TEXTURE_2D_MULTISAMPLE) {
			aux_message(0, "source is not GL_TEXTURE_2D_MULTISAMPLE\n");
			return;
		}
		if ((dst_texture_id >> 16) != GL_TEXTURE_2D) {
			aux_message(0, "destination is not GL_TEXTURE_2D\n");
			return;
		}
	}

	// max images is 7 (+depth)
	// since C7624: OpenGL does not allow greater than 8 image uniforms
	// clang-format off
	const char *c_comp =  {
		"#version 440\n"
		"layout (local_size_x = def_local_size, local_size_y = def_local_size) in;\n"
		"uniform uint u_multisample;\n"
		"uniform sampler2DMS u_color0_ms;\n"
		"uniform sampler2DMS u_color1_ms;\n"
		"uniform sampler2DMS u_color2_ms;\n"
		"uniform sampler2DMS u_color3_ms;\n"
		"uniform sampler2DMS u_color4_ms;\n"
		"uniform sampler2DMS u_color5_ms;\n"
		"uniform sampler2DMS u_color6_ms;\n"
		"uniform sampler2DMS u_depth_ms;\n"
		"writeonly uniform image2D u_color0;\n"
		"writeonly uniform image2D u_color1;\n"
		"writeonly uniform image2D u_color2;\n"
		"writeonly uniform image2D u_color3;\n"
		"writeonly uniform image2D u_color4;\n"
		"writeonly uniform image2D u_color5;\n"
		"writeonly uniform image2D u_color6;\n"
		"writeonly uniform image2D u_depth;\n"
		"void main()\n"
		"{\n"
		"    float u = gl_GlobalInvocationID.x;\n"
		"    float v = gl_GlobalInvocationID.y;\n"
		"    ivec2 fragcoord = ivec2(u, v);\n"
		"    vec4 color0 = vec4(0);\n"
		"    vec4 color1 = vec4(0);\n"
		"    vec4 color2 = vec4(0);\n"
		"    vec4 color3 = vec4(0);\n"
		"    vec4 color4 = vec4(0);\n"
		"    vec4 color5 = vec4(0);\n"
		"    vec4 color6 = vec4(0);\n"
		"    float depth = 0;\n"
		"    for (int i = 0; i < u_multisample; i++) {\n"
		"        if ((def_mask & (0x1<<0)) != 0) color0 += texelFetch(u_color0_ms, fragcoord, i);\n"
		"        if ((def_mask & (0x1<<1)) != 0) color1 += texelFetch(u_color1_ms, fragcoord, i);\n"
		"        if ((def_mask & (0x1<<2)) != 0) color2 += texelFetch(u_color2_ms, fragcoord, i);\n"
		"        if ((def_mask & (0x1<<3)) != 0) color3 += texelFetch(u_color3_ms, fragcoord, i);\n"
		"        if ((def_mask & (0x1<<4)) != 0) color4 += texelFetch(u_color4_ms, fragcoord, i);\n"
		"        if ((def_mask & (0x1<<5)) != 0) color5 += texelFetch(u_color5_ms, fragcoord, i);\n"
		"        if ((def_mask & (0x1<<6)) != 0) color6 += texelFetch(u_color6_ms, fragcoord, i);\n"
		"        if ((def_mask & (0x1<<7)) != 0) depth += texelFetch(u_depth_ms, fragcoord, i).r;\n"
		"    }\n"
		"    if ((def_mask & (0x1<<0)) != 0) imageStore(u_color0, fragcoord, color0 / u_multisample);\n"
		"    if ((def_mask & (0x1<<1)) != 0) imageStore(u_color1, fragcoord, color1 / u_multisample);\n"
		"    if ((def_mask & (0x1<<2)) != 0) imageStore(u_color2, fragcoord, color2 / u_multisample);\n"
		"    if ((def_mask & (0x1<<3)) != 0) imageStore(u_color3, fragcoord, color3 / u_multisample);\n"
		"    if ((def_mask & (0x1<<4)) != 0) imageStore(u_color4, fragcoord, color4 / u_multisample);\n"
		"    if ((def_mask & (0x1<<5)) != 0) imageStore(u_color5, fragcoord, color5 / u_multisample);\n"
		"    if ((def_mask & (0x1<<6)) != 0) imageStore(u_color6, fragcoord, color6 / u_multisample);\n"
		"    if ((def_mask & (0x1<<7)) != 0) imageStore(u_depth, fragcoord, vec4(depth / u_multisample));\n"
		"}\n"
	};
	// clang-format on
};
}  // namespace spu::libspu
