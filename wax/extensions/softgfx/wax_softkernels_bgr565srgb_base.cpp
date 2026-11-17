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
 * Base kernel set for an sRGB RGB_565 canvas.
 *
 * sRGB formats only. Combine with addExtraSoftKernelsForBGR565sRGBCanvas() for
 * hand-tuned versions of the most common operations.
 *
 * Kernels are selected at compile time by a SoftKernelSpec, see wax_softkernelspec.h.
 * To make a kernel set of your own, copy this file and change the spec.
 */

#include <wax_softkernels_bgr565srgb_base.h>
#include <wax_softkernelset.impl.h>
#include <wax_c_softkernels_bgr565srgb_base.h>
#include <wax_c_object.h>

namespace wax
{
	namespace
	{
		constexpr uint32_t c_srcFormats = maskOf(	SoftFormat::XRGB_8_sRGB, SoftFormat::ARGB_8_sRGB,
													SoftFormat::Index_8_sRGB, SoftFormat::Alpha_8,
													SoftFormat::RGB_565_sRGB );

		constexpr uint32_t c_destFormats = maskOf(	SoftFormat::XRGB_8_sRGB, SoftFormat::ARGB_8_sRGB,
													SoftFormat::RGB_565_sRGB );

		constexpr SoftKernelSpec c_bgr565sRGBBaseSoftKernels = {

			maskOf( TintMode::None, TintMode::Flat, TintMode::GradientX ),
			maskOf( BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::BlendFixedColor ),
			c_srcFormats,
			c_destFormats,

			{
				{	maskOf(	SoftBlitType::StraightBlit,
							SoftBlitType::TransformBlitNearest, SoftBlitType::TransformClipBlitNearest,
							SoftBlitType::TransformBlitBilinear, SoftBlitType::TransformClipBlitBilinear ),
					maskOf(TintMode::None),
					maskOf(BlendMode::Replace, BlendMode::Blend, BlendMode::BlendFixedColor),
					c_srcFormats,
					c_destFormats },

				{	maskOf(SoftBlitType::StraightBlit),
					maskOf(TintMode::None, TintMode::Flat),
					maskOf(BlendMode::Add),
					c_srcFormats,
					c_destFormats }
			}
		};
	}

	//____ addBaseSoftKernelsForBGR565sRGBCanvas() ____________________________

	bool addBaseSoftKernelsForBGR565sRGBCanvas( SoftBackend * pBackend )
	{
		return addSoftKernels<c_bgr565sRGBBaseSoftKernels>(pBackend);
	}
}

//____ wax_addBaseSoftKernelsForBGR565sRGBCanvas() _____________________________

int wax_addBaseSoftKernelsForBGR565sRGBCanvas( wax_obj backend )
{
	auto pBackend = static_cast<wax::SoftBackend*>(reinterpret_cast<wax::Object*>(backend));
	return wax::addBaseSoftKernelsForBGR565sRGBCanvas(pBackend);
}
