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
 * Base kernel set for a big endian, linear BGR_565 canvas on hardware with 5 bits
 * of green (RGB555), so the lowest bit of green is never set.
 *
 * Linear formats only. Combine with addExtraSoftKernelsForRGB555BECanvas() for
 * hand-tuned versions of the most common operations.
 *
 * Kernels are selected at compile time by a SoftKernelSpec, see wax_softkernelspec.h.
 * To make a kernel set of your own, copy this file and change the spec.
 */

#include <wax_softkernels_rgb555be_base.h>
#include <wax_softkernelset.impl.h>
#include <wax_c_softkernels_rgb555be_base.h>
#include <wax_c_object.h>

namespace wax
{
	namespace
	{
		constexpr uint32_t c_srcFormats = maskOf(	SoftFormat::XRGB_8_linear, SoftFormat::ARGB_8_linear,
													SoftFormat::Index_8_linear, SoftFormat::Alpha_8,
													SoftFormat::BGR_565_BE_linear );

		constexpr uint32_t c_destFormats = maskOf(	SoftFormat::XRGB_8_linear, SoftFormat::ARGB_8_linear,
													SoftFormat::Alpha_8, SoftFormat::BGR_565_BE_linear );

		constexpr SoftKernelSpec c_rgb555BEBaseSoftKernels = {

			maskOf( TintMode::None, TintMode::Flat, TintMode::GradientX ),

			maskOf( BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::Subtract,
					BlendMode::Multiply, BlendMode::Min, BlendMode::Max, BlendMode::BlendFixedColor ),

			c_srcFormats,
			c_destFormats,

			{
				{	maskOf(	SoftBlitType::StraightBlit, SoftBlitType::StraightTile,
							SoftBlitType::TransformBlitNearest, SoftBlitType::TransformTileNearest ),
					maskOf(TintMode::None),
					maskOf(BlendMode::Replace, BlendMode::Blend, BlendMode::BlendFixedColor),
					c_srcFormats,
					c_destFormats }
			},

			true		// bG5Green: never set the lowest bit of green on BGR_565_BE_linear canvases.
		};
	}

	//____ addBaseSoftKernelsForRGB555BECanvas() ______________________________

	bool addBaseSoftKernelsForRGB555BECanvas( SoftBackend * pBackend )
	{
		return addSoftKernels<c_rgb555BEBaseSoftKernels>(pBackend);
	}
}

//____ wax_addBaseSoftKernelsForRGB555BECanvas() _______________________________

int wax_addBaseSoftKernelsForRGB555BECanvas( wax_obj backend )
{
	auto pBackend = static_cast<wax::SoftBackend*>(reinterpret_cast<wax::Object*>(backend));
	return wax::addBaseSoftKernelsForRGB555BECanvas(pBackend);
}
