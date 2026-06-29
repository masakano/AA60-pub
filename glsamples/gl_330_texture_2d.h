//
//
//
namespace {
/* clang-format off */
const char *c_vert = {
    "#version 330 core                                                                      \n"
    "#define POSITION    0                                                                  \n"
    "#define TEXCOORD    4                                                                  \n"
    "layout(std140, column_major) uniform;                                                  \n"
    "uniform mat4 u_worldscreen;                                                             \n"
    "layout(location = POSITION) in vec2 a_position;                                        \n"
    "layout(location = TEXCOORD) in vec2 a_texcoord;                                        \n"
    "out vec2 f_texcoord;                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    f_texcoord = a_texcoord;                                                         \n"
    "    gl_Position = u_worldscreen * vec4(a_position, 0.0, 1.0);                           \n"
    "}                                                                                      \n"
};
const char *c_frag = {
    "#version 330 core                                                                      \n"
    "#define FRAG_COLOR    0                                                                \n"
    "layout(std140, column_major) uniform;                                                  \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                     \n"
    "layout(location = FRAG_COLOR, index = 0) out vec4 Color;                               \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    Color = texture(u_diffuse, f_texcoord);                                           \n"
    "}                                                                                      \n"
};
} // namespace
