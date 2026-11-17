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

/*
 * Minimal kernel set for 8-bit linear canvases.
 *
 * Only linear 8-bit formats, no gradient tints and only the Replace, Blend
 * and Invert blend modes.
 *
 * Kernels are selected at compile time by a SoftKernelSpec, see wax_softkernelspec.h.
 * To make a kernel set of your own, copy this file and change the spec.
 */

#include <wax_softkernels_tinylinear.h>
#include <wax_softkernelset.impl.h>
#include <wax_c_softkernels_tinylinear.h>
#include <wax_c_object.h>

namespace wax
{
	namespace
	{
		constexpr uint32_t c_srcFormats = maskOf(	SoftFormat::XRGB_8_linear, SoftFormat::ARGB_8_linear,
													SoftFormat::Index_8_linear, SoftFormat::Alpha_8 );

		constexpr uint32_t c_destFormats = maskOf( SoftFormat::XRGB_8_linear, SoftFormat::ARGB_8_linear );

		constexpr SoftKernelSpec c_linearTinySoftKernels = {

			maskOf( TintMode::None, TintMode::Flat ),
			maskOf( BlendMode::Replace, BlendMode::Blend, BlendMode::Invert ),
			c_srcFormats,
			c_destFormats,

			{
				{	maskOf(SoftBlitType::StraightBlit, SoftBlitType::StraightTile, SoftBlitType::TransformBlitNearest),
					maskOf(TintMode::None, TintMode::Flat),
					maskOf(BlendMode::Replace, BlendMode::Blend),
					c_srcFormats,
					c_destFormats }
			}
		};
	}

	//____ addLinearTinySoftKernels() _________________________________________

	bool addLinearTinySoftKernels( SoftBackend * pBackend )
	{
		return addSoftKernels<c_linearTinySoftKernels>(pBackend);
	}
}

//____ wax_addLinearTinySoftKernels() __________________________________________

int wax_addLinearTinySoftKernels( wax_obj backend )
{
	auto pBackend = static_cast<wax::SoftBackend*>(reinterpret_cast<wax::Object*>(backend));
	return wax::addLinearTinySoftKernels(pBackend);
}
