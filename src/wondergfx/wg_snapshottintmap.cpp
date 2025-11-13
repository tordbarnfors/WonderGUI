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


#include <wg_snapshottintmap.h>
#include <wg_gfxbase.h>

namespace wg
{

	const TypeInfo SnapshotTintmap::TYPEINFO = { "SnapshotTintmap", &Tintmap::TYPEINFO };

	//____ create() ___________________________________________________________

	SnapshotTintmap_p SnapshotTintmap::create(const Blueprint& bp)
	{
		if (!bp.from || !bp.to || !bp.transition || bp.timestamp < 0 )
		{
			//TODO: Error handling!

			return nullptr;
		}

		return SnapshotTintmap_p(new SnapshotTintmap(bp.from,bp.to,bp.transition,bp.timestamp));
	}

	SnapshotTintmap_p SnapshotTintmap::create(Tintmap * pFrom, Tintmap * pTo, ColorTransition * pTransition, int timestamp)
	{
		if (!pFrom || !pTo || !pTransition || timestamp < 0 )
		{
			//TODO: Error handling!

			return nullptr;
		}

		return SnapshotTintmap_p(new SnapshotTintmap(pFrom, pTo, pTransition, timestamp));
	}



	//____ constructor ________________________________________________________

	SnapshotTintmap::SnapshotTintmap(Tintmap * pFrom, Tintmap * pTo, ColorTransition * pTransition, int timestamp)
	: 	m_pFrom(pFrom),
		m_pTo(pTo),
		m_pTransition(pTransition),
		m_timestamp(timestamp)
	{
		_setFlags();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SnapshotTintmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ exportColors() ________________________________________________

	void SnapshotTintmap::exportColors(SizeI tintmapSize, HiColor* pOutputX, HiColor* pOutputY )
	{
		//TODO: Optimize! No need to blend colors in a direction that only source or destination (or neither) have.

		HiColor * pFromX = nullptr, * pToX = nullptr;
		HiColor * pFromY = nullptr, * pToY = nullptr;

		int bufferSizeX = 0;
		int bufferSizeY = 0;

		if( pOutputX )
		{
			bufferSizeX = sizeof(HiColor) * tintmapSize.w * 2;
			pFromX = (HiColor*) GfxBase::memStackAlloc(bufferSizeX);
			pToX = pFromX + tintmapSize.w;
		}

		if( pOutputY )
		{
			bufferSizeY = sizeof(HiColor) * tintmapSize.h * 2;
			pFromY = (HiColor*) GfxBase::memStackAlloc(bufferSizeY);
			pToY = pFromY + tintmapSize.h;
		}

		m_pFrom->exportColors(tintmapSize, pFromX, pFromY);
		m_pTo->exportColors(tintmapSize, pToX, pToY);

		if( pOutputX )
			m_pTransition->snapshot(m_timestamp, tintmapSize.w, pFromX, pToX, pOutputX);

		if( pOutputY )
			m_pTransition->snapshot(m_timestamp, tintmapSize.h, pFromY, pToY, pOutputY);

		// Note: rember to do this in inverse allocation order.

		GfxBase::memStackFree(bufferSizeY);
		GfxBase::memStackFree(bufferSizeX);
	}

	//____ exportGradient() ______________________________________________________

	Gradient SnapshotTintmap::exportGradient()
	{
		Gradient g1 = m_pFrom->exportGradient();
		Gradient g2 = m_pTo->exportGradient();

		Gradient out;

		out.topLeft = m_pTransition->snapshot(m_timestamp, g1.topLeft, g2.topLeft );
		out.topRight = m_pTransition->snapshot(m_timestamp, g1.topRight, g2.topRight );
		out.bottomLeft = m_pTransition->snapshot(m_timestamp, g1.bottomLeft, g2.bottomLeft );
		out.bottomRight = m_pTransition->snapshot(m_timestamp, g1.bottomRight, g2.bottomRight );

		return out;
	}

	//____ alpha() _______________________________________________________________

	int SnapshotTintmap::alpha( const CoordSPX& pos, const RectSPX& area )
	{
		HiColor from = HiColor::White;
		HiColor to = HiColor::White;

		from.a = m_pFrom->alpha(pos, area);
		to.a = m_pTo->alpha(pos, area);

		return m_pTransition->snapshot(m_timestamp, from, to ).a;
	}

	//____ _setFlags() ________________________________________________________

	void SnapshotTintmap::_setFlags()
	{
		m_bOpaque = m_pFrom->isOpaque() && m_pTo->isOpaque();
		m_bHorizontal = m_pFrom->isHorizontal() || m_pTo->isHorizontal();
		m_bVertical = m_pFrom->isVertical() || m_pTo->isVertical();
	}
}



