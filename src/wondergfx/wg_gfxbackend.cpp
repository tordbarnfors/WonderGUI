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

#include <wg_gfxbackend.h>

namespace wg
{
	const TypeInfo GfxBackend::TYPEINFO = { "GfxBackend", &Object::TYPEINFO };

	// Transforms for flipping movement over SOURCE when blitting

const Transform			GfxBackend::s_blitFlipTransforms[GfxFlip_size] = {
	{ 1,0,0,1 },			// Normal
	{ -1,0,0,1 },			// FlipX
	{ 1,0,0,-1 },			// FlipY
	{ 0,-1,1,0 },			// Rot90
	{ 0,1,1,0 },			// Rot90FlipX
	{ 0,-1,-1,0 },			// Rot90FlipY
	{ -1,0,0,-1 },			// Rot180
	{ 1,0,0,-1 },			// Rot180FlipX
	{ -1,0,0,1 },			// Rot180FlipY
	{ 0,1,-1,0 },			// Rot270
	{ 0,-1,-1,0 },			// Rot270FlipX
	{ 0,1,1,0 }				// Rot270FlipY
};

const int GfxBackend::s_defaultBlur[9] = { 6553, 6553, 6553, 6553, 6553, 6553, 6553, 6553, 6553 };
const spx GfxBackend::s_defaultBlurRadius = 4 * 64;



	//____ typeInfo() _________________________________________________________

	const TypeInfo& GfxBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}


} // namespace wg
