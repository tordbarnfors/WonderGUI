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
#include <wax_cylinderwarp.h>
#include <wax_base.h>

#include <algorithm>
#include <cmath>
#include <vector>

namespace wax
{
	const TypeInfo CylinderWarp::TYPEINFO = { "CylinderWarp", &WarpGenerator::TYPEINFO };

	//____ create() ___________________________________________________________

	CylinderWarp_p CylinderWarp::create(const Blueprint& bp, WarpmapFactory* pFactory)
	{
		if (!_validateSettings(bp.size, bp.sourceSize, bp.precision) || (bp.axis != Axis::X && bp.axis != Axis::Y) ||
			!_isValidSpan(bp.span) || bp.sweep == 0.f)
			return nullptr;

		if (!pFactory)
			pFactory = Base::defaultWarpmapFactory();

		if (!pFactory)
			return nullptr;

		return CylinderWarp_p(new CylinderWarp(bp, pFactory));
	}

	//____ constructor ________________________________________________________

	CylinderWarp::CylinderWarp(const Blueprint& bp, WarpmapFactory* pFactory)
		: WarpGenerator(bp.size, bp.sourceSize, bp.precision, pFactory)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		m_axis			= bp.axis;
		m_span			= bp.span;
		m_startAngle	= bp.startAngle;
		m_sweep			= bp.sweep;

		m_generatedAxis		= bp.axis;
		m_offsetFactor		= _offsetFactor(bp.sweep / 360.0, _lengthAcross(bp.axis, bp.sourceSize));
	}

	//____ destructor _________________________________________________________

	CylinderWarp::~CylinderWarp()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& CylinderWarp::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setAxis() __________________________________________________________

	bool CylinderWarp::setAxis(Axis axis)
	{
		if (axis != Axis::X && axis != Axis::Y)
			return false;

		if (axis != m_axis)
		{
			m_axis = axis;
			m_bDirty = true;
		}
		return true;
	}

	//____ setSpan() __________________________________________________________

	bool CylinderWarp::setSpan(float span)
	{
		if (!_isValidSpan(span))
			return false;

		if (span != m_span)
		{
			m_span = span;
			m_bDirty = true;
		}
		return true;
	}

	//____ setAngles() ________________________________________________________

	bool CylinderWarp::setAngles(float startAngle, float sweep)
	{
		if (sweep == 0.f)
			return false;

		if (startAngle != m_startAngle || sweep != m_sweep)
		{
			m_startAngle = startAngle;
			m_sweep = sweep;
			m_bDirty = true;
		}
		return true;
	}

	//____ rotationOffset() ___________________________________________________

	CoordSPX CylinderWarp::rotationOffset(float degrees) const
	{
		spx offset = _offsetFor(_toTurn(degrees * (1.0 / 360)), m_offsetFactor);
		return m_generatedAxis == Axis::X ? CoordSPX(0, offset) : CoordSPX(offset, 0);
	}

	//____ _strips() _________________________________________________________
	/*
		Edges of the strips across the axis, in 1/256 spx from the start. Across it, the angle seen
		at distance y from the middle is asin(y / R), R being the radius. Interpolating that over
		strips of c pixels is off by about c^2 / 8 times its second derivative, which in pixels on
		screen is c^2 * sin(a) / (8 * R * cos(a)^2) at angle a. So from the middle outwards, each strip
		is as wide as keeps that within precision at its outer edge, at least 2 pixels and at most 64.
		Worked out in floating point, once per generate for a few edges. Strips are made wider where
		needed to keep two vertices for each edge within Warpmap::maxVertices.
	*/

	const std::vector<int64_t>& CylinderWarp::_strips() const
	{
		StripCache& cache = m_stripCache;

		if (!cache.bValid || cache.axis != m_axis || cache.size != m_size || cache.span != m_span || cache.precision != m_precision)
		{
			_calcStrips(m_strips);
			cache = { true, m_axis, m_size, m_span, m_precision };
		}

		return m_strips;
	}

	//____ _calcStrips() ______________________________________________________

	void CylinderWarp::_calcStrips(std::vector<int64_t>& edges) const
	{
		const int64_t c_pixel = 64 << 8;
		const int lengthAcross = m_axis == Axis::X ? m_size.h : m_size.w;

		edges.clear();

		const int maxStrips = Warpmap::maxVertices / 2 - 4;
		const int minWidth = (lengthAcross + maxStrips - 1) / maxStrips;				// Up to two more strips from the middle out.

		const double half = lengthAcross / 2.0;
		const double rim = m_span / 2 * 3.14159265358979323846 / 180;
		const double radius = half / std::max(std::sin(rim), 1e-9);
		const double precision = std::max(double(m_precision), 0.01);

		auto width = [&](double y)
		{
			double s = std::min(y / radius, 1.0);
			double c2 = std::max(1.0 - s * s, 1e-12);
			return s > 0 ? std::sqrt(8 * precision * radius * c2 / s) : 64.0;
		};

		std::vector<double> outwards = { 0.0 };
		double y = 0.0;
		while (y < half)
		{
			const double lo = std::max(2.0, double(minWidth));
			double c = std::clamp(width(y), lo, std::max(64.0, lo));
			c = std::clamp(std::min(c, width(std::min(y + c, half))), lo, std::max(64.0, lo));
			y = std::min(y + c, half);
			if (half - y < 1.0)
				y = half;
			outwards.push_back(y);
		}

		for (int i = int(outwards.size()) - 1; i > 0; i--)
			edges.push_back(int64_t((half - outwards[i]) * c_pixel + 0.5));
		for (double o : outwards)
			edges.push_back(int64_t((half + o) * c_pixel + 0.5));

		edges.front() = 0;
		edges.back() = lengthAcross * c_pixel;
	}

	//____ _meshSize() ________________________________________________________

	void CylinderWarp::_meshSize(int& nVertices, int& nTriangles) const
	{
		int nStrips = int(_strips().size()) - 1;
		nVertices = (nStrips + 1) * 2;
		nTriangles = nStrips * 2;
	}

	//____ _rim() _____________________________________________________________
	// Sine and cosine (30 binals) of the angle to the rims, half the span.

	void CylinderWarp::_rim(int32_t& sin, int32_t& cos) const
	{
		_sinCos(_toTurn(m_span * (1.0 / 720)), sin, cos);
		sin = std::max(sin, 1);
	}

	//____ _periodAcross() ____________________________________________________
	/*
		The length of the source across the axis covers |sweep| degrees, so a full turn is that
		length * 360 / |sweep|.
	*/

	spx CylinderWarp::_periodAcross() const
	{
		spx length = m_axis == Axis::X ? m_sourceSize.h : m_sourceSize.w;
		double period = length * 360.0 / std::abs(double(m_sweep)) + 0.5;
		return spx(std::clamp(period, 1.0, double(Warpmap::maxValue)));
	}

	//____ _uPeriod() _________________________________________________________

	spx CylinderWarp::_uPeriod() const
	{
		return m_axis == Axis::Y ? _periodAcross() : 0;
	}

	//____ _vPeriod() _________________________________________________________

	spx CylinderWarp::_vPeriod() const
	{
		return m_axis == Axis::X ? _periodAcross() : 0;
	}

	//____ _generateMesh() ___________________________________________________
	/*
		Along the axis the source is stretched over the length of the cylinder, so each strip is a
		quad across the cylinder, with the edges of its strip at both ends. Across it, at distance y
		from the middle, the angle is asin(y / R), from the side facing the viewer, and the position
		is (angle - startAngle) / sweep * length of the source, counted from startAngle in the
		direction of sweep within a turn, as for CircularWarp. The angle is found from y and the depth
		sqrt(R^2 - y^2) with integers, positions in 1/256 spx.
	*/

	void CylinderWarp::_generateMesh(Warpmap::Vertex* pVertices, Warpmap::Triangle* pTriangles) const
	{
		const int c_fractionBits = 8;
		const int64_t c_pixel = 64 << c_fractionBits;

		const bool bAxisX = m_axis == Axis::X;

		const int lengthAlong = bAxisX ? m_size.w : m_size.h;
		const int lengthAcross = bAxisX ? m_size.h : m_size.w;
		const spx sourceAlong = bAxisX ? m_sourceSize.w : m_sourceSize.h;
		const spx sourceAcross = bAxisX ? m_sourceSize.h : m_sourceSize.w;

		const std::vector<int64_t>& edges = _strips();

		int32_t sin, cos;
		_rim(sin, cos);

		int64_t middle = lengthAcross * c_pixel / 2;
		int64_t radius = ((middle << 30) + sin / 2) / sin;			// Half the size across / sin(span / 2).

		int shift = 0;											// Keeps radius^2 within 64 bits.
		while ((radius >> shift) >= (int64_t(1) << 31))
			shift++;

		int64_t r = radius >> shift;

		uint32_t start = _toTurn(m_startAngle / 360.0);
		Factor factor = _factor(double(sourceAcross) * 360.0 / (std::abs(double(m_sweep)) * 0x1p32));
		bool bForward = m_sweep > 0.f;

		const int64_t period = _periodAcross();
		auto floorDivPeriod = [](int64_t a, int64_t b) { int64_t q = a / b; return (q * b > a) ? q - 1 : q; };
		int64_t previous = 0;

		int nEdges = int(edges.size());
		for (int i = 0; i < nEdges; i++)
		{
			spx pos = spx((edges[i] + (1 << (c_fractionBits - 1))) >> c_fractionBits);		// Rounded to spx.

			int64_t y = (int64_t(pos) << c_fractionBits) - middle;
			int64_t absY = std::min((y < 0 ? -y : y) >> shift, r);

			int64_t depth = int64_t(_sqrt(uint64_t(r * r - absY * absY)));
			uint32_t angle = _polar(depth, y < 0 ? -absY : absY);

			// Kept close to the edge before, rather than wrapped into the period, so that a strip
			// across the seam takes the short way.

			int64_t value = _multiply(bForward ? angle - start : start - angle, factor);
			if (i > 0)
			{
				int64_t step = value - previous;
				value = previous + step - floorDivPeriod(2 * step + period, 2 * period) * period;
			}
			previous = value;

			spx across = _toSourceValue(value);

			for (int end = 0; end < 2; end++)
			{
				spx along = end == 0 ? 0 : lengthAlong * 64;
				spx alongValue = end == 0 ? 0 : sourceAlong;

				pVertices[i * 2 + end] = bAxisX ? Warpmap::Vertex{ along, pos, alongValue, across } : Warpmap::Vertex{ pos, along, across, alongValue };
			}

			if (i > 0)
			{
				int a = (i - 1) * 2;
				*pTriangles++ = { a, a + 1, a + 3 };
				*pTriangles++ = { a, a + 3, a + 2 };
			}
		}
	}

} // namespace wax
