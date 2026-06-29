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
    "    gl_Position = vec4(mix(                                                            \n"
    "             vec2(-1), vec2(3), bvec2(gl_VertexID == 1, gl_VertexID == 2)), 0,1);      \n"
    "}                                                                                      \n"
};

const char *c_frag_read = {
    "#version 420 core                                                                      \n"
    "layout(binding = 0, rgba8) coherent uniform image2D u_diffuse;                         \n"
    "layout(location = 0, index = 0) out vec4 color;                                        \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    color = imageLoad(u_diffuse, ivec2(gl_FragCoord.xy));                              \n"
    "}                                                                                      \n"
};

const char *c_vert_write = {
    "#version 420 core                                                                      \n"
    "const int vertexCount = 3;                                                             \n"
    "#if 1                                                                                  \n"
    "const vec2 positions[vertexCount] = vec2[](                                            \n"
    "    vec2(-1.0f,-1.0f),                                                                 \n"
    "    vec2( 1.0f,-1.0f),                                                                 \n"
    "    vec2(-1.0f, 1.0f));                                                                \n"
    "#else                                                                                  \n"
    "// original                                                                            \n"
    "const vec2 positions[vertexCount] = vec2[](                                            \n"
    "    vec2(-1.0f,-1.0f),                                                                 \n"
    "    vec2( 3.0f,-1.0f),                                                                 \n"
    "    vec2(-1.0f, 3.0f));                                                                \n"
    "#endif                                                                                 \n"
    "out gl_PerVertex                                                                       \n"
    "{                                                                                      \n"
    "    vec4 gl_Position;                                                                  \n"
    "};                                                                                     \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);                              \n"
    "}                                                                                      \n"
};

const char *c_frag_write = {
    "#version 420 core                                                                      \n"
    "layout(binding = 0, rgba8) coherent uniform image2D u_diffuse;                         \n"
    "void main()                                                                            \n"
    "{                                                                                      \n"
    "    imageStore(u_diffuse, ivec2(gl_FragCoord.xy), vec4(1.0, 0.5, 0.0, 1.0));           \n"
    "}                                                                                      \n"
};

/* clang-format on */
class App : public BaseApp {
public:
	uint32_t m_vertexArrayName;
	Vec4i m_imageSize;

	SpuShader m_shaders[2];  // 0:READ  1:SAVE
	uint32_t u_diffuse;
	uint32_t m_arrayId;

	App(const char *name) : BaseApp(name, true, {0.0, 0.5, 1.0, 1.0}) {}

	void init(const Attrs &attrs) override
	{
		BaseApp::init(attrs);

		// texture #0
		{
			Attrs attrs = {
			        {"target",      GL_TEXTURE_2D          },
			        {"iformat",     GL_RGBA8               },
			        {"width",       int32_t(viewport(0).sx)},
			        {"height",      int32_t(viewport(0).sy)},
			        {"base_level",  0                      },
			        {"max_level",   1                      },
			        {"mag_filter",  GL_NEAREST             },
			        {"min_filter",  GL_NEAREST             },
			        {"auto_mipmap", 0                      },
			};
			u_diffuse = spu_texture_new(attrs);
			m_imageSize = Vec4i(int32_t(viewport(0).sx), int32_t(viewport(0).sy), 0, 0);
		}

		// clear / send
		{
			auto tx = 16u;
			auto ty = 16u;
			std::vector<uint32_t> pix(tx * ty, 0xff007fff);

			// clear
			pix[0] = 0xff007f00;  // clear pixel
			spu_texture_send(u_diffuse, pix.data(), GL_RGBA8, nullptr, nullptr, true);

			// send
			for (auto oy = 0; oy < viewport(0).sy; oy += ty * 2) {
				for (auto ox = 0; ox < viewport(0).sx; ox += tx * 2) {
					int32_t dst_loc[4] = {ox, oy, 0, 0};
					uint32_t size[4] = {tx, ty, 1, 1};
					spu_texture_send(u_diffuse, pix.data(), GL_RGBA8, dst_loc, size);
				}
			}
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
				        {"u_diffuse", &u_diffuse},
				};
				loadShader(m_shaders[i], shader_attrs, unif_attrs);
			}

			auto access0 = GL_READ_ONLY;
			auto access1 = GL_WRITE_ONLY;

			spu_shader_set(m_shaders[0].id(), "u_diffuse.access", &access0);
			spu_shader_set(m_shaders[1].id(), "u_diffuse.access", &access1);
		}

		// array
		{
			Attrs attrs = {
			        {"nelem", 3},
			};
			m_arrayId = spu_array_new(attrs);
		}
	}

	void render() override
	{
		// Renderer to image
		{
			m_shaders[1].use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
		}

		// Read from image
		{
			setScissor(0, 32);
			m_shaders[0].use();
			spu_array_draw(m_arrayId, GL_TRIANGLES);
			setScissor(0, 0);
		}
	}

	// utility
	void setScissor(int32_t frame_id, float border)
	{
		auto w = viewport(0).sx;
		auto h = viewport(0).sy;

		auto scissor = Rectf(border, border, w - border * 2, h - border * 2);
		spu_frame_set(frame_id, "scissor0", scissor);
	}
};

static ObjectRegistry<SpuPage>::Creator<App> spu_sketch_creator("gl_420_image_store");
}  // namespace
}  // namespace spu
