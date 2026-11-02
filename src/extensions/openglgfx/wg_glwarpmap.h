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
#ifndef WG_GLWARPMAP_DOT_H
#define WG_GLWARPMAP_DOT_H
#pragma once

#include <wg_warpmap.h>

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

// Warps are drawn in blocks of cells about this many pixels wide and high, and blocks that draw
// nothing are skipped. Smaller blocks skip more of thin shapes but cost more where everything is
// drawn, as pixels along the edges of the triangles are shaded twice. Used by warpVertexShader.

#define WG_GL_WARP_BLOCK_PIXELS 64

namespace wg
{
	class GlWarpmap;
	typedef	StrongPtr<GlWarpmap>	GlWarpmap_p;
	typedef	WeakPtr<GlWarpmap>	GlWarpmap_wp;

	//____ GlWarpmap ___________________________________________________________
	/**
	 * @brief Warpmap for GlBackend.
	 *
	 * The cells are kept in a texture buffer for the shaders, two RGBA32F entries per cell, row by
	 * row: the top-left and top-right corners, then the bottom-left and bottom-right corners, as
	 * given by cellCorners(), so already moved by whole periods to be close to the top-left one,
	 * converted from spx to pixels of the source.
	 *
	 * After the cells come blocks of _blockCells() x _blockCells() cells, two entries each, row by
	 * row: the lowest and highest positions of their corners, then how far beyond them coverage
	 * can reach (half the change in position over a pixel), so GlBackend can skip blocks that
	 * draw nothing without looking at their cells.
	 */

	class GlWarpmap : public Warpmap
	{
		friend class GlBackend;
	public:

		//.____ Creation __________________________________________

		static GlWarpmap_p	create( const Blueprint& blueprint );
		static GlWarpmap_p	create( const Blueprint& blueprint, const CoordSPX * pNodes, int pitch = 0 );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		GlWarpmap( const Blueprint& bp, const CoordSPX * pNodes = nullptr, int pitch = 0 );
		virtual ~GlWarpmap();

		void	_nodesUpdated( const RectI& region ) override;

		inline SizeI	_cells() const { return { m_nodes.w - 1, m_nodes.h - 1 }; }

		static int		_blockCells( int cellSize ) { return std::max(1, WG_GL_WARP_BLOCK_PIXELS / cellSize); }
		SizeI			_blocks() const;

		GLuint		m_bufferId = 0;
		GLuint		m_textureId = 0;
	};

} // namespace wg
#endif // WG_GLWARPMAP_DOT_H
