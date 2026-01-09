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
#include <wg_statictintmap.h>
#include <wg_gfxbase.h>

namespace wg
{

	const TypeInfo StaticTintmap::TYPEINFO = { "StaticTintmap", &Tintmap::TYPEINFO };

	//____ create() ___________________________________________________________

	StaticTintmap_p	StaticTintmap::create(SizeI size, const HiColor * pColorstripX, const HiColor * pColorstripY)
	{
		return StaticTintmap_p(new StaticTintmap(size, pColorstripX, pColorstripY));
	}


	//____ constructor ________________________________________________________

	StaticTintmap::StaticTintmap(SizeI size, const HiColor * pColorstripX, const HiColor * pColorstripY)
	{
		m_size.w = pColorstripX ? size.w : 0;
		m_size.h = pColorstripY ? size.h : 0;

		int totalColors = m_size.w + m_size.h;

		m_pColors = new HiColor[totalColors];

		memcpy(m_pColors, pColorstripX, m_size.w * sizeof(HiColor) );
		memcpy(m_pColors + m_size.w, pColorstripY, m_size.h * sizeof(HiColor) );

		int alpha = 0;
		for( int i = 0 ; i < totalColors ; i++ )
			alpha += m_pColors[i].a;

		m_bOpaque = (alpha == totalColors * 4096);

		m_bHorizontal = pColorstripX != nullptr;
		m_bVertical = pColorstripY != nullptr;
	}

	//____ destructor ____________________________________________________________

	StaticTintmap::~StaticTintmap()
	{
		delete [] m_pColors;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticTintmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ exportColors() ______________________________________________

	void StaticTintmap::exportColors(SizeI tintmapSize, HiColor* pOutputX, HiColor* pOutputY )
	{
		if( pOutputX )
		{
			int copyAmount = std::min(tintmapSize.w, m_size.w);

			if(copyAmount > 0)
				memcpy( pOutputX, m_pColors, copyAmount * sizeof(HiColor));

			if( copyAmount < tintmapSize.w )
			{
				for( int i = copyAmount ; i < tintmapSize.w ; i++ )
					pOutputX[i] = HiColor::White;
			}
		}

		if( pOutputY )
		{
			int copyAmount = std::min(tintmapSize.h, m_size.h);

			if(copyAmount > 0)
				memcpy( pOutputY, m_pColors + m_size.w, copyAmount * sizeof(HiColor));

			if( copyAmount < tintmapSize.h )
			{
				for( int i = copyAmount ; i < tintmapSize.h ; i++ )
					pOutputY[i] = HiColor::White;
			}
		}
	}

	//____ alpha() _______________________________________________________________

	int StaticTintmap::alpha( const CoordSPX& pos, const RectSPX& area )
	{
		int horizontalAlpha = pos.x < m_size.w ? m_pColors[pos.x].a : 4096;
		int verticalAlpha = pos.y < m_size.h ? m_pColors[m_size.w + pos.y].a : 4096;

		return verticalAlpha * horizontalAlpha;
	}
}



