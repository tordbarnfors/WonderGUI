/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Glware Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG_GLEDGEMAP_DOT_H
#define WG_GLEDGEMAP_DOT_H
#pragma once

#include <wg_edgemap.h>

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

namespace wg
{

	class GlEdgemap;
	typedef StrongPtr<GlEdgemap> GlEdgemap_p;
	typedef WeakPtr<GlEdgemap> GlEdgemap_wp;

	class GlEdgemap : public Edgemap
	{
		friend class GlGfxDevice;
		friend class GlBackend;
	public:

		//.____ Creation __________________________________________
		
		static GlEdgemap_p	create( const Blueprint& blueprint );
		static GlEdgemap_p	create( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const float * pSamples, int edges, int edgePitch = 0, int samplePitch = 0);
		static GlEdgemap_p	create( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const spx * pSamples, int edges, int edgePitch = 0, int samplePitch = 0);


    	//.____ Identification __________________________________________

		const TypeInfo&     typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

	protected:

		GlEdgemap(const Blueprint& bp);
		~GlEdgemap();

		void	_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) override;
		void	_colorsUpdated(int beginSegment, int endSegment) override;

		GLuint		m_bufferId;
		GLuint		m_textureId;

		int			m_colorsOfs;			// Offset to colorStrips in buffer, measured in bytes.
   };



} // namespace wg
#endif // WG_GLEDGEMAP_DOT_H
