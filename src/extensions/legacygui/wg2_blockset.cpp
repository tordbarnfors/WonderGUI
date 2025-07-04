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

#include <new>

#include <wg2_blockset.h>


//____ CreateFromRow() _________________________________________________________

wg::BlockSkin_p WgBlockset::CreateFromRow( wg::Surface * pSurf, const WgRect& rect, int nBlocks, int padding, int flags )
{
	int w = (rect.w - (nBlocks-1)*padding)/nBlocks;
	int h = rect.h;

	wg::CoordI ofs = { w + padding, 0 };

	wg::BlockSkin::Blueprint bp;
	bp.surface = pSurf;
	bp.firstBlock = wg::Rect(rect);
	bp.blockSpacing = padding;
	bp.axis = wg::Axis::X;
	
	if( nBlocks > 1 )
	{
		bp.states[0].state = wg::StateEnum::Hovered;
		//		p->setBlock(wg::StateEnum::Hovered, rect + ofs);
	}
	if( nBlocks > 2 )
	{
		bp.states[1].state = wg::StateEnum::Pressed;
		bp.states[2].state = wg::StateEnum::Selected;
//		p->setBlock(wg::StateEnum::Pressed, rect + ofs*2);
//		p->setBlock(wg::StateEnum::Selected, rect + ofs*2);
	}

	if( nBlocks > 3 )
	{
		bp.states[3].state = wg::StateEnum::Disabled;
//			p->setBlock(wg::StateEnum::Disabled, rect + ofs*3);
	}
	if( nBlocks > 4 )
	{
		bp.states[4].state = wg::State::SelectedHovered;
		bp.states[5].state = wg::State::SelectedPressed;
//		p->setBlock(wg::StateEnum::SelectedHovered, rect + ofs*4);
//		p->setBlock(wg::StateEnum::SelectedPressed, rect + ofs*4);
	}

	auto p = wg::BlockSkin::create(bp);
	return p;
}

//____ CreateFromColumn() ______________________________________________________

wg::BlockSkin_p WgBlockset::CreateFromColumn( wg::Surface * pSurf, const WgRect& rect, int nBlocks, int padding, int flags )
{
	int w = rect.w;
	int h = (rect.h - (nBlocks-1)*padding)/nBlocks;

	wg::CoordI ofs = { 0, h + padding };

	wg::BlockSkin::Blueprint bp;
	bp.surface = pSurf;
	bp.firstBlock = wg::Rect(rect);
	bp.blockSpacing = padding;
	bp.axis = wg::Axis::Y;
	
	if( nBlocks > 1 )
	{
		bp.states.push_back(wg::State(wg::StateEnum::Hovered) );
	}
	if( nBlocks > 2 )
	{
		bp.states.push_back(wg::State(wg::StateEnum::Pressed));
		bp.states.push_back(wg::State(wg::StateEnum::Selected));
//		p->setBlock(wg::StateEnum::Pressed, rect + ofs*2);
//		p->setBlock(wg::StateEnum::Selected, rect + ofs*2);
	}

	if( nBlocks > 3 )
	{
		bp.states.push_back(wg::State(wg::StateEnum::Disabled));
//			p->setBlock(wg::StateEnum::Disabled, rect + ofs*3);
	}
	if( nBlocks > 4 )
	{
		bp.states.push_back(wg::State(wg::State::SelectedHovered));
		bp.states.push_back(wg::State(wg::State::SelectedPressed));
//		p->setBlock(wg::StateEnum::SelectedHovered, rect + ofs*4);
//		p->setBlock(wg::StateEnum::SelectedPressed, rect + ofs*4);
	}

	auto p = wg::BlockSkin::create(bp);
	return p;
}



