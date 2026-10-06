/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#include <wg_c_gfxutil.h>
#include <wg_gfxutil.h>

#include <cstddef>

using namespace wg;

static_assert( sizeof(wg_pixelDescription) == sizeof(PixelDescription) && offsetof(wg_pixelDescription, R_mask) == offsetof(PixelDescription, R_mask),
			   "wg_pixelDescription out of sync with PixelDescription" );
static_assert( (int) WG_PIXFMT_ARGB_16 == (int) PixelFormat::ARGB_16 && (int) WG_PIXFMT_BGR_565 == (int) PixelFormat::BGR_565,
			   "wg_pixelFormat out of sync with PixelFormat" );
static_assert( (int) WG_COLORSPACE_SRGB == (int) ColorSpace::sRGB, "wg_colorSpace out of sync with ColorSpace" );

const wg_pixelDescription* wg_pixelFormatToDescription( wg_pixelFormat format )
{
	return (const wg_pixelDescription*) &Util::pixelFormatToDescription((PixelFormat) format);
}

wg_pixelFormat wg_pixelDescriptionToFormat(const wg_pixelDescription * pDescription)
{
	return (wg_pixelFormat) Util::pixelDescriptionToFormat( * reinterpret_cast<const PixelDescription *>(pDescription));
}

