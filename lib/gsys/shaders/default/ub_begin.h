//
//
//
#ifdef __GLSL__

#define UNIF_NAMESPACE(host_namespace)
#define UNIF_BEGIN(sym) uniform sym
#define UNIF_END(sym) sym
common:

#else

#include <smath/mat4f.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#define UNIF_NAMESPACE(host_namespace) namespace host_namespace {
#define UNIF_BEGIN(sym) struct sym
#define UNIF_END(sym)

#define vec4 vec4f_t
#define vec3 vec3f_t
#define vec2 vec2f_t
#define ivec4 vec4i_t
#define ivec3 vec3i_t
#define ivec2 vec2i_t
#define mat4 Mat4f
#define bool uint

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif
