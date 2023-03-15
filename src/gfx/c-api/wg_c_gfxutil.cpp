/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_c_gfxutil.h>
#include <wg_gfxutil.h>

using namespace wg;

const wg_pixelDescription* wg_pixelFormatToDescription( wg_pixelFormat format )
{
	return (const wg_pixelDescription*) &Util::pixelFormatToDescription((PixelFormat) format);
}

wg_pixelFormat wg_pixelDescriptionToFormat(const wg_pixelDescription * pDescription)
{
	return (wg_pixelFormat) Util::pixelDescriptionToFormat( * reinterpret_cast<const PixelDescription *>(pDescription));
}

wg_pixelFormat wg_translatePixelFormat( wg_pixelFormat format)
{
	return (wg_pixelFormat) Util::translatePixelFormat( (PixelFormat) format );
}
