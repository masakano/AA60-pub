//
// BulletTimeTrigger :
//
#include <spu++/spu_site.h>
#include <math/matrix.hpp>
#include <shapes/array.hpp>
#include <images/brushed_metal.hpp>
#include <shapes/plane.hpp>

namespace spu::oglplus {
namespace {

/* clang-format off */
const char *c_floor_vert =  {
    "#version 330                                                 \n"
    "                                                             \n"
    "uniform mat4 u_worldview;                                    \n"
    "uniform mat4 u_viewsceen;                                    \n"
    "uniform vec3 u_eye_position;                                 \n"
    "uniform vec3 u_light_position;                               \n"
    "                                                             \n"
    "in vec4 a_position;                                          \n"
    "in vec3 a_normal, a_tangent;                                 \n"
    "in vec2 a_texcoord;                                          \n"
    "                                                             \n"
    "out vec3 g_normal, g_tangent, g_bitangent;                   \n"
    "out vec3 g_light_dir, g_view_dir;                            \n"
    "out vec2 g_texcoord;                                         \n"
    "                                                             \n"
    "void main()                                                  \n"
    "{                                                            \n"
    "       gl_Position = a_position;                             \n"
    "       g_light_dir = u_light_position - gl_Position.xyz;     \n"
    "       g_view_dir = u_eye_position - gl_Position.xyz;        \n"
    "       g_normal = a_normal;                                  \n"
    "       g_tangent = a_tangent;                                \n"
    "       g_bitangent = cross(g_normal, g_tangent);             \n"
    "       g_texcoord = a_texcoord * 100.0;                      \n"
    "       gl_Position = u_viewsceen * u_worldview * gl_Position;\n"
    "}                                                            \n"
};

const char *c_floor_frag =  {
    "#version 330                                                    \n"
    "                                                                \n"
    "const vec3 u_color1 = vec3(0.5, 0.7, 0.6);                      \n"
    "const vec3 u_color2 = vec3(0.7, 0.9, 0.8);                      \n"
    "                                                                \n"
    "uniform sampler2D u_texture;                                    \n"
    "uniform float u_light_multiplier;                               \n"
    "                                                                \n"
    "in vec3 g_normal, g_tangent, g_bitangent;                       \n"
    "in vec3 g_light_dir, g_view_dir;                                \n"
    "in vec2 g_texcoord;                                             \n"
    "                                                                \n"
    "out vec3 final_color;                                           \n"
    "                                                                \n"
    "void main()                                                     \n"
    "{                                                               \n"
    "       vec3 sample = texture(u_texture, g_texcoord).rgb;        \n"
    "       vec3 light_color = vec3(1.0, 1.0, 0.9);                  \n"
    "       vec3 normal = normalize(                                 \n"
    "               2.0*g_normal +                                   \n"
    "               (sample.r - 0.5)*g_tangent +                     \n"
    "               (sample.g - 0.5)*g_bitangent                     \n"
    "       );                                                       \n"
    "       vec3 light_refl = reflect(                               \n"
    "               -normalize(g_light_dir),                         \n"
    "               normal                                           \n"
    "       );                                                       \n"
    "       float specular = u_light_multiplier * pow(max(dot(       \n"
    "               normalize(light_refl),                           \n"
    "               normalize(g_view_dir)                            \n"
    "       )+0.04, 0.0), 16+sample.b*48)*pow(0.4+sample.b*1.6, 4.0);\n"
    "       normal = normalize(g_normal*3.0 + normal);               \n"
    "       float diffuse = u_light_multiplier * pow(max(dot(        \n"
    "               normalize(normal),                               \n"
    "               normalize(g_light_dir)                           \n"
    "       ), 0.0), 2.0);                                           \n"
    "       float ambient = 0.6;                                     \n"
    "       vec3 u_color = mix(u_color1, u_color2, sample.b);        \n"
    "       final_color =                                            \n"
    "               u_color * ambient +                              \n"
    "               light_color * u_color * diffuse +                \n"
    "               light_color * specular;                          \n"
    "}                                                               \n"
};


const char *c_camera_comp =  {
    "#version 440                                                                     \n"
    "layout(local_size_x = 2, local_size_y = 1, local_size_z = 1) in;                 \n"
    "                                                                                 \n"
    "uniform float u_interval;                                                        \n"
    "uniform float u_mass;                                                            \n"
    "uniform float u_target_distance;                                                 \n"
    "uniform float u_elevation;                                                       \n"
    "uniform uint u_cube_center_index;                                                \n"
    "                                                                                 \n"
    "layout(std140) uniform u_cube_position_block {                                   \n"
    "       vec3 cube_positions[512];                                                 \n"
    "};                                                                               \n"
    "                                                                                 \n"
    "readonly buffer a_position { vec4 v[]; } b_position;                             \n"
    "readonly buffer a_velocity { vec4 v[]; } b_velocity;                             \n"
    "writeonly buffer a_tfb_position { vec4 v[]; } b_tfb_position;                    \n"
    "writeonly buffer a_tfb_velocity { vec4 v[]; } b_tfb_velocity;                    \n"
    "                                                                                 \n"
    "vec3 in_position()                                                               \n"
    "{                                                                                \n"
    "       uint index = gl_GlobalInvocationID.x;                                     \n"
    "       return b_position.v[index].xyz;                                           \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 in_velocity()                                                               \n"
    "{                                                                                \n"
    "       uint index = gl_GlobalInvocationID.x;                                     \n"
    "       return b_velocity.v[index].xyz;                                           \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "bool is_camera()                                                                 \n"
    "{                                                                                \n"
    "       return gl_GlobalInvocationID.x == 0;                                      \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 spring_force(vec3 p, vec3 ps, float k, float l)                             \n"
    "{                                                                                \n"
    "       vec3 v = ps - p;                                                          \n"
    "       float ds = (length(v) - l);                                               \n"
    "       return k * sign(ds) * min(abs(ds), l) * normalize(v);                     \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 springs()                                                                   \n"
    "{                                                                                \n"
    "       vec3 position = in_position();                                            \n"
    "       if (is_camera()) {                                                        \n"
    "               return spring_force(                                              \n"
    "                       position,                                                 \n"
    "                       cube_positions[u_cube_center_index],                      \n"
    "                       3.0,                                                      \n"
    "                       u_target_distance                                         \n"
    "               );                                                                \n"
    "       }                                                                         \n"
    "       else {                                                                    \n"
    "               return spring_force(                                              \n"
    "                       position,                                                 \n"
    "                       cube_positions[u_cube_center_index],                      \n"
    "                       9.0,                                                      \n"
    "                       0.1                                                       \n"
    "               );                                                                \n"
    "       }                                                                         \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 elevation()                                                                 \n"
    "{                                                                                \n"
    "       if (is_camera()) {                                                        \n"
    "               float ds = (u_elevation - in_position().y);                       \n"
    "               float k = 7.0;                                                    \n"
    "               vec3 v = vec3(0.0, 1.0, 0.0);                                     \n"
    "               return k * sign(ds) * min(abs(ds), u_elevation) * v;              \n"
    "       }                                                                         \n"
    "       else return vec3(0.0, 0.0, 0.0);                                          \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 drag()                                                                      \n"
    "{                                                                                \n"
    "       return -0.08 * in_velocity();                                             \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "void main()                                                                      \n"
    "{                                                                                \n"
    "       uint index = gl_GlobalInvocationID.x;                                     \n"
    "       vec3 force = drag() + elevation() + springs();                            \n"
    "       vec3 velocity = in_velocity() + (force * u_interval) / u_mass;            \n"
    "       b_tfb_velocity.v[index] = vec4(velocity, 1.0);                            \n"
    "       b_tfb_position.v[index] = vec4(in_position() + velocity * u_interval,1.0);\n"
    "}                                                                                \n"
};
	

const char *c_physics_comp =  {
    "#version 440                                                                     \n"
    "layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;                \n"
    "                                                                                 \n"
    "uniform vec3 u_impulse_center;                                                   \n"
    "uniform float u_impulse_strength;                                                \n"
    "uniform float u_interval;                                                        \n"
    "uniform float u_mass;                                                            \n"
    "uniform float u_spring_a_length;                                                 \n"
    "uniform float u_spring_b_length;                                                 \n"
    "uniform float u_spring_c_length;                                                 \n"
    "uniform float u_spring_a_strength;                                               \n"
    "uniform float u_spring_b_strength;                                               \n"
    "uniform float u_spring_c_strength;                                               \n"
    "                                                                                 \n"
    "layout(std140) uniform u_position_block {                                        \n"
    "       vec3 positions[512];                                                      \n"
    "};                                                                               \n"
    "                                                                                 \n"
    "struct vertex_spring_indices {                                                   \n"
    "       ivec4 spring_ax;                                                          \n"
    "       ivec4 spring_ay;                                                          \n"
    "       ivec4 spring_az;                                                          \n"
    "       ivec4 spring_bx;                                                          \n"
    "       ivec4 spring_by;                                                          \n"
    "       ivec4 spring_bz;                                                          \n"
    "       ivec4 spring_cup;                                                         \n"
    "       ivec4 spring_cdn;                                                         \n"
    "};                                                                               \n"
    "                                                                                 \n"
    "layout(std140) uniform u_spring_index_block {                                    \n"
    "       vertex_spring_indices vertex_springs[512];                                \n"
    "};                                                                               \n"
    "                                                                                 \n"
    "readonly buffer a_position { vec4 v[]; } b_position;                             \n"
    "readonly buffer a_velocity { vec4 v[]; } b_velocity;                             \n"
    "writeonly buffer a_tfb_position { vec4 v[]; } b_tfb_position;                    \n"
    "writeonly buffer a_tfb_velocity { vec4 v[]; } b_tfb_velocity;                    \n"
    "                                                                                 \n"
    "vec3 in_position()                                                               \n"
    "{                                                                                \n"
    "       uint index = gl_GlobalInvocationID.x;                                     \n"
    "       return b_position.v[index].xyz;                                           \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 in_velocity()                                                               \n"
    "{                                                                                \n"
    "       uint index = gl_GlobalInvocationID.x;                                     \n"
    "       return b_velocity.v[index].xyz;                                           \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 gravity()                                                                   \n"
    "{                                                                                \n"
    "       float acting = -max(sign(in_position().y), 0.0);                          \n"
    "       vec3 grav_accel = vec3(0.0, acting * 9.81, 0.0);                          \n"
    "       return u_mass * grav_accel;                                               \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 floor()                                                                     \n"
    "{                                                                                \n"
    "       vec3 position =  in_position();                                           \n"
    "       vec3 velocity = in_velocity();                                            \n"
    "       float is_under = -min(sign(position.y), 0.0);                             \n"
    "       float going_down = -min(sign(velocity.y), 0.0);                           \n"
    "       float acting = is_under * going_down;                                     \n"
    "       vec3 bounce_velocity = vec3(0.0, -velocity.y, 0.0);                       \n"
    "       vec3 friction_velocity = vec3(-velocity.x, 0.0, -velocity.z);             \n"
    "       vec3 bounce = acting * u_mass * bounce_velocity / u_interval;             \n"
    "       vec3 friction = is_under * u_mass * friction_velocity / u_interval;       \n"
    "       return 1.95 * bounce + 0.3 * friction;                                    \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 spring_force(vec3 p, vec3 ps, float k, float l)                             \n"
    "{                                                                                \n"
    "       vec3 v = ps - p;                                                          \n"
    "       float ds = (length(v) - l);                                               \n"
    "       return k * sign(ds) * min(abs(ds), l) * normalize(v);                     \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 spring_X(int spring_vertex_id, float k, float l)                            \n"
    "{                                                                                \n"
    "       if (spring_vertex_id < 0) return vec3(0.0, 0.0, 0.0);                     \n"
    "       else return spring_force(                                                 \n"
    "               in_position(),                                                    \n"
    "               positions[spring_vertex_id],                                      \n"
    "               k, l                                                              \n"
    "       );                                                                        \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 spring_X1(ivec4 indices, float k, float l)                                  \n"
    "{                                                                                \n"
    "       return  spring_X(indices.x, k, l*1)+                                      \n"
    "               spring_X(indices.y, k, l*1)+                                      \n"
    "               spring_X(indices.z, k, l*2)+                                      \n"
    "               spring_X(indices.w, k, l*2);                                      \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 spring_X2(ivec4 indices, float k, float l)                                  \n"
    "{                                                                                \n"
    "       return  spring_X(indices.x, k, l)+                                        \n"
    "               spring_X(indices.y, k, l)+                                        \n"
    "               spring_X(indices.z, k, l)+                                        \n"
    "               spring_X(indices.w, k, l);                                        \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 spring_A(ivec4 indices)                                                     \n"
    "{                                                                                \n"
    "       return spring_X1(                                                         \n"
    "               indices,                                                          \n"
    "               u_spring_a_strength,                                              \n"
    "               u_spring_a_length                                                 \n"
    "       );                                                                        \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 spring_B(ivec4 indices)                                                     \n"
    "{                                                                                \n"
    "       return spring_X2(                                                         \n"
    "               indices,                                                          \n"
    "               u_spring_b_strength,                                              \n"
    "               u_spring_b_length                                                 \n"
    "       );                                                                        \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 spring_C(ivec4 indices)                                                     \n"
    "{                                                                                \n"
    "       return spring_X2(                                                         \n"
    "               indices,                                                          \n"
    "               u_spring_c_strength,                                              \n"
    "               u_spring_c_length                                                 \n"
    "       );                                                                        \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 springs()                                                                   \n"
    "{                                                                                \n"
    "       uint index = gl_GlobalInvocationID.x;                                     \n"
    "       vec3 result = vec3(0.0, 0.0, 0.0);                                        \n"
    "       vertex_spring_indices vsi = vertex_springs[index];                        \n"
    "       result += spring_A(vsi.spring_ax);                                        \n"
    "       result += spring_A(vsi.spring_ay);                                        \n"
    "       result += spring_A(vsi.spring_az);                                        \n"
    "       result += spring_B(vsi.spring_bx);                                        \n"
    "       result += spring_B(vsi.spring_by);                                        \n"
    "       result += spring_B(vsi.spring_bz);                                        \n"
    "       result += spring_C(vsi.spring_cup);                                       \n"
    "       result += spring_C(vsi.spring_cdn);                                       \n"
    "       return result;                                                            \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 drag()                                                                      \n"
    "{                                                                                \n"
    "       return -0.002 * u_spring_a_length * in_velocity();                        \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "vec3 impulse()                                                                   \n"
    "{                                                                                \n"
    "       vec3 v = in_position() - u_impulse_center;                                \n"
    "       return u_impulse_strength * normalize(v)/length(v);                       \n"
    "}                                                                                \n"
    "                                                                                 \n"
    "void main()                                                                      \n"
    "{                                                                                \n"
    "       uint index = gl_GlobalInvocationID.x;                                     \n"
    "       vec3 force = drag() + gravity() + springs() + floor() + impulse();        \n"
    "       vec3 velocity = in_velocity() + (force * u_interval) / u_mass;            \n"
    "       b_tfb_velocity.v[index] = vec4(velocity, 1.0);                            \n"
    "       b_tfb_position.v[index] = vec4(in_position() + velocity * u_interval,1.0);\n"
    "}                                                                                \n"
};
	

const char *c_cube_vert =  {
    "#version 330                                                  \n"
    "                                                              \n"
    "uniform vec3 u_light_position;                                \n"
    "in vec3 a_position;                                           \n"
    "                                                              \n"
    "out vec3 g_light_dir;                                         \n"
    "                                                              \n"
    "void main()                                                   \n"
    "{                                                             \n"
    "       gl_Position = vec4(a_position, 1.0);                   \n"
    "       g_light_dir = normalize(u_light_position - a_position);\n"
    "}                                                             \n"
};

const char *c_cube_geom =  {
    "#version 330                                      \n"
    "                                                  \n"
    "layout(triangles_adjacency) in;                   \n"
    "layout(triangle_strip, max_vertices = 3) out;     \n"
    "                                                  \n"
    "uniform mat4 u_worldview;                         \n"
    "uniform mat4 u_viewsceen;                         \n"
    "                                                  \n"
    "in vec3 g_light_dir[6];                           \n"
    "                                                  \n"
    "out vec3 f_light_dir, f_normal;                   \n"
    "                                                  \n"
    "void make_vertex(int index)                       \n"
    "{                                                 \n"
    "       gl_Position =                              \n"
    "               u_viewsceen*                       \n"
    "               u_worldview*                       \n"
    "               gl_in[index].gl_Position;          \n"
    "       f_light_dir = g_light_dir[index];          \n"
    "       EmitVertex();                              \n"
    "}                                                 \n"
    "                                                  \n"
    "vec3 face_normal(int a, int b, int c)             \n"
    "{                                                 \n"
    "       return normalize(cross(                    \n"
    "               gl_in[c].gl_Position.xyz-          \n"
    "               gl_in[a].gl_Position.xyz,          \n"
    "               gl_in[b].gl_Position.xyz-          \n"
    "               gl_in[a].gl_Position.xyz           \n"
    "       ));                                        \n"
    "}                                                 \n"
    "                                                  \n"
    "void main()                                       \n"
    "{                                                 \n"
    "       vec3 fn  = face_normal(0, 2, 4);           \n"
    "       vec3 fn1 = face_normal(1, 2, 0);           \n"
    "       vec3 fn3 = face_normal(3, 4, 2);           \n"
    "       vec3 fn5 = face_normal(5, 0, 4);           \n"
    "       const float a = 0.400, b = (1.0 - a) * 0.5;\n"
    "       f_normal = fn*a + fn5*b + fn1*b;           \n"
    "       make_vertex(0);                            \n"
    "       f_normal = fn*a + fn1*b + fn3*b;           \n"
    "       make_vertex(2);                            \n"
    "       f_normal = fn*a + fn3*b + fn5*b;           \n"
    "       make_vertex(4);                            \n"
    "       EndPrimitive();                            \n"
    "}                                                 \n"
};

const char *c_cube_frag =  {
    "#version 330                                             \n"
    "                                                         \n"
    "uniform vec3 u_ambient_color;                            \n"
    "uniform vec3 u_albedo_color;                             \n"
    "uniform float u_light_multiplier;                        \n"
    "                                                         \n"
    "in vec3 f_light_dir, f_normal;                           \n"
    "                                                         \n"
    "out vec3 final_color;                                    \n"
    "                                                         \n"
    "void main()                                              \n"
    "{                                                        \n"
    "       float ambient = 0.8;                              \n"
    "       float diffuse = u_light_multiplier * sqrt(max(dot(\n"
    "               f_light_dir,                              \n"
    "               f_normal                                  \n"
    "       ) - 0.1, 0.0));                                   \n"
    "       final_color =                                     \n"
    "               ambient * u_ambient_color+                \n"
    "               diffuse * u_albedo_color;                 \n"
    "}                                                        \n"
};

const char *c_shadow_vert =  {
    "#version 330                                       \n"
    "                                                   \n"
    "uniform vec3 u_light_position;                     \n"
    "                                                   \n"
    "in vec3 a_position;                                \n"
    "                                                   \n"
    "out vec3 g_light_dir;                              \n"
    "                                                   \n"
    "void main()                                        \n"
    "{                                                  \n"
    "       gl_Position = vec4(a_position, 1.0);        \n"
    "       g_light_dir = u_light_position - a_position;\n"
    "}                                                  \n"
};

const char *c_shadow_geom =  {
    "#version 330                                   \n"
    "                                               \n"
    "layout(triangles_adjacency) in;                \n"
    "layout(triangle_strip, max_vertices = 12) out; \n"
    "                                               \n"
    "uniform mat4 u_worldview;                      \n"
    "uniform mat4 u_viewsceen;                      \n"
    "                                               \n"
    "in vec3 g_light_dir[6];                        \n"
    "                                               \n"
    "void make_near_vertex(int index)               \n"
    "{                                              \n"
    "       gl_Position =                           \n"
    "               u_viewsceen*                    \n"
    "               u_worldview*                    \n"
    "               gl_in[index].gl_Position;       \n"
    "       EmitVertex();                           \n"
    "}                                              \n"
    "                                               \n"
    "void make_far_vertex(int index)                \n"
    "{                                              \n"
    "       vec3 pos = gl_in[index].gl_Position.xyz;\n"
    "       pos -= g_light_dir[index];              \n"
    "       gl_Position =                           \n"
    "               u_viewsceen*                    \n"
    "               u_worldview*                    \n"
    "               vec4(pos, 1.0);                 \n"
    "       EmitVertex();                           \n"
    "}                                              \n"
    "                                               \n"
    "void make_plane(int a, int b)                  \n"
    "{                                              \n"
    "       make_near_vertex(a);                    \n"
    "       make_near_vertex(b);                    \n"
    "       make_far_vertex(a);                     \n"
    "       make_far_vertex(b);                     \n"
    "       EndPrimitive();                         \n"
    "}                                              \n"
    "                                               \n"
    "vec3 face_normal(int a, int b, int c)          \n"
    "{                                              \n"
    "       return cross(                           \n"
    "               gl_in[c].gl_Position.xyz-       \n"
    "               gl_in[a].gl_Position.xyz,       \n"
    "               gl_in[b].gl_Position.xyz-       \n"
    "               gl_in[a].gl_Position.xyz        \n"
    "       );                                      \n"
    "}                                              \n"
    "                                               \n"
    "vec3 face_light_dir(int a, int b, int c)       \n"
    "{                                              \n"
    "       return (                                \n"
    "               g_light_dir[a]+                 \n"
    "               g_light_dir[b]+                 \n"
    "               g_light_dir[c]                  \n"
    "       );                                      \n"
    "}                                              \n"
    "                                               \n"
    "void main()                                    \n"
    "{                                              \n"
    "       vec3 ld = face_light_dir(0, 2, 4);      \n"
    "       vec3 fn  = face_normal(0, 2, 4);        \n"
    "       if (dot(fn, ld) >= 0.0)                 \n"
    "       {                                       \n"
    "               make_plane(2, 0);               \n"
    "               make_plane(4, 2);               \n"
    "               make_plane(0, 4);               \n"
    "       }                                       \n"
    "}                                              \n"
};

/* clang-format on */
class BulletTimeTrigger {
public:
	void updateAndStartIf(double interval, bool can_start, double duration)
	{
		m_remaining -= interval;
		if (can_start && canRestart()) {
			m_remaining = duration;
		}
	}

	bool on()
	{
		if (started() && !m_status) {
			m_status = true;
			return true;
		}
		return false;
	}

	bool off()
	{
		if (!started() && m_status) {
			m_status = false;
			return true;
		}
		return false;
	}

private:
	double m_remaining = 0;
	bool m_status = false;

	bool started() const { return m_remaining > 0.0; }
	bool canRestart() const { return m_remaining < -2.0; }
};

class Uniforms {
public:
	Mat4f u_worldview;
	Mat4f u_viewsceen;

	Vec3f u_eye_position = ezero();
	Vec3f u_impulse_center = ezero();
	Vec3f u_ambient_color = ezero();
	Vec3f u_albedo_color = ezero();
	Vec3f u_light_position = ezero();

	float u_light_multiplier = 0.0;
	float u_interval = 0.0;
	float u_impulse_strength = 0.0;
	float u_mass = 0.0;
	float u_elevation = 0.0;
	float u_target_distance = 0.0;
	float u_spring_a_length = 0.0;
	float u_spring_b_length = 0.0;
	float u_spring_c_length = 0.0;
	float u_spring_a_strength = 0.0;
	float u_spring_b_strength = 0.0;
	float u_spring_c_strength = 0.0;

	int32_t u_cube_center_index = 0;
	int32_t u_texture = 0;

	struct {
		int32_t spring_ax[4];
		int32_t spring_ay[4];
		int32_t spring_az[4];
		int32_t spring_bx[4];
		int32_t spring_by[4];
		int32_t spring_bz[4];
		int32_t spring_c_up[4];
		int32_t spring_c_dn[4];
	} u_spring_index_block[512];

	Uniforms()
	{
		memset(u_spring_index_block, 0, sizeof(u_spring_index_block));

		m_attrs = {
		        {"u_worldview",          &u_worldview         },
		        {"u_viewsceen",          &u_viewsceen         },
		        {"u_eye_position",       &u_eye_position      },
		        {"u_impulse_center",     &u_impulse_center    },
		        {"u_ambient_color",      &u_ambient_color     },
		        {"u_albedo_color",       &u_albedo_color      },
		        {"u_light_multiplier",   &u_light_multiplier  },
		        {"u_interval",           &u_interval          },
		        {"u_impulse_strength",   &u_impulse_strength  },
		        {"u_mass",               &u_mass              },
		        {"u_light_position",     &u_light_position    },
		        {"u_elevation",          &u_elevation         },
		        {"u_target_distance",    &u_target_distance   },
		        {"u_spring_a_length",    &u_spring_a_length   },
		        {"u_spring_b_length",    &u_spring_b_length   },
		        {"u_spring_c_length",    &u_spring_c_length   },
		        {"u_spring_a_strength",  &u_spring_a_strength },
		        {"u_spring_b_strength",  &u_spring_b_strength },
		        {"u_spring_c_strength",  &u_spring_c_strength },
		        {"u_spring_index_block", &u_spring_index_block},
		        {"u_cube_center_index",  &u_cube_center_index },
		        {"u_texture",            &u_texture           },
		};
		/* don't register not to overwrite
		   emplace_back("u_position_block", &positions[0]);
		   emplace_back("u_cube_position_block", &cube_positions[0]);
		*/
	}

	void makeShader(SpuShader &shader, const char *vert, const char *geom, const char *frag) const
	{
		Attrs attrs = {
		        {"vert", vert},
		        {"geom", geom},
		        {"frag", frag},
		};
		shapes::loadShader(shader, attrs, Attrs(*this));
	}

	void makeComputeShader(SpuShader &shader, const char *comp) const
	{
		Attrs attrs = {
		        {"comp", comp},
		};
		shapes::loadShader(shader, attrs, Attrs(*this));
	}

	explicit operator Attrs() const { return m_attrs; }

private:
	Attrs m_attrs;
};

class FloorArray : public shapes::Array {
public:
	explicit FloorArray(Uniforms &unifs)
	{
		Attrs shader_attrs = {
		        {"frag", c_floor_frag},
		        {"vert", c_floor_vert},
		};
		Array::initShader(shader_attrs, Attrs(unifs));

		auto shape = shapes::Plane(Vec3f(200, 0, 0), Vec3f(0, 0, -200));

		Array::initArray(shape, {"position", "normal", "tangent", "texcoord"});

		auto image = images::BrushedMetalUByte(512, 512, 5120, -3, +3, 32, 128);

		Attrs attrs = {
		        {"target",     GL_TEXTURE_2D          },
                        {"iformat",    GL_RGB8                },
		        {"data",       image.data()           },
                        {"width",      512                    },
		        {"height",     512                    },
                        {"min_filter", GL_LINEAR_MIPMAP_LINEAR},
		        {"mag_filter", GL_LINEAR              },
		};
		m_texture.init(attrs);
		unifs.u_texture = m_texture.id();
	}

private:
	SpuTexture m_texture;
};

class CubeArray : public SpuArray {
public:
	uint32_t m_vertex_count;
	uint32_t m_index_count;
	int32_t n;

	CubeArray(int32_t n, Uniforms &unifs) : n(n)
	{
		// init
		{
			m_vertex_count = n * n * n;
			m_index_count = 6 * (n - 1) * (n * 4 + 1);
		}

		// program
		{
			unifs.makeShader(m_cubeShader, c_cube_vert, c_cube_geom, c_cube_frag);
			unifs.makeShader(m_shadowShader, c_shadow_vert, c_shadow_geom, nullptr);
		}

		// array
		{
			auto indices = makeIndices(1);  // next adjancency
			Attrs attrs = {
			        {"shader_id",    m_cubeShader.id()},
			        {"a.a_position", 4                },
			        {"nelem",        m_vertex_count   },
			};

			SpuArray::init(attrs);
			SpuArray::send(indices, -1);
			this->set(Attrs({Attr("restart", m_vertex_count)}));
		}

		// uniforms
		{
			memcpy(&unifs.u_spring_index_block, makeSprings().data(),
			       sizeof(unifs.u_spring_index_block));
		}
	}

	void link(const SpuArray &physics_array, int32_t slot) { SpuArray::link(0, physics_array, slot); }

	void draw()
	{
		m_cubeShader.use();
		SpuArray::draw(GL_TRIANGLE_STRIP_ADJACENCY, 0, 0, 1, 0, 0);
	}

	void drawShadow()
	{
		m_shadowShader.use();
		SpuArray::draw(GL_TRIANGLE_STRIP_ADJACENCY, 0, 0, 1, 0, 0);
	}

	std::vector<Vec4f> makePositions(const float size, const Mat4f &transform) const
	{
		std::vector<Vec4f> positions;

		const auto s_2 = size * 0.5f;
		const auto s_n = size / (n - 1);

		for (auto iz = 0; iz != n; iz++) {
			const auto z = -s_2 + s_n * iz;
			for (auto iy = 0; iy != n; iy++) {
				const auto y = -s_2 + s_n * iy;
				for (auto ix = 0; ix != n; ix++) {
					const auto x = -s_2 + s_n * ix;
					positions.emplace_back(transform * Vec4f(x, y, z, 1.0f));
				}
			}
		}
		assert(positions.size() == m_vertex_count);
		return positions;
	}

	std::vector<uint32_t> makeIndices(const int32_t k)
	{
		std::vector<uint32_t> indices(m_index_count);

		auto ip = begin(indices);

		for (auto f = 0; f != 6; ++f) {
			for (auto j = 0; j != n - 1; j += k) {
				*ip++ = getIndex(f, 0, j + 0);
				*ip++ = getIndex(f, -k, j + k);
				*ip++ = getIndex(f, 0, j + k);
				*ip++ = getIndex(f, +k, j - k);

				for (auto i = k; i != n - 1; i += k) {
					*ip++ = getIndex(f, i + 0, j + 0);
					*ip++ = getIndex(f, i - k, j + 2 * k);
					*ip++ = getIndex(f, i + 0, j + k);
					*ip++ = getIndex(f, i + k, j - k);
				}

				*ip++ = getIndex(f, n - 1 + 0, j + 0);
				*ip++ = getIndex(f, n - 1 - k, j + 2 * k);
				*ip++ = getIndex(f, n - 1 + 0, j + k);
				*ip++ = getIndex(f, n - 1 + k, j + 0);
				*ip++ = m_vertex_count;
			}
		}
		assert(ip - begin(indices) == m_index_count);
		return indices;
	}

	std::vector<int32_t> makeSprings()
	{
		// clang-format off
		std::vector<Vec4i> local_links = {
			// SpringAX
			{ -1, +0, +0, 0 }, { +1, +0, +0, 0 },
			{ -2, +0, +0, 0 }, { +2, +0, +0, 0 },
			// SpringAY
			{ +0, -1, +0, 0 }, { +0, +1, +0, 0 },
			{ +0, -2, +0, 0 }, { +0, +2, +0, 0 },
			// SpringAZ
			{ +0, +0, -1, 0 }, { +0, +0, +1, 0 },
			{ +0, +0, -2, 0 }, { +0, +0, +2, 0 },
			// SpringBX
			{ +0, +1, -1, 0 }, { +0, +1, +1, 0 },
			{ +0, -1, +1, 0 }, { +0, -1, -1, 0 },
			// SpringBY
			{ -1, +0, +1, 0 }, { +1, +0, +1, 0 },
			{ +1, +0, -1, 0 }, { -1, +0, -1, 0 },
			// SpringBZ
			{ +1, -1, +0, 0 }, { +1, +1, +0, 0 },
			{ -1, +1, +0, 0 }, { -1, -1, +0, 0 },
			// SpringCUp
			{ -1, +1, -1, 0 }, { +1, +1, -1, 0 },
			{ +1, +1, +1, 0 }, { -1, +1, +1, 0 },
			// SpringCDn
			{ -1, -1, -1, 0 }, { +1, -1, -1, 0 },
			{ +1, -1, +1, 0 }, { -1, -1, +1, 0 },
		};
		// clang-format on

		std::vector<int32_t> indices;
		for (auto z = 0; z != n; ++z) {
			for (auto y = 0; y != n; ++y) {
				for (auto x = 0; x != n; ++x) {
					std::vector<int32_t> links = getLinks(Vec4i(x, y, z, 0), local_links);
					indices.insert(end(indices), begin(links), end(links));
				}
			}
		}
		assert(indices.size() == size_t(m_vertex_count) * size_t(4 * 8));
		return indices;
	}

private:
	enum Flip {
		e_pi,  // i
		e_pj,  // j
		e_ni,  // n-1-i
		e_nj,  // n-1-j
	};

	SpuShader m_cubeShader;
	SpuShader m_shadowShader;

	int32_t flip(Flip f, int32_t i, int32_t j) const
	{
		switch (f) {
		case e_pi: return i;
		case e_pj: return j;
		case e_ni: return n - 1 - i;
		case e_nj: return n - 1 - j;
		default: assert(0);
		}
	}

	int32_t getIndex(const uint32_t f, int32_t i0, int32_t j0, Flip fi = e_pi, Flip fj = e_pj)
	{
		struct Adjency {
			int32_t f;
			Flip fi;
			Flip fj;
		};
		const Adjency adj_e[] = {
		        {4, e_pi, e_pj},
                        {5, e_pi, e_pj}, // +x -> +z, -x -> -z
		        {1, e_nj, e_pi},
                        {1, e_pj, e_ni}, // +y -> -x, -y -> -x
		        {1, e_pi, e_pj},
                        {0, e_pi, e_pj}, // +z -> -x, -z -> +x
		};
		const Adjency adj_w[] = {
		        {5, e_pi, e_pj},
                        {4, e_pi, e_pj}, // +x -> -z, -x -> +z
		        {0, e_pj, e_ni},
                        {0, e_nj, e_pi}, // +y -> +x, -y -> +x
		        {0, e_pi, e_pj},
                        {1, e_pi, e_pj}, // +z -> +x, -z -> -x
		};
		const Adjency adj_s[] = {
		        {3, e_pj, e_ni},
                        {3, e_nj, e_pi}, // +x -> -y, -x -> -y
		        {4, e_pi, e_pj},
                        {5, e_ni, e_nj}, // +y -> +z, -y -> -z
		        {3, e_pi, e_pj},
                        {3, e_ni, e_nj}, // +z -> -y, -z -> -y
		};
		const Adjency adj_n[] = {
		        {2, e_nj, e_pi},
                        {2, e_pj, e_ni}, // +x -> +y, -x -> +y
		        {5, e_ni, e_nj},
                        {4, e_pi, e_pj}, // +y -> -z, -y -> +z
		        {2, e_pi, e_pj},
                        {2, e_ni, e_nj}, // +z -> +y, -z -> +y
		};

		int32_t i = flip(fi, i0, j0);
		int32_t j = flip(fj, i0, j0);

		if (i < 0) {
			i += n - 1;
			const auto &adj = adj_e[f];
			return getIndex(adj.f, i, j, adj.fi, adj.fj);
		}
		if (i > n - 1) {
			i -= n - 1;
			const auto &adj = adj_w[f];
			return getIndex(adj.f, i, j, adj.fi, adj.fj);
		}
		if (j < 0) {
			j += n - 1;
			const auto &adj = adj_s[f];
			return getIndex(adj.f, i, j, adj.fi, adj.fj);
		}
		if (j > n - 1) {
			j -= n - 1;
			const auto &adj = adj_n[f];
			return getIndex(adj.f, i, j, adj.fi, adj.fj);
		}

		int32_t get_x[6] = {n - 1, 0, i, i, i, n - 1 - i};
		int32_t get_y[6] = {j, j, n - 1, 0, j, j};
		int32_t get_z[6] = {n - 1 - i, i, n - 1 - j, j, n - 1, 0};

		int32_t x = get_x[f];
		int32_t y = get_y[f];
		int32_t z = get_z[f];

		return z * n * n + y * n + x;
	}

	std::vector<int32_t> getLinks(const Vec4i &base, const std::vector<Vec4i> &links) const
	{
		std::vector<int32_t> indices;
		for (const auto &offset: links) {
			int32_t index = -1;
			Vec4i p = base + offset;
			if (0 <= p.x && p.x < n && 0 <= p.y && p.y < n && 0 <= p.z && p.z < n) {
				index = p.z * n * n + p.y * n + p.x;
			}
			indices.push_back(index);
		}
		return indices;
	}
};

class Camera {
public:
	explicit Camera(Uniforms &unifs)
	{
		// compute
		{
			auto &array = m_compArray;
			auto &shader = array.getShader();

			unifs.makeComputeShader(shader, c_camera_comp);
			unifs.u_elevation = 3.5;
			unifs.u_target_distance = 10.0;

			Vec4f positions[] = {
			        {0, unifs.u_elevation, unifs.u_target_distance, 1.0f},
			        {0, unifs.u_elevation, 0,                       1.0f},
			};

			Vec4f velocities[] = {
			        {0, 0, 0, 0},
			        {0, 0, 0, 0},
			};

			// input (position)
			Attrs attrs0 = {
			        {"shader_id",    shader.id()  },
			        {"a.a_position", 4            },
			        {"data",         &positions[0]},
			        {"nelem",        2            },
			};

			// input (velocity)
			Attrs attrs1 = {
			        {"a.a_velocity", 4             },
			        {"data",         &velocities[0]},
			        {"nelem",        2             },
			};
			// output (position)
			Attrs attrs2 = {
			        {"a.a_tfb_position", 4},
			        {"nelem",            2},
			};

			// output (velocity)
			Attrs attrs3 = {
			        {"a.a_tfb_velocity", 4},
			        {"nelem",            2},
			};

			array.aux(attrs0, 0);
			array.aux(attrs1, 1);
			array.aux(attrs2, 2);
			array.aux(attrs3, 3);
		}
	}

	const Vec3f &getPosition() const { return m_position; }
	const Vec3f &getTarget() const { return m_target; }

	Mat4f worldview() const { return math::lookat(m_position, m_target, ey()); }

	void link(const SpuShader &physics_shader, const char *name)
	{
		uint32_t ubo_id;
		spu_shader_loc(physics_shader.id(), &name, nullptr, nullptr, &ubo_id, 1);
		m_compArray.getShader().set("u_cube_position_block.buffer_id", &ubo_id);
	}

	void update(int32_t n)
	{
		for (auto i = 0; i < n; i++) {
			m_compArray.compute();
			m_compArray.copy(0, m_compArray, 2);
			m_compArray.copy(1, m_compArray, 3);
		}
		auto *ptr = m_compArray.map<float *>("r", 2);
		m_position = {ptr[0], ptr[1], ptr[2]};
		m_target = {ptr[4], ptr[5], ptr[6]};
		m_compArray.unmap(2);
	}

private:
	Vec3f m_position = ezero();
	Vec3f m_target = ezero();
	SpuComputeArray m_compArray;
};

class Physics {
public:
	SpuComputeArray &getArray() { return m_compArray; }

	Physics(const Uniforms &unifs, CubeArray &cube)
	{
		auto &array = m_compArray;
		auto &shader = array.getShader();
		// program
		unifs.makeComputeShader(shader, c_physics_comp);

		// array
		uint32_t ubo_buffer_id;
		const char *names = "u_position_block";
		spu_shader_loc(shader.id(), &names, nullptr, nullptr, &ubo_buffer_id, 1);

		const float size = 2.0;
		const auto transform = Mat4f().trans({0.0, 9.0, 0.0}) * Mat4f(Quatf(radians(44.0), eone()));

		const std::vector<Vec4f> positions = cube.makePositions(size, transform);
		const std::vector<Vec4f> velocities(positions.size(), ezero<Vec4f>());

		// input (position)
		Attrs attrs0 = {
		        {"shader_id",    shader.id()     },
                        {"a.a_position", 4               },
                        {"buffer_id",    ubo_buffer_id   },
		        {"data",         positions.data()},
                        {"nelem",        positions.size()},
		};
		array.aux(attrs0, 0);

		// input (velocity)
		Attrs attrs1 = {
		        {"a.a_velocity", 4                },
		        {"data",         velocities.data()},
		        {"nelem",        velocities.size()},
		};
		array.aux(attrs1, 1);

		// output (position)
		Attrs attrs2 = {
		        {"a.a_tfb_position", 4               },
		        {"data",             positions.data()},
		        {"nelem",            positions.size()},
		};
		array.aux(attrs2, 2);

		// output (velocity)
		Attrs attrs3 = {
		        {"a.a_tfb_velocity", 4                },
		        {"data",             velocities.data()},
		        {"nelem",            velocities.size()},
		};
		array.aux(attrs3, 3);
		array.getDim().x = (positions.size() + 63) / 64;  // local_size_x = 64
	}

	void update(int32_t n)
	{
		for (auto i = 0; i < n; i++) {
			m_compArray.compute();
			m_compArray.copy(0, m_compArray, 2);
			m_compArray.copy(1, m_compArray, 3);
		}
	}

private:
	SpuComputeArray m_compArray;
};

class App : public SpuPage {
public:
	const int32_t c_cube_dim = 8;

	double m_prevImpulseStrength = 0;
	Uniforms m_unifs;
	CubeArray m_cube;
	FloorArray m_floor;
	Camera m_camera;
	Physics m_physics;
	BulletTimeTrigger m_btTrg;

	int32_t m_bgstencil = 1;
	Vec4f m_bgcolor = {0.6, 0.6, 0.6, 0.0};
	float m_esec = 0;  // not seconds()

	App(const char *name)
	        : SpuPage(name, true), m_cube(c_cube_dim, m_unifs), m_floor(m_unifs), m_camera(m_unifs),
	          m_physics(m_unifs, m_cube)
	{
	}

	void init(const Attrs &attrs) override
	{
		SpuPage::init(attrs);

		// uniforms
		{
			const auto light_pos = Vec3f(20.0, 30.0, 25.0);
			const auto size = 2.0f;
			const auto mass = 0.5f;
			const auto s_n = size / (c_cube_dim - 1);
			const auto k = 1.1f;

			m_unifs.u_mass = mass / m_cube.m_vertex_count;
			m_unifs.u_spring_a_length = s_n;
			m_unifs.u_spring_b_length = sqrt(2.0) * s_n;
			m_unifs.u_spring_c_length = sqrt(3.0) * s_n;

			m_unifs.u_spring_a_strength = mass * 4.2 * k;
			m_unifs.u_spring_b_strength = mass * 4.0 * k;
			m_unifs.u_spring_c_strength = mass * 3.8 * k;
			m_unifs.u_cube_center_index = m_cube.m_vertex_count / 2;

			m_cube.link(m_physics.getArray(), 2);  // borrow slot #2

			m_camera.link(
			        m_physics.getArray().getShader(),
			        "u_position_block");  // borrow u_positon_block

			m_unifs.u_impulse_strength = 0.0;
			m_unifs.u_light_position = light_pos;
		}

		// renderstate
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.depth_test = true;
			renderstate.flags.cull_face = true;
			renderstate.depth_func = GL_LEQUAL;
			renderstate.flags.ccw = false;
			renderstate.cull_face = GL_BACK;
			renderstate.poly_offset = {-0.01, -1.0};
			// renderstate.use();
		}

		// bg
		{
			Attrs bg_attrs = {
			        {"bgcolor0",  m_bgcolor  },
			        {"bgstencil", m_bgstencil},
			};
			SpuPage::set(bg_attrs);
		}
	}

	void render() override
	{
		auto dsec = getSeconds().delta();
		if (dsec == 0) return;

		dsec = std::min(dsec, 0.1);

		// reshape
		m_unifs.u_viewsceen = math::perspective(viewport(0), 70, 1, 200);

		updateImpulse();
		updatePhysics(dsec);

		// update the 'bullet-time' trigger
		m_btTrg.updateAndStartIf(dsec, m_camera.getTarget().y < 1.0 && m_esec < 20, 1.5);

		// slow down the time if necessary
		if (m_btTrg.on()) {
			getSeconds().setPace(0.1);
		}

		// restore normal time flow if necessary
		if (m_btTrg.off()) {
			getSeconds().setPace(1.0);
		}

		m_unifs.u_worldview = m_camera.worldview();
		m_unifs.u_eye_position = m_camera.getPosition();
		m_unifs.u_light_multiplier = 0.2;

		// draw model
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.cull_face = GL_BACK;
			renderstate.write_mask = {1, 1, 1, 1, 1};
			renderstate.flags.stencil_test = false;
			renderstate.depth_func = GL_LEQUAL;

			renderstate.use();

			drawScene();
		}

		// draw shadow stencil
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.write_mask = {0, 0, 0, 0, 0};
			renderstate.flags.stencil_test = true;
			renderstate.stencil_func = {
			        GL_ALWAYS, 0, ~0u, GL_KEEP, GL_KEEP, GL_INCR,
			        GL_ALWAYS, 0, ~0u, GL_KEEP, GL_KEEP, GL_DECR,
			};
			renderstate.flags.cull_face = false;
			renderstate.use();

			m_cube.drawShadow();
		}

		// Draw stencilled parts of objects only with full light
		{
			auto &renderstate = SpuPage::getRenderstate();
			renderstate.flags.cull_face = true;
			renderstate.cull_face = GL_BACK;
			renderstate.write_mask = {1, 1, 1, 1, 1};
			renderstate.stencil_func = {
			        GL_EQUAL, 1, ~0u, GL_KEEP, GL_KEEP, GL_KEEP,
			        GL_EQUAL, 1, ~0u, GL_KEEP, GL_KEEP, GL_KEEP,
			};
			renderstate.use();

			m_unifs.u_light_multiplier = 1.0;
			drawScene();
		}
	}

	void updateImpulse()
	{
		// auto time  = esec;
		auto esec = getSeconds().current();
		auto impulse_strength = 100.0 * (-sin(esec / 5.0 * math::two_pi()) - 0.99);

		if (impulse_strength < 0.0) {
			impulse_strength = 0.0;
		}
		impulse_strength = pow(impulse_strength, 12.0) * 2.0;

		if (impulse_strength != 0.0) {
			if (m_prevImpulseStrength == 0.0) {
				auto pos = m_camera.getTarget();
				m_unifs.u_impulse_center
				        = {+4.5f - frand() * 9.0f + pos.x, -4.5f - frand(),
				           +4.5f - frand() * 9.0f + pos.z};
			}
			m_unifs.u_impulse_strength = impulse_strength;
		}
		m_prevImpulseStrength = impulse_strength;
	}

	void updatePhysics(double dsec)
	{
		// cube
		{
			const auto n = 20u;
			m_unifs.u_interval = dsec / n;
			m_physics.update(n);
		}
		// camera
		{
			const auto n = 10u;
			m_unifs.u_interval = dsec / n;
			m_camera.update(n);
		}
	}

	void drawScene()
	{
		m_floor.draw(nullptr);

		{
			m_unifs.u_ambient_color = {0.3, 0.4, 0.2};
			m_unifs.u_albedo_color = {0.6, 1.0, 0.4};
			m_cube.draw();
		}

		{
			SpuScopedRenderstate renderstate(true);
			renderstate.flags.line_offset = true;
			renderstate.flags.fill = false;
			renderstate.use();

			m_unifs.u_ambient_color = {0.1, 0.2, 0.0};
			m_unifs.u_albedo_color = {0.3, 0.4, 0.2};
			m_cube.draw();
		}
	}
};

static ObjectRegistry<SpuPage>::Creator<App> sandbox_creator("040_jelly_cube");
}  // namespace
}  // namespace spu::oglplus
