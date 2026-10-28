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
#ifndef WAX_UTIL_DOT_H
#define WAX_UTIL_DOT_H
#pragma once

#include <wax_geo.h>
#include <wax_types.h>
#include <wax_patches.h>

#include <algorithm>

namespace wax
{

	class Device;

	//____ Util ______________________________________________________________

	namespace Util		/** @private */
	{

		// Helper to create endian-aware uint32_t from four chars

		static constexpr uint32_t makeEndianSpecificToken(char a, char b, char c, char d) {
			#if WAX_IS_BIG_ENDIAN
				return (a << 24) | (b << 16) | (c << 8) | d;
			#else
				return a | (b << 8) | (c << 16) | (d << 24);
			#endif
		}



		struct Matrix22
		{
			int xx, xy;
			int yx, yy;
		};

		extern const Matrix22 _flipMatrix[GfxFlip_size];
		extern const Matrix22 _unflipMatrix[GfxFlip_size];
		extern const Matrix22 _flipOrigoMatrix[GfxFlip_size];
		extern const Matrix22 _unflipOrigoMatrix[GfxFlip_size];

		inline const Matrix22& flipMatrix(GfxFlip flip)
		{
			return _flipMatrix[(int)flip];
		}

		inline SizeI flipSize(SizeI size, GfxFlip flip)
		{
			const Matrix22& mtx = _flipMatrix[(int)flip];

			return { abs(size.w * mtx.xx + size.h * mtx.xy), abs(size.w * mtx.yx + size.h * mtx.yy) };
		}

		inline SizeI unflipSize(SizeI size, GfxFlip flip)
		{
			const Matrix22& mtx = _unflipMatrix[(int)flip];

			return { abs(size.w * mtx.xx + size.h * mtx.xy), abs(size.w * mtx.yx + size.h * mtx.yy) };
		}

		inline CoordI flipCoord(CoordI coord, GfxFlip flip, SizeI canvasSize )
		{
			const Matrix22& mtx = _flipMatrix[(int)flip];
			const Matrix22& omtx = _flipOrigoMatrix[(int)flip];

			int ofsX = canvasSize.w * omtx.xx + canvasSize.h * omtx.xy;
			int ofsY = canvasSize.w * omtx.yx + canvasSize.h * omtx.yy;
			
			return { ofsX + coord.x * mtx.xx + coord.y * mtx.xy, ofsY + coord.x * mtx.yx + coord.y * mtx.yy };
		}

		inline CoordI unflipCoord(CoordI coord, GfxFlip flip, SizeI canvasSize )
		{
			const Matrix22& mtx = _unflipMatrix[(int)flip];
			const Matrix22& omtx = _unflipOrigoMatrix[(int)flip];

			int ofsX = canvasSize.w * omtx.xx + canvasSize.h * omtx.xy;
			int ofsY = canvasSize.w * omtx.yx + canvasSize.h * omtx.yy;
			
			return { ofsX + coord.x * mtx.xx + coord.y * mtx.xy, ofsY + coord.x * mtx.yx + coord.y * mtx.yy };
		}

		inline RectI flipRect(RectI rect, GfxFlip flip, SizeI canvasSize )
		{
			const Matrix22& mtx = _flipMatrix[(int)flip];
			const Matrix22& omtx = _flipOrigoMatrix[(int)flip];

			int ofsX = canvasSize.w * omtx.xx + canvasSize.h * omtx.xy;
			int ofsY = canvasSize.w * omtx.yx + canvasSize.h * omtx.yy;
			
			CoordI c1 = rect.topLeft();
			CoordI c2 = rect.bottomRight();

			CoordI trans1 = { c1.x * mtx.xx + c1.y * mtx.xy, c1.x * mtx.yx + c1.y * mtx.yy };
			CoordI trans2 = { c2.x * mtx.xx + c2.y * mtx.xy, c2.x * mtx.yx + c2.y * mtx.yy };

			CoordI topLeft = { std::min(trans1.x, trans2.x), std::min(trans1.y, trans2.y) };
			CoordI bottomRight = { std::max(trans1.x, trans2.x), std::max(trans1.y, trans2.y) };

			return { ofsX + topLeft.x, ofsY + topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
		}

		inline RectI unflipRect(RectI rect, GfxFlip flip, SizeI canvasSize )
		{
			const Matrix22& mtx = _unflipMatrix[(int)flip];
			const Matrix22& omtx = _unflipOrigoMatrix[(int)flip];

			int ofsX = canvasSize.w * omtx.xx + canvasSize.h * omtx.xy;
			int ofsY = canvasSize.w * omtx.yx + canvasSize.h * omtx.yy;
			
			CoordI c1 = rect.topLeft();
			CoordI c2 = rect.bottomRight();

			CoordI trans1 = { c1.x * mtx.xx + c1.y * mtx.xy, c1.x * mtx.yx + c1.y * mtx.yy };
			CoordI trans2 = { c2.x * mtx.xx + c2.y * mtx.xy, c2.x * mtx.yx + c2.y * mtx.yy };

			CoordI topLeft = { std::min(trans1.x, trans2.x), std::min(trans1.y, trans2.y) };
			CoordI bottomRight = { std::max(trans1.x, trans2.x), std::max(trans1.y, trans2.y) };

			return { ofsX + topLeft.x, ofsY + topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
		}

		inline binalInt toBinalInt(float floatSPX)
		{
			return binalInt(floatSPX * BINAL_MUL/64);
		}

		inline binalInt toBinalInt(spx intSPX)
		{
			return binalInt(intSPX) * BINAL_MUL/64;
		}

	
		inline RectI roundToPixels(const RectSPX& r);
		inline CoordI roundToPixels(const CoordSPX& r);
		inline SizeI roundToPixels(const SizeSPX& r);
		inline BorderI roundToPixels(const BorderSPX& r);
		inline int roundToPixels(spx r);

		inline pts spxToPts(spx value, int scale)
		{
			return pts(value)/scale;
		}

		inline Coord spxToPts(const CoordSPX& coord, int scale)
		{
			return Coord(pts(coord.x) / scale, pts(coord.y) / scale);
		}

		inline Size spxToPts(const SizeSPX& size, int scale)
		{
			return Size(pts(size.w) / scale, pts(size.h) / scale);
		}

		inline Border spxToPts(const BorderSPX& border, int scale)
		{
			return Border(	pts(border.top) / scale, pts(border.right) / scale,
							pts(border.bottom) / scale, pts(border.left) / scale);
		}

		inline Rect spxToPts(const RectSPX& rect, int scale)
		{
			return Rect(pts(rect.x) / scale, pts(rect.y) / scale,
						pts(rect.w) / scale, pts(rect.h) / scale);
		}


		inline spx ptsToSpx(pts value, int scale)
		{
			return spx(value * scale);
		}

		inline CornersSPX ptsToSpx(const Corners& corners, int scale)
		{
			return CornersSPX(spx(corners.topLeft * scale), spx(corners.topRight * scale),
							  spx(corners.bottomRight * scale), spx(corners.bottomLeft * scale));
		}

		inline CoordSPX ptsToSpx(const Coord& coord, int scale)
		{
			return CoordSPX(spx(coord.x * scale), spx(coord.y * scale));
		}

		inline SizeSPX ptsToSpx(const Size& size, int scale)
		{
			return SizeSPX(spx(size.w * scale), spx(size.h * scale));
		}

		inline BorderSPX ptsToSpx(const Border& border, int scale)
		{
			return BorderSPX(spx(border.top * scale), spx(border.right * scale),
				spx(border.bottom * scale), spx(border.left * scale));
		}

		inline RectSPX ptsToSpx(const Rect& rect, int scale)
		{
			return RectSPX(spx(rect.x * scale), spx(rect.y * scale),
				spx(rect.w * scale), spx(rect.h * scale));
		}

		/**
		 * Clamps the corner radii of a rounded box so that corners never overlap.
		 *
		 * Negative radii are set to 0. If the radii along any side of the box add up
		 * to more than the length of that side, all four radii are scaled down by the
		 * same factor, the smallest that makes them fit (as in CSS). The shape keeps its
		 * proportions and a radius larger than half the shorter side gives a pill.
		 *
		 * Radii are given as top-left, top-right, bottom-right, bottom-left.
		 */

		inline void clampCornerRadii(const SizeSPX& size, spx radius[4])
		{
			for (int i = 0; i < 4; i++)
				if (radius[i] < 0)
					radius[i] = 0;

			int64_t w = size.w > 0 ? size.w : 0;
			int64_t h = size.h > 0 ? size.h : 0;

			// Find the side needing most scaling, as the fraction num/den.

			int64_t num = 1, den = 1;

			auto check = [&](int64_t length, int64_t sum)
			{
				if (sum > length && length * den < num * sum)
				{
					num = length;
					den = sum;
				}
			};

			check(w, int64_t(radius[0]) + radius[1]);
			check(h, int64_t(radius[1]) + radius[2]);
			check(w, int64_t(radius[2]) + radius[3]);
			check(h, int64_t(radius[3]) + radius[0]);

			if (num < den)
			{
				for (int i = 0; i < 4; i++)
					radius[i] = spx(radius[i] * num / den);
			}
		}

		inline void clampCornerRadii(const SizeSPX& size, CornersSPX& radius)
		{
			spx r[4] = { radius.topLeft, radius.topRight, radius.bottomRight, radius.bottomLeft };
			clampCornerRadii(size, r);
			radius = CornersSPX(r[0], r[1], r[2], r[3]);
		}

		/**
		 * Checks if a point is inside a box with rounded corners. Radii are top-left, top-right,
		 * bottom-right, bottom-left, each corner circular and touching the two sides meeting
		 * there, as clamped by clampCornerRadii() for the box to look as drawn.
		 */

		inline bool isInsideRoundedBox(const CoordSPX& point, const RectSPX& box, const spx radius[4])
		{
			if (!box.contains(point))
				return false;

			int64_t x = point.x;
			int64_t y = point.y;

			auto outsideCorner = [&](int64_t cx, int64_t cy, int64_t r, bool bLeft, bool bTop)
			{
				if ((bLeft ? x >= cx : x < cx) || (bTop ? y >= cy : y < cy))
					return false;

				return (x - cx) * (x - cx) + (y - cy) * (y - cy) > r * r;
			};

			int64_t left = box.x, top = box.y, right = int64_t(box.x) + box.w, bottom = int64_t(box.y) + box.h;

			return !(outsideCorner(left + radius[0], top + radius[0], radius[0], true, true) ||
					 outsideCorner(right - radius[1], top + radius[1], radius[1], false, true) ||
					 outsideCorner(right - radius[2], bottom - radius[2], radius[2], false, false) ||
					 outsideCorner(left + radius[3], bottom - radius[3], radius[3], true, false));
		}

		inline bool isInsideRoundedBox(const CoordSPX& point, const RectSPX& box, const CornersSPX& radius)
		{
			spx r[4] = { radius.topLeft, radius.topRight, radius.bottomRight, radius.bottomLeft };
			return isInsideRoundedBox(point, box, r);
		}

		/**
		 * Radii of the inner corners of an outline inside a box with the given (clamped)
		 * corner radii, as drawn by Device::drawRoundedBox(): the radius minus the thicker
		 * of the two sides meeting there, circular and touching both sides of the inside.
		 * With even outlines they are concentric with the outer corners. With uneven ones
		 * they can be larger than the inside.
		 */

		inline void innerCornerRadii(const spx radius[4], const BorderSPX& outline, spx innerRadius[4])
		{
			innerRadius[0] = std::max(radius[0] - std::max(outline.left, outline.top), 0);
			innerRadius[1] = std::max(radius[1] - std::max(outline.top, outline.right), 0);
			innerRadius[2] = std::max(radius[2] - std::max(outline.right, outline.bottom), 0);
			innerRadius[3] = std::max(radius[3] - std::max(outline.bottom, outline.left), 0);
		}

		inline CornersSPX innerCornerRadii(const CornersSPX& radius, const BorderSPX& outline)
		{
			spx r[4] = { radius.topLeft, radius.topRight, radius.bottomRight, radius.bottomLeft };
			spx inner[4];
			innerCornerRadii(r, outline, inner);
			return CornersSPX(inner[0], inner[1], inner[2], inner[3]);
		}

		inline spx align(spx x)
		{
			return (x + 32) & 0xFFFFFFC0;
		}

		inline CoordSPX align(const CoordSPX& coord)
		{
			return { (coord.x + 32) & -64, (coord.y + 32) & -64 };
		}

		inline SizeSPX align(const SizeSPX& size)
		{
			return { (size.w + 32) & -64, (size.h + 32) & -64 };
		}

		inline BorderSPX align(const BorderSPX& border)
		{
			return { (border.top + 32) & -64, (border.right + 32) & -64,
					 (border.bottom + 32) & -64, (border.left + 32) & -64 };
		}

		inline RectSPX align(const RectSPX& rect)
		{
			RectSPX out;

			out.x = (rect.x + 32) & -64;
			out.y = (rect.y + 32) & -64;

			out.w = ((rect.x + rect.w + 32) & -64) - out.x;
			out.h = ((rect.y + rect.h + 32) & -64) - out.y;

			return out;
		}

		inline spx alignUp(spx x)
		{
			return (x + 63) & 0xFFFFFFC0;
		}

		inline CoordSPX alignUp(const CoordSPX& coord)
		{
			return { (coord.x + 63) & -64, (coord.y + 63) & -64 };
		}

		inline SizeSPX alignUp(const SizeSPX& size)
		{
			return { (size.w + 63) & -64, (size.h + 63) & -64 };
		}

		inline BorderSPX alignUp(const BorderSPX& border)
		{
			return { (border.top + 63) & -64, (border.right + 63) & -64,
					 (border.bottom + 63) & -64, (border.left + 63) & -64 };
		}

		inline RectSPX alignUp(const RectSPX& rect)
		{
			//NOTE: Actually aligns down position in order to expand rectangle outwards.

			RectSPX out;

			out.x = (rect.x) & -64;
			out.y = (rect.y) & -64;

			out.w = ((rect.x + rect.w + 63) & -64) - out.x;
			out.h = ((rect.y + rect.h + 63) & -64) - out.y;

			return out;
		}

		inline spx alignDown(spx x)
		{
			return (x) & 0xFFFFFFC0;
		}

		inline CoordSPX alignDown(const CoordSPX& coord)
		{
			return { (coord.x) & -64, (coord.y) & -64 };
		}

		inline SizeSPX alignDown(const SizeSPX& size)
		{
			return { (size.w) & -64, (size.h) & -64 };
		}

		inline BorderSPX alignDown(const BorderSPX& border)
		{
			return { (border.top) & -64, (border.right) & -64,
					 (border.bottom) & -64, (border.left) & -64 };
		}

		inline RectSPX alignDown(const RectSPX& rect)
		{
			//NOTE: Actually aligns down position in order to expand rectangle outwards.

			RectSPX out;

			out.x = (rect.x + 63) & -64;
			out.y = (rect.y + 63) & -64;

			out.w = ((rect.x + rect.w) & -64) - out.x;
			out.h = ((rect.y + rect.h) & -64) - out.y;

			return out;
		}

		inline bool isAligned(spx value)
		{
			return ( (value & 63) == 0 );
		}

		inline bool isAligned(const CoordSPX& coord)
		{
			return ( ((coord.x | coord.y) & 63) == 0 );
		}

		inline bool isAligned(const SizeSPX& size)
		{
			return ( (( size.w | size.h) & 63) == 0 );
		}


		inline bool isAligned(const RectSPX& rect)
		{
			return ( ((rect.x | rect.y | rect.w | rect.h) & 63) == 0 );
		}

		double	squareRoot(double a);
		double	powerOfTen(int num);
		int		gcd(int a, int b);


		const PixelDescription& pixelFormatToDescription( PixelFormat format );						// Byte order of the system.
		PixelDescription		pixelFormatToDescription( PixelFormat format, bool bBigEndian );
		PixelFormat				pixelDescriptionToFormat(const PixelDescription& description);		// Byte order is ignored.
	

		uint32_t mostSignificantBit(uint32_t value);


		extern uint8_t _limitUint8Table[768];

		inline uint8_t limitUint8( int value )
		{
			return _limitUint8Table[value+256];
		}

		inline uint32_t endianSwap( uint32_t in )
		{
			return ((in & 0xff000000) >> 24) | ((in & 0x00ff0000) >> 8) | ((in & 0x0000ff00) << 8) | (in << 24);
		}

		inline uint16_t endianSwap( uint16_t in )
		{
			return (in >> 8 | in << 8);
		}

		constexpr bool isSystemBigEndian()
		{
	#if WAX_IS_LITTLE_ENDIAN
			return false;
	#else
			return true;
	#endif
		}


		inline uint32_t alignUp8(uint32_t value);

	}

	RectI Util::roundToPixels(const RectSPX& r)
	{
		int x2 = r.x + r.w;
		int y2 = r.y + r.h;

		RectI out;

		out.x = (r.x + 32) >> 6;
		out.y = (r.y + 32) >> 6;

		out.w = ((x2 + 32) >> 6) - out.x;
		out.h = ((y2 + 32) >> 6) - out.y;

		return out;
	}

	CoordI Util::roundToPixels(const CoordSPX& r)
	{
		return { (r.x + 32) >> 6, (r.y + 32) >> 6 };
	}

	SizeI Util::roundToPixels(const SizeSPX& r)
	{
		return { (r.w + 32) >> 6, (r.h + 32) >> 6 };
	}

	BorderI Util::roundToPixels(const BorderSPX& r)
	{
		return { (r.top + 32) >> 6, (r.right + 32) >> 6, (r.bottom + 32) >> 6, (r.left + 32) >> 6 };
	}


	int Util::roundToPixels(spx r)
	{
		return (r + 32) >> 6;
	}

	uint32_t Util::alignUp8(uint32_t value)
	{
		return (value + 7) & 0xFFFFFFF8;
	}

} // namespace wax
#endif // WAX_UTIL_DOT_H
