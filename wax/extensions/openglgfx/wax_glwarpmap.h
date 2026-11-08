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
#ifndef WAX_GLWARPMAP_DOT_H
#define WAX_GLWARPMAP_DOT_H
#pragma once

#include <wax_warpmap.h>

#ifdef WIN32
#	include <GL/glew.h>
#else
#	ifdef __APPLE__
#		define GL_SILENCE_DEPRECATION
#		include <OpenGL/gl3.h>
#	else
#		include <GL/glew.h>
#	endif
#endif

namespace wax
{
	class GlWarpmap;
	typedef	StrongPtr<GlWarpmap>	GlWarpmap_p;
	typedef	WeakPtr<GlWarpmap>	GlWarpmap_wp;

	//____ GlWarpmap ___________________________________________________________
	/**
	 * @brief Warpmap for GlBackend.
	 *
	 * The mesh is kept on the GPU as it is, in a vertex buffer of the vertices (x, y, u, v, w and
	 * coverage as integers, the layout of Warpmap::Vertex) and an index buffer of the triangles (the
	 * corners as unsigned shorts, the layout of Warpmap::Triangle), with a vertex array for both. Content that
	 * changes is copied to them, with nothing worked out on the CPU. Drawing only gives where to
	 * draw, the source offset and the periods.
	 *
	 * The vertex shader passes u * w, v * w and w on without perspective correction, which makes
	 * them linear over the canvas, so that the fragment shaders get the position in the source as
	 * their ratio, and its derivatives exactly from those of the three, also in the pixels around a
	 * triangle that are shaded for it. u and v are taken relative to those of the first vertex, as
	 * integers, to keep the precision of floats for meshes far into the source.
	 */

	class GlWarpmap : public Warpmap
	{
		friend class GlBackend;
	public:

		//.____ Creation __________________________________________

		static GlWarpmap_p	create( const Blueprint& blueprint );
		static GlWarpmap_p	create( const Blueprint& blueprint, const Vertex * pVertices, const Triangle * pTriangles );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		GlWarpmap( const Blueprint& bp, const Vertex * pVertices = nullptr, const Triangle * pTriangles = nullptr );
		virtual ~GlWarpmap();

		void	_updated( int firstVertex, int nVertices, int firstTriangle, int nTriangles ) override;

		GLuint		m_vertexBufferId = 0;
		GLuint		m_indexBufferId = 0;
		GLuint		m_vertexArrayId = 0;
	};

} // namespace wax
#endif // WAX_GLWARPMAP_DOT_H
