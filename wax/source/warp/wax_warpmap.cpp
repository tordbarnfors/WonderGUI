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
#include <wax_warpmap.h>

#include <algorithm>
#include <cstring>

namespace wax
{

	const TypeInfo Warpmap::TYPEINFO = { "Warpmap", &Object::TYPEINFO };

	//____ constructor ________________________________________________________

	Warpmap::Warpmap(const Blueprint& bp, const Vertex* pVertices, const Triangle* pTriangles)
		: m_size(bp.size), m_nVertices(bp.vertices), m_nTriangles(bp.triangles), m_uPeriod(bp.uPeriod), m_vPeriod(bp.vPeriod)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);

		m_pVertices = new Vertex[m_nVertices];
		m_pTriangles = new Triangle[m_nTriangles];

		// Start out mapping the rectangle straight onto the source, pixel for pixel, if there is
		// room for it. Everything else has all corners in the first vertex and draws nothing.

		bool bStraight = m_nVertices >= 4 && m_nTriangles >= 2;

		if (pVertices)
		{
			for (int i = 0; i < m_nVertices; i++)
				m_pVertices[i] = _clamped(pVertices[i]);
		}
		else
		{
			for (int i = 0; i < m_nVertices; i++)
				m_pVertices[i] = { 0, 0, 0, 0 };

			if (bStraight)
			{
				spx w = std::min(m_size.w * 64, maxPosition);
				spx h = std::min(m_size.h * 64, maxPosition);

				m_pVertices[1] = { w, 0, w, 0 };
				m_pVertices[2] = { w, h, w, h };
				m_pVertices[3] = { 0, h, 0, h };
			}
		}

		if (pTriangles)
			std::memcpy(m_pTriangles, pTriangles, m_nTriangles * sizeof(Triangle));
		else
		{
			for (int i = 0; i < m_nTriangles; i++)
				m_pTriangles[i] = { 0, 0, 0 };

			if (bStraight)
			{
				m_pTriangles[0] = { 0, 1, 2 };
				m_pTriangles[1] = { 0, 2, 3 };
			}
		}
	}

	//____ destructor _________________________________________________________

	Warpmap::~Warpmap()
	{
		delete[] m_pVertices;
		delete[] m_pTriangles;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Warpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setPeriods() _______________________________________________________

	bool Warpmap::setPeriods(spx uPeriod, spx vPeriod)
	{
		if (uPeriod < 0 || vPeriod < 0 || uPeriod > maxValue || vPeriod > maxValue)
			return false;

		m_uPeriod = uPeriod;
		m_vPeriod = vPeriod;

		_updated(0, 0, 0, 0);
		return true;
	}

	//____ importVertices() ___________________________________________________

	bool Warpmap::importVertices(int first, int count, const Vertex* pSource)
	{
		if (first < 0 || count < 0 || first > m_nVertices - count || (count > 0 && !pSource))
			return false;

		for (int i = 0; i < count; i++)
			m_pVertices[first + i] = _clamped(pSource[i]);

		_updated(first, count, 0, 0);
		return true;
	}

	//____ importTriangles() __________________________________________________

	bool Warpmap::importTriangles(int first, int count, const Triangle* pSource)
	{
		if (first < 0 || count < 0 || first > m_nTriangles - count || (count > 0 && !pSource))
			return false;

		if (!_isValid(pSource, count, m_nVertices))
			return false;

		std::memcpy(m_pTriangles + first, pSource, count * sizeof(Triangle));

		_updated(0, 0, first, count);
		return true;
	}

	//____ importMesh() _______________________________________________________

	bool Warpmap::importMesh(const Vertex* pVertices, const Triangle* pTriangles, spx uPeriod, spx vPeriod)
	{
		if (uPeriod < 0 || vPeriod < 0 || uPeriod > maxValue || vPeriod > maxValue)
			return false;

		if (pTriangles && !_isValid(pTriangles, m_nTriangles, m_nVertices))
			return false;

		if (pVertices)
		{
			for (int i = 0; i < m_nVertices; i++)
				m_pVertices[i] = _clamped(pVertices[i]);
		}

		if (pTriangles)
			std::memcpy(m_pTriangles, pTriangles, m_nTriangles * sizeof(Triangle));

		m_uPeriod = uPeriod;
		m_vPeriod = vPeriod;

		_updated(0, pVertices ? m_nVertices : 0, 0, pTriangles ? m_nTriangles : 0);
		return true;
	}

	//____ exportVertices() ___________________________________________________

	bool Warpmap::exportVertices(int first, int count, Vertex* pDestination) const
	{
		if (first < 0 || count < 0 || first > m_nVertices - count || (count > 0 && !pDestination))
			return false;

		std::memcpy(pDestination, m_pVertices + first, count * sizeof(Vertex));
		return true;
	}

	//____ exportTriangles() __________________________________________________

	bool Warpmap::exportTriangles(int first, int count, Triangle* pDestination) const
	{
		if (first < 0 || count < 0 || first > m_nTriangles - count || (count > 0 && !pDestination))
			return false;

		std::memcpy(pDestination, m_pTriangles + first, count * sizeof(Triangle));
		return true;
	}

	//____ triangleAt() _______________________________________________________

	int Warpmap::triangleAt(CoordSPX point) const
	{
		for (int i = 0; i < m_nTriangles; i++)
		{
			if (isInside(i, point))
				return i;
		}

		return -1;
	}

	//____ isInside() _________________________________________________________
	/*
		With the corners in the order that gives the triangle a positive area, so that the inside is
		where each edge function e = dx * (py - y0) - dy * (px - x0) is positive (y going down), a
		point on an edge is inside if it is a left edge (going up, dy < 0) or a horizontal top edge
		(going right, dx > 0). Two triangles sharing an edge go along it in opposite directions, so
		exactly one of them has it. Positions are within +-2^26 and differences within +-2^32, so
		products fit in 64 bits.
	*/

	bool Warpmap::isInside(int triangle, CoordSPX point) const
	{
		if (triangle < 0 || triangle >= m_nTriangles)
			return false;

		const Triangle& t = m_pTriangles[triangle];
		const Vertex* p[3] = { &m_pVertices[t.a], &m_pVertices[t.b], &m_pVertices[t.c] };

		int64_t area = int64_t(p[1]->x - p[0]->x) * (p[2]->y - p[0]->y) - int64_t(p[1]->y - p[0]->y) * (p[2]->x - p[0]->x);

		if (area == 0)
			return false;

		if (area < 0)
			std::swap(p[1], p[2]);

		for (int i = 0; i < 3; i++)
		{
			const Vertex* p0 = p[i];
			const Vertex* p1 = p[(i + 1) % 3];

			int64_t dx = int64_t(p1->x) - p0->x;
			int64_t dy = int64_t(p1->y) - p0->y;
			int64_t e = dx * (int64_t(point.y) - p0->y) - dy * (int64_t(point.x) - p0->x);

			if (e < 0 || (e == 0 && !(dy < 0 || (dy == 0 && dx > 0))))
				return false;
		}

		return true;
	}

	//____ positionIn() _______________________________________________________

	bool Warpmap::positionIn(int triangle, double x, double y, CoordF& position) const
	{
		if (triangle < 0 || triangle >= m_nTriangles)
			return false;

		const Triangle& t = m_pTriangles[triangle];
		const Vertex& v0 = m_pVertices[t.a];
		const Vertex& v1 = m_pVertices[t.b];
		const Vertex& v2 = m_pVertices[t.c];

		double e1x = double(v1.x) - v0.x, e1y = double(v1.y) - v0.y;
		double e2x = double(v2.x) - v0.x, e2y = double(v2.y) - v0.y;
		double area = e1x * e2y - e1y * e2x;

		if (area == 0.0)
			return false;

		double dx = x * 64 - v0.x;
		double dy = y * 64 - v0.y;

		double b1 = (dx * e2y - dy * e2x) / area;
		double b2 = (e1x * dy - e1y * dx) / area;
		double b0 = 1.0 - b1 - b2;

		double w0 = b0 * v0.w, w1 = b1 * v1.w, w2 = b2 * v2.w;
		double w = w0 + w1 + w2;

		if (w <= 0.0)
			return false;

		position.x = float((w0 * v0.u + w1 * v1.u + w2 * v2.u) / w);
		position.y = float((w0 * v0.v + w1 * v1.v + w2 * v2.v) / w);
		return true;
	}

	//____ _validateBlueprint() _______________________________________________

	bool Warpmap::_validateBlueprint(const Blueprint& bp, const Triangle* pTriangles)
	{
		if (bp.size.w <= 0 || bp.size.h <= 0)
			return false;

		if (bp.vertices < 3 || bp.vertices > maxVertices || bp.triangles < 1 || bp.triangles > maxTriangles)
			return false;

		if (bp.uPeriod < 0 || bp.vPeriod < 0 || bp.uPeriod > maxValue || bp.vPeriod > maxValue)
			return false;

		if (pTriangles && !_isValid(pTriangles, bp.triangles, bp.vertices))
			return false;

		return true;
	}

	//____ _clamped() _________________________________________________________

	Warpmap::Vertex Warpmap::_clamped(const Vertex& vertex)
	{
		Vertex v;
		v.x = std::clamp(vertex.x, -maxPosition, maxPosition);
		v.y = std::clamp(vertex.y, -maxPosition, maxPosition);
		v.u = std::clamp(vertex.u, -maxValue, maxValue);
		v.v = std::clamp(vertex.v, -maxValue, maxValue);
		v.w = std::clamp(vertex.w, 1, maxWeight);
		return v;
	}

	//____ _isValid() _________________________________________________________

	bool Warpmap::_isValid(const Triangle* pTriangles, int count, int nVertices)
	{
		for (int i = 0; i < count; i++)
		{
			const Triangle& t = pTriangles[i];

			if (t.a < 0 || t.b < 0 || t.c < 0 || t.a >= nVertices || t.b >= nVertices || t.c >= nVertices)
				return false;
		}

		return true;
	}

} // namespace wax
