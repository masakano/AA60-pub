//
// SpuPad :
//
// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an
// example of using this. If you use this binding you'll need to call 4 functions:
// ImGui_ImplXXXX_Init(), ImGui_ImplXXXX_NewFrame(), ImGui::Render() and
// ImGui_ImplXXXX_Shutdown(). If you are new to ImGui, see examples/README.txt and documentation
// at the top of imgui.cpp. https://github.com/ocornut/imgui

#include <spu++/spu++.h>
#include "imgui.h"
#include "imgui_impl_spu.h"

using namespace spu;

// Data
namespace {
//SpuRenderstate s_renderstate;
SpuGesture s_gesture;
SpuShader s_shader;
SpuArray s_array;
SpuTexture s_fontTexture;
double s_time = 0.0f;
bool s_mousePressed[3] = {false, false, false};
float s_mouseWheel = 0.0f;

Mat4f u_proj_matrix;
uint32_t u_texture;

struct SpuKeyMap {
	ImGuiKey imgui_key;
	int16_t spu_key;
};

constexpr SpuKeyMap c_key_maps[] = {
        {ImGuiKey_Tab,        '\t'                   },
        {ImGuiKey_LeftArrow,  spu::SpuPad::e_left    },
        {ImGuiKey_RightArrow, spu::SpuPad::e_right   },
        {ImGuiKey_UpArrow,    spu::SpuPad::e_up      },
        {ImGuiKey_DownArrow,  spu::SpuPad::e_down    },
        {ImGuiKey_PageUp,     spu::SpuPad::e_pageup  },
        {ImGuiKey_PageDown,   spu::SpuPad::e_pagedown},
        {ImGuiKey_Home,       spu::SpuPad::e_home    },
        {ImGuiKey_End,        spu::SpuPad::e_end     },
        {ImGuiKey_Delete,     spu::SpuPad::e_delete  },
        {ImGuiKey_Backspace,  '\b'                   },
        {ImGuiKey_Enter,      '\n'                   },
        {ImGuiKey_Escape,     spu::SpuPad::e_escape  },
        {ImGuiKey_A,          'a'                    },
        {ImGuiKey_C,          'c'                    },
        {ImGuiKey_V,          'v'                    },
        {ImGuiKey_X,          'x'                    },
        {ImGuiKey_Y,          'y'                    },
        {ImGuiKey_Z,          'z'                    },
};
};  // namespace

// This is the main rendering function that you have to implement and provide to ImGui (via
// setting up 'RenderDrawListsFn' in the ImGuiIO structure) If text or lines are blurry when
// integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or
// (0.375f,0.375f)
void ImGui_ImplSpu_RenderDrawData(ImDrawData* draw_data)
{
	SpuScopedRenderstate renderstate(1);
	renderstate = SpuRenderstate();

	// Avoid rendering when minimized, scale coordinates for retina displays (screen
	auto& io = ImGui::GetIO();
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// Setup render state: alpha-blending enabled, no face culling, no depth testing,
	// scissor enabled

	renderstate.flags.blend = true;
	renderstate.flags.cull_face = false;
	renderstate.flags.depth_test = false;
	renderstate.flags.scissor_test = true;

	renderstate.blend_eq = {
	        GL_FUNC_ADD,
	        GL_FUNC_ADD,
	};
	renderstate.blend_func = {
	        GL_SRC_ALPHA,
	        GL_ONE_MINUS_SRC_ALPHA,
	        GL_SRC_ALPHA,
	        GL_ONE_MINUS_SRC_ALPHA,
	};
	renderstate.use();

	float sx = io.DisplaySize.x;
	float sy = io.DisplaySize.y;

	u_proj_matrix = {
	        {2.0f / sx, 0.0f,       0.0f,  0.0f},
	        {0.0f,      2.0f / -sy, 0.0f,  0.0f},
	        {0.0f,      0.0f,       -1.0f, 0.0f},
	        {-1.0f,     1.0f,       0.0f,  1.0f},
	};

	// uint32_t srgb_save = 0;
	vec4f_t scissor_save;

	// spu_frame_get(-1, "srgb", &srgb_save);
	spu_frame_get(-1, "scissor0", &scissor_save);  // not correct?
	// spu_frame_set(-1, "srgb", 0);

	ImVec2 clip_off = draw_data->DisplayPos;  // (0,0) unless using multi-viewports
	ImVec2 clip_scale
	        = draw_data->FramebufferScale;  // (1,1) unless using retina display which are often (2,2)

	for (auto n = 0; n < draw_data->CmdListsCount; n++) {
		const auto* cmd_list = draw_data->CmdLists[n];
		auto idx_buffer_offset = 0;

		s_array.send(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size, 0);
		s_array.send(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size, -1, sizeof(ImDrawIdx));

		for (auto cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const auto* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback) {
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else {
				// Project scissor/clipping rectangles into framebuffer space
				ImVec2 clip_min
				        = {(pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
				           (pcmd->ClipRect.y - clip_off.y) * clip_scale.y};

				ImVec2 clip_max
				        = {(pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
				           (pcmd->ClipRect.w - clip_off.y) * clip_scale.y};

				if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y) {
					continue;
				}

				vec4f_t scissor = {
				        clip_min.x,
				        sy - clip_max.y,
				        clip_max.x - clip_min.x,
				        clip_max.y - clip_min.y,
				};

				Attrs frame_attrs = {
				        {"scissor0", scissor},
				};
				spu_frame_set(-1, frame_attrs);

				u_texture = (uint32_t)(intptr_t)pcmd->GetTexID();
				// patch
				if ((u_texture & 0xffff0000) == 0) {
					// safety
					u_texture |= (GL_TEXTURE_2D << 16);
					Attrs attrs = {
					        {"texture_id", u_texture},
					};
					u_texture = spu_texture_new(attrs);
				}
				s_shader.use();
				s_array.draw(GL_TRIANGLES, idx_buffer_offset, pcmd->ElemCount);
			}
			idx_buffer_offset += pcmd->ElemCount;
		}
	}
	Attrs frame_unset_attrs = {
	        //{"srgb",     srgb_save   },
	        {"scissor0", scissor_save},
	};
	spu_frame_set(-1, frame_unset_attrs);
	spu_shader_use(0);  // reset shader
}

const char* ImGui_ImplSpu_GetClipboardText(void*)
{
	aux_message(0, "GetClipBoardText not supported\n");
	return 0;
}

void ImGui_ImplSpu_SetClipboardText(void*, const char*) { aux_message(0, "SetClipBoardText not supported\n"); }

void ImGui_ImplSpu_MouseButtonCallback(GLFWwindow*, int, int, int)
{
	auto& curr = s_gesture.curr();
	auto& prev = s_gesture.curr();
	auto is_dras_L = prev.mouse_L && curr.mouse_L;
	auto is_dras_R = prev.mouse_R && curr.mouse_R;

	if (is_dras_L && is_dras_R) {
		s_mousePressed[2] = true;
	}
	else if (is_dras_L) {
		s_mousePressed[0] = true;
	}
	else if (is_dras_R) {
		s_mousePressed[1] = true;
	}
}

void ImGui_ImplSpu_ScrollCallback(GLFWwindow*, double, double)
{
	s_mouseWheel += s_gesture.wheel();  // Use fractional mouse wheel, 1.0 unit 5 lines.
}

void ImGui_ImplSpu_KeyCallback(GLFWwindow*, int, int, int, int)
{
	auto& io = ImGui::GetIO();

	for (auto key_map: c_key_maps) {
		io.AddKeyEvent(key_map.imgui_key, s_gesture.press(key_map.spu_key));
	}
	auto& pad = s_gesture.curr();
	io.AddKeyEvent(ImGuiMod_Ctrl, pad.key_ctrl);
	io.AddKeyEvent(ImGuiMod_Shift, pad.key_shift);
	io.AddKeyEvent(ImGuiMod_Alt, pad.key_alt);
	io.AddKeyEvent(ImGuiMod_Super, pad.key_alt && pad.key_shift);  // tentative
}

void ImGui_ImplSpu_CharCallback(GLFWwindow*, unsigned int)
{
	auto& io = ImGui::GetIO();
	for (auto key = 1; key < 256; key++) {
		if (s_gesture.pressed(key)) {
			io.AddInputCharacter(key);
		}
	}
}

bool ImGui_ImplSpu_CreateFontsTexture(void)
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;

	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	// Load as RGBA 32-bits (75% of the memory is wasted, but default
	// font is so small) because it is more likely to be compatible with
	// user's existing shaders. If your ImTextureId represent a
	// higher-level concept than just a GL texture id, consider calling
	// GetTexDataAsAlpha8() instead to save on GPU memory.
	Attrs texture_attrs = {
	        {"target",     GL_TEXTURE_2D},
                {"iformat",    GL_RGBA8     },
                {"width",      width        },
	        {"height",     height       },
                {"min_filter", GL_LINEAR    },
                {"mas_filter", GL_LINEAR    },
	        {"data",       (void*)pixels},
                {"max_level",  0            },
	};
	s_fontTexture.init(texture_attrs);

	// Store our identifier
	io.Fonts->SetTexID((ImTextureID)(intptr_t)s_fontTexture.id());
	return true;
}

bool ImGui_ImplSpu_CreateDeviceObjects(void)
{
	const GLchar* vertex_shader
	        = "#version 330\n"
	          "uniform mat4 u_proj_matrix;\n"
	          "in vec2 a_position;\n"
	          "in vec2 a_uv;\n"
	          "in vec4 a_color;\n"
	          "out vec2 f_uv;\n"
	          "out vec4 f_color;\n"
	          "void main()\n"
	          "{\n"
	          "	f_uv = a_uv;\n"
	          "	f_color = a_color;\n"
	          "	gl_Position = u_proj_matrix * vec4(a_position.xy,0,1);\n"
	          "}\n";

	const GLchar* fragment_shader
	        = "#version 330\n"
	          "uniform sampler2D u_texture;\n"
	          "in vec2 f_uv;\n"
	          "in vec4 f_color;\n"
	          "out vec4 final_color;\n"
	          "void main()\n"
	          "{\n"
	          "	final_color = f_color * texture( u_texture, f_uv.st);\n"
	          "}\n";

	Attrs shader_attrs = {
	        {"vert", vertex_shader  },
	        {"frag", fragment_shader},
	};

	Attrs unif_attrs = {
	        {"u_proj_matrix", &u_proj_matrix},
	        {"u_texture",     &u_texture    },
	};
	s_shader.init(shader_attrs);
	s_shader.addUniforms(unif_attrs);

	Attrs array_attrs = {
	        {"shader_id",    s_shader.id()   },
                {"a.a_position", 2               },
                {"a.a_uv",       2               },
	        {"format",       GL_UNSIGNED_BYTE},
                {"normalize",    1               },
                {"a.a_color",    4               },
	};
	s_array.init(array_attrs);
	ImGui_ImplSpu_CreateFontsTexture();
	return true;
}

void ImGui_ImplSpu_InvalidateDeviceObjects()
{
	s_array.dispose();
	s_shader.dispose();
	s_fontTexture.dispose();
	ImGui::GetIO().Fonts->SetTexID(ImTextureID_Invalid);
}

bool ImGui_ImplSpu_Init(GLFWwindow*, bool, const char*)
{
	ImGuiIO& io = ImGui::GetIO();

	// Alternatively you can set this to NULL and call ImGui::GetDrawData() after
	// ImGui::Render() to get the same ImDrawData pointer.

	// io.RenderDrawListsFn = ImGui_ImplSpu_RenderDrawLists;
	io.SetClipboardTextFn = ImGui_ImplSpu_SetClipboardText;
	io.GetClipboardTextFn = ImGui_ImplSpu_GetClipboardText;

#if 0  // unsuported
	io.ClipboardUserData = s_Window;
#endif
	// gestre
	s_gesture.init(nullptr);

	return true;
}

void ImGui_ImplSpu_Shutdown()
{
	ImGui_ImplSpu_InvalidateDeviceObjects();
	// ImGui::Shutdown(); // imgul 1.72
}

void ImGui_ImplSpu_NewFrame()
{
	s_gesture.update();

#if 0	
	if (s_fontTexture.id() == 0) {
		ImGui_ImplSpu_CreateDeviceObjects();
	}
#endif
	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	auto w = s_gesture.curr().winsize[0];
	auto h = s_gesture.curr().winsize[1];
	auto display_w = w;
	auto display_h = h;

	io.DisplaySize = ImVec2((float)w, (float)h);
	io.DisplayFramebufferScale
	        = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

	auto current_time = (double)spu::get_microsec() / 1000000;
	io.DeltaTime = s_time > 0.0 ? (float)(current_time - s_time) : (float)(1.0f / 60.0f);
	s_time = current_time;

	auto& curr = s_gesture.curr();
	io.MousePos = ImVec2(curr.cursor[0], curr.winsize[1] - curr.cursor[1]);

	for (auto i = 0; i < 3; i++) {
		io.MouseDown[i] = s_mousePressed[i];
		s_mousePressed[i] = false;
	}

	io.MouseWheel = s_mouseWheel;
	s_mouseWheel = 0.0f;

	// Hide OS mouse cursor if ImGui is drawing it
	/* unsupported  */
	ImGui_ImplSpu_MouseButtonCallback();
	ImGui_ImplSpu_ScrollCallback();
	ImGui_ImplSpu_KeyCallback();
	ImGui_ImplSpu_CharCallback();
	// Start the frame
	ImGui::NewFrame();
#if 0
	if (ImGui::IsAnyItemHovered()) {
		SpuGesture::setGrab(&s_gesture);
	}
	else if (SpuGesture::getGrab() == &s_gesture) {
		SpuGesture::setGrab(nullptr);
	}
#endif
}
