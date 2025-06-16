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

#ifndef WG_UTIL_DOT_H
#define WG_UTIL_DOT_H
#pragma once

#include <wg_gfxutil.h>
#include <wg_types.h>
#include <wg_geo.h>
#include <wg_patches.h>
#include <wg_state.h>

namespace wg
{

	class GfxDevice;
	class Surface;
	struct NinePatch;

	//____ Util _________________________________________________________________

	namespace Util		/** @private */
	{

		bool		markTestStretchRect( CoordSPX ofs, Surface * pSurface, const RectSPX& source, const RectSPX& area, int opacityTreshold );
		bool		markTestTileRect(CoordSPX ofs, Surface* pSurface, const RectSPX& area, int scale, int opacityTreshold);
		bool		markTestNinePatch(CoordSPX ofs, Surface* pSurface, const NinePatch& patch, const RectSPX& _dest, int scale, int opacityTreshold);


		CoordSPX 	placementToOfs( Placement placement, RectSPX base );
		RectSPX		placementToRect( Placement placement, RectSPX base, SizeSPX rect );
		CoordSPX	placeRectAroundCoord(Placement placement, CoordSPX coord, SizeSPX rectSize);

		SizeSPX		sizeFromPolicy( SizePolicy2D sizePolicy, SizeSPX maxSize, SizeSPX orgSize );
		RectSPX 	rectFromPolicy( SizePolicy2D sizePolicy, Placement placement, RectSPX bounds, SizeSPX orgSize );

	
		SizeSPX		scaleToFit(SizeSPX object, SizeSPX boundaries);

		spx 		sizeFromConstraint( spx defaultSize, spx specifiedSize, SizeConstraint policy );

		inline Axis dirToAxis( Direction dir ) { return (dir == Direction::Up || dir == Direction::Down) ? Axis::Y : Axis::X; }

		std::tuple<int,int,int> calcStateToIndexParam(int nbStates, const State* pStates);					// returns: indexTabEntries, mask, shift
		void 		generateStateToIndexTab(uint8_t * pDest, int nbStates, const State* pStates);


		struct ClipPopData				/** @private */
		{
			ClipPopData() : bInitialized(false) {}
			ClipPopData(int _nRects, const RectSPX * _pRects, int _reservedMem ) : nRects(_nRects), pRects(_pRects), reservedMem(_reservedMem), bInitialized(true) {}

			const RectSPX * pRects;
			int nRects;
			int reservedMem;
			bool bInitialized;
		};

		ClipPopData	patchesToClipList( GfxDevice * pDevice, const RectSPX& clip, const PatchesSPX& patches );
		ClipPopData	patchesToClipList( GfxDevice * pDevice, const PatchesSPX& patches );
		ClipPopData limitClipList( GfxDevice * pDevice, const RectSPX& clip );
		ClipPopData pushClipList(GfxDevice * pDevice);
		void 		popClipList( GfxDevice * pDevice, const ClipPopData& popData );


		// A simple checksum algorithm that just performs a long division
		// with a standard CRC polynomial. Quicker and less complex than a standard
		// CRC on modern hardware with FPU but probably gives a checksum of
		// somewhat lower quality.

		class Checksum8
		{
		public:
			Checksum8() { remainder = 0; }

			inline void add8( uint8_t x ) { remainder = ((remainder << 8) + x)%dividend;}
			inline void add16( uint16_t x ) { remainder = ((remainder << 16) + x)%dividend;}
			inline void add32( uint32_t x ) { remainder = (uint32_t)(((((uint64_t)remainder) << 32) + x)%dividend);}

			void add( const void * pData, uint32_t nBytes );

			uint8_t getChecksum() { return (remainder & 0xFF); }
			inline void clear() { remainder = 0; }

		private:
			uint32_t	remainder;
			const static int dividend = 0x107;
		};
	}

} // namespace wg
#endif // WG_UTIL_DOT_H
