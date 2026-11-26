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
#ifndef WAX_SDLGPUEDGEMAP_DOT_H
#define WAX_SDLGPUEDGEMAP_DOT_H
#pragma once

#include <wax_gpuedgemap.h>
#include <wax_sdlgpucontext.h>

#include <vector>

namespace wax
{

	class SdlGpuEdgemap;
	typedef StrongPtr<SdlGpuEdgemap> SdlGpuEdgemap_p;
	typedef WeakPtr<SdlGpuEdgemap> SdlGpuEdgemap_wp;

	//____ SdlGpuEdgemap ________________________________________________________
	/**
	 * @brief Edgemap for SdlGpuBackend.
	 *
	 * Its content is laid out by GpuEdgemap in memory, as for GlEdgemap, since SDL_GPU has no
	 * buffers the CPU can write to where the GPU reads them. It is uploaded whole to a storage
	 * buffer when a session that draws it ends, if it has changed, which is when the content
	 * should be read, see the Wax design. The upload cycles the buffer, so sessions still in
	 * flight keep reading what they had.
	 *
	 * Needs the device set by SdlGpuBackend::setDevice().
	 */

	class SdlGpuEdgemap : public GpuEdgemap
	{
		friend class SdlGpuBackend;

	public:

		//.____ Creation __________________________________________

		static SdlGpuEdgemap_p	create(const Blueprint& blueprint);
		static SdlGpuEdgemap_p	create(const Blueprint& blueprint, SampleOrigin origin, const float* pSamples, int edges, int edgePitch = 0, int samplePitch = 0);
		static SdlGpuEdgemap_p	create(const Blueprint& blueprint, SampleOrigin origin, const spx* pSamples, int edges, int edgePitch = 0, int samplePitch = 0);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

	protected:

		SdlGpuEdgemap(const Blueprint& bp);
		~SdlGpuEdgemap();

		float *		_allocBuffer( size_t bytes ) override;
		void		_commitBuffer() override;

		void		_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) override;
		void		_colorsUpdated(int beginSegment, int endSegment) override;

		// For SdlGpuBackend: has the content uploaded, if changed, and the buffer the shaders read.

		void			_recordUpload( SdlGpuContext::BufferUploads& uploads );
		SDL_GPUBuffer *	_buffer() const { return m_pBuffer; }

		std::vector<float>	m_content;				// What GpuEdgemap fills in.
		std::vector<float>	m_newContent;			// Between _allocBuffer() and _commitBuffer().
		bool				m_bChanged = true;		// m_content has changed since uploaded.

		SDL_GPUBuffer *		m_pBuffer = nullptr;	// The size of m_content.
	};

} // namespace wax
#endif // WAX_SDLGPUEDGEMAP_DOT_H
