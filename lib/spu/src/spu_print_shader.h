//
//
//
#pragma once
namespace {
const char *c_vert
        = "#version 430\n"
          "in vec3 a_position; \n"
          "in vec2 a_texcoord; \n"
          "in vec3 a_color; \n"
          "in mat4 a_instance_nodetext; \n"
          "uniform mat4 u_textscreen; \n"
          "out vec2 f_texcoord; \n"
          "out vec3 f_color; \n"
          "void main() \n"
          "{ \n"
          "	gl_Position = u_textscreen * a_instance_nodetext * vec4(a_position, 1); \n"
          "	f_texcoord = a_texcoord; \n"
          "	f_color = a_color; \n"
          "} \n";

const char *c_frag
        = "#version 430\n"
          "uniform sampler2D u_font_texture; \n"
          "uniform vec3 u_color; \n"
          "uniform float u_smoothstep; \n"
          "in vec2 f_texcoord; \n"
          "in vec3 f_color; \n"
          "out vec4 final_color0; \n"
          "void main() \n"
          "{ \n"
          "	const float c_cutoff = 0.5; \n"
          "	float sdf = textureLod(u_font_texture, f_texcoord, 0).r - c_cutoff; \n"
          "	float alpha = smoothstep(-u_smoothstep, +u_smoothstep, sdf); \n"
          "	if (alpha < 0.01) discard; \n"
          "	final_color0 = vec4(u_color * f_color, alpha); \n"
          "} \n";
}  // namespace
