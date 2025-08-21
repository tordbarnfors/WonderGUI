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

	//____ exportHorizontalColors() ______________________________________________

	void SnapshotTintmap::exportHorizontalColors(spx length, HiColor* pOutput)
	{
		if( m_bHorizontal )
		{
			int nColors = length / 64;

			int bufferSize = sizeof(HiColor) * nColors * 2;
			auto pBuffer = (HiColor*) GfxBase::memStackAlloc(bufferSize);

			auto pFromColors = pBuffer;
			auto pToColors = pBuffer + nColors;

			m_pFrom->exportHorizontalColors(length, pFromColors);
			m_pTo->exportHorizontalColors(length, pToColors);

			m_pTransition->snapshot(m_timestamp, nColors, pFromColors, pToColors, pOutput);

			GfxBase::memStackFree(bufferSize);
		}
		else
			_fill(length, pOutput, HiColor::White);
	}

	//____ exportVerticalColors() ________________________________________________

	void SnapshotTintmap::exportVerticalColors(spx length, HiColor* pOutput)
	{
		if( m_bVertical )
		{
			int nColors = length / 64;

			int bufferSize = sizeof(HiColor) * nColors * 2;
			auto pBuffer = (HiColor*) GfxBase::memStackAlloc(bufferSize);

			auto pFromColors = pBuffer;
			auto pToColors = pBuffer + nColors;

			m_pFrom->exportVerticalColors(length, pFromColors);
			m_pTo->exportVerticalColors(length, pToColors);

			m_pTransition->snapshot(m_timestamp, nColors, pFromColors, pToColors, pOutput);

			GfxBase::memStackFree(bufferSize);
		}
		else
			_fill(length, pOutput, HiColor::White);
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


	//____ _setFlags() ________________________________________________________

	void SnapshotTintmap::_setFlags()
	{
		m_bOpaque = m_pFrom->isOpaque() && m_pTo->isOpaque();
		m_bHorizontal = m_pFrom->isHorizontal() || m_pTo->isHorizontal();
		m_bVertical = m_pFrom->isVertical() || m_pTo->isVertical();
	}
}



