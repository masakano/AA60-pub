//
//
//
#pragma once

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include "block.h"
#include "const.h"
#include "translate_tab.h"
#include <spu++/dds/dds.h>

namespace spu::dds {

inline bool Image::setFormat(uint32_t fourcc)
{
	for (auto &tab: c_format_tab) {
		if (tab.fourcc == fourcc) {
			m_iformat = tab.iformat;
			m_pformat = tab.pformat;
			m_type = tab.type;
			m_elementSize = tab.size;
			m_isCompressed = false;
			return true;
		}
	}
	for (auto &tab: c_compressed_format_tab) {
		if (tab.fourcc == fourcc) {
			m_iformat = tab.iformat;
			m_pformat = tab.iformat;
			m_type = GL_UNSIGNED_BYTE;
			m_elementSize = tab.size;
			m_isCompressed = true;
			return true;
		}
	}
	return false;
}

inline void Image::parseDDS(const void *ddsh_ptr, const void *ddsh10_ptr)
{
	const DDS_HEADER *ddsh = (DDS_HEADER *)ddsh_ptr;
	const DDS_HEADER_10 *ddsh10 = (DDS_HEADER_10 *)ddsh10_ptr;

	// figure out what the image format is
	if ((ddsh->ddspf.dwFlags & DDSF_FOURCC) != 0u) {
		if (!setFormat(ddsh->ddspf.dwFourCC)) {
			if (!parseDX10(ddsh10)) {
				aux_error(1, "unsupported format\n");
			}
		}
	}
	else {
		bool is_found = false;
		for (auto &tab: c_color_mask_tab) {
			if ((tab.flags == 0 || tab.flags == ddsh->ddspf.dwFlags)
			    && (tab.bitcount == ddsh->ddspf.dwRGBBitCount)
			    && ((tab.maskR == 0 && tab.maskG == 0 && tab.maskB == 0 && tab.maskA == 0)
			        || (tab.maskR == ddsh->ddspf.dwRBitMask && tab.maskG == ddsh->ddspf.dwGBitMask
			            && tab.maskB == ddsh->ddspf.dwBBitMask
			            && tab.maskA == ddsh->ddspf.dwABitMask))) {
				is_found = true;

				m_iformat = tab.iformat;
				m_pformat = tab.pformat;
				m_type = tab.type;
				m_elementSize = tab.size;
				m_isCompressed = false;
				break;
			}
		}
		aux_error(!is_found, "error in decoding DDS file\n");
	}
}

inline bool Image::parseDX10(const void *ptr)
{
	const DDS_HEADER_10 &header = *(const DDS_HEADER_10 *)ptr;

	switch (header.resourceDimension) {
	case DDS10_RESOURCE_DIMENSION_TEXTURE1D:
	case DDS10_RESOURCE_DIMENSION_TEXTURE2D:
	case DDS10_RESOURCE_DIMENSION_TEXTURE3D:
		// do I really need to do anything here ?
		break;
	case DDS10_RESOURCE_DIMENSION_UNKNOWN:
	case DDS10_RESOURCE_DIMENSION_BUFFER:
	default:
		// unsupported formats
		return false;
	};

	if (!setFormat(header.dxgiFormat)) {
		return false;
	}

	m_layers = header.arraySize;
	m_isCubemap = (header.miscFlag & 0x4) != 0;

	return true;
}
}  // namespace spu::dds

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
