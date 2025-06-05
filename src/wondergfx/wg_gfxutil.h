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

#ifndef WG_GFXUTIL_DOT_H
#define WG_GFXUTIL_DOT_H
#pragma once

#include <wg_geo.h>
#include <wg_gfxtypes.h>
#include <wg_patches.h>

#include <algorithm>

namespace wg
{

	class GfxDevice;

	//____ Util _________________________________________________________________

	namespace Util		/** @private */
	{
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




		double	squareRoot(double a);
		double	powerOfTen(int num);
		int		gcd(int a, int b);


		const PixelDescription& pixelFormatToDescription( PixelFormat format );
		PixelFormat	pixelDescriptionToFormat(const PixelDescription& description);

		PixelFormat clarifyPixelFormat( PixelFormat format);	// Translate pixel types without colorSpace info to ones with.
	

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
	#if WG_IS_LITTLE_ENDIAN
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
		return (value + 7) & 0xFFFFFFFF;
	}

} // namespace wg
#endif // WG_GFXUTIL_DOT_H
