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

		// Pieces along the rim keep the outline within the outline tolerance, also on the ring at the
		// inner edge of the outline. Rings inside it have as few as keeps them within the tolerance,
		// r * a^2 / 8 for pieces of angle a at radius r, which goes up with the radius, so the
		// number of pieces never goes down outwards. At least 16, of at most 22.5 degrees, as the
		// errors of larger ones grow faster than worked out above, other than on rings within an
		// eighth of the radius, which are small enough for pieces up to twice that long. A ring
		// keeps the pieces of the ring inside it where those are enough, and otherwise has at least
		// a quarter more, so that most rings have as many as the next and their triangles span half
		// a piece.

		const double rimAngle = std::sqrt(8 * _outlineTolerance() / std::max(outer, 1.0));
		int rimPieces = int(std::clamp(std::ceil(2 * c_pi / std::max(rimAngle, 1e-6)), 16.0, 4096.0));

		auto piecesAt = [&](int inside, double r)
		{
			if (r >= inner)
				return rimPieces;

			double angle = std::sqrt(8 * tolerance / std::max(r, 1e-6));
			int n = int(std::clamp(std::ceil(2 * c_pi / angle), r <= outer / 8 ? 8.0 : 16.0, double(rimPieces)));
			return n <= inside ? inside : std::min(std::max(n, inside * 5 / 4), rimPieces);
		};

		// The sides of the triangles between two rings span half a piece where the rings have as
		// many pieces, every other ring being turned by half a piece, and otherwise up to half a
		// piece of each, see _generateMesh().

		auto acrossAngle = [&](int inside, int outside)
		{
			return inside == outside ? c_pi / inside : c_pi / inside + c_pi / outside;
		};

		auto step = [&](double a, int pieces, double limitR, double allowed)
		{
			double curve = std::abs(dds(std::max(a, r0)));
			double b = curve > 0 ? std::min(a + std::sqrt(8 * allowed * ds(a) / curve), limitR) : limitR;

			while (b - a > c_minStep && !fits(a, b, acrossAngle(pieces, piecesAt(pieces, b)), allowed))
				b = a + (b - a) * 0.8;

			if (limitR - b < c_minStep)
				b = limitR;
			return b;
		};

		std::vector<double> radii;
		std::vector<int> pieces;

		auto makeRings = [&](double relax)
		{
			const double allowed = tolerance * 0.7 * relax;

			radii.clear();
			pieces.clear();

			// The middle polygon, out to where the falloff starts, or as far as linear positions keep
			// within the tolerance when it starts close to the middle. Positions are linear from the
			// middle out to it, so its pieces don't matter there.

			double core = std::min(r0, inner);
			if (core < inner)
				core = std::max(core, step(0.0, rimPieces, inner, allowed));
			radii.push_back(core);
			pieces.push_back(piecesAt(0, core));

			double r = core;
			while (r < inner)
			{
				r = step(r, pieces.back(), inner, allowed);
				radii.push_back(r);
				pieces.push_back(piecesAt(pieces.back(), r));
			}

			if (bFalloff && R > inner)
			{
				radii.push_back(R);
				pieces.push_back(rimPieces);
			}
			radii.push_back(outer);
			pieces.push_back(rimPieces);

			// Where rings are as close as allowed, the source being squeezed, the pieces can't change
			// from one to the next within the tolerance. The ring inside then has as many as the one
			// outside, which moves the change inwards, to where it fits.

			for (int k = int(radii.size()) - 2; k >= 0; k--)
				if (pieces[k] != pieces[k + 1] && !fits(radii[k], radii[k + 1], acrossAngle(pieces[k], pieces[k + 1]), allowed))
					pieces[k] = pieces[k + 1];
		};

		// Coarser where needed to keep within the limits of Warpmap.

		double relax = 1.0;
		for (;;)
		{
			makeRings(relax);

			int nVertices, nTriangles;
			_counts(pieces, nVertices, nTriangles);
			if (nVertices <= Warpmap::maxVertices && nTriangles <= Warpmap::maxTriangles)
				break;

			if (rimPieces > 16 && rimPieces >= int(radii.size()))
				rimPieces = std::max(16, rimPieces * 7 / 8);
			else
				relax *= 1.5;
		}

		const double toU = double(m_sourceSize.w) / m_size.w * 256;
		const double toV = double(m_sourceSize.h) / m_size.h * 256;

		layout.rings.resize(radii.size());
		layout.pieces = pieces;

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
		Vertices: a ring of them for each ring. Triangles: the middle polygon split into a strip, and
		one for each piece of the two rings around each gap between them.
	*/

	void LensWarp::_counts(const std::vector<int>& pieces, int& nVertices, int& nTriangles)
	{
		nVertices = 0;
		nTriangles = pieces[0] - 2;
		for (size_t k = 0; k < pieces.size(); k++)
		{
			nVertices += pieces[k];
			if (k + 1 < pieces.size())
				nTriangles += pieces[k] + pieces[k + 1];
		}
	}

	//____ _meshSize() ________________________________________________________

	void LensWarp::_meshSize(int& nVertices, int& nTriangles) const
	{
		_counts(_layout().pieces, nVertices, nTriangles);
	}

	//____ _generateMesh() ____________________________________________________
	/*
		Vertices of a ring are evenly spaced, clockwise from straight up, every other ring turned by
		half a piece, so where two rings have as many pieces, the vertices of one are between those
		of the other. The middle polygon, the first ring, goes down both sides from its first vertex
		and is split into a strip of triangles from top to bottom, which gives a backend few
		triangles on each row. Positions in 1/256 spx, sines and cosines with 30 binals.
	*/

	void LensWarp::_generateMesh(Warpmap::Vertex* pVertices, Warpmap::Triangle* pTriangles) const
	{
		const Layout& l = _layout();
		const int rings = int(l.rings.size());

		const int64_t cx = int64_t(m_size.w) << 13, cy = int64_t(m_size.h) << 13;
		const int64_t cu = int64_t(m_sourceSize.w) << 7, cv = int64_t(m_sourceSize.h) << 7;

		auto round = [](int64_t value) { return spx((value + 128) >> 8); };

		// Vertex j of ring k is at (2 * j + turned) / (2 * pieces) of a turn, turned being 1 for
		// every other ring.

		int first = 0;
		for (int k = 0; k < rings; k++)
		{
			const Ring& ring = l.rings[k];
			const int n = l.pieces[k];
			const uint64_t turn = uint64_t(2 * n);

			for (int j = 0; j < n; j++)
			{
				uint64_t at = uint64_t(2 * j + (k & 1));
				uint32_t angle = uint32_t(0xC0000000u + uint32_t((at << 32) / turn));
				int32_t sin, cos;
				_sinCos(angle, sin, cos);

				pVertices[first + j] = { round(cx + ((ring.radius * cos) >> 30)), round(cy + ((ring.radius * sin) >> 30)),
										 round(cu + ((ring.u * cos) >> 30)), round(cv + ((ring.v * sin) >> 30)), Warpmap::weightOne, ring.coverage };
			}
			first += n;
		}

		// The middle polygon, from its first vertex, the one closest to straight up, down both
		// sides.

		{
			const int n = l.pieces[0];
			auto vertex = [&](int j) { return (j + n) % n; };

			int right = 1, left = -1;
			*pTriangles++ = { vertex(0), vertex(right), vertex(left) };
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

		// Between two rings, walking around both and taking the next piece of the ring whose next
		// piece has its middle first, so that each side between the rings spans at most half a
		// piece of each. Piece j of ring k has its middle at (2 * j + 1 + turned) / (2 * pieces) of
		// a turn, compared without dividing.

		first = 0;
		for (int k = 0; k + 1 < rings; k++)
		{
			const int nIn = l.pieces[k], nOut = l.pieces[k + 1];
			const int turnedIn = k & 1, turnedOut = 1 - turnedIn;
			const int firstOut = first + nIn;

			auto in = [&](int j) { return first + (j % nIn); };
			auto out = [&](int j) { return firstOut + (j % nOut); };

			int i = 0, o = 0;
			while (i < nIn || o < nOut)
			{
				if (o == nOut || (i < nIn && int64_t(2 * i + 1 + turnedIn) * nOut < int64_t(2 * o + 1 + turnedOut) * nIn))
				{
					*pTriangles++ = { in(i), out(o), in(i + 1) };
					i++;
				}
				else
				{
					*pTriangles++ = { out(o), out(o + 1), in(i) };
					o++;
				}
			}
			first = firstOut;
		}
	}

} // namespace wax
