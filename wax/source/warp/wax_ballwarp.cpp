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

#include <wax_ballwarp.h>
#include <wax_base.h>

#include <algorithm>
#include <cmath>

namespace wax
{
	const TypeInfo BallWarp::TYPEINFO = { "BallWarp", &WarpGenerator::TYPEINFO };

	//____ create() ___________________________________________________________

	BallWarp_p BallWarp::create(const Blueprint& bp, WarpmapFactory* pFactory)
	{
		if (!_validateSettings(bp.size, bp.sourceSize, bp.precision) || !_isValidTilt(bp.tilt) || !(bp.roll == bp.roll))
			return nullptr;

		if (!pFactory)
			pFactory = Base::defaultWarpmapFactory();

		if (!pFactory)
			return nullptr;

		return BallWarp_p(new BallWarp(bp, pFactory));
	}

	//____ constructor ________________________________________________________

	BallWarp::BallWarp(const Blueprint& bp, WarpmapFactory* pFactory)
		: WarpGenerator(bp.size, bp.sourceSize, bp.precision, pFactory)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		m_tilt = bp.tilt;
		m_roll = bp.roll;

		m_offsetFactor = _offsetFactor(1.0, bp.sourceSize.w);
	}

	//____ destructor _________________________________________________________

	BallWarp::~BallWarp()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& BallWarp::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setOrientation() ___________________________________________________

	bool BallWarp::setOrientation(float tilt, float roll)
	{
		if (!_isValidTilt(tilt) || !(roll == roll))
			return false;

		if (tilt != m_tilt || roll != m_roll)
		{
			m_tilt = tilt;
			m_roll = roll;
			m_bDirty = true;
		}
		return true;
	}

	//____ radius() ___________________________________________________________
	// Half the smaller side of the Warpmap, less half a pixel for the antialiased outline.

	spx BallWarp::radius() const
	{
		return std::max(std::min(m_size.w, m_size.h) * 32 - 32, 0);
	}

	//____ rotationOffset() ___________________________________________________

	CoordSPX BallWarp::rotationOffset(float degrees) const
	{
		return { _offsetFor(_toTurn(degrees * (1.0 / 360)), m_offsetFactor), 0 };
	}

	//____ _layout() __________________________________________________________
	/*
		Positions are linear over pieces of the grid seen on screen. A piece of a ball of radius r
		spanning an angle a is off by up to about r * a^2 / 4, twice as much as for a circle, as the
		pieces are sheared where the ball turns away, so pieces span sqrt(4 * precision / r) along
		both. Worked out in floating point when generating, a few operations. Made coarser where needed to keep within the limits of Warpmap.
	*/

	void BallWarp::_layout(int& nLon, int& nLat) const
	{
		const double c_pi = 3.14159265358979323846;

		double r = std::max(radius() / 64.0, 1.0);
		double precision = _piecePrecision();
		double angle = std::sqrt(3.4 * precision / r);


		// At least 16 meridians and 8 parallels, pieces of at most 22.5 degrees, as the error of larger
		// ones grows faster than r * a^2 / 4.

		nLon = int(std::clamp(std::ceil(2 * c_pi / std::max(angle, 1e-6)), 16.0, 4096.0));
		nLat = int(std::clamp(std::ceil(c_pi / std::max(angle, 1e-6)), 8.0, 2048.0));

		int nVertices, nTriangles;
		_counts(nLon, nLat, nVertices, nTriangles);

		while ((nVertices > Warpmap::maxVertices || nTriangles > Warpmap::maxTriangles) && (nLon > 16 || nLat > 8))
		{
			if (nLon > 2 * nLat || nLat <= 8)
				nLon = std::max(16, nLon * 7 / 8);
			else
				nLat = std::max(8, nLat * 7 / 8);

			_counts(nLon, nLat, nVertices, nTriangles);
		}
	}

	//____ _counts() __________________________________________________________
	/*
		Vertices: the grid, the first of each pole row being the pole for the piece next to it, and
		two for each crossing of a meridian or parallel with the outline, inside and outside it.
		Triangles: two for each piece of the grid, one more for each crossing on its sides when cut,
		and two for each part of the outline, between two crossings. The outline crosses each meridian
		and each parallel at most twice.
	*/

	void BallWarp::_counts(int nLon, int nLat, int& nVertices, int& nTriangles)
	{
		int crossings = _maxCrossings(nLon, nLat);
		nVertices = (nLon + 1) * (nLat + 1) + 2 * crossings;
		nTriangles = 2 * nLon * nLat + 4 * crossings;
	}

	//____ _meshSize() ________________________________________________________

	void BallWarp::_meshSize(int& nVertices, int& nTriangles) const
	{
		int nLon, nLat;
		_layout(nLon, nLat);
		_counts(nLon, nLat, nVertices, nTriangles);
	}

	//____ _generateMesh() ____________________________________________________
	/*
		A point at longitude L (0 facing the viewer, positive to the east) and latitude P is, on a ball
		of radius 1 with y going down, (cos P sin L, -sin P, cos P cos L). Tilting it by T around the
		x axis, the north pole towards the viewer, and rolling it by R clockwise around the line of
		view gives its position on screen and z, its depth towards the viewer:

			y' = y cos T + z sin T				z' = z cos T - y sin T = cos P cos L cos T + sin P sin T
			x'' = x cos R - y' sin R			y'' = x sin R + y' cos R

		What is seen is z' >= z0, where the ball is within half a pixel of its outline, which then
		comes from triangles going out to half a pixel outside it, fading out. Along a meridian
		z' = A cos P + B sin P, which is z0 at P = atan2(B, A) +- acos(z0 / |A, B|), along a parallel
		z' = C cos L + D, which is z0 at L = +-acos((z0 - D) / C). Pieces of the grid are cut there,
		the part seen made into a fan of triangles. Positions are in 1/256 spx and values with 30 binals.
	*/

	void BallWarp::_generateMesh(Warpmap::Vertex* pVertices, Warpmap::Triangle* pTriangles) const
	{
		const int64_t c_one = int64_t(1) << 30;
		const int64_t c_half = int64_t(1) << 31;					// Half a turn.
		const int64_t c_quarter = int64_t(1) << 30;

		int nLon, nLat;
		_layout(nLon, nLat);

		int nVertices, nTriangles;
		_counts(nLon, nLat, nVertices, nTriangles);

		const int columns = nLon + 1;
		const int gridVertices = columns * (nLat + 1);
		const int maxCrossings = _maxCrossings(nLon, nLat);

		// The ball, half a pixel inside the outline and half a pixel outside it.

		const int64_t radius = this->radius();
		const int64_t inner = std::max(radius - 32, int64_t(0));
		const int64_t r = radius << 8;
		const int64_t cx = int64_t(m_size.w) << 13;
		const int64_t cy = int64_t(m_size.h) << 13;

		const int64_t ratio = radius > 0 ? (inner << 30) / radius : 0;
		const int64_t z0 = int64_t(_sqrt(uint64_t(c_one * c_one - ratio * ratio)));
		const int64_t outward = inner > 0 ? ((radius + 32) << 30) / inner : 0;		// From inside the outline to outside it.

		int32_t sinT, cosT, sinR, cosR;
		_sinCos(_toTurn(m_tilt * (1.0 / 360)), sinT, cosT);
		_sinCos(_toTurn(m_roll * (1.0 / 360)), sinR, cosR);

		const spx width = m_sourceSize.w, height = m_sourceSize.h;

		// Meridians and parallels, the north pole first.

		int scratch = int(sizeof(int64_t)) * (2 * columns + 2 * (nLat + 1)) + int(sizeof(int32_t)) * (4 * columns + 4 * (nLat + 1))
					+ int(sizeof(int32_t)) * (columns * nLat + (nLat + 1) * nLon) + gridVertices;
		auto pScratch = (uint8_t*) Base::memStackAlloc(scratch);

		auto pLon = (int64_t*) pScratch;							// Angles in 1/2^32 turns.
		auto pLat = pLon + columns;
		auto pColU = pLat + (nLat + 1);
		auto pRowV = pColU + columns;
		auto pColSin = (int32_t*) (pRowV + (nLat + 1));
		auto pColCos = pColSin + columns;
		auto pRowSin = pColCos + columns;
		auto pRowCos = pRowSin + (nLat + 1);
		auto pMeridianCrossing = pRowCos + (nLat + 1);				// Vertex for each side along a meridian, -1 for none.
		auto pParallelCrossing = pMeridianCrossing + columns * nLat;	// And along a parallel.
		auto pSeen = (uint8_t*) (pParallelCrossing + (nLat + 1) * nLon);

		for (int i = 0; i < columns; i++)
		{
			pLon[i] = -c_half + (int64_t(1) << 32) * i / nLon;
			pColU[i] = int64_t(width) * i / nLon;
			_sinCos(uint32_t(pLon[i]), pColSin[i], pColCos[i]);
		}

		for (int j = 0; j <= nLat; j++)
		{
			pLat[j] = c_quarter - c_half * j / nLat;
			pRowV[j] = int64_t(height) * j / nLat;
			_sinCos(uint32_t(pLat[j]), pRowSin[j], pRowCos[j]);
		}

		// Position on screen (30 binals of the radius) and z of a point of the ball.

		auto project = [&](int64_t sinP, int64_t cosP, int64_t sinL, int64_t cosL, int64_t& xs, int64_t& ys, int64_t& z)
		{
			int64_t x = (cosP * sinL) >> 30;
			int64_t y = -sinP;
			int64_t zz = (cosP * cosL) >> 30;

			int64_t y1 = (y * cosT + zz * sinT) >> 30;
			z = (zz * cosT - y * sinT) >> 30;

			xs = (x * cosR - y1 * sinR) >> 30;
			ys = (x * sinR + y1 * cosR) >> 30;
		};

		auto toSpx = [](int64_t value) { return spx(std::clamp((value + 128) >> 8, -int64_t(Warpmap::maxPosition), int64_t(Warpmap::maxPosition))); };

		auto writeVertex = [&](int index, int64_t xs, int64_t ys, int64_t u, int64_t v, int coverage)
		{
			pVertices[index] = { toSpx(cx + ((xs * r) >> 30)), toSpx(cy + ((ys * r) >> 30)), _toSourceValue(u), _toSourceValue(v), Warpmap::weightOne, coverage };
		};

		// The grid. The first vertex of each pole row is the pole of the piece next to it, with u of
		// the middle of the piece.

		for (int j = 0; j <= nLat; j++)
		{
			bool bPole = j == 0 || j == nLat;

			for (int i = 0; i < columns; i++)
			{
				int64_t xs, ys, z;
				project(pRowSin[j], pRowCos[j], pColSin[i], pColCos[i], xs, ys, z);

				int64_t u = bPole && i < nLon ? (pColU[i] + pColU[i + 1]) / 2 : pColU[i];
				writeVertex(j * columns + i, xs, ys, u, pRowV[j], Warpmap::coverageOne);
				pSeen[j * columns + i] = z >= z0 ? 1 : 0;
			}
		}

		// Crossings of the outline, each a vertex inside it and one outside it with the same position
		// in the source.

		int nextVertex = gridVertices;
		const int vertexEnd = gridVertices + 2 * maxCrossings;

		auto addCrossing = [&](int64_t sinP, int64_t cosP, int64_t sinL, int64_t cosL, int64_t u, int64_t v)
		{
			if (nextVertex + 2 > vertexEnd)
				return -1;

			int64_t xs, ys, z;
			project(sinP, cosP, sinL, cosL, xs, ys, z);

			int index = nextVertex;
			writeVertex(index, xs, ys, u, v, Warpmap::coverageOne);
			writeVertex(index + 1, (xs * outward) >> 30, (ys * outward) >> 30, u, v, 0);
			nextVertex += 2;
			return index;
		};

		auto zAt = [&](int j, int i)
		{
			int64_t xs, ys, z;
			project(pRowSin[j], pRowCos[j], pColSin[i], pColCos[i], xs, ys, z);
			return z;
		};

		// Angle of a crossing between a and b (angles), from acos(k) around the middle of the two
		// solutions, or from z going linearly between them if none of them is between a and b.

		auto crossing = [&](int64_t a, int64_t b, int64_t middle, int64_t k, int64_t za, int64_t zb, bool bSolvable)
		{
			int64_t lo = std::min(a, b), hi = std::max(a, b);

			if (bSolvable && k >= -c_one && k <= c_one)
			{
				int64_t s = int64_t(_sqrt(uint64_t(c_one * c_one - k * k)));
				int64_t delta = int64_t(_polar(k, s));					// acos(k), 0 -> half a turn.

				for (int64_t candidate : { middle + delta, middle - delta })
				{
					int64_t angle = int64_t(int32_t(uint32_t(candidate)));		// Within +- half a turn.
					if (angle >= lo && angle <= hi)
						return angle;
				}
			}

			return za == zb ? a : a + (b - a) * (z0 - za) / (zb - za);
		};

		for (int i = 0; i < columns; i++)
		{
			for (int j = 0; j < nLat; j++)
			{
				int index = -1;

				if (pSeen[j * columns + i] != pSeen[(j + 1) * columns + i])
				{
					// Along the meridian: z' = A cos P + B sin P.

					int64_t A = (int64_t(pColCos[i]) * cosT) >> 30;
					int64_t B = sinT;
					int64_t length;
					int64_t psi = int64_t(_polar(A, B, &length));
					int64_t k = length > 0 ? (z0 << 30) / length : 0;

					int64_t angle = crossing(pLat[j], pLat[j + 1], psi, k, zAt(j, i), zAt(j + 1, i), length > 0);

					int32_t sinP, cosP;
					_sinCos(uint32_t(angle), sinP, cosP);
					index = addCrossing(sinP, cosP, pColSin[i], pColCos[i], pColU[i], ((c_quarter - angle) * height) >> 31);
				}

				pMeridianCrossing[i * nLat + j] = index;
			}
		}

		for (int j = 0; j <= nLat; j++)
		{
			for (int i = 0; i < nLon; i++)
			{
				int index = -1;

				if (j > 0 && j < nLat && pSeen[j * columns + i] != pSeen[j * columns + i + 1])
				{
					// Along the parallel: z' = C cos L + D.

					int64_t C = (int64_t(pRowCos[j]) * cosT) >> 30;
					int64_t D = (int64_t(pRowSin[j]) * sinT) >> 30;
					int64_t k = C != 0 ? ((z0 - D) << 30) / C : 0;

					int64_t angle = crossing(pLon[i], pLon[i + 1], 0, k, zAt(j, i), zAt(j, i + 1), C != 0);

					int32_t sinL, cosL;
					_sinCos(uint32_t(angle), sinL, cosL);
					index = addCrossing(pRowSin[j], pRowCos[j], sinL, cosL, ((angle + c_half) * width) >> 32, pRowV[j]);
				}

				pParallelCrossing[j * nLon + i] = index;
			}
		}

		for (int index = nextVertex; index < vertexEnd; index++)
			pVertices[index] = { 0, 0, 0, 0 };

		// Pieces of the grid, the part seen of each as a fan, and the outline between crossings as
		// quads from inside it to outside it.

		int nextTriangle = 0;

		auto addTriangle = [&](int a, int b, int c)
		{
			if (nextTriangle < nTriangles)
				pTriangles[nextTriangle++] = { a, b, c };
		};

		for (int j = 0; j < nLat; j++)
		{
			for (int i = 0; i < nLon; i++)
			{
				// Corners clockwise from the top left, with the crossing of the side after each. A piece
				// at a pole has the pole as its one corner there.

				int corners[4], sides[4];
				int n = 0;

				int topLeft = j * columns + i, topRight = topLeft + 1;
				int bottomLeft = topLeft + columns, bottomRight = bottomLeft + 1;

				if (j == 0)
				{
					corners[n] = topLeft;		sides[n++] = pMeridianCrossing[(i + 1) * nLat + j];
				}
				else
				{
					corners[n] = topLeft;		sides[n++] = pParallelCrossing[j * nLon + i];
					corners[n] = topRight;		sides[n++] = pMeridianCrossing[(i + 1) * nLat + j];
				}

				if (j == nLat - 1)
				{
					corners[n] = nLat * columns + i;	sides[n++] = pMeridianCrossing[i * nLat + j];
				}
				else
				{
					corners[n] = bottomRight;	sides[n++] = pParallelCrossing[(j + 1) * nLon + i];
					corners[n] = bottomLeft;	sides[n++] = pMeridianCrossing[i * nLat + j];
				}

				// What is seen, with the kind of each: 0 corner, 1 leaving what is seen, 2 coming back.

				int polygon[8], kinds[8];
				int m = 0;

				for (int k = 0; k < n; k++)
				{
					bool bSeen = pSeen[corners[k]] != 0;
					bool bNextSeen = pSeen[corners[(k + 1) % n]] != 0;

					if (bSeen)
					{
						polygon[m] = corners[k];
						kinds[m++] = 0;
					}

					if (bSeen != bNextSeen && sides[k] >= 0)
					{
						polygon[m] = sides[k];
						kinds[m++] = bSeen ? 1 : 2;
					}
				}

				if (m < 3)
					continue;

				for (int k = 1; k + 1 < m; k++)
					addTriangle(polygon[0], polygon[k], polygon[k + 1]);

				for (int k = 0; k < m; k++)
				{
					int next = (k + 1) % m;
					if (kinds[k] == 1 && kinds[next] == 2)
					{
						int a = polygon[k], b = polygon[next];
						addTriangle(a, b, b + 1);
						addTriangle(a, b + 1, a + 1);
					}
				}
			}
		}

		while (nextTriangle < nTriangles)
			pTriangles[nextTriangle++] = { 0, 0, 0 };

		Base::memStackFree(scratch);
	}

} // namespace wax
