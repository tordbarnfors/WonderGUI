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
#include <wax_c_util.h>
#include <wax_util.h>

#include <cstddef>

using namespace wax;

static_assert( sizeof(wax_pixelDescription) == sizeof(PixelDescription) && offsetof(wax_pixelDescription, R_mask) == offsetof(PixelDescription, R_mask),
			   "wax_pixelDescription out of sync with PixelDescription" );
static_assert( (int) WAX_PIXFMT_ARGB_16 == (int) PixelFormat::ARGB_16 && (int) WAX_PIXFMT_BGR_565 == (int) PixelFormat::BGR_565,
			   "wax_pixelFormat out of sync with PixelFormat" );
static_assert( (int) WAX_COLORSPACE_SRGB == (int) ColorSpace::sRGB, "wax_colorSpace out of sync with ColorSpace" );

const wax_pixelDescription* wax_pixelFormatToDescription( wax_pixelFormat format )
{
	return (const wax_pixelDescription*) &Util::pixelFormatToDescription((PixelFormat) format);
}

wax_pixelFormat wax_pixelDescriptionToFormat(const wax_pixelDescription * pDescription)
{
	return (wax_pixelFormat) Util::pixelDescriptionToFormat( * reinterpret_cast<const PixelDescription *>(pDescription));
}

