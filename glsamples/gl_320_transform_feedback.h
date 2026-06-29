//
//
//
namespace {
/* clang-format off */
const char *c_vert_feed = {
    "#version 330                                                                           \n"
    "layout(std140, column_major) uniform;                                                  \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "layout (location = 0) in vec4 a_position;                                              \n"
    "out block                                                                              \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} Out;                                                                                 \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = u_worldscreen * a_position;                                           \n"
    "    Out.color = vec4(clamp(vec2(a_position), 0.0, 1.0), 0.0, 1.0);                     \n"
    "}                                                                                      \n"
};
	
const char *c_frag = {
    "#version 330                                                                           \n"
    "layout(std140, column_major) uniform;                                                  \n"
    "in block                                                                               \n"
    "{                                                                                      \n"
    "    vec4 color;                                                                        \n"
    "} In;                                                                                  \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = In.color;                                                                  \n"
    "}                                                                                      \n"
};
} // namespace
