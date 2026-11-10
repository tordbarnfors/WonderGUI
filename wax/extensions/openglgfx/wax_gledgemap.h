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
#ifndef WAX_GLEDGEMAP_DOT_H
#define WAX_GLEDGEMAP_DOT_H
#pragma once

#include <wax_gpuedgemap.h>
#include <vector>

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

	class GlEdgemap;
	typedef StrongPtr<GlEdgemap> GlEdgemap_p;
	typedef WeakPtr<GlEdgemap> GlEdgemap_wp;

	//____ GlEdgemap __________________________________________________________
	/**
	 * @brief Edgemap for GlBackend.
	 *
	 * GpuEdgemap fills in what the shaders read in memory of ours, which is uploaded to a buffer
	 * texture when a draw that reads it is rendered, see _upload().
	 */

	class GlEdgemap : public GpuEdgemap
	{
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

		float *	_allocBuffer( size_t bytes ) override;
		void	_commitBuffer() override;

		void	_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) override;
		void	_colorsUpdated(int beginSegment, int endSegment) override;

		void	_upload();				// Uploads our content to the buffer if it has changed.

		GLuint				m_bufferId = 0;
		GLuint				m_textureId = 0;		// Buffer texture of float4 reading m_bufferId.
		std::vector<float>	m_content;				// What GpuEdgemap fills in.
		std::vector<float>	m_newContent;			// Made by _allocBuffer(), until _commitBuffer().
		bool				m_bChanged = true;		// m_content has changed since uploaded.
	};

} // namespace wax
#endif // WAX_GLEDGEMAP_DOT_H
