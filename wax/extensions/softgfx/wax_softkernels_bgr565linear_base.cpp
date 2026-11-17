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
 * Base kernel set for a linear RGB_565 canvas.
 *
 * Linear formats only. Combine with addExtraSoftKernelsForBGR565LinearCanvas() for
 * hand-tuned versions of the most common operations.
 *
 * Kernels are selected at compile time by a SoftKernelSpec, see wax_softkernelspec.h.
 * To make a kernel set of your own, copy this file and change the spec.
 */

#include <wax_softkernels_bgr565linear_base.h>
#include <wax_softkernelset.impl.h>
#include <wax_c_softkernels_bgr565linear_base.h>
#include <wax_c_object.h>

namespace wax
{
	namespace
	{
		constexpr uint32_t c_srcFormats = maskOf(	SoftFormat::XRGB_8_linear, SoftFormat::ARGB_8_linear,
													SoftFormat::Index_8_linear, SoftFormat::Alpha_8,
													SoftFormat::RGB_565_linear );

		constexpr uint32_t c_destFormats = maskOf(	SoftFormat::XRGB_8_linear, SoftFormat::ARGB_8_linear,
													SoftFormat::Alpha_8, SoftFormat::RGB_565_linear );

		constexpr SoftKernelSpec c_bgr565LinearBaseSoftKernels = {

			maskOf( TintMode::None, TintMode::Flat, TintMode::GradientX ),

			maskOf( BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::Subtract,
					BlendMode::Multiply, BlendMode::Min, BlendMode::Max, BlendMode::BlendFixedColor ),

			c_srcFormats,
			c_destFormats,

			{
				// Blits without tint onto the RGB_565 canvas itself.

				{	maskOf(SoftBlitType::StraightBlit, SoftBlitType::TransformBlitNearest),
					maskOf(TintMode::None),
					maskOf(BlendMode::Replace, BlendMode::Blend, BlendMode::BlendFixedColor),
					c_srcFormats,
					maskOf(SoftFormat::RGB_565_linear) }
			}
		};
	}

	//____ addBaseSoftKernelsForBGR565LinearCanvas() __________________________

	bool addBaseSoftKernelsForBGR565LinearCanvas( SoftBackend * pBackend )
	{
		return addSoftKernels<c_bgr565LinearBaseSoftKernels>(pBackend);
	}
}

//____ wax_addBaseSoftKernelsForBGR565LinearCanvas() ___________________________

int wax_addBaseSoftKernelsForBGR565LinearCanvas( wax_obj backend )
{
	auto pBackend = static_cast<wax::SoftBackend*>(reinterpret_cast<wax::Object*>(backend));
	return wax::addBaseSoftKernelsForBGR565LinearCanvas(pBackend);
}
