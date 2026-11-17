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
 * Default kernel set, for desktop applications.
 *
 * Supports all tint modes, blend modes and the common pixel formats, but only has
 * one-pass blits for the most common operations: blits without tint, replacing
 * or blending onto 8-bit RGB canvases.
 *
 * Kernels are selected at compile time by a SoftKernelSpec, see wax_softkernelspec.h.
 * To make a kernel set of your own, copy this file and change the spec.
 */

#include <wax_softkernels_default.h>
#include <wax_softkernelset.impl.h>
#include <wax_c_softkernels_default.h>
#include <wax_c_object.h>

namespace wax
{
	namespace
	{
		constexpr uint32_t c_srcFormats = maskOf(	SoftFormat::XRGB_8_sRGB, SoftFormat::XRGB_8_linear,
													SoftFormat::ARGB_8_sRGB, SoftFormat::ARGB_8_linear,
													SoftFormat::Index_8_sRGB, SoftFormat::Index_8_linear,
													SoftFormat::Index_16_sRGB, SoftFormat::Index_16_linear,
													SoftFormat::Alpha_8,
													SoftFormat::RGB_565_sRGB, SoftFormat::RGB_565_linear,
													SoftFormat::BGR_565_BE_linear );

		constexpr uint32_t c_destFormats = maskOf(	SoftFormat::XRGB_8_sRGB, SoftFormat::XRGB_8_linear,
													SoftFormat::ARGB_8_sRGB, SoftFormat::ARGB_8_linear,
													SoftFormat::Alpha_8,
													SoftFormat::RGB_565_sRGB, SoftFormat::RGB_565_linear,
													SoftFormat::BGR_565_BE_linear );

		constexpr uint32_t c_rgb8Formats = maskOf(	SoftFormat::XRGB_8_sRGB, SoftFormat::XRGB_8_linear,
													SoftFormat::ARGB_8_sRGB, SoftFormat::ARGB_8_linear );

		constexpr SoftKernelSpec c_defaultSoftKernels = {

			maskOf( TintMode::None, TintMode::Flat, TintMode::GradientX ),

			maskOf( BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::Subtract, BlendMode::Multiply,
					BlendMode::Invert, BlendMode::Min, BlendMode::Max, BlendMode::Morph, BlendMode::BlendFixedColor ),

			c_srcFormats,
			c_destFormats,

			{
				// All blits without tint, replacing or blending onto 8-bit RGB canvases.

				{ c_allSoftBlits, maskOf(TintMode::None), maskOf(BlendMode::Replace, BlendMode::Blend), c_srcFormats, c_rgb8Formats }
			}
		};
	}

	//____ addDefaultSoftKernels() ____________________________________________

	bool addDefaultSoftKernels( SoftBackend * pBackend )
	{
		return addSoftKernels<c_defaultSoftKernels>(pBackend);
	}
}

//____ wax_addDefaultSoftKernels() _____________________________________________

int wax_addDefaultSoftKernels( wax_obj backend )
{
	auto pBackend = static_cast<wax::SoftBackend*>(reinterpret_cast<wax::Object*>(backend));
	return wax::addDefaultSoftKernels(pBackend);
}
