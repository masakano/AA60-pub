//
// App :
//
#include "base_app.h"
namespace spu {
namespace {
/* clang-format off */
const char *c_vert_read = {
    "#version 420 core                                                                      \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(mix(vec2(-1), vec2(3), bvec2(gl_VertexID == 1, gl_VertexID == 2)), 0, 1);\n"
    "}                                                                                      \n"
};

const char *c_frag_read = {
    "#version 420 core                                                                      \n"
    "#extension GL_ARB_shader_image_size : require                                          \n"
    "layout(binding = 0, rgba8) uniform image2D u_diffuse;                                  \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    ivec2 u = 7 * ivec2(gl_FragCoord.xy) / imageSize(u_diffuse);                       \n"
    "    color = imageLoad(u_diffuse, ivec2(gl_FragCoord.xy));                              \n"
    "    if ((u.y == 1 || u.y == 3 || u.y == 3 || u.y == 5) &&                              \n"
    "        (u.x == 1 || u.x == 3 || u.x == 3 || u.x == 5)) {                              \n"
    "        color *= 2.0;                                                                  \n"
    "    }                                                                                  \n"
    //"    color = vec4(1);                              \n"
    "}                                                                                      \n"
};
	
const char *c_vert_write = {
    "#version 420 core                                                                      \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position =                                                                      \n"
    "       vec4(mix(vec2(-1), vec2(3), bvec2(gl_VertexID == 1, gl_VertexID == 2)), 0, 1);  \n"
    "}                                                                                      \n"
};
	
const char *c_frag_write = {
    "#version 420 core                                                                      \n"
    "#extension GL_ARB_shader_image_size : require                                          \n"
    "layout(binding = 0, rgba8) uniform coherent image2D u_diffuse;                         \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    ivec2 size = imageSize(u_diffuse);                                                 \n"
    "    imageStore(                                                                        \n"
    "        u_diffuse,                                                                     \n"
    "        ivec2(gl_FragCoord.xy),                                                        \n"
    "        vec4(vec2(gl_FragCoord.xy) / vec2(size), 0.0, 1.0) * vec4(1.0, 0.5, 0.0, 1.0));\n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	SpuShader m_shaders[2];

	uint32_t u_diffuse;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, false) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture
		{
			Attrs attrs = {
			        {"target",     GL_TEXTURE_2D          },
			        {"iformat",    GL_RGBA8               },
			        {"width",      int32_t(viewport(0).sx)},
			        {"height",     int32_t(viewport(0).sy)},
			        {"swizzle_r",  GL_RED                 },
			        {"swizzle_g",  GL_GREEN               },
			        {"swizzle_b",  GL_BLUE                },
			        {"swizzle_a",  GL_ALPHA               },
			        {"base_level", 0                      },
			        {"max_level",  1                      },
			        {"mag_filter", GL_NEAREST             },
			        {"min_filter", GL_NEAREST             },
			};
			u_diffuse = spu_texture_new(attrs);
		}
		// program
		{
			const char *verts[] = {
			        c_vert_read,
			        c_vert_write,
			};

			const char *frags[] = {
			        c_frag_read,
			        c_frag_write,
			};

			for (auto i = 0; i < 2; i++) {
				Attrs shader_attrs = {
				        {"vert", verts[i]},
				        {"frag", frags[i]},
				};
				Attrs unif_attrs = {
				        {"u_diffuse", &u_diffuse}
                                };
				loadShader(m_shaders[i], shader_attrs, unif_attrs);
			}
		}
		// array
		{
			Attrs attrs = {
			        {"a.0",   1},
			        {"nelem", 3},
			};
			m_arrayId = spu_array_new(attrs);
		}
	}

	void render() override
	{
		clearImage();  // debug

		// Renderer to image
		{
			m_shaders[1].use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}

		// Read from image
		{
			m_shaders[0].use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}
	}

	void clearImage()
	{
		auto pix = 0x000000ffu;  // black
		spu_texture_send(u_diffuse, &pix, GL_RGBA8, nullptr, nullptr, true);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_430_image_store");
}  // namespace
}  // namespace spu
