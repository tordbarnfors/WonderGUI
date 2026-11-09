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
		changes fast, so rings are as close as keeps both within 0.7 of the tolerance. Pieces along the
		rim are off by up to r * a^2 / 8 for an angle a, kept within the outline tolerance. Between
		the vertices of a ring, positions are off by less, as the source follows the same circle, and
		the two seldom add up at the same place, which keeps lenses within the tolerance, as measured.
		Worked out in floating point when the settings change, a few hundred operations, after which
		the vertices take only integers.
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
		const double tolerance = _pieceTolerance();

		// Rings at least two pixels apart. Closer ones are only needed where the falloff squeezes
		// the source into each pixel, which they don't improve on screen, as it is still sampled
		// once per pixel, but which they make slower to draw.

		const double c_minStep = 2.0;

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

		// Whether positions linear between rings at a and b keep within the tolerance, with pieces
		// of the given angle. Along a radius: s'' is linear over the falloff and 0 elsewhere, so it
		// is largest at an end or where the falloff starts, and s' is smallest at an end. Across:
		// every other ring is turned by half a piece, so the sides of the triangles between rings
		// span half a piece. Halfway along one, the source is at s(a) and s(b) weighted equally,
		// which turns it towards the end at b by a / 2 * (s(b) / (s(a) + s(b)) - 1/2), while the
		// point is turned by a / 2 * (b / (a + b) - 1/2), which is off by r times the difference.

		auto fits = [&](double a, double b, double across, double allowed)
		{
			double curve = std::max(std::abs(dds(a)), std::abs(dds(b)));
			if (a < r0 && b > r0)
				curve = std::max(curve, std::abs(dds(r0)));
			double slope = std::min(ds(a), ds(b));
			double along = curve * (b - a) * (b - a) / (8 * slope);

			double sa = s(a), sb = s(b);
			double turn = sa + sb > 0 ? b * across * std::abs(sb / (sa + sb) - b / (a + b)) : 0.0;

			return along + turn <= allowed;
		};

		// Rings close to the middle have fewer pieces: half as many or fewer, at most 3 halvings,
		// where pieces twice as long still keep the ring within the tolerance, up to radius limit(h)
		// for h halvings. Fewer than 16 only within an eighth of the radius, as larger rings need
		// pieces of at most 22.5 degrees, see below. Outwards, the number of pieces at most doubles
		// from one ring to the next.

		const int c_maxHalvings = 3;
		int pieces = 16;
		int maxHalvings = 0;
		double pieceAngle = 0.0;

		auto limit = [&](int halvings)
		{
			double angle = pieceAngle * (1 << halvings);
			return halvings == 0 ? 1e30 : 8 * tolerance / (angle * angle);
		};

		auto halvingsAt = [&](double r)
		{
			int h = 0;
			while (h < maxHalvings && r <= limit(h + 1) && ((pieces >> (h + 1)) >= 16 || r <= outer / 8))
				h++;
			return h;
		};

		// The sides of the triangles between two rings span half a piece of the outer one where they
		// have as many pieces, up to one and a half where the inner one has half as many.

		auto acrossAngle = [&](int innerHalvings, int outerHalvings)
		{
			double angle = pieceAngle * (1 << outerHalvings);
			return innerHalvings == outerHalvings ? angle / 2 : angle * 1.5;
		};

		auto step = [&](double a, int halvings, double limitR, double relax)
		{
			const double allowed = tolerance * 0.7 * relax;
			if (halvings > 0)
				limitR = std::min(limitR, std::max(limit(halvings - 1), a + c_minStep));

			double curve = std::abs(dds(std::max(a, r0)));
			double b = curve > 0 ? std::min(a + std::sqrt(8 * allowed * ds(a) / curve), limitR) : limitR;

			while (b - a > c_minStep && !fits(a, b, acrossAngle(halvings, std::min(halvings, halvingsAt(b))), allowed))
				b = a + (b - a) * 0.8;

			if (limitR - b < c_minStep)
				b = limitR;
			return b;
		};

		std::vector<double> radii;
		std::vector<int> halvings;

		auto makeRings = [&](double relax)
		{
			pieceAngle = 2 * c_pi / pieces;
			maxHalvings = 0;
			while (maxHalvings < c_maxHalvings && (pieces >> (maxHalvings + 1)) >= 8 && (pieces & ((2 << maxHalvings) - 1)) == 0)
				maxHalvings++;

			radii.clear();
			halvings.clear();

			// The middle polygon, out to where the falloff starts, or as far as linear positions keep
			// within the tolerance when it starts close to the middle. Positions are linear from the
			// middle out to it, so its pieces don't matter there.

			double core = std::min(r0, inner);
			if (core < inner)
				core = std::max(core, step(0.0, 0, inner, relax));
			radii.push_back(core);
			halvings.push_back(halvingsAt(core));

			double r = core;
			while (r < inner)
			{
				int h = halvings.back();
				r = step(r, h, inner, relax);
				radii.push_back(r);
				halvings.push_back(std::min(h, halvingsAt(r)));
			}

			if (bFalloff && R > inner)
			{
				radii.push_back(R);
				halvings.push_back(0);
			}
			radii.push_back(outer);
			halvings.push_back(0);

			// The rings of the outline have all pieces, so the ring inside them has at most one
			// halving.

			for (int k = int(halvings.size()) - 2; k >= 0; k--)
				halvings[k] = std::min(halvings[k], halvings[k + 1] + 1);
		};

		auto counts = [&](int& nVertices, int& nTriangles)
		{
			nVertices = 0;
			for (int h : halvings)
				nVertices += pieces >> h;
			nTriangles = (pieces >> halvings[0]) - 2;
			for (size_t k = 0; k + 1 < halvings.size(); k++)
				nTriangles += halvings[k] == halvings[k + 1] ? 2 * (pieces >> halvings[k]) : 3 * (pieces >> halvings[k]);
		};

		// At least 16 pieces, of at most 22.5 degrees, as the errors of larger ones grow faster than
		// worked out above, other than on rings close to the middle, which are small enough for
		// pieces up to twice that long. Where the middle polygon is small enough for rings to be
		// halved, a multiple of 2, 4 or 8, as many times as halving leaves at least 8.

		// Pieces along the rim keep the outline within the outline tolerance, and rings inside it
		// can have fewer by the tolerance.

		pieceAngle = std::sqrt(8 * _outlineTolerance() / std::max(outer, 1.0));
		pieces = int(std::clamp(std::ceil(2 * c_pi / std::max(pieceAngle, 1e-6)), 16.0, 4096.0));
		pieceAngle = 2 * c_pi / pieces;
		if (std::min(r0, inner) < std::min(limit(1), outer))
		{
			int multiple = 1;
			while (multiple < (1 << c_maxHalvings) && pieces / (multiple * 2) >= 8)
				multiple *= 2;
			pieces = (pieces + multiple - 1) / multiple * multiple;
		}

		// Coarser where needed to keep within the limits of Warpmap.

		double relax = 1.0;
		for (;;)
		{
			makeRings(relax);

			int nVertices, nTriangles;
			counts(nVertices, nTriangles);
			if (nVertices <= Warpmap::maxVertices && nTriangles <= Warpmap::maxTriangles)
				break;

			if (pieces > 16 && pieces >= int(radii.size()))
				pieces = std::max(16, (pieces * 7 / 8) & ~7);
			else
				relax *= 1.5;
		}

		const double toU = double(m_sourceSize.w) / m_size.w * 256;
		const double toV = double(m_sourceSize.h) / m_size.h * 256;

		layout.pieces = pieces;
		layout.rings.resize(radii.size());

		// Vertex j of a ring is at (offset + j * 2^(h + 1)) / (2 * pieces) of a turn from straight up,
		// h being its halvings, and the vertices of a ring between those of the ring outside it: half
		// a piece of that one further on.

		int offset = 0;
		for (int i = int(radii.size()) - 1; i >= 0; i--)
		{
			double r = radii[i];
			Ring& ring = layout.rings[i];
			ring.radius = std::llround(r * 64 * 256);
			ring.u = std::llround(s(r) * toU);
			ring.v = std::llround(s(r) * toV);
			ring.coverage = int(std::lround(std::clamp(outer - r, 0.0, 1.0) * Warpmap::coverageOne));
			ring.halvings = halvings[i];
			ring.offset = offset;
			offset += 1 << halvings[i];
		}
	}

	//____ _counts() __________________________________________________________
	/*
		Vertices: a ring of them for each ring. Triangles: the middle polygon split into a strip,
		two for each piece between two rings with as many pieces, and three for each piece of the
		inner ring between two where it has half as many.
	*/

	void LensWarp::_counts(const Layout& layout, int& nVertices, int& nTriangles)
	{
		const std::vector<Ring>& rings = layout.rings;
		const int pieces = layout.pieces;

		nVertices = 0;
		for (const Ring& ring : rings)
			nVertices += pieces >> ring.halvings;

		nTriangles = (pieces >> rings[0].halvings) - 2;
		for (size_t k = 0; k + 1 < rings.size(); k++)
			nTriangles += (rings[k].halvings == rings[k + 1].halvings ? 2 : 3) * (pieces >> rings[k].halvings);
	}

	//____ _meshSize() ________________________________________________________

	void LensWarp::_meshSize(int& nVertices, int& nTriangles) const
	{
		_counts(_layout(), nVertices, nTriangles);
	}

	//____ _generateMesh() ____________________________________________________
	/*
		Vertices of a ring are evenly spaced, clockwise from where its offset says, each ring turned
		by half a piece of the ring outside it, so the vertices of a ring are between those of the
		next. The middle polygon, the first ring, goes down both sides from its vertex closest to
		straight up and is split into a strip of triangles from top to bottom, which gives a backend
		few triangles on each row. Positions in 1/256 spx, sines and cosines with 30 binals.
	*/

	void LensWarp::_generateMesh(Warpmap::Vertex* pVertices, Warpmap::Triangle* pTriangles) const
	{
		const Layout& l = _layout();
		const int pieces = l.pieces;
		const int rings = int(l.rings.size());
		const uint64_t turn = uint64_t(2 * pieces);

		const int64_t cx = int64_t(m_size.w) << 13, cy = int64_t(m_size.h) << 13;
		const int64_t cu = int64_t(m_sourceSize.w) << 7, cv = int64_t(m_sourceSize.h) << 7;

		auto round = [](int64_t value) { return spx((value + 128) >> 8); };

		// First vertex of each ring, as rings have different numbers of vertices.

		const int firstBytes = int(sizeof(int)) * (rings + 1);
		int* first = (int*) Base::memStackAlloc(firstBytes);
		first[0] = 0;
		for (int k = 0; k < rings; k++)
			first[k + 1] = first[k] + (pieces >> l.rings[k].halvings);

		for (int k = 0; k < rings; k++)
		{
			const Ring& ring = l.rings[k];
			const int n = pieces >> ring.halvings;
			const uint64_t spacing = uint64_t(2) << ring.halvings;

			for (int j = 0; j < n; j++)
			{
				uint64_t at = (uint64_t(ring.offset) + spacing * j) % turn;
				uint32_t angle = uint32_t(0xC0000000u + uint32_t((at << 32) / turn));
				int32_t sin, cos;
				_sinCos(angle, sin, cos);

				pVertices[first[k] + j] = { round(cx + ((ring.radius * cos) >> 30)), round(cy + ((ring.radius * sin) >> 30)),
											round(cu + ((ring.u * cos) >> 30)), round(cv + ((ring.v * sin) >> 30)), Warpmap::weightOne, ring.coverage };
			}
		}

		// The middle polygon, from its vertex closest to straight up, down both sides.

		{
			const Ring& ring = l.rings[0];
			const int n = pieces >> ring.halvings;
			const int spacing = 2 << ring.halvings;
			const int top = int(((turn - uint64_t(ring.offset) % turn) % turn + spacing / 2) / spacing) % n;

			auto vertex = [&](int j) { return first[0] + ((j % n) + n) % n; };

			int right = top + 1, left = top - 1;
			*pTriangles++ = { vertex(top), vertex(right), vertex(left) };
			bool bRight = true;
			while (right - left < n - 1)
			{
				if (bRight)
				{
					*pTriangles++ = { vertex(right), vertex(right + 1), vertex(left) };
					right++;
				}
				else
				{
					*pTriangles++ = { vertex(right), vertex(left - 1), vertex(left) };
					left--;
				}
				bRight = !bRight;
			}
		}

		// Between two rings. Vertex j of the inner ring is between vertices j and j + 1 of the outer
		// one where they have as many, or between 2j and 2j + 1 where the inner one has half as many.

		for (int k = 0; k + 1 < rings; k++)
		{
			const int nIn = pieces >> l.rings[k].halvings;
			const int nOut = pieces >> l.rings[k + 1].halvings;
			auto in = [&](int j) { return first[k] + (j % nIn); };
			auto out = [&](int j) { return first[k + 1] + (j % nOut); };

			if (nIn == nOut)
			{
				for (int j = 0; j < nIn; j++)
				{
					*pTriangles++ = { out(j), out(j + 1), in(j) };
					*pTriangles++ = { in(j), out(j + 1), in(j + 1) };
				}
			}
			else
			{
				for (int j = 0; j < nIn; j++)
				{
					*pTriangles++ = { out(2 * j), out(2 * j + 1), in(j) };
					*pTriangles++ = { in(j), out(2 * j + 1), out(2 * j + 2) };
					*pTriangles++ = { in(j), out(2 * j + 2), in(j + 1) };
				}
			}
		}

		Base::memStackFree(firstBytes);
	}

} // namespace wax
