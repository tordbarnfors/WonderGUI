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
	 * The triangles are kept on the GPU, set up when the content changes:
	 *
	 * - A vertex buffer, with a vertex array for it, of three vertices for each triangle: its
	 *   position in pixels of the Warpmap (two floats) and the index of the triangle (an int), which
	 *   the shaders take for the whole triangle.
	 *
	 * - A texture buffer of four RGBA32F entries for each triangle, with its mapping as functions of
	 *   the position q of a point relative to its first corner, in pixels: W = a*q.x + b*q.y + c and
	 *   likewise U and V, the position in the source (in pixels) being (u0, v0) + (U, V) / W. The
	 *   entries are (corner.x, corner.y, Wa, Wb), (Wc, Ua, Ub, Uc), (Va, Vb, Vc, 0), (u0, v0, 0, 0).
	 *   Without perspective W is 1. With it, the weights are scaled so that the largest is 1.
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

		struct VertexGL
		{
			GLfloat		x, y;
			GLint		triangle;
		};

		GLuint		m_vertexBufferId = 0;
		GLuint		m_vertexArrayId = 0;
		GLuint		m_bufferId = 0;				// Mappings of the triangles.
		GLuint		m_textureId = 0;
	};

} // namespace wax
#endif // WAX_GLWARPMAP_DOT_H
