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

		//____ forEachBits() ___________________________________________________

		// Calls Body::run<a, b, ...>(pBackend) for each combination of a bit a set in the first
		// mask, b set in the second and so on. Only set bits are instantiated, so compile time
		// grows with what is selected.
		//
		// The loops are function templates expanding index sequences, with the bodies as static
		// member templates, rather than generic lambdas. MSVC doesn't take what a lambda gets from
		// its enclosing function template, such as a local constexpr variable or the spec, as a
		// constant expression in all the ways the bodies need.

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

		template<uint32_t... masks>
		struct MaskList {};

		template<class Body, int... bits>
		struct BitLoop
		{
			static void run(SoftBackend* pBackend, MaskList<>)
			{
				Body::template run<bits...>(pBackend);
			}

			template<uint32_t mask, uint32_t... rest>
			static void run(SoftBackend* pBackend, MaskList<mask, rest...>)
			{
				runBits(pBackend, MaskList<mask, rest...>{}, std::make_index_sequence<popCount(mask)>{});
			}

			template<uint32_t mask, uint32_t... rest, std::size_t... I>
			static void runBits(SoftBackend* pBackend, MaskList<mask, rest...>, std::index_sequence<I...>)
			{
				(BitLoop<Body, bits..., c_bitList<mask>[I]>::run(pBackend, MaskList<rest...>{}), ...);
			}
		};

		template<class Body, uint32_t... masks>
		void forEachBits(SoftBackend* pBackend)
		{
			BitLoop<Body>::run(pBackend, MaskList<masks...>{});
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

		constexpr SoftFormat kernelFormat(bool bG5Green, SoftFormat destFormat)
		{
			return bG5Green && destFormat == SoftFormat::BGR_565_BE_linear ? SoftFormat::BGR_565_BE_linear_G5 : destFormat;
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

		//____ FirstPassBlits ________________________________________________

		// Blits without tint or blend from source format s to DEST, which is the HiColor buffer
		// (SoftFormat::Undefined) or the 8-bit linear buffer (SoftFormat::ARGB_8_linear).

		template<SoftFormat DEST>
		struct FirstPassBlits
		{
			template<int s>
			static void run(SoftBackend* pBackend)
			{
				constexpr SoftFormat S = SoftFormat(s);
				constexpr TintMode T = TintMode::None;
				constexpr BlendMode B = BlendMode::Replace;

				addStraightBlit<S, ReadOp::Normal, T, B, DEST>(pBackend);
				addStraightBlit<S, ReadOp::Tile, T, B, DEST>(pBackend);
				addStraightBlit<S, ReadOp::Blur, T, B, DEST>(pBackend);

				addTransformBlit<S, SampleMethod::Nearest, ReadOp::Normal, T, B, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Nearest, ReadOp::Clip, T, B, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Nearest, ReadOp::Tile, T, B, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Nearest, ReadOp::Blur, T, B, DEST>(pBackend);

				addTransformBlit<S, SampleMethod::Bilinear, ReadOp::Normal, T, B, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Bilinear, ReadOp::Clip, T, B, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Bilinear, ReadOp::Tile, T, B, DEST>(pBackend);
				addTransformBlit<S, SampleMethod::Bilinear, ReadOp::Blur, T, B, DEST>(pBackend);
			}
		};

		//____ SecondPassBlits _______________________________________________

		// Blits with tint t and blend b from SRC, which is the HiColor buffer (SoftFormat::Undefined)
		// or the 8-bit linear buffer (SoftFormat::ARGB_8_linear), to destination format d.

		template<const SoftKernelSpec& spec, SoftFormat SRC>
		struct SecondPassBlits
		{
			template<int t, int b, int d>
			static void run(SoftBackend* pBackend)
			{
				constexpr SoftFormat D = SoftFormat(d);

				addStraightBlit<SRC, ReadOp::Normal, TintMode(t), BlendMode(b), D, kernelFormat(spec.bG5Green, D)>(pBackend);
			}
		};

		//____ MovesFromFast8 ________________________________________________

		// Plain moves from the 8-bit linear buffer to destination format d.

		template<const SoftKernelSpec& spec>
		struct MovesFromFast8
		{
			template<int d>
			static void run(SoftBackend* pBackend)
			{
				constexpr SoftFormat D = SoftFormat(d);

				addStraightBlit<SoftFormat::ARGB_8_linear, ReadOp::Normal, TintMode::None, BlendMode::Replace,
								D, kernelFormat(spec.bG5Green, D)>(pBackend);
			}
		};

		//____ FillKernels ___________________________________________________

		template<const SoftKernelSpec& spec>
		struct FillKernels
		{
			template<int t, int b, int d>
			static void run(SoftBackend* pBackend)
			{
				constexpr TintMode T = TintMode(t);
				constexpr BlendMode B = BlendMode(b);
				constexpr SoftFormat D = SoftFormat(d);
				constexpr SoftFormat K = kernelFormat(spec.bG5Green, D);

				pBackend->setFillKernel(T, B, D, _fill<T, B, K>);
			}
		};

		//____ LineKernels ___________________________________________________

		// Line, segment strip and coverage kernels.

		template<const SoftKernelSpec& spec>
		struct LineKernels
		{
			template<int b, int d>
			static void run(SoftBackend* pBackend)
			{
				constexpr BlendMode B = BlendMode(b);
				constexpr SoftFormat D = SoftFormat(d);
				constexpr SoftFormat K = kernelFormat(spec.bG5Green, D);

				pBackend->setLineKernel(B, D, _draw_line<B, TintMode::None, K>);
				pBackend->setClipLineKernel(B, D, _clip_draw_line<B, TintMode::None, K>);

				pBackend->setSegmentStripKernel(StripSource::Colors, B, D, _draw_segment_strip<StripSource::Colors, B, K>);

				if constexpr (contains(spec.tintModes, TintMode::GradientX))
					pBackend->setSegmentStripKernel(StripSource::Tintmaps, B, D, _draw_segment_strip<StripSource::Tintmaps, B, K>);

				pBackend->setCoverageKernel(B, D, _draw_coverage_span<B, K>);
			}
		};

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

		//____ CustomBlits ___________________________________________________

		// Blits of entry of the spec's customBlits, with tint t and blend b, from source format s
		// to destination format d.

		template<const SoftKernelSpec& spec, int entry>
		struct CustomBlits
		{
			template<int t, int b, int s, int d>
			static void run(SoftBackend* pBackend)
			{
				constexpr SoftFormat D = SoftFormat(d);

				addCustomBlits<spec.customBlits[entry].blitTypes, SoftFormat(s), TintMode(t), BlendMode(b),
							   D, kernelFormat(spec.bG5Green, D)>(pBackend);
			}
		};

		template<const SoftKernelSpec& spec, int entry>
		void addCustomBlitEntry(SoftBackend* pBackend)
		{
			if constexpr (spec.customBlits[entry].blitTypes != 0)
				forEachBits<CustomBlits<spec, entry>,
							spec.customBlits[entry].tintModes,
							spec.customBlits[entry].blendModes,
							spec.customBlits[entry].srcFormats & spec.srcFormats,
							spec.customBlits[entry].destFormats & spec.destFormats>(pBackend);
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

		forEachBits<FillKernels<spec>, spec.tintModes, spec.blendModes, spec.destFormats>(pBackend);
		forEachBits<LineKernels<spec>, spec.blendModes, spec.destFormats>(pBackend);

		// Second pass of two-pass blits. Plain moves from the 8-bit linear buffer are always
		// needed, since some blits go through it whatever the formats.

		if constexpr (bHiColorBlits)
			forEachBits<SecondPassBlits<spec, SoftFormat::Undefined>, spec.tintModes, spec.blendModes, spec.destFormats>(pBackend);

		if constexpr (bFast8Blits)
			forEachBits<SecondPassBlits<spec, SoftFormat::ARGB_8_linear>, spec.tintModes, spec.blendModes, spec.destFormats>(pBackend);
		else
			forEachBits<MovesFromFast8<spec>, spec.destFormats>(pBackend);

		// First pass of two-pass blits.

		forEachBits<FirstPassBlits<SoftFormat::Undefined>, spec.srcFormats>(pBackend);

		if constexpr (bFast8Blits)
			forEachBits<FirstPassBlits<SoftFormat::ARGB_8_linear>, spec.srcFormats>(pBackend);

		// One-pass custom blits.

		addCustomBlitEntries<spec>(pBackend, std::make_index_sequence<SoftKernelSpec::c_maxCustomBlits>{});

		return true;
	}
}

#endif //WAX_SOFTKERNELSET_IMPL_DOT_H
