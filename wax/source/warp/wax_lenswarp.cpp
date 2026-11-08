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

#include <wax_lenswarp.h>
#include <wax_base.h>

#include <algorithm>
#include <cmath>

namespace wax
{
	const TypeInfo LensWarp::TYPEINFO = { "LensWarp", &WarpGenerator::TYPEINFO };

	//____ create() ___________________________________________________________

	LensWarp_p LensWarp::create(const Blueprint& bp, WarpmapFactory* pFactory)
	{
		if (!_validateSettings(bp.size, bp.sourceSize, bp.precision) || !_isValidMagnification(bp.magnification) || !_isValidFalloff(bp.falloff))
			return nullptr;

		if (!pFactory)
			pFactory = Base::defaultWarpmapFactory();

		if (!pFactory)
			return nullptr;

		return LensWarp_p(new LensWarp(bp, pFactory));
	}

	//____ constructor ________________________________________________________

	LensWarp::LensWarp(const Blueprint& bp, WarpmapFactory* pFactory)
		: WarpGenerator(bp.size, bp.sourceSize, bp.precision, pFactory)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		m_magnification = bp.magnification;
		m_falloff = bp.falloff;
	}

	//____ destructor _________________________________________________________

	LensWarp::~LensWarp()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& LensWarp::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setMagnification() _________________________________________________

	bool LensWarp::setMagnification(float magnification)
	{
		if (!_isValidMagnification(magnification))
			return false;

		if (magnification != m_magnification)
		{
			m_magnification = magnification;
			m_bDirty = true;
		}
		return true;
	}

	//____ setFalloff() _______________________________________________________

	bool LensWarp::setFalloff(float falloff)
	{
		if (!_isValidFalloff(falloff))
			return false;

		if (falloff != m_falloff)
		{
			m_falloff = falloff;
			m_bDirty = true;
		}
		return true;
	}

	//____ radius() ___________________________________________________________
	// Half the smaller side of the Warpmap, less half a pixel for the antialiased outline.

	spx LensWarp::radius() const
	{
		return std::max(std::min(m_size.w, m_size.h) * 32 - 32, 0);
	}

	//____ _layout() __________________________________________________________

	const LensWarp::Layout& LensWarp::_layout() const
	{
		Layout& l = m_layout;

		if (!l.bValid || l.size != m_size || l.sourceSize != m_sourceSize || l.precision != m_precision ||
			l.magnification != m_magnification || l.falloff != m_falloff)
		{
			l.size = m_size;
			l.sourceSize = m_sourceSize;
			l.precision = m_precision;
			l.magnification = m_magnification;
			l.falloff = m_falloff;
			_calcLayout(l);
			l.bValid = true;
		}
		return l;
	}

	//____ _calcLayout() ______________________________________________________
	/*
		A point r pixels from the middle of the lens shows the source s(r) pixels from its middle,
		in the same direction. Within r0 = R * (1 - falloff), R being the radius, s(r) = r / m. From
		there to the rim it is the cubic Hermite curve from r0 / m with slope 1 / m to R with slope 1,
		which meets what is around the lens without a seam. Its rise is at least its length and its
		slopes at most 1, so it never turns back (Fritsch-Carlson). Outside the rim it goes on
		straight, for the triangles out to half a pixel beyond it, where they fade out.

		Positions are linear between rings, which is off by up to |s''| * d^2 / 8 pixels of the source
		over d pixels, seen magnified by 1 / s', and turned along the diagonals of the pieces where s
		changes fast, so rings are as close as keeps both within 0.7 of precision. Pieces along the
		rim are off by up to r * a^2 / 8 for an angle a, kept within half of it. The two seldom add up
		at the same place, which keeps lenses within precision, as measured, with about as much to
		spare as the other generators. Worked out in floating point when the settings change, a few
		hundred operations, after which the vertices take only integers.
	*/

	void LensWarp::_calcLayout(Layout& layout) const
	{
		const double c_pi = 3.14159265358979323846;

		const double R = radius() / 64.0;
		const double inner = std::max(R - 0.5, 0.0);					// Full coverage out to here,
		const double outer = R + 0.5;									// none from here.

		const double m = m_magnification;
		const double w = R * m_falloff;
		const double r0 = R - w;
		const bool bFalloff = w > 1e-9;
		const double p0 = r0 / m, m0 = 1 / m;
		const double precision = _piecePrecision();
		const double c_minStep = 1.0 / 32;								// Closest rings. Positions are in spx, which shows where the source is squeezed hard.

		// s(r), s'(r) and s''(r).

		auto s = [&](double r)
		{
			if (r <= r0 || !bFalloff)
				return r / m;
			if (r >= R)
				return R + (r - R);
			double t = (r - r0) / w;
			double t2 = t * t, t3 = t2 * t;
			return (2 * t3 - 3 * t2 + 1) * p0 + (t3 - 2 * t2 + t) * w * m0 + (-2 * t3 + 3 * t2) * R + (t3 - t2) * w;
		};

		auto ds = [&](double r)
		{
			if (r < r0 || !bFalloff)
				return m0;
			if (r >= R)
				return 1.0;
			double t = (r - r0) / w;
			double t2 = t * t;
			return ((6 * t2 - 6 * t) * p0 + (3 * t2 - 4 * t + 1) * w * m0 + (-6 * t2 + 6 * t) * R + (3 * t2 - 2 * t) * w) / w;
		};

		auto dds = [&](double r)
		{
			if (r < r0 || r > R || !bFalloff)
				return 0.0;
			double t = (r - r0) / w;
			return ((12 * t - 6) * p0 + (6 * t - 4) * w * m0 + (-12 * t + 6) * R + (6 * t - 2) * w) / (w * w);
		};

		// Whether positions linear between rings at a and b keep within the precision, with pieces
		// of the given angle. Along a radius: s'' is linear over the falloff and 0 elsewhere, so it
		// is largest at an end or where the falloff starts, and s' is smallest at an end. Across
		// the diagonals of the pieces: halfway along one, the source is at s(a) and s(b) weighted
		// equally, which turns it towards the end at b by a * (s(b) / (s(a) + s(b)) - 1/2), while
		// the point is turned by a * (b / (a + b) - 1/2), which is off by r times the difference.

		auto fits = [&](double a, double b, double angle, double allowed)
		{
			double curve = std::max(std::abs(dds(a)), std::abs(dds(b)));
			if (a < r0 && b > r0)
				curve = std::max(curve, std::abs(dds(r0)));
			double slope = std::min(ds(a), ds(b));
			double along = curve * (b - a) * (b - a) / (8 * slope);

			double sa = s(a), sb = s(b);
			double across = sa + sb > 0 ? b * angle * std::abs(sb / (sa + sb) - b / (a + b)) : 0.0;

			return along + across <= allowed;
		};

		auto step = [&](double a, double limit, double angle, double relax)
		{
			const double allowed = precision * 0.7 * relax;
			double curve = std::abs(dds(std::max(a, r0)));
			double b = curve > 0 ? std::min(a + std::sqrt(8 * allowed * ds(a) / curve), limit) : limit;

			while (b - a > c_minStep && !fits(a, b, angle, allowed))
				b = a + (b - a) * 0.8;

			if (limit - b < c_minStep)
				b = limit;
			return b;
		};

		std::vector<double> radii;

		auto makeRings = [&](int pieces, double relax)
		{
			const double angle = 2 * c_pi / pieces;

			radii.clear();

			// The middle polygon, out to where the falloff starts, or as far as linear positions keep
			// within precision when it starts close to the middle.

			double core = std::min(r0, inner);
			if (core < inner)
				core = std::max(core, step(0.0, inner, angle, relax));
			radii.push_back(core);

			double r = core;
			while (r < inner)
			{
				r = step(r, inner, angle, relax);
				radii.push_back(r);
			}

			if (bFalloff && R > inner)
				radii.push_back(R);
			radii.push_back(outer);
		};

		double pieceAngle = std::sqrt(4 * precision / std::max(outer, 1.0));
		int pieces = int(std::clamp(std::ceil(2 * c_pi / std::max(pieceAngle, 1e-6)), 8.0, 4096.0));

		// Coarser where needed to keep within the limits of Warpmap.

		double relax = 1.0;
		for (;;)
		{
			makeRings(pieces, relax);

			int nVertices, nTriangles;
			_counts(pieces, int(radii.size()), nVertices, nTriangles);
			if (nVertices <= Warpmap::maxVertices && nTriangles <= Warpmap::maxTriangles)
				break;

			if (pieces > 8 && pieces >= int(radii.size()))
				pieces = std::max(8, pieces * 7 / 8);
			else
				relax *= 1.5;
		}

		const double toU = double(m_sourceSize.w) / m_size.w * 256;
		const double toV = double(m_sourceSize.h) / m_size.h * 256;

		layout.pieces = pieces;
		layout.rings.resize(radii.size());

		for (size_t i = 0; i < radii.size(); i++)
		{
			double r = radii[i];
			Ring& ring = layout.rings[i];
			ring.radius = std::llround(r * 64 * 256);
			ring.u = std::llround(s(r) * toU);
			ring.v = std::llround(s(r) * toV);
			ring.coverage = int(std::lround(std::clamp(outer - r, 0.0, 1.0) * Warpmap::coverageOne));
		}
	}

	//____ _counts() __________________________________________________________
	/*
		Vertices: a ring of them for each ring. Triangles: the middle polygon split into a strip,
		and two for each piece between two rings.
	*/

	void LensWarp::_counts(int pieces, int rings, int& nVertices, int& nTriangles)
	{
		nVertices = pieces * rings;
		nTriangles = (pieces - 2) + 2 * pieces * (rings - 1);
	}

	//____ _meshSize() ________________________________________________________

	void LensWarp::_meshSize(int& nVertices, int& nTriangles) const
	{
		const Layout& l = _layout();
		_counts(l.pieces, int(l.rings.size()), nVertices, nTriangles);
	}

	//____ _generateMesh() ____________________________________________________
	/*
		Vertex j of each ring is at an angle of j / pieces of a turn clockwise from straight up, so
		the middle polygon goes down both sides from its top vertex, and is split into a strip of
		triangles from top to bottom, which gives a backend few triangles on each row. Positions in
		1/256 spx, sines and cosines with 30 binals.
	*/

	void LensWarp::_generateMesh(Warpmap::Vertex* pVertices, Warpmap::Triangle* pTriangles) const
	{
		const Layout& l = _layout();
		const int pieces = l.pieces;
		const int rings = int(l.rings.size());

		const int64_t cx = int64_t(m_size.w) << 13, cy = int64_t(m_size.h) << 13;
		const int64_t cu = int64_t(m_sourceSize.w) << 7, cv = int64_t(m_sourceSize.h) << 7;

		auto round = [](int64_t value) { return spx((value + 128) >> 8); };

		for (int j = 0; j < pieces; j++)
		{
			uint32_t angle = uint32_t(0xC0000000u + uint32_t((uint64_t(j) << 32) / uint64_t(pieces)));
			int32_t sin, cos;
			_sinCos(angle, sin, cos);

			for (int k = 0; k < rings; k++)
			{
				const Ring& ring = l.rings[k];
				pVertices[k * pieces + j] = { round(cx + ((ring.radius * cos) >> 30)), round(cy + ((ring.radius * sin) >> 30)),
											  round(cu + ((ring.u * cos) >> 30)), round(cv + ((ring.v * sin) >> 30)), Warpmap::weightOne, ring.coverage };
			}
		}

		// The middle polygon, from its top vertex down both sides.

		int right = 1, left = pieces - 1;
		*pTriangles++ = { 0, right, left };
		bool bRight = true;
		while (left - right > 1)
		{
			if (bRight)
			{
				*pTriangles++ = { right, right + 1, left };
				right++;
			}
			else
			{
				*pTriangles++ = { right, left - 1, left };
				left--;
			}
			bRight = !bRight;
		}

		// Two triangles for each piece between two rings.

		for (int k = 0; k + 1 < rings; k++)
		{
			int a = k * pieces, b = a + pieces;
			for (int j = 0; j < pieces; j++)
			{
				int j1 = j + 1 == pieces ? 0 : j + 1;
				*pTriangles++ = { a + j, a + j1, b + j1 };
				*pTriangles++ = { a + j, b + j1, b + j };
			}
		}
	}

} // namespace wax
