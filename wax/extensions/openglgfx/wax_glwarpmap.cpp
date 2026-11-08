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

#include <cstddef>

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
		static_assert(sizeof(Vertex) == 6 * sizeof(GLint) && sizeof(Triangle) == 3 * sizeof(GLushort), "Vertices and triangles are uploaded as they are");

		GLint previousArray = 0, previousBuffer = 0;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousArray);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &previousBuffer);

		glGenBuffers(1, &m_vertexBufferId);
		glGenBuffers(1, &m_indexBufferId);
		glGenVertexArrays(1, &m_vertexArrayId);

		glBindVertexArray(m_vertexArrayId);

		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, m_nVertices * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

		glVertexAttribIPointer(0, 2, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, x));
		glVertexAttribIPointer(1, 2, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, u));
		glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, w));
		glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, coverage));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferId);		// Kept by the vertex array.
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nTriangles * sizeof(Triangle), nullptr, GL_DYNAMIC_DRAW);

		glBindVertexArray(GLuint(previousArray));
		glBindBuffer(GL_ARRAY_BUFFER, GLuint(previousBuffer));

		_updated(0, m_nVertices, 0, m_nTriangles);
	}

	//____ destructor _________________________________________________________

	GlWarpmap::~GlWarpmap()
	{
		glDeleteVertexArrays(1, &m_vertexArrayId);
		glDeleteBuffers(1, &m_indexBufferId);
		glDeleteBuffers(1, &m_vertexBufferId);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GlWarpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _updated() _________________________________________________________
	// Copies what changed to the buffers. Periods are given when drawing. Goes through the copy
	// target, which no vertex array or other state of the backend depends on.

	void GlWarpmap::_updated( int firstVertex, int nVertices, int firstTriangle, int nTriangles )
	{
		if (nVertices > 0)
		{
			glBindBuffer(GL_COPY_WRITE_BUFFER, m_vertexBufferId);
			glBufferSubData(GL_COPY_WRITE_BUFFER, firstVertex * sizeof(Vertex), nVertices * sizeof(Vertex), m_pVertices + firstVertex);
		}

		if (nTriangles > 0)
		{
			glBindBuffer(GL_COPY_WRITE_BUFFER, m_indexBufferId);
			glBufferSubData(GL_COPY_WRITE_BUFFER, firstTriangle * sizeof(Triangle), nTriangles * sizeof(Triangle), m_pTriangles + firstTriangle);
		}

		glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
	}

} // namespace wax
