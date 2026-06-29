//
//
//
#pragma once

#include "spu_texture_object.h"

// #define MAINTENANCE

namespace spu::libspu::spu_texture {

inline void texImageMutable_raw(
        uint32_t target, uint32_t level, uint32_t iformat, uint32_t pformat, uint32_t ptype, int32_t w,
        int32_t h, int32_t d, int32_t multi_sample, const void *pix)
{
#ifdef MAINTENANCE
	aux_printf("texImageMutable_raw:\n");
	aux_printf("    target  : %s\n", opengl_const(target));
	aux_printf("    level   : %d\n", level);
	aux_printf("    iformat : %s\n", opengl_const(iformat));
	aux_printf("    pformat : %s\n", opengl_const(pformat));
	aux_printf("    width   : %d\n", w);
	aux_printf("    height  : %d\n", h);
	aux_printf("    ptype   : %s\n", opengl_const(ptype));
	aux_printf("    pix     : %p\n", pix);
#endif

	// different from TexStorage
	if (iformat == GL_DEPTH_COMPONENT32F || iformat == GL_DEPTH_COMPONENT24
	    || iformat == GL_DEPTH_COMPONENT16) {
		iformat = GL_DEPTH_COMPONENT;
	}

	switch (target) {
	case GL_TEXTURE_1D:
		if (iformat) {
			F(glTexImage1D, target, level, iformat, w, 0, pformat, ptype, pix);
		}
		else {
			F(glTexSubImage1D, target, level, 0, w, pformat, ptype, pix);
		}
		break;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_RECTANGLE:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: {
		int32_t u = spu_gl_compress_ratio(pformat);
		if (u != 0) {  // compressed
			int32_t size = (w * h + 15) / 16 * u;
			if (iformat) {
				pformat = iformat;
			}
			F(glCompressedTexImage2D, target, level, pformat, w, h, 0, size, pix);
		}
		else if (iformat) {
			F(glTexImage2D, target, level, iformat, w, h, 0, pformat, ptype, pix);
		}
		else {
			F(glTexSubImage2D, target, level, 0, 0, w, h, pformat, ptype, pix);
		}
		break;
	}

	case GL_TEXTURE_2D_MULTISAMPLE:
		if (multi_sample <= 0) {
			aux_error(true, "%s: invalid multisample [%d]\n", opengl_const(target), multi_sample);
		}
		if (iformat) {
			F(glTexImage2DMultisample, target, multi_sample, iformat, w, h, GL_TRUE);
		}
		else {
			aux_error(true, "iformat specified target=[%s]\n", opengl_const(target));
		}
		break;

	case GL_TEXTURE_CUBE_MAP_ARRAY:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D: {
		int32_t u = spu_gl_compress_ratio(pformat);
		if (u != 0) {
			int32_t size = (w * h + 15) / 16 * u * d;

			aux_error(
			        iformat && iformat != pformat, "pformat mismatch (%s)\n",
			        opengl_const(pformat));
			F(glCompressedTexImage3D, target, level, pformat, w, h, d, 0, size, pix);
		}
		else if (iformat) {
			F(glTexImage3D, target, level, iformat, w, h, d, 0, pformat, ptype, pix);
		}
		else {
			F(glTexSubImage3D, target, level, 0, 0, 0, w, h, d, pformat, ptype, pix);
		}
		break;
	}
	default: aux_error(true, "unsupported target [%s]\n", opengl_const(target));
	}
}

inline void texStorage_raw(uint32_t target, uint32_t level, uint32_t iformat, int32_t w, int32_t h, int32_t d)
{
// #define MAINTENANCE
#ifdef MAINTENANCE
	aux_printf("texStorage_raw:\n");
	aux_printf("     target  : %s\n", opengl_const(target));
	aux_printf("     iformat : %s\n", opengl_const(iformat));
	aux_printf("     width   : %d\n", w);
	aux_printf("     height  : %d\n", h);
	aux_printf("     depth   : %d\n", d);
	aux_printf("     level   : %d\n", level);
#endif

	switch (target) {
	case GL_TEXTURE_1D: F(glTexStorage1D, target, level, iformat, w); break;
	case GL_TEXTURE_RECTANGLE:
	case GL_TEXTURE_2D:
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_CUBE_MAP: F(glTexStorage2D, target, level, iformat, w, h); break;
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		aux_error(
		        target == GL_TEXTURE_CUBE_MAP_ARRAY && d % 6 != 0,
		        "depth must be multiple of 6 in cubemap arry\n");

		F(glTexStorage3D, target, level, iformat, w, h, d);
		break;
	default: assert(0);
	}
}

inline void texImageImutable_raw(
        uint32_t target, uint32_t level, uint32_t pformat, uint32_t ptype, int32_t w, int32_t h, int32_t d,
        int32_t dx, int32_t dy, int32_t dz, const void *pix)
{
// #define MAINTENANCE
#ifdef MAINTENANCE
	aux_printf("glTexImageImutable_raw:\n");
	aux_printf("    target  : %s\n", opengl_const(target));
	aux_printf("    level   : %d\n", level);
	aux_printf("    dx      : %d\n", dx);
	aux_printf("    dy      : %d\n", dy);
	aux_printf("    dz      : %d\n", dz);
	aux_printf("    w       : %d\n", w);
	aux_printf("    h       : %d\n", h);
	aux_printf("    d       : %d\n", d);
	aux_printf("    pformat : %s\n", opengl_const(pformat));
	aux_printf("    ptype   : %s\n", opengl_const(ptype));
	aux_printf("    pix     : %p\n", pix);
#endif
	switch (target) {
	case GL_TEXTURE_1D: F(glTexSubImage1D, target, level, dx, w, pformat, ptype, pix); break;
	case GL_TEXTURE_RECTANGLE:
	case GL_TEXTURE_2D:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
		F(glTexSubImage2D, target, level, dx, dy, w, h, pformat, ptype, pix);
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_3D: F(glTexSubImage3D, target, level, dx, dy, dz, w, h, d, pformat, ptype, pix); break;
	default: aux_error(true, "unsupported target [%s]\n", opengl_const(target));
	}
}

inline void texCopyImage_raw(
        const Handle &src_handle, int32_t src_level, int32_t sx, int32_t sy, int32_t sz,
        const Handle &dst_handle, int32_t dst_level, int32_t dx, int32_t dy, int32_t dz, int32_t w, int32_t h,
        int32_t d)
{
#ifdef MAINTENANCE
	aux_printf("TexCopyImage_raw:\n");
	aux_printf("   src.id      : %d\n", src_handle.id);
	aux_printf("   src.target  : %s\n", opengl_const(src_handle.target));
	aux_printf("   src.l,x,y,z : %d x %d x %d x %d\n", src_level, sx, sy, sz);
	aux_printf("   dst.id      : %d\n", dst_handle.id);
	aux_printf("   dst.target  : %s\n", opengl_const(dst_handle.target));
	aux_printf("   dst.l,x,y,z : %d x %d x %d x %d\n", dst_level, dx, dy, dz);
	aux_printf("   width       : %d\n", w);
	aux_printf("   height      : %d\n", h);
	aux_printf("   depth       : %d\n", d);
	aux_printf("\n");
#endif
	if (src_handle.id == 0) {
		F(glBindTexture, dst_handle.target, dst_handle.id);
		F(glCopyTexSubImage2D, dst_handle.target, dst_level, dx, dy, sx, sy, w, h);
	}
	else {
		F(glCopyImageSubData, src_handle.id, src_handle.target, src_level, sx, sy, sz, dst_handle.id,
		  dst_handle.target, dst_level, dx, dy, dz, w, h, d);
	}
}
}  // namespace spu::libspu::spu_texture
