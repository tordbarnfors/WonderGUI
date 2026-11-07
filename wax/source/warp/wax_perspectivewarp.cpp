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
		if (!_validateSettings(bp.size, bp.sourceSize, bp.cellSize, bp.precision) || !_isValidQuad(bp.corners))
			return nullptr;

		if (!pFactory)
			pFactory = Base::defaultWarpmapFactory();

		if (!pFactory)
			return nullptr;

		return PerspectiveWarp_p(new PerspectiveWarp(bp, pFactory));
	}

	//____ constructor ________________________________________________________

	PerspectiveWarp::PerspectiveWarp(const Blueprint& bp, WarpmapFactory* pFactory)
		: WarpGenerator(bp.size, bp.sourceSize, bp.cellSize, bp.precision, pFactory)
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

		const int c_extra = 6;

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

		and the Warpmap needs the inverse, which is the adjugate of that matrix. It is calculated once
		per generate, in floating point without any library calls, then scaled so that the terms use
		the range of int64 for the nodes, which are calculated with integers.
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

		double rows[3][3] = {	{ (e - f * h) * m_sourceSize.w, (c * h - b) * m_sourceSize.w, (b * f - c * e) * m_sourceSize.w },
								{ (f * g - d) * m_sourceSize.h, (a - c * g) * m_sourceSize.h, (c * d - a * f) * m_sourceSize.h },
								{ d * h - e * g, b * g - a * h, a * e - b * d } };

		double cx = (x[0] + x[1] + x[2] + x[3]) / 4, cy = (y[0] + y[1] + y[2] + y[3]) / 4;
		if (rows[2][0] * cx + rows[2][1] * cy + rows[2][2] < 0)
			for (auto& row : rows)
				for (double& value : row)
					value = -value;

		// Scaled so that every term and sum is within 2^61 within the Warpmap.

		double largest = 0.0;
		for (auto& row : rows)
			largest = std::max(largest, std::abs(row[0]) * m_size.w + std::abs(row[1]) * m_size.h + std::abs(row[2]));

		Projection p = {};
		if (!(largest > 0.0 && largest < 0x1p1000))			// Not for valid corners, but also catches NaN.
			return p;

		double scale = 0x1p61 / largest;
		auto toFixed = [scale](double value) { value *= scale; return int64_t(value + (value < 0 ? -0.5 : 0.5)); };

		for (int i = 0; i < 3; i++)
		{
			p.u[i] = toFixed(rows[0][i]);
			p.v[i] = toFixed(rows[1][i]);
			p.w[i] = toFixed(rows[2][i]);
		}
		return p;
	}

	//____ _calcCellSize() ____________________________________________________
	/*
		Along a line through the plane, the source position is (alpha*x + beta) / (gamma*x + delta).
		Interpolating that over cells of c pixels is off by about c^2 / 8 times its second derivative,
		which in pixels on screen is c^2 / (4 * d), d being the distance to the horizon (where the
		divisor is 0) along the line. It is shortest straight towards the horizon, where it is
		W / |gradient of W|, and smallest at the corner nearest the horizon. That is the same as a
		circle of radius d / 2.

		Without perspective W is constant and interpolation is exact, so cells span the Warpmap.
	*/

	SizeI PerspectiveWarp::_calcCellSize() const
	{
		Projection p = _projection();

		if (p.w[0] == 0 && p.w[1] == 0)
			return m_size;

		double nearest = 0x1p1000;
		for (int i = 0; i < 4; i++)
		{
			double w = (double(p.w[0]) * m_corners[i].x + double(p.w[1]) * m_corners[i].y) / 64 + double(p.w[2]);
			nearest = std::min(nearest, w);
		}

		// Radius in spx is 32 * d, so its square is 1024 * W^2 / |gradient|^2.

		double gradient2 = double(p.w[0]) * p.w[0] + double(p.w[1]) * p.w[1];
		double radius2 = nearest > 0.0 ? 1024.0 * nearest * nearest / gradient2 : 0.0;

		int64_t radius = int64_t(_sqrt(uint64_t(std::min(radius2, 0x1p62))));
		int cell = _cellSizeForRadius(radius);
		return { cell, cell };
	}

	//____ _generateNodes() ___________________________________________________
	/*
		With integers only: per node a few multiplications and a division for each of u and v. At
		the horizon and beyond, which isn't part of the plane, nodes are far outside the source in
		the direction the plane goes off to, as they are just before the horizon.
	*/

	void PerspectiveWarp::_generateNodes(CoordSPX* pNodes, SizeI nodes, SizeI cellSize) const
	{
		Projection p = _projection();

		auto divide = [](int64_t n, int64_t w)
		{
			int64_t half = w / 2;
			return _toNodeValue((n + (n < 0 ? -half : half)) / w);
		};

		const spx c_far = Warpmap::maxNodeValue;

		for (int row = 0; row < nodes.h; row++)
		{
			int64_t y = std::min(row * cellSize.h, m_size.h);

			int64_t u0 = p.u[1] * y + p.u[2];
			int64_t v0 = p.v[1] * y + p.v[2];
			int64_t w0 = p.w[1] * y + p.w[2];

			for (int col = 0; col < nodes.w; col++)
			{
				int64_t x = std::min(col * cellSize.w, m_size.w);

				int64_t u = p.u[0] * x + u0;
				int64_t v = p.v[0] * x + v0;
				int64_t w = p.w[0] * x + w0;

				if (w > 0)
					*pNodes++ = { divide(u, w), divide(v, w) };
				else
					*pNodes++ = { u < 0 ? -c_far : c_far, v < 0 ? -c_far : c_far };
			}
		}
	}

} // namespace wax
