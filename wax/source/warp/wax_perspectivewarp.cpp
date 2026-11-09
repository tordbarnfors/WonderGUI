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
#include <wax_perspectivewarp.h>
#include <wax_base.h>

#include <algorithm>
#include <cmath>

namespace wax
{
	const TypeInfo PerspectiveWarp::TYPEINFO = { "PerspectiveWarp", &WarpGenerator::TYPEINFO };

	//____ create() ___________________________________________________________

	PerspectiveWarp_p PerspectiveWarp::create(const Blueprint& bp, WarpmapFactory* pFactory)
	{
		if (!_validateSettings(bp.size, bp.sourceSize, 1.f) || !_isValidQuad(bp.corners))
			return nullptr;

		if (!pFactory)
			pFactory = Base::defaultWarpmapFactory();

		if (!pFactory)
			return nullptr;

		return PerspectiveWarp_p(new PerspectiveWarp(bp, pFactory));
	}

	//____ constructor ________________________________________________________

	PerspectiveWarp::PerspectiveWarp(const Blueprint& bp, WarpmapFactory* pFactory)
		: WarpGenerator(bp.size, bp.sourceSize, 1.f, pFactory)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		m_corners = bp.corners;
	}

	//____ destructor _________________________________________________________

	PerspectiveWarp::~PerspectiveWarp()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PerspectiveWarp::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setCorners() _______________________________________________________

	bool PerspectiveWarp::setCorners(const CornersT<CoordSPX>& corners)
	{
		if (!_isValidQuad(corners))
			return false;

		if (corners != m_corners)
		{
			m_corners = corners;
			m_bDirty = true;
		}
		return true;
	}

	//____ rotated() __________________________________________________________
	/*
		Integers only, angles in 1/2^32 of a turn and their sines and cosines with 30 binals. Points
		are relative to the pivot, z away from the viewer. Each corner is rotated, then projected
		onto the screen along the line from the eye. Positions within about +-2^24 pixels.
	*/

	CornersT<CoordSPX> PerspectiveWarp::rotated(const RectSPX& rect, float tilt, float turn, float roll, spx viewDistance,
												CoordF pivot, std::optional<CoordSPX> eye)
	{
		// Positions have 6 binals more than spx while rotating and projecting, so rounding is done once.

		static constexpr int c_extra = 6;

		auto round = [](double value) { return int64_t(value + (value < 0 ? -0.5 : 0.5)); };
		auto unscale = [](int64_t value) { return (value + (int64_t(1) << 29)) >> 30; };

		int64_t px = (int64_t(rect.x) << c_extra) + round(double(pivot.x) * rect.w * (1 << c_extra));
		int64_t py = (int64_t(rect.y) << c_extra) + round(double(pivot.y) * rect.h * (1 << c_extra));

		int64_t ex = eye ? int64_t(eye->x) << c_extra : px;
		int64_t ey = eye ? int64_t(eye->y) << c_extra : py;
		int64_t distance = int64_t(viewDistance) << c_extra;

		int32_t sinRoll, cosRoll, sinTurn, cosTurn, sinTilt, cosTilt;
		_sinCos(_toTurn(roll / 360.0), sinRoll, cosRoll);
		_sinCos(_toTurn(turn / 360.0), sinTurn, cosTurn);
		_sinCos(_toTurn(tilt / 360.0), sinTilt, cosTilt);

		const int64_t left = rect.x, top = rect.y, right = int64_t(rect.x) + rect.w, bottom = int64_t(rect.y) + rect.h;
		const int64_t points[4][2] = { { left, top }, { right, top }, { right, bottom }, { left, bottom } };

		CornersT<CoordSPX> result;

		for (int i = 0; i < 4; i++)
		{
			int64_t x = (points[i][0] << c_extra) - px;
			int64_t y = (points[i][1] << c_extra) - py;

			// Roll, clockwise on screen.

			int64_t x1 = unscale(x * cosRoll - y * sinRoll);
			int64_t y1 = unscale(x * sinRoll + y * cosRoll);

			// Turn, the right side moving away.

			int64_t x2 = unscale(x1 * cosTurn);
			int64_t z2 = unscale(x1 * sinTurn);

			// Tilt, the top moving away.

			int64_t y3 = unscale(y1 * cosTilt + z2 * sinTilt);
			int64_t z3 = unscale(z2 * cosTilt - y1 * sinTilt);

			int64_t sx = px + x2;
			int64_t sy = py + y3;

			if (viewDistance > 0)
			{
				int64_t depth = distance + z3;
				if (depth <= 0)
				{
					// At or behind the eye, which can't be seen. All corners at one point are refused.

					CoordSPX point = { spx(ex >> c_extra), spx(ey >> c_extra) };
					return { point, point, point, point };
				}

				auto project = [depth, distance](int64_t value, int64_t eye)
				{
					int64_t n = (value - eye) * distance;
					return eye + (n + (n < 0 ? -depth / 2 : depth / 2)) / depth;
				};

				sx = project(sx, ex);
				sy = project(sy, ey);
			}

			const int64_t c_limit = int64_t(1) << 30;
			auto toSpx = [c_limit](int64_t value) { return spx(std::clamp((value + (1 << (c_extra - 1))) >> c_extra, -c_limit, c_limit)); };
			result[i] = { toSpx(sx), toSpx(sy) };
		}

		return result;
	}

	//____ _isValidQuad() _____________________________________________________
	// Convex, with some area: all corners turn the same way.

	bool PerspectiveWarp::_isValidQuad(const CornersT<CoordSPX>& corners)
	{
		int positive = 0, negative = 0;

		for (int i = 0; i < 4; i++)
		{
			CoordSPX a = corners[i], b = corners[(i + 1) % 4], c = corners[(i + 2) % 4];

			double cross = (double(b.x) - a.x) * (double(c.y) - b.y) - (double(b.y) - a.y) * (double(c.x) - b.x);
			if (cross > 0)
				positive++;
			else if (cross < 0)
				negative++;
		}

		return positive == 4 || negative == 4;
	}

	//____ _projection() ______________________________________________________
	/*
		The projective transform from the source to the corners, as in Heckbert's "Fundamentals of
		Texture Mapping and Image Warping": a unit square (s,t) is mapped to x,y by

			x = (a*s + b*t + c) / (g*s + h*t + 1),  y = (d*s + e*t + f) / (g*s + h*t + 1)

		and the Warpmap needs the inverse, which is the adjugate of that matrix. Calculated once per
		generate, in floating point without any library calls. All zero for corners without area.
	*/

	PerspectiveWarp::Projection PerspectiveWarp::_projection() const
	{
		double x[4], y[4];
		for (int i = 0; i < 4; i++)
		{
			x[i] = m_corners[i].x / 64.0;
			y[i] = m_corners[i].y / 64.0;
		}

		// Unit square to the corners.

		double a, b, c, d, e, f, g, h;

		double dx1 = x[1] - x[2], dx2 = x[3] - x[2], dx3 = x[0] - x[1] + x[2] - x[3];
		double dy1 = y[1] - y[2], dy2 = y[3] - y[2], dy3 = y[0] - y[1] + y[2] - y[3];

		if (dx3 == 0.0 && dy3 == 0.0)
		{
			// A parallelogram, no perspective.

			g = h = 0.0;
		}
		else
		{
			double den = dx1 * dy2 - dx2 * dy1;
			g = (dx3 * dy2 - dx2 * dy3) / den;
			h = (dx1 * dy3 - dx3 * dy1) / den;
		}

		a = x[1] - x[0] + g * x[1];
		b = x[3] - x[0] + h * x[3];
		c = x[0];
		d = y[1] - y[0] + g * y[1];
		e = y[3] - y[0] + h * y[3];
		f = y[0];

		// Its inverse, the corners to the unit square, scaled to the source. W is made positive on
		// the plane.

		Projection p = {	{ (e - f * h) * m_sourceSize.w, (c * h - b) * m_sourceSize.w, (b * f - c * e) * m_sourceSize.w },
							{ (f * g - d) * m_sourceSize.h, (a - c * g) * m_sourceSize.h, (c * d - a * f) * m_sourceSize.h },
							{ d * h - e * g, b * g - a * h, a * e - b * d } };

		double cx = (x[0] + x[1] + x[2] + x[3]) / 4, cy = (y[0] + y[1] + y[2] + y[3]) / 4;
		if (p.w[0] * cx + p.w[1] * cy + p.w[2] < 0)
		{
			for (int i = 0; i < 3; i++)
			{
				p.u[i] = -p.u[i];
				p.v[i] = -p.v[i];
				p.w[i] = -p.w[i];
			}
		}

		for (int i = 0; i < 3; i++)
		{
			if (!(std::abs(p.u[i]) < 0x1p1000 && std::abs(p.v[i]) < 0x1p1000 && std::abs(p.w[i]) < 0x1p1000))		// Also catches NaN.
				return {};
		}

		return p;
	}

	//____ _meshSize() ________________________________________________________

	void PerspectiveWarp::_meshSize(int& nVertices, int& nTriangles) const
	{
		nVertices = c_maxVertices;
		nTriangles = c_maxTriangles;
	}

	//____ _generateMesh() ____________________________________________________
	/*
		The quadrilateral of the corners, with each side moved a pixel outwards so that its border
		is antialiased within the triangles, then clipped to where W is at least half of what it
		is at the corners, which keeps it off the horizon, and to the rectangle of the Warpmap. The
		resulting convex polygon, of at most 9 corners, is split into a fan of triangles from its
		first corner. Corners are rounded to spx, then given the exact position in the source and
		W there as weight, scaled so that the largest is Warpmap::maxWeight. Unused vertices and
		triangles are all in the first vertex.

		All in floating point, once per generate and for a handful of points, without library
		calls other than a square root for each corner.
	*/

	void PerspectiveWarp::_generateMesh(Warpmap::Vertex* pVertices, Warpmap::Triangle* pTriangles) const
	{
		for (int i = 0; i < c_maxVertices; i++)
			pVertices[i] = { 0, 0, 0, 0 };
		for (int i = 0; i < c_maxTriangles; i++)
			pTriangles[i] = { 0, 0, 0 };

		Projection p = _projection();
		if (p.w[0] == 0.0 && p.w[1] == 0.0 && p.w[2] == 0.0)
			return;

		auto W = [&](double x, double y) { return p.w[0] * x + p.w[1] * y + p.w[2]; };

		// Corners in pixels, and which way they go round.

		double cx[4], cy[4];
		for (int i = 0; i < 4; i++)
		{
			cx[i] = m_corners[i].x / 64.0;
			cy[i] = m_corners[i].y / 64.0;
		}

		double area = 0;
		for (int i = 0; i < 4; i++)
			area += cx[i] * cy[(i + 1) % 4] - cx[(i + 1) % 4] * cy[i];
		double turn = area > 0 ? 1.0 : -1.0;

		// Corners moved out to where the sides, moved a pixel outwards, meet. Very sharp corners are
		// moved at most 8 pixels.

		const double c_margin = 1.0;
		const double c_maxMiter = 8.0;

		double px[c_maxVertices + 1], py[c_maxVertices + 1];
		int n = 4;

		auto outwards = [&](int from, int to, double& nx, double& ny)
		{
			double dx = cx[to] - cx[from], dy = cy[to] - cy[from];
			double length = std::sqrt(dx * dx + dy * dy);
			nx = turn * dy / length;
			ny = -turn * dx / length;
		};

		for (int i = 0; i < 4; i++)
		{
			double n1x, n1y, n2x, n2y;
			outwards((i + 3) % 4, i, n1x, n1y);
			outwards(i, (i + 1) % 4, n2x, n2y);

			double den = 1.0 + n1x * n2x + n1y * n2y;
			double ox = c_margin * (n1x + n2x) / den;
			double oy = c_margin * (n1y + n2y) / den;

			double length2 = ox * ox + oy * oy;
			if (length2 > c_maxMiter * c_maxMiter)
			{
				double scale = c_maxMiter / std::sqrt(length2);
				ox *= scale;
				oy *= scale;
			}

			px[i] = cx[i] + ox;
			py[i] = cy[i] + oy;
		}

		// Clipped to a * x + b * y + c >= 0, one line at a time.

		auto clip = [&](double a, double b, double c)
		{
			double qx[c_maxVertices + 1], qy[c_maxVertices + 1];
			int m = 0;

			for (int i = 0; i < n; i++)
			{
				int j = (i + 1) % n;
				double fi = a * px[i] + b * py[i] + c;
				double fj = a * px[j] + b * py[j] + c;

				if (fi >= 0)
				{
					qx[m] = px[i];
					qy[m] = py[i];
					m++;
				}

				if ((fi >= 0) != (fj >= 0))
				{
					double t = fi / (fi - fj);
					qx[m] = px[i] + (px[j] - px[i]) * t;
					qy[m] = py[i] + (py[j] - py[i]) * t;
					m++;
				}
			}

			n = m;
			for (int i = 0; i < n; i++)
			{
				px[i] = qx[i];
				py[i] = qy[i];
			}
		};

		if (p.w[0] != 0.0 || p.w[1] != 0.0)
		{
			double lowest = W(cx[0], cy[0]);
			for (int i = 1; i < 4; i++)
				lowest = std::min(lowest, W(cx[i], cy[i]));

			clip(p.w[0], p.w[1], p.w[2] - lowest / 2);
		}

		clip(1, 0, 0);
		clip(-1, 0, m_size.w);
		clip(0, 1, 0);
		clip(0, -1, m_size.h);

		// Rounded to spx, dropping corners that end up in the same place as the one before.

		Warpmap::Vertex* pOut = pVertices;
		double weights[c_maxVertices];
		int nOut = 0;

		for (int i = 0; i < n; i++)
		{
			spx x = spx(px[i] * 64 + (px[i] < 0 ? -0.5 : 0.5));
			spx y = spx(py[i] * 64 + (py[i] < 0 ? -0.5 : 0.5));

			if (nOut > 0 && ((pOut[nOut - 1].x == x && pOut[nOut - 1].y == y) || (pOut[0].x == x && pOut[0].y == y)))
				continue;

			double fx = x / 64.0, fy = y / 64.0;
			double w = W(fx, fy);
			if (!(w > 0.0))
				continue;

			double u = (p.u[0] * fx + p.u[1] * fy + p.u[2]) / w;
			double v = (p.v[0] * fx + p.v[1] * fy + p.v[2]) / w;

			const double c_limit = Warpmap::maxValue;
			pOut[nOut] = { x, y, spx(std::clamp(u, -c_limit, c_limit)), spx(std::clamp(v, -c_limit, c_limit)) };
			weights[nOut] = w;
			nOut++;
		}

		if (nOut < 3)
		{
			for (int i = 0; i < c_maxVertices; i++)
				pVertices[i] = { 0, 0, 0, 0 };
			return;
		}

		double largest = 0;
		for (int i = 0; i < nOut; i++)
			largest = std::max(largest, weights[i]);

		for (int i = 0; i < nOut; i++)
			pOut[i].w = std::max(1, int(weights[i] / largest * Warpmap::maxWeight + 0.5));

		for (int i = nOut; i < c_maxVertices; i++)
			pVertices[i] = pVertices[0];

		for (int i = 0; i < nOut - 2; i++)
			pTriangles[i] = { 0, i + 1, i + 2 };
	}

} // namespace wax
