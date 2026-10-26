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
	 * The cells are kept in a texture buffer for the shaders, two RGBA32F entries per cell, row by
	 * row: the top-left and top-right corners, then the bottom-left and bottom-right corners, as
	 * given by cellCorners(), so already moved by whole periods to be close to the top-left one.
	 */

	class GlWarpmap : public Warpmap
	{
		friend class GlBackend;
	public:

		//.____ Creation __________________________________________

		static GlWarpmap_p	create( const Blueprint& blueprint );
		static GlWarpmap_p	create( const Blueprint& blueprint, const CoordF * pNodes, int pitch = 0 );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		GlWarpmap( const Blueprint& bp );
		virtual ~GlWarpmap();

		void	_nodesUpdated( const RectI& region ) override;

		inline SizeI	_cells() const { return { m_nodes.w - 1, m_nodes.h - 1 }; }

		GLuint		m_bufferId = 0;
		GLuint		m_textureId = 0;
	};

} // namespace wax
#endif // WAX_GLWARPMAP_DOT_H
