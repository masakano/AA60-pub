//
//
//
namespace {
/* clang-format off */
const char *c_vert = {
    "#version 330                                                       \n"
    "uniform mat4 u_worldscreen;                                               \n"
    "in vec2 a_position;						\n"
    "in vec2 a_texcoord;						\n"
    "out vec2 f_texcoord;                                                 \n"
    "void main()                                                        \n"
    "{                                                                  \n"
    "    f_texcoord = a_texcoord;                                     \n"
    "    gl_Position = u_worldscreen * vec4(a_position,0.0,1.0);        \n"
    "}                                                                  \n"
};

const char *c_frag = {
    "#version 330                                                                           \n"
    "uniform sampler2D u_diffuse;                                                           \n"
    "in vec2 f_texcoord;                                                                     \n"
    "out vec4 color;                                                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = textureLod(u_diffuse, f_texcoord, 0);                                     \n"
    "}                                                                                      \n"
};
} // namespace
