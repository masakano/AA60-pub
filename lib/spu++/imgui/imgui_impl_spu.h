//
//
//
#pragma once
struct GLFWwindow;

IMGUI_API bool ImGui_ImplSpu_Init(
        GLFWwindow* window = nullptr, bool install_callbacks = false, const char* glsl_version = NULL);

IMGUI_API void ImGui_ImplSpu_Shutdown();

IMGUI_API void ImGui_ImplSpu_NewFrame();

IMGUI_API void ImGui_ImplSpu_RenderDrawData(ImDrawData* draw_data);

IMGUI_API void ImGui_ImplSpu_InvalidateDeviceObjects();

IMGUI_API bool ImGui_ImplSpu_CreateDeviceObjects();

IMGUI_API void ImGui_ImplSpu_MouseButtonCallback(
        GLFWwindow* window = nullptr, int button = 0, int action = 0, int mods = 0);

IMGUI_API void ImGui_ImplSpu_ScrollCallback(
        GLFWwindow* window = nullptr, double xoffset = 0, double yoffset = 0);

IMGUI_API void ImGui_ImplSpu_KeyCallback(
        GLFWwindow* window = nullptr, int key = 0, int scancode = 0, int action = 0, int mods = 0);

IMGUI_API void ImGui_ImplSpu_CharCallback(GLFWwindow* window = nullptr, unsigned int c = 0);
