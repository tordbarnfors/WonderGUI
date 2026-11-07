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
#include <wax_glwarpmap.h>
#include <wax_base.h>

#include <algorithm>
#include <cstddef>
#include <vector>

namespace wax
{
	const TypeInfo GlWarpmap::TYPEINFO = { "GlWarpmap", &Warpmap::TYPEINFO };

	//____ create() ___________________________________________________________

	GlWarpmap_p GlWarpmap::create( const Blueprint& bp )
	{
		if (!_validateBlueprint(bp))
			return nullptr;

		return GlWarpmap_p(new GlWarpmap(bp));
	}

	GlWarpmap_p GlWarpmap::create( const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles )
	{
		if (!pVertices || !pTriangles || !_validateBlueprint(bp, pTriangles))
			return nullptr;

		return GlWarpmap_p(new GlWarpmap(bp, pVertices, pTriangles));
	}

	//____ constructor ________________________________________________________

	GlWarpmap::GlWarpmap( const Blueprint& bp, const Vertex * pVertices, const Triangle * pTriangles ) : Warpmap(bp, pVertices, pTriangles)
	{
		GLint previousArray = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousArray);

		glGenBuffers(1, &m_vertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, m_nTriangles * 3 * sizeof(VertexGL), nullptr, GL_DYNAMIC_DRAW);

		glGenVertexArrays(1, &m_vertexArrayId);
		glBindVertexArray(m_vertexArrayId);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexGL), (void*)offsetof(VertexGL, x));
		glVertexAttribIPointer(1, 1, GL_INT, sizeof(VertexGL), (void*)offsetof(VertexGL, triangle));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(GLuint(previousArray));
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glGenBuffers(1, &m_bufferId);
		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);
		glBufferData(GL_TEXTURE_BUFFER, m_nTriangles * 16 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_BUFFER, m_textureId);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_bufferId);
		glBindTexture(GL_TEXTURE_BUFFER, 0);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		_updated(0, m_nVertices, 0, m_nTriangles);
	}

	//____ destructor _________________________________________________________

	GlWarpmap::~GlWarpmap()
	{
		glDeleteTextures(1, &m_textureId);
		glDeleteBuffers(1, &m_bufferId);
		glDeleteVertexArrays(1, &m_vertexArrayId);
		glDeleteBuffers(1, &m_vertexBufferId);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GlWarpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _updated() _________________________________________________________
	/*
		Sets up all triangles again, since a vertex can be a corner of any of them, unless only the
		periods changed, which are given when drawing. Mappings are worked out in double and stored
		relative to the first corner, to keep the precision of floats in the shaders.
	*/

	void GlWarpmap::_updated( int firstVertex, int nVertices, int firstTriangle, int nTriangles )
	{
		if (nVertices == 0 && nTriangles == 0)
			return;

		std::vector<VertexGL> vertices(m_nTriangles * 3);
		std::vector<GLfloat> mappings(m_nTriangles * 16, 0.f);

		for (int t = 0; t < m_nTriangles; t++)
		{
			const Triangle& tri = m_pTriangles[t];
			const Vertex* c[3] = { &m_pVertices[tri.a], &m_pVertices[tri.b], &m_pVertices[tri.c] };

			for (int i = 0; i < 3; i++)
				vertices[t * 3 + i] = { c[i]->x / 64.f, c[i]->y / 64.f, GLint(t) };

			double r1x = (double(c[1]->x) - c[0]->x) / 64, r1y = (double(c[1]->y) - c[0]->y) / 64;
			double r2x = (double(c[2]->x) - c[0]->x) / 64, r2y = (double(c[2]->y) - c[0]->y) / 64;
			double area = r1x * r2y - r1y * r2x;

			GLfloat* pOut = &mappings[t * 16];
			pOut[0] = c[0]->x / 64.f;
			pOut[1] = c[0]->y / 64.f;
			pOut[12] = c[0]->u / 64.f;
			pOut[13] = c[0]->v / 64.f;

			if (area == 0.0)
			{
				pOut[4] = 1.f;			// W = 1, which no pixel is drawn with anyway.
				continue;
			}

			// a, b and c of a function with values f at the corners.

			auto linear = [&](const double f[3], double* pABC)
			{
				double d1 = f[1] - f[0], d2 = f[2] - f[0];
				pABC[0] = (d1 * r2y - d2 * r1y) / area;
				pABC[1] = (d2 * r1x - d1 * r2x) / area;
				pABC[2] = f[0];
			};

			bool bPerspective = !(c[0]->w == c[1]->w && c[1]->w == c[2]->w);
			int largest = std::max({ c[0]->w, c[1]->w, c[2]->w });

			double w[3], u[3], v[3];
			for (int i = 0; i < 3; i++)
			{
				w[i] = bPerspective ? double(c[i]->w) / largest : 1.0;
				u[i] = w[i] * (double(c[i]->u) - c[0]->u) / 64;
				v[i] = w[i] * (double(c[i]->v) - c[0]->v) / 64;
			}

			double fw[3], fu[3], fv[3];
			linear(w, fw);
			linear(u, fu);
			linear(v, fv);

			pOut[2] = GLfloat(fw[0]);
			pOut[3] = GLfloat(fw[1]);
			pOut[4] = GLfloat(fw[2]);
			pOut[5] = GLfloat(fu[0]);
			pOut[6] = GLfloat(fu[1]);
			pOut[7] = GLfloat(fu[2]);
			pOut[8] = GLfloat(fv[0]);
			pOut[9] = GLfloat(fv[1]);
			pOut[10] = GLfloat(fv[2]);
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(VertexGL), vertices.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);
		glBufferSubData(GL_TEXTURE_BUFFER, 0, mappings.size() * sizeof(GLfloat), mappings.data());
		glBindBuffer(GL_TEXTURE_BUFFER, 0);
	}

} // namespace wax
