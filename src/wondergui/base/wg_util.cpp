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

#include <wg_util.h>
#include <wg_geo.h>
#include <wg_surface.h>
#include <wg_gfxdevice.h>
#include <wg_patches.h>
#include <wg_base.h>
#include <wg_state.h>

#include <algorithm>

namespace wg
{

	//____ markTestStretchRect() __________________________________________________

	bool Util::markTestStretchRect( CoordSPX ofs, Surface * pSurface, const RectSPX& source, const RectSPX& area, int opacityTreshold )
	{
		// Sanity check & shortcuts.
		if( !pSurface || !area.contains(ofs) || source.isEmpty() || area.isEmpty() || opacityTreshold > 4096 )
			return false;

		if( pSurface->isOpaque() || opacityTreshold <= 0 )
			return true;

		// Make cordinates relative area.

		ofs.x -= area.x;
		ofs.y -= area.y;

		// Convert offset in area to offset in bitmap.

		CoordSPX sourceOfs;

		sourceOfs.x = (int) (ofs.x/((double)area.w) * source.w);
		sourceOfs.y = (int) (ofs.y/((double)area.h) * source.h);

		// Do alpha test

		int alpha = pSurface->alpha( sourceOfs );

		return (alpha >= opacityTreshold);
	}

	//____ markTestTileRect() __________________________________________________

	bool Util::markTestTileRect(CoordSPX ofs, Surface* pSurface, const RectSPX& area, int scale, int opacityTreshold)
	{
		// Sanity check & shortcuts.
		if (!pSurface || !area.contains(ofs) || area.isEmpty() || opacityTreshold > 4096)
			return false;

		if (pSurface->isOpaque() || opacityTreshold <= 0)
			return true;

		// Make cordinates relative area.

		ofs.x -= area.x;
		ofs.y -= area.y;

		// Convert offset in area to offset in bitmap.

		SizeSPX tileOnScreen = pSurface->pixelSize() * 64 * scale / pSurface->scale();

		CoordSPX sourceOfs = ofs * pSurface->scale() / scale;

		SizeI size = pSurface->pixelSize()*64;

		sourceOfs.x %= size.w;
		sourceOfs.y %= size.h;

		// Do alpha test

		int alpha = pSurface->alpha(sourceOfs);

		return (alpha >= opacityTreshold);
	}

	bool Util::markTestNinePatch(CoordSPX ofs, Surface* pSurface, const NinePatch& patch, const RectSPX& _dest, int scale, int opacityTreshold)
	{
		const BorderSPX destFrame = align(ptsToSpx(patch.frame, scale));
		const BorderSPX sourceFrame = align(ptsToSpx(patch.frame,pSurface->scale()));
		const RectSPX _source = align(ptsToSpx(patch.block, pSurface->scale()));

		const int surfScale = pSurface->scale();

		// Sanity check & shortcuts.

		if (sourceFrame.isEmpty() && patch.rigidPartXSections == YSections::None && patch.rigidPartYSections == XSections::None )
			return markTestStretchRect(ofs, pSurface, _source, _dest, opacityTreshold);

		if (!pSurface || !_dest.contains(ofs) || _source.isEmpty() || _dest.isEmpty() || opacityTreshold > 4096)
			return false;

		if (pSurface->isOpaque() || opacityTreshold <= 0)
			return true;

		//

		RectSPX source;
		RectSPX dest;

		XSections markedSectionX;
		YSections markedSectionY;

		if (ofs.x < _dest.x + destFrame.left)
		{
			// left section
			markedSectionX = XSections::Left; 

			source.x = _source.x;
			source.w = sourceFrame.left;
			dest.x = _dest.x;
			dest.w = destFrame.left;
		}
		else if (ofs.x < _dest.x + _dest.w - destFrame.right)
		{
			// mid section
			markedSectionX = XSections::Center;

			source.x = _source.x + sourceFrame.left;
			source.w = _source.w - sourceFrame.width();
			dest.x = _dest.x + destFrame.left;
			dest.w = _dest.w - destFrame.width();
		}
		else
		{
			// right section
			markedSectionX = XSections::Right;

			source.x = _source.x + _source.w - sourceFrame.right;
			source.w = sourceFrame.right;
			dest.x = _dest.x + _dest.w - destFrame.right;
			dest.w = destFrame.right;
		}

		if (ofs.y < _dest.y + destFrame.top)
		{
			// top section
			markedSectionY = YSections::Top;

			source.y = _source.y;
			source.h = sourceFrame.top;
			dest.y = _dest.y;
			dest.h = destFrame.top;
		}
		else if (ofs.y < _dest.y + _dest.h - destFrame.bottom)
		{
			// mid section
			markedSectionY = YSections::Center;

			source.y = _source.y + sourceFrame.top;
			source.h = _source.h - sourceFrame.height();
			dest.y = _dest.y + destFrame.top;
			dest.h = _dest.h - destFrame.height();
		}
		else
		{
			// bottom section
			markedSectionY = YSections::Bottom;

			source.y = _source.y + _source.h - sourceFrame.bottom;
			source.h = sourceFrame.bottom;
			dest.y = _dest.y + _dest.h - destFrame.bottom;
			dest.h = destFrame.bottom;
		}

		// Make coordinates relative area.

		ofs.x -= dest.x;
		ofs.y -= dest.y;


		// Convert offset in area to offset in bitmap.

//		This is a simplified version of code below which ignores rigid parts, since the calculation for the rigid parts is broken.

		{
			RectSPX sourceOfs;

			sourceOfs.x = (int)(ofs.x / ((double)dest.w) * source.w);
			sourceOfs.y = (int)(ofs.y / ((double)dest.h) * source.h);

			// Do alpha test

			int alpha = pSurface->alpha( CoordSPX(source.x + sourceOfs.x, source.y + sourceOfs.y) );

			return (alpha >= opacityTreshold);
		}
/*

		RectSPX sourceOfs;

		if (markedSectionX == XSections::Center && (patch.rigidPartXSections & markedSectionY) != YSections::None)
		{
			int leftSrcLen = patch.rigidPartXOfs*surfScale;
			int rightSrcLen = source.w - patch.rigidPartXOfs*surfScale - patch.rigidPartXLength*surfScale;

			int totalSrcLen = leftSrcLen + rightSrcLen;

			spx midDstLen = std::min(spx(patch.rigidPartXLength*surfScale), dest.w);

			spx leftDstLen = (dest.w - midDstLen) * leftSrcLen / totalSrcLen;
			spx rightDstLen = dest.w - midDstLen - leftDstLen;

			if (ofs.x < leftDstLen)
				sourceOfs.x = int(ofs.x / double(leftDstLen) * leftSrcLen);
			else if (ofs.x < leftDstLen + midDstLen)
				sourceOfs.x = int(ofs.x - leftDstLen) + leftSrcLen;
			else
				sourceOfs.x = int((ofs.x - leftDstLen - midDstLen) / double(rightDstLen) * rightSrcLen) + leftSrcLen + patch.rigidPartXLength*surfScale;
		}
		else
			sourceOfs.x = (int)(ofs.x / ((double)dest.w) * source.w);
	
		
		if (markedSectionY == YSections::Center && (patch.rigidPartYSections & markedSectionX) != XSections::None)
		{
			int topSrcLen = patch.rigidPartYOfs*surfScale;
			int bottomSrcLen = source.h - patch.rigidPartYOfs*surfScale - patch.rigidPartYLength*surfScale;

			int totalSrcLen = topSrcLen + bottomSrcLen;

			spx midDstLen = std::min(spx(patch.rigidPartYLength*surfScale), dest.h);

			spx topDstLen = (dest.h - midDstLen) * topSrcLen / totalSrcLen;
			spx bottomDstLen = dest.h - midDstLen - topDstLen;

			if (ofs.y < topDstLen)
				sourceOfs.y = int( ofs.y / double(topDstLen) * topSrcLen);
			else if (ofs.y < topDstLen + midDstLen)
				sourceOfs.y = int(ofs.y - topDstLen) + topSrcLen;
			else
				sourceOfs.y = int((ofs.y - topDstLen - midDstLen) / double(bottomDstLen) * bottomSrcLen) + topSrcLen + patch.rigidPartYLength*surfScale;
		}
		else
			sourceOfs.y = (int)(ofs.y / ((double)dest.h) * source.h);

		// Do alpha test

		int alpha = pSurface->alpha( CoordSPX(source.x + sourceOfs.x, source.y + sourceOfs.y) );

		return (alpha >= opacityTreshold);
 */

	}




	//____ sizeFromConstraint() __________________________________________________________

	spx Util::sizeFromConstraint( spx defaultSize, spx specifiedSize, SizeConstraint policy )
	{
		switch( policy )
		{
			case SizeConstraint::None:
				return defaultSize;
			case SizeConstraint::Equal:
				return specifiedSize;
			case SizeConstraint::LessOrEqual:
				if( defaultSize > specifiedSize )
					return specifiedSize;
			case SizeConstraint::GreaterOrEqual:
				if( defaultSize < specifiedSize )
					return specifiedSize;
		}
		return defaultSize;
	}

	//____ Checksum8::add() ________________________________________________________

	void Util::Checksum8::add( const void * pData, uint32_t nBytes )
	{
		uint32_t x = remainder;

		for( uint32_t i = 0 ; i < nBytes ; i++ )
			x = ((x << 8) + ((uint8_t*)pData)[i])%dividend;

		remainder = x;
	}

	//____ placementToOfs() ________________________________________________________

	CoordSPX Util::placementToOfs( Placement placement, RectSPX base )
	{
		switch( placement )
		{
			default:
			case Placement::NorthWest:
				return CoordSPX() + base.pos();

			case Placement::North:
				return CoordSPX( align(base.w/2),0 ) + base.pos();

			case Placement::NorthEast:
				return CoordSPX( base.w,0 ) + base.pos();

			case Placement::East:
				return CoordSPX( base.w, align(base.h/2) ) + base.pos();

			case Placement::SouthEast:
				return CoordSPX( base.w, base.h ) + base.pos();

			case Placement::South:
				return CoordSPX( align(base.w/2), base.h ) + base.pos();

			case Placement::SouthWest:
				return CoordSPX( 0, base.h ) + base.pos();

			case Placement::West:
				return CoordSPX( 0, align(base.h/2) ) + base.pos();

			case Placement::Center:
				return CoordSPX( align(base.w/2), align(base.h/2) ) + base.pos();
		}
	}

	//____ placementToRect() ________________________________________________________

	RectSPX Util::placementToRect( Placement placement, RectSPX bounds, SizeSPX size )
	{
		switch( placement )
		{
			default:
			case Placement::NorthWest:
				return RectSPX(0,0, size) + bounds.pos();

			case Placement::North:
				return RectSPX( align(bounds.w/2 - size.w/2), 0, size ) + bounds.pos();

			case Placement::NorthEast:
				return RectSPX( bounds.w - size.w, 0, size ) + bounds.pos();

			case Placement::East:
				return RectSPX( bounds.w - size.w, align(bounds.h/2 - size.h/2), size ) + bounds.pos();

			case Placement::SouthEast:
				return RectSPX( bounds.w - size.w, bounds.h - size.h, size ) + bounds.pos();

			case Placement::South:
				return RectSPX( align(bounds.w/2 - size.w/2), bounds.h - size.h, size ) + bounds.pos();

			case Placement::SouthWest:
				return RectSPX( 0, bounds.h - size.h, size ) + bounds.pos();

			case Placement::West:
				return RectSPX( 0, align(bounds.h/2 - size.h/2), size ) + bounds.pos();

			case Placement::Center:
				return RectSPX( align(bounds.w/2 - size.w/2), align(bounds.h/2 - size.h/2), size ) + bounds.pos();
		}
	}

	//____ placeRectAroundCoord() _____________________________________________

	CoordSPX Util::placeRectAroundCoord(Placement placement, CoordSPX coord, SizeSPX rectSize)
	{
		const static int placementMulX[Placement_size] = { -2, -2, -1, 0, 0, 0, -1, -2, -2, -1 };
		const static int placementMulY[Placement_size] = { -2, -2, -2, -2, -1, 0, 0, 0, -1, -1 };

		return coord + CoordSPX(placementMulX[int(placement)] * rectSize.w / 2, placementMulY[int(placement)] * rectSize.h / 2);
	}

	//____ rectFromPolicy() ______________________________________________________

	RectSPX Util::rectFromPolicy( SizePolicy2D sizePolicy, Placement placement, RectSPX bounds, SizeSPX orgSize )
	{
		RectSPX dest = align(placementToRect(placement, bounds, sizeFromPolicy(sizePolicy, bounds.size(), orgSize)));

		return dest;
	}

	//____ sizeFromPolicy() ______________________________________________________

	SizeSPX Util::sizeFromPolicy( SizePolicy2D policy, SizeSPX maxSize, SizeSPX orgSize )
	{
		if( maxSize.w <= 0 || maxSize.h <= 0 )
			return RectSPX();

		switch( policy )
		{
			default:
			case SizePolicy2D::Original:
				return orgSize;

			case SizePolicy2D::Stretch:
				return maxSize;

			case SizePolicy2D::Scale:
			{
				if( maxSize.w == 0 || maxSize.h == 0 )
					return {0,0};

				SizeSPX	size;

				float	fracX = orgSize.w / (float) maxSize.w;
				float	fracY = orgSize.h / (float) maxSize.h;

				if( fracX > fracY )
				{
					size.w = maxSize.w;
					size.h = int (orgSize.h / fracX);
				}
				else
				{
					size.h = maxSize.h;
					size.w = (int) (orgSize.w / fracY);
				}

				return size;
			}

			case SizePolicy2D::StretchDown:
			{
				SizeSPX size;
				size.w = std::min(orgSize.w,maxSize.w);
				size.h = std::min(orgSize.h,maxSize.h);
				return size;
			}

			case SizePolicy2D::ScaleDown:
			{
				if( maxSize.w == 0 || maxSize.h == 0 )
					return {0,0};

				SizeSPX	size;

				float	fracX = orgSize.w / (float) maxSize.w;
				float	fracY = orgSize.h / (float) maxSize.h;

				if( fracX <= 1.f && fracY <= 1.f )
					return orgSize;

				if( fracX > fracY )
				{
					size.w = maxSize.w;
					size.h = int (orgSize.h / fracX);
				}
				else
				{
					size.h = maxSize.h;
					size.w = (int) (orgSize.w / fracY);
				}

				return size;
			}
		}

	}

	//____ scaleToFit() _______________________________________________________

	SizeSPX Util::scaleToFit(SizeSPX object, SizeSPX boundaries)
	{
		float wScale = object.w / (float)boundaries.w;
		float hScale = object.h / (float)boundaries.h;

		float useScale = std::max(wScale, hScale);

		return object / useScale;
	}

	//____ generateStateToIndexTab() ___________________________________

	void Util::generateStateToIndexTab(uint8_t * pDest, int nbStates, const State* pStates)
	{
		int indexTableSize;
		int indexMask;
		int indexShift;

		std::tie(indexTableSize,indexMask,indexShift) = calcStateToIndexParam(nbStates,pStates);

		for( int i = 0 ; i < indexTableSize ; i++ )
		{
			int wanted = i << indexShift;
			pDest[i] = State( (StateEnum) wanted).bestMatch(nbStates, pStates);
		}
	}


	//____ calcStateToIndexParam() ________________________________________

	std::tuple<int,int,int> Util::calcStateToIndexParam(int nbStates, const State* pStates)
	{

		uint8_t	usedBits = 0;
		for (int i = 0; i < nbStates; i++)
			usedBits |= pStates[i].index();

		// We start with values excluding Disabled, which is a special case.

		int indexTableSize = 64;
		int indexMask = 0x3F;
		int indexShift = 0;

		if (usedBits == 0)
		{
			indexMask = 0;
			indexTableSize = 1;			// We always lookup default.
		}
		else
		{
			if ((usedBits & (int(StateEnum::Hovered) | int(StateEnum::Pressed))) == 0)
			{
				indexMask = 0xF;
				indexTableSize = 16;
				if ((usedBits & int(StateEnum::Focused)) == 0)
				{
					indexMask = 0x7;
					indexTableSize = 8;
					if ((usedBits & int(StateEnum::Checked)) == 0)
					{
						indexMask = 0x3;
						indexTableSize = 4;
						if ((usedBits & int(StateEnum::Selected)) == 0)
						{
							indexMask = 0x1;
							indexTableSize = 2;
						}
					}
				}
			}

			if ((usedBits & int(StateEnum::Flagged)) == 0)
			{
				indexShift++;
				indexTableSize /= 2;

				if ((usedBits & int(StateEnum::Selected)) == 0)
				{
					indexShift++;
					indexTableSize /= 2;

					if ((usedBits & int(StateEnum::Checked)) == 0)
					{
						indexShift++;
						indexTableSize /= 2;

						if ((usedBits & int(StateEnum::Focused)) == 0)
						{
							indexShift++;
							indexTableSize /= 2;

							if ((usedBits & (int(StateEnum::Hovered) | int(StateEnum::Pressed))) == 0)
							{
								indexShift += 2;
								indexTableSize /= 4;
							}
						}
					}
				}
			}

			// Handle disabled, a maximum of 8 entries are added

			if ((usedBits & int(StateEnum::Disabled)) != 0)
			{
				indexTableSize += 8 >> std::min(indexShift,3);
				indexMask |= int(StateEnum::Disabled);
			}
		}

		return std::make_tuple(indexTableSize,indexMask,indexShift);
	}

	//____ patchesToClipList() ____________________________________________________________________

	Util::ClipPopData Util::patchesToClipList( GfxDevice * pDevice, const RectSPX& clip, const PatchesSPX& patches )
	{
		int nOldRects 				= pDevice->clipListSize();
		const RectSPX * pOldRects 	= pDevice->clipList();

		int nRects = patches.size();
		const RectSPX * pRects = patches.begin();

		int allocSize = nRects * sizeof(RectSPX);

		RectSPX * pNewRects = (RectSPX*) Base::memStackAlloc(allocSize);
		int nNewRects = 0;

		for( int i = 0 ; i < nRects ; i++ )
		{
			pNewRects[nNewRects] = RectSPX::overlap(pRects[i], clip);
			if( !pNewRects[nNewRects].isEmpty() )
				nNewRects++;
		}

		pDevice->setClipList(nNewRects, pNewRects);
		return { nOldRects, pOldRects, allocSize };
	}

	Util::ClipPopData Util::patchesToClipList( GfxDevice * pDevice, const PatchesSPX& patches )
	{
		int nOldRects 				= pDevice->clipListSize();
		const RectI * pOldRects 	= pDevice->clipList();

		int nRects = patches.size();
		const RectSPX * pRects = patches.begin();

		int allocSize = nRects * sizeof(RectSPX);

		RectSPX * pNewRects = (RectSPX*) Base::memStackAlloc(allocSize);

		for( int i = 0 ; i < nRects ; i++ )
			pNewRects[i] = pRects[i];

		pDevice->setClipList(nRects, pNewRects);
		return { nOldRects, pOldRects, allocSize };
	}

	//____ limitClipList() ____________________________________________________________________

	Util::ClipPopData Util::limitClipList( GfxDevice * pDevice, const RectSPX& _clip )
	{
		RectSPX clip = _clip;

		if( clip.contains(pDevice->clipBounds()))
			return ClipPopData();

		int nRects 				= pDevice->clipListSize();
		const RectSPX * pRects 	= pDevice->clipList();
		int allocSize = nRects * sizeof(RectSPX);

		RectSPX * pNewRects = (RectSPX*) Base::memStackAlloc(allocSize);
		int nNewRects = 0;

		for( int i = 0 ; i < nRects ; i++ )
		{
			if (clip.isOverlapping(pRects[i]))
				pNewRects[nNewRects++] = RectSPX::overlap(pRects[i], clip);
		}

		pDevice->setClipList(nNewRects, pNewRects);
		return { nRects, pRects, allocSize };
	}

	//____ pushClipList() __________________________________________________________________

	Util::ClipPopData Util::pushClipList(GfxDevice * pDevice)
	{
		return { pDevice->clipListSize(), pDevice->clipList(), 0 };
	}


	//____ popClipList() ____________________________________________________________________

	void Util::popClipList( GfxDevice * pDevice, const ClipPopData& popData )
	{
		if( popData.bInitialized )
		{
			pDevice->setClipList(popData.nRects, popData.pRects);
			Base::memStackFree(popData.reservedMem);
		}
	}

} // namespace wg
