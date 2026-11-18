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

/**
 * addSoftKernels<spec>(): instantiates and registers the kernels selected by a SoftKernelSpec.
 *
 * Include this in the one source file that instantiates a kernel set. It pulls in all kernel
 * templates (wax_softkernelstanza.impl.h), which is a lot of code, and every instantiation
 * compiles the selected kernels. See wax_softkernelspec.h for how to write a spec.
 *
 * The spec needs to be a constexpr variable at namespace scope, since it is passed by reference
 * as a template parameter.
 */

#ifndef WAX_SOFTKERNELSET_IMPL_DOT_H
#define WAX_SOFTKERNELSET_IMPL_DOT_H
#pragma once

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <wax_softkernelspec.h>
#include <wax_softbackend.h>
#include <wax_softkernelstanza.impl.h>

namespace wax
{
	namespace softkernelset_detail
	{
		using ReadOp = SoftBackend::ReadOp;
		using StripSource = SoftBackend::StripSource;

		constexpr bool contains(uint32_t mask, int value) { return (mask & (uint32_t(1) << value)) != 0; }

		template<class Enum>
		constexpr bool contains(uint32_t mask, Enum value) { return contains(mask, int(value)); }

		constexpr int popCount(uint32_t mask)
		{
			int n = 0;
			for (; mask != 0; mask >>= 1)
				n += int(mask & 1);
			return n;
		}

		//____ forEachBit() ____________________________________________________

		template<uint32_t mask>
		constexpr std::array<int, popCount(mask)> bitList()
		{
			std::array<int, popCount(mask)> list{};
			int n = 0;
			for (int i = 0; i < 32; i++)
				if (contains(mask, i))
					list[n++] = i;
			return list;
		}

		template<uint32_t mask>
		inline constexpr auto c_bitList = bitList<mask>();

		template<uint32_t mask, class Func, std::size_t... I>
		void forEachBit(Func&& func, std::index_sequence<I...>)
		{
			(func(std::integral_constant<int, c_bitList<mask>[I]>{}), ...);
		}

		// Calls func(std::integral_constant<int,N>) for each bit N set in mask.
		// Only set bits are instantiated, so compile time grows with what is selected.

		template<uint32_t mask, class Func>
		void forEachBit(Func&& func)
		{
			forEachBit<mask>(std::forward<Func>(func), std::make_index_sequence<popCount(mask)>{});
		}

		//____ hasFormat() _____________________________________________________

		constexpr bool hasFormat(uint32_t formats, bool bLinear)
		{
			for (int i = 1; i < SoftFormat_size; i++)
				if (contains(formats, i) && SoftFormatInfo::isLinear(SoftFormat(i)) == bLinear)
					return true;
			return false;
		}

		//____ onlyDestinations() ______________________________________________

		constexpr bool onlyDestinations(uint32_t formats)
		{
			for (int i = 0; i < SoftFormat_size; i++)
				if (contains(formats, i) && !SoftFormatInfo::isDestination(SoftFormat(i)))
					return false;
			return true;
		}

		//____ kernelFormat() __________________________________________________

		// Format written by the kernels drawing on a canvas of the given format.

		constexpr SoftFormat kernelFormat(const SoftKernelSpec& spec, SoftFormat destFormat)
		{
			return spec.bG5Green && destFormat == SoftFormat::BGR_565_BE_linear ? SoftFormat::BGR_565_BE_linear_G5 : destFormat;
		}

		//____ addStraightBlit() _______________________________________________

		// DEST is the format kernels are set for, KERNEL_DEST the format they write.

		template<SoftFormat SRC, ReadOp OP, TintMode TINT, BlendMode BLEND, SoftFormat DEST, SoftFormat KERNEL_DEST = DEST>
		void addStraightBlit(SoftBackend* pBackend)
		{
			pBackend->setStraightBlitKernel(SRC, OP, TINT, BLEND, DEST, _straight_blit<SRC, TINT, BLEND, KERNEL_DEST, OP>);
		}

		//____ addTransformBlit() ______________________________________________

		template<SoftFormat SRC, SampleMethod SAMPLE, ReadOp OP, TintMode TINT, BlendMode BLEND, SoftFormat DEST, SoftFormat KERNEL_DEST = DEST>
		void addTransformBlit(SoftBackend* pBackend)
		{
			pBackend->setTransformBlitKernel(SRC, SAMPLE, OP, TINT, BLEND, DEST, _transform_blit<SRC, SAMPLE, TINT, BLEND, KERNEL_DEST, OP>);
		}

		//____ addFirstPassBlits() _____________________________________________

		// Blits without tint or blend from each source format to DEST, which is the HiColor
		// buffer (SoftFormat::Undefined) or the 8-bit linear buffer (SoftFormat::ARGB_8_linear).

		template<uint32_t srcFormats, SoftFormat DEST>
		void addFirstPassBlits(SoftBackend* pBackend)
		{
			// No tint and Replace. Not as local constexpr variables, as MSVC doesn't take those
			// as constant expressions in a lambda, so they can't be template arguments there.

			forEachBit<srcFormats>([&](auto s) {
				constexpr SoftFormat S = SoftFormat(decltype(s)::value);

				addStraightBlit<S, ReadOp::Normal, TintMode::None, BlendMode::Replace, DEST>(pBackend);
				addStraightBlit<S, ReadOp::Tile, TintMode::None, BlendMode::Replace, DEST>(pBackend);
				addStraightBlit<S, ReadOp::Blur, TintMode::None, BlendMode::Replace, DEST>(pBackend);
			});

			forEachBit<srcFormats>([&](auto s) {
				constexpr SoftFormat S = SoftFormat(decltype(s)::value);

				addTransformBlit<S, SampleMethod::Nearest, ReadOp::Normal, TintMode::None, BlendMode::Replace, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Nearest, ReadOp::Clip, TintMode::None, BlendMode::Replace, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Nearest, ReadOp::Tile, TintMode::None, BlendMode::Replace, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Nearest, ReadOp::Blur, TintMode::None, BlendMode::Replace, DEST>(pBackend);

				addTransformBlit<S, SampleMethod::Bilinear, ReadOp::Normal, TintMode::None, BlendMode::Replace, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Bilinear, ReadOp::Clip, TintMode::None, BlendMode::Replace, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Bilinear, ReadOp::Tile, TintMode::None, BlendMode::Replace, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Bilinear, ReadOp::Blur, TintMode::None, BlendMode::Replace, DEST>(pBackend);
			});
		}

		//____ addSecondPassBlits() ____________________________________________

		// Blits with tint and blend from SRC, which is the HiColor buffer (SoftFormat::Undefined)
		// or the 8-bit linear buffer (SoftFormat::ARGB_8_linear), to each destination format.

		template<const SoftKernelSpec& spec, SoftFormat SRC>
		void addSecondPassBlits(SoftBackend* pBackend)
		{
			forEachBit<spec.tintModes>([&](auto t) {
			forEachBit<spec.blendModes>([&](auto b) {
			forEachBit<spec.destFormats>([&](auto d) {
				constexpr SoftFormat D = SoftFormat(decltype(d)::value);

				addStraightBlit<SRC, ReadOp::Normal, TintMode(decltype(t)::value), BlendMode(decltype(b)::value),
								D, kernelFormat(spec, D)>(pBackend);
			});
			});
			});
		}

		//____ addCustomBlits() ________________________________________________

		template<uint32_t blitTypes, SoftBlitType TYPE, SampleMethod SAMPLE, ReadOp OP,
				 SoftFormat SRC, TintMode TINT, BlendMode BLEND, SoftFormat DEST, SoftFormat KERNEL_DEST>
		void addCustomTransformBlit(SoftBackend* pBackend)
		{
			if constexpr (contains(blitTypes, TYPE))
				addTransformBlit<SRC, SAMPLE, OP, TINT, BLEND, DEST, KERNEL_DEST>(pBackend);
		}

		template<uint32_t blitTypes, SoftFormat S, TintMode T, BlendMode B, SoftFormat D, SoftFormat KD>
		void addCustomBlits(SoftBackend* pBackend)
		{
			using BT = SoftBlitType;
			using SM = SampleMethod;

			if constexpr (contains(blitTypes, BT::StraightBlit))
				addStraightBlit<S, ReadOp::Normal, T, B, D, KD>(pBackend);
			if constexpr (contains(blitTypes, BT::StraightTile))
				addStraightBlit<S, ReadOp::Tile, T, B, D, KD>(pBackend);
			if constexpr (contains(blitTypes, BT::StraightBlur))
				addStraightBlit<S, ReadOp::Blur, T, B, D, KD>(pBackend);

			addCustomTransformBlit<blitTypes, BT::TransformBlitNearest, SM::Nearest, ReadOp::Normal, S, T, B, D, KD>(pBackend);
			addCustomTransformBlit<blitTypes, BT::TransformBlitBilinear, SM::Bilinear, ReadOp::Normal, S, T, B, D, KD>(pBackend);
			addCustomTransformBlit<blitTypes, BT::TransformClipBlitNearest, SM::Nearest, ReadOp::Clip, S, T, B, D, KD>(pBackend);
			addCustomTransformBlit<blitTypes, BT::TransformClipBlitBilinear, SM::Bilinear, ReadOp::Clip, S, T, B, D, KD>(pBackend);
			addCustomTransformBlit<blitTypes, BT::TransformTileNearest, SM::Nearest, ReadOp::Tile, S, T, B, D, KD>(pBackend);
			addCustomTransformBlit<blitTypes, BT::TransformTileBilinear, SM::Bilinear, ReadOp::Tile, S, T, B, D, KD>(pBackend);
			addCustomTransformBlit<blitTypes, BT::TransformBlurNearest, SM::Nearest, ReadOp::Blur, S, T, B, D, KD>(pBackend);
			addCustomTransformBlit<blitTypes, BT::TransformBlurBilinear, SM::Bilinear, ReadOp::Blur, S, T, B, D, KD>(pBackend);
		}

		// What an entry of customBlits selects, as variable templates rather than local constexpr
		// variables, since MSVC doesn't take those as constant expressions in a lambda.

		template<const SoftKernelSpec& spec, int entry>
		inline constexpr uint32_t c_customBlitTypes = spec.customBlits[entry].blitTypes;

		template<const SoftKernelSpec& spec, int entry>
		inline constexpr uint32_t c_customTintModes = spec.customBlits[entry].tintModes;

		template<const SoftKernelSpec& spec, int entry>
		inline constexpr uint32_t c_customBlendModes = spec.customBlits[entry].blendModes;

		template<const SoftKernelSpec& spec, int entry>
		inline constexpr uint32_t c_customSrcFormats = spec.customBlits[entry].srcFormats & spec.srcFormats;

		template<const SoftKernelSpec& spec, int entry>
		inline constexpr uint32_t c_customDestFormats = spec.customBlits[entry].destFormats & spec.destFormats;

		template<const SoftKernelSpec& spec, int entry>
		void addCustomBlitEntry(SoftBackend* pBackend)
		{
			if constexpr (c_customBlitTypes<spec, entry> != 0)
			{
				forEachBit<c_customTintModes<spec, entry>>([&](auto t) {
				forEachBit<c_customBlendModes<spec, entry>>([&](auto b) {
				forEachBit<c_customSrcFormats<spec, entry>>([&](auto s) {
				forEachBit<c_customDestFormats<spec, entry>>([&](auto d) {
					constexpr SoftFormat D = SoftFormat(decltype(d)::value);

					addCustomBlits<c_customBlitTypes<spec, entry>, SoftFormat(decltype(s)::value), TintMode(decltype(t)::value),
								   BlendMode(decltype(b)::value), D, kernelFormat(spec, D)>(pBackend);
				});
				});
				});
				});
			}
		}

		template<const SoftKernelSpec& spec, std::size_t... I>
		void addCustomBlitEntries(SoftBackend* pBackend, std::index_sequence<I...>)
		{
			(addCustomBlitEntry<spec, int(I)>(pBackend), ...);
		}
	}

	//____ addSoftKernels() ____________________________________________________

	/**
	 * Adds the kernels selected by spec to pBackend. Kernels already set for other
	 * combinations are kept, so kernel sets can be combined.
	 */

	template<const SoftKernelSpec& spec>
	bool addSoftKernels(SoftBackend* pBackend)
	{
		using namespace softkernelset_detail;

		static_assert(!contains(spec.tintModes, TintMode::GradientY) && !contains(spec.tintModes, TintMode::GradientXY),
					  "SoftKernelSpec: TintMode GradientY and GradientXY are not used by SoftBackend.");
		static_assert(!contains(spec.blendModes, BlendMode::Undefined) && !contains(spec.blendModes, BlendMode::Ignore),
					  "SoftKernelSpec: BlendMode Undefined and Ignore have no kernels.");
		static_assert(!contains(spec.srcFormats, SoftFormat::Undefined),
					  "SoftKernelSpec: SoftFormat::Undefined is not a source format.");
		static_assert(onlyDestinations(spec.destFormats),
					  "SoftKernelSpec: destFormats contains a format that can not be a canvas.");
		static_assert(!spec.bG5Green || contains(spec.destFormats, SoftFormat::BGR_565_BE_linear),
					  "SoftKernelSpec: bG5Green set without BGR_565_BE_linear as destination format.");
		static_assert(spec.tintModes != 0 && spec.blendModes != 0 && spec.destFormats != 0,
					  "SoftKernelSpec: needs at least one tint mode, blend mode and destination format.");

		// Two-pass blits go through the 8-bit linear buffer when both source and destination are
		// linear, otherwise through the HiColor buffer, which is only needed when sRGB is involved.

		constexpr bool bFast8Blits = hasFormat(spec.srcFormats, true) && hasFormat(spec.destFormats, true);
		constexpr bool bHiColorBlits = hasFormat(spec.srcFormats, false) || hasFormat(spec.destFormats, false);

		// Fill kernels

		forEachBit<spec.tintModes>([&](auto t) {
		forEachBit<spec.blendModes>([&](auto b) {
		forEachBit<spec.destFormats>([&](auto d) {
			constexpr TintMode T = TintMode(decltype(t)::value);
			constexpr BlendMode B = BlendMode(decltype(b)::value);
			constexpr SoftFormat D = SoftFormat(decltype(d)::value);
			constexpr SoftFormat K = kernelFormat(spec, D);

			pBackend->setFillKernel(T, B, D, _fill<T, B, K>);
		});
		});
		});

		// Line, segment strip and coverage kernels

		forEachBit<spec.blendModes>([&](auto b) {
		forEachBit<spec.destFormats>([&](auto d) {
			constexpr BlendMode B = BlendMode(decltype(b)::value);
			constexpr SoftFormat D = SoftFormat(decltype(d)::value);
			constexpr SoftFormat K = kernelFormat(spec, D);

			pBackend->setLineKernel(B, D, _draw_line<B, TintMode::None, K>);
			pBackend->setClipLineKernel(B, D, _clip_draw_line<B, TintMode::None, K>);

			pBackend->setSegmentStripKernel(StripSource::Colors, B, D, _draw_segment_strip<StripSource::Colors, B, K>);

			if constexpr (contains(spec.tintModes, TintMode::GradientX))
				pBackend->setSegmentStripKernel(StripSource::Tintmaps, B, D, _draw_segment_strip<StripSource::Tintmaps, B, K>);

			pBackend->setCoverageKernel(B, D, _draw_coverage_span<B, K>);
		});
		});

		// Second pass of two-pass blits. Plain moves from the 8-bit linear buffer are always
		// needed, since some blits go through it whatever the formats.

		if constexpr (bHiColorBlits)
			addSecondPassBlits<spec, SoftFormat::Undefined>(pBackend);

		if constexpr (bFast8Blits)
			addSecondPassBlits<spec, SoftFormat::ARGB_8_linear>(pBackend);
		else
		{
			forEachBit<spec.destFormats>([&](auto d) {
				constexpr SoftFormat D = SoftFormat(decltype(d)::value);

				addStraightBlit<SoftFormat::ARGB_8_linear, ReadOp::Normal, TintMode::None, BlendMode::Replace,
								D, kernelFormat(spec, D)>(pBackend);
			});
		}

		// First pass of two-pass blits.

		addFirstPassBlits<spec.srcFormats, SoftFormat::Undefined>(pBackend);

		if constexpr (bFast8Blits)
			addFirstPassBlits<spec.srcFormats, SoftFormat::ARGB_8_linear>(pBackend);

		// One-pass custom blits.

		addCustomBlitEntries<spec>(pBackend, std::make_index_sequence<SoftKernelSpec::c_maxCustomBlits>{});

		return true;
	}
}

#endif //WAX_SOFTKERNELSET_IMPL_DOT_H
