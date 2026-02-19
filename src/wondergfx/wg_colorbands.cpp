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
#include <wg_colorbands.h>
#include <wg_gfxbase.h>

namespace wg
{

	const TypeInfo Colorbands::TYPEINFO = { "Colorbands", &Tintmap::TYPEINFO };

	//____ create() ___________________________________________________________

	Colorbands_p Colorbands::create(const Blueprint& blueprint)
	{
		auto pColorbands = Colorbands_p(new Colorbands(blueprint.axis, blueprint.bands.data(), blueprint.bands.data() + blueprint.bands.size()));
		if (blueprint.finalizer)
			pColorbands->setFinalizer(blueprint.finalizer);
		return pColorbands;
	}

	Colorbands_p Colorbands::create(Axis axis, const std::initializer_list<Band> bands)
	{
		return Colorbands_p(new Colorbands(axis, bands.begin(), bands.end() ));
	}

	Colorbands_p Colorbands::create(Axis axis, const Band* begin, const Band* end)
	{
		return Colorbands_p(new Colorbands(axis, begin, end));
	}


	//____ constructor ________________________________________________________

	Colorbands::Colorbands(Axis axis, const Band* begin, const Band* end)
	{
		m_axis = axis;
		m_bands.insert(m_bands.end(), begin, end);
		m_bands.back().end = 1.f;

		bool bCorrected = false;
		float prevEnd = 0.f;
		for (auto& band : m_bands)
		{
			if (band.end < prevEnd || band.end > 1.f)
			{
				limit(band.end, prevEnd, 1.f);
				bCorrected = true;
			}
		}

		if (bCorrected)
			GfxBase::throwError(ErrorLevel::Warning, ErrorCode::InvalidParam, "Band end value out of bounds. corrected.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);

		_setFlags();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Colorbands::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ exportColors() ______________________________________________

	void Colorbands::exportColors(SizeI tintmapSize, HiColor* pOutputX, HiColor* pOutputY)
	{
		if (tintmapSize.w <= 0 || tintmapSize.h <= 0)
		{
			GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Invalid tintmap size specified.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		if( m_axis == Axis::X )
		{
			if( pOutputX )
				_export(tintmapSize.w, pOutputX);

			if (pOutputY)
				_fill(tintmapSize.h, pOutputY, HiColor::White);
		}
		else
		{
			if (pOutputX)
				_fill(tintmapSize.w, pOutputX, HiColor::White);

			if (pOutputY)
				_export(tintmapSize.h, pOutputY);

		}

	}

	//____ alpha() _______________________________________________________________

	int Colorbands::alpha(const CoordSPX& pos, const RectSPX& area)
	{
		if (m_axis == Axis::X)
		{
			float ofs = float(pos.x) / area.w;
			for (const Band& band : m_bands)
			{
				if (ofs < band.end)
					return band.color.a;
			}
		}
		else
		{
			float ofs = float(pos.y) / area.h;
			for (const Band& band : m_bands)
			{
				if (ofs < band.end)
					return band.color.a;
			}
		}

		return m_bands.back().color.a;
	}

	//____ _export() _______________________________________________________________

	void Colorbands::_export(int entries, HiColor* pDest)
	{
		int ofs = 0;
		for (auto& band : m_bands)
		{
			int end = int(band.end * entries);
			while(ofs < end)
				pDest[ofs++] = band.color;
		}
	}

	//____ _setFlags() ________________________________________________________

	void Colorbands::_setFlags()
	{
		for (const Band& band : m_bands)
		{
			if (band.color.a != 4096)
			{
				m_bOpaque = false;
				break;
			}
		}

		m_bHorizontal = m_axis == Axis::X;
		m_bVertical = m_axis == Axis::Y;
	}
}



