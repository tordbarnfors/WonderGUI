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

#ifndef WAX_SOFTKERNELSPEC_DOT_H
#define WAX_SOFTKERNELSPEC_DOT_H
#pragma once

#include <cstdint>

#include <wax_types.h>
#include <wax_enumextras.h>
#include <wax_softformat.h>

namespace wax
{
	/**
	 * Selection of software rendering kernels, decided at compile time.
	 *
	 * SoftBackend draws through kernels: small render loops specialized for a combination
	 * of tint mode, blend mode, source format and destination format. Every combination
	 * is a separate function, so the selection decides both what SoftBackend can draw
	 * and how much code is added to the binary.
	 *
	 * A SoftKernelSpec selects the kernels. Pass it to addSoftKernels<spec>() (see
	 * wax_softkernelset.impl.h), which instantiates and registers exactly those kernels:
	 *
	 *		#include <wax_softkernelset.impl.h>
	 *
	 *		constexpr SoftKernelSpec c_myKernels = {
	 *			maskOf(TintMode::None, TintMode::Flat),								// Tint modes
	 *			maskOf(BlendMode::Replace, BlendMode::Blend),						// Blend modes
	 *			maskOf(SoftFormat::ARGB_8_linear, SoftFormat::Alpha_8),				// Source formats
	 *			maskOf(SoftFormat::ARGB_8_linear),									// Destination formats
	 *			{																	// Custom blits
	 *				{ maskOf(SoftBlitType::StraightBlit), maskOf(TintMode::None), maskOf(BlendMode::Blend),
	 *				  maskOf(SoftFormat::ARGB_8_linear), maskOf(SoftFormat::ARGB_8_linear) }
	 *			}
	 *		};
	 *
	 *		addSoftKernels<c_myKernels>(pSoftBackend);
	 *
	 * The four first masks decide what is supported. Fill, line, segment and coverage kernels
	 * are added for every tint mode, blend mode and destination format, and blits go through
	 * two passes: source format to an intermediate buffer, then intermediate buffer to
	 * destination with tint and blend. Every combination of the masks can thereby be drawn,
	 * with a number of kernels growing roughly with the sum of the formats rather than their
	 * product.
	 *
	 * Custom blits add one-pass kernels that go directly from source to destination, which
	 * is faster, but adds one kernel for every combination. Use them for the blits that
	 * matter for performance. Custom blits only cover formats that also are in the
	 * source and destination masks.
	 */

	//____ SoftBlitType ________________________________________________________

	enum class SoftBlitType : uint8_t
	{
		StraightBlit,
		StraightTile,
		StraightBlur,

		TransformBlitNearest,
		TransformClipBlitNearest,
		TransformTileNearest,
		TransformBlurNearest,

		TransformBlitBilinear,
		TransformClipBlitBilinear,
		TransformTileBilinear,
		TransformBlurBilinear
	};

	const static int SoftBlitType_size = (int)SoftBlitType::TransformBlurBilinear + 1;

	//____ maskOf() ____________________________________________________________

	/// Bitmask with one bit set for each given enum value, e.g. maskOf(BlendMode::Replace, BlendMode::Blend).

	template<class... Enum>
	constexpr uint32_t maskOf(Enum... values) { return ((uint32_t(1) << int(values)) | ... | uint32_t(0)); }

	//____ Common masks ________________________________________________________

	constexpr uint32_t c_allStraightSoftBlits = maskOf(SoftBlitType::StraightBlit, SoftBlitType::StraightTile, SoftBlitType::StraightBlur);

	constexpr uint32_t c_allTransformSoftBlits = maskOf(	SoftBlitType::TransformBlitNearest, SoftBlitType::TransformClipBlitNearest,
															SoftBlitType::TransformTileNearest, SoftBlitType::TransformBlurNearest,
															SoftBlitType::TransformBlitBilinear, SoftBlitType::TransformClipBlitBilinear,
															SoftBlitType::TransformTileBilinear, SoftBlitType::TransformBlurBilinear );

	constexpr uint32_t c_allSoftBlits = c_allStraightSoftBlits | c_allTransformSoftBlits;

	//____ CustomSoftBlitSpec __________________________________________________

	/// One-pass blit kernels for all combinations of the given masks.

	struct CustomSoftBlitSpec
	{
		uint32_t	blitTypes = 0;		///< Mask of SoftBlitType.
		uint32_t	tintModes = 0;		///< Mask of TintMode.
		uint32_t	blendModes = 0;		///< Mask of BlendMode.
		uint32_t	srcFormats = 0;		///< Mask of SoftFormat. Only formats also in SoftKernelSpec::srcFormats are used.
		uint32_t	destFormats = 0;	///< Mask of SoftFormat. Only formats also in SoftKernelSpec::destFormats are used.
	};

	//____ SoftKernelSpec ______________________________________________________

	struct SoftKernelSpec
	{
		const static int c_maxCustomBlits = 8;

		uint32_t			tintModes = 0;		///< Mask of TintMode.
		uint32_t			blendModes = 0;		///< Mask of BlendMode.
		uint32_t			srcFormats = 0;		///< Mask of SoftFormat.
		uint32_t			destFormats = 0;	///< Mask of SoftFormat. Only formats that can be canvas, see SoftFormatInfo::isDestination().

		CustomSoftBlitSpec	customBlits[c_maxCustomBlits] = {};	///< Unused entries are left empty.

		/// Draw on BGR_565_BE_linear canvases without setting the lowest bit of green, using kernels
		/// for SoftFormat::BGR_565_BE_linear_G5. For hardware with 5 bits of green (RGB555), where
		/// that bit has another meaning.

		bool				bG5Green = false;
	};

	static_assert(TintMode_size <= 32 && BlendMode_size <= 32 && SoftFormat_size <= 32 && SoftBlitType_size <= 32,
				  "SoftKernelSpec masks are 32 bits.");
}

#endif //WAX_SOFTKERNELSPEC_DOT_H
