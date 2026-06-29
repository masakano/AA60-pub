//
// EglDevice :
//
#ifndef _WIN32
#include "dev.h"
#include <EGL/egl.h>
#include <spu/GL/gl.h>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

namespace spu::libspu::egl {

namespace {

DeviceConfig *s_dev_config = nullptr;
SpuPad s_pad;
EGLDisplay s_display = EGL_NO_DISPLAY;
EGLSurface s_surface;
EGLContext s_context = EGL_NO_CONTEXT;
}  // namespace

void EglDevice::init(DeviceConfig *dev_config)
{
	s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	aux_error(s_display == EGL_NO_DISPLAY, "failed to eglGetDisplay\n");

	EGLint major;
	EGLint minor;
	EGLBoolean eglStatus = eglInitialize(s_display, &major, &minor);

	aux_error(eglStatus == EGL_FALSE, "failed to eglInitialize\n");
	aux_message(0, "EGL Version %d.%d\n", major, minor);

	s_dev_config = dev_config;
	s_dev_config->name = "egl";
}

void EglDevice::terminate()
{
	// 6. Terminate EGL when finished
	eglTerminate(s_display);
	s_display = EGL_NO_DISPLAY;
	s_context = EGL_NO_CONTEXT;
}

void EglDevice::open(const Attrs &attrs)
{
	const vec4i_t def_window = {0, 0, 640, 480};
	vec4i_t window = attrs.get("window", def_window);

	s_pad.winsize[0] = window.z;
	s_pad.winsize[1] = window.w;

	// 2. RGBやdepthバッファのビット数等の設定を選択 (temporary)
	static const EGLint configAttribs[] = {EGL_SURFACE_TYPE,
	                                       EGL_PBUFFER_BIT,
	                                       EGL_BLUE_SIZE,
	                                       8,
	                                       EGL_GREEN_SIZE,
	                                       8,
	                                       EGL_RED_SIZE,
	                                       8,
	                                       EGL_DEPTH_SIZE,
	                                       8,
	                                       EGL_RENDERABLE_TYPE,
	                                       EGL_OPENGL_BIT,
	                                       EGL_NONE};
	EGLint numConfigs;
	EGLConfig eglCfg;
	EGLBoolean eglStatus = eglChooseConfig(s_display, configAttribs, &eglCfg, 1, &numConfigs);
	aux_error(eglStatus == EGL_FALSE, "failed to eglChooseConfig\n");

	// 3. サーフェスの生成
	const EGLint pbufferAttribs[] = {EGL_WIDTH, s_pad.winsize[0], EGL_HEIGHT, s_pad.winsize[1], EGL_NONE};
	s_surface = eglCreatePbufferSurface(s_display, eglCfg, pbufferAttribs);
	aux_error(s_surface == EGL_NO_SURFACE, "failed to eglCreatePbufferSurface\n");

	// 4. API（OpenGL）のバインド
	eglStatus = eglBindAPI(EGL_OPENGL_API);
	aux_error(eglStatus == EGL_FALSE, "failed to eglBindAPI\n");

	// 5. コンテキストを生成し、カレントにする
	s_context = eglCreateContext(s_display, eglCfg, EGL_NO_CONTEXT, nullptr);
	aux_error(s_context == EGL_NO_CONTEXT, "failed to eglCreateContext\n");

	eglStatus = eglMakeCurrent(s_display, s_surface, s_surface, s_context);
	aux_error(eglStatus == EGL_FALSE, "failed to eglMakeCurrent\n");

	s_dev_config->sync_pad(s_pad);
	s_dev_config->sync_pad(s_pad);
}

void EglDevice::close() { /* do nothing */ }

bool EglDevice::swap()
{
	glFlush();
	s_pad.swap_count++;
	s_dev_config->sync_pad(s_pad);
	return true;
}

void EglDevice::poll() { /* do nothing */ }
int32_t EglDevice::get(const hash32_t & /*key*/, void * /*value*/) { return 0; }
}  // namespace spu::libspu::egl
#endif
