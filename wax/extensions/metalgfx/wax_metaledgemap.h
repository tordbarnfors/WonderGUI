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
#ifndef WAX_METALEDGEMAP_DOT_H
#define WAX_METALEDGEMAP_DOT_H
#pragma once

#import "MetalKit/MetalKit.h"

#include <wax_gpuedgemap.h>


namespace wax
{

	class MetalEdgemap;
	typedef StrongPtr<MetalEdgemap> MetalEdgemap_p;
	typedef WeakPtr<MetalEdgemap> MetalEdgemap_wp;

	//____ MetalEdgemap ________________________________________________________
	/**
	 * @brief Edgemap for MetalBackend.
	 *
	 * GpuEdgemap fills in the buffer the shaders read, this class only creates it, as a shared
	 * buffer the CPU writes to directly.
	 */

	class MetalEdgemap : public GpuEdgemap
	{
		friend class MetalBackend;
	public:

		//.____ Creation __________________________________________

		static MetalEdgemap_p	create( const Blueprint& blueprint );
		static MetalEdgemap_p	create( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const float * pSamples, int edges, int edgePitch = 0, int samplePitch = 0);
		static MetalEdgemap_p	create( const Edgemap::Blueprint& blueprint, SampleOrigo origo, const spx * pSamples, int edges, int edgePitch = 0, int samplePitch = 0);


		//.____ Identification __________________________________________

		const TypeInfo&     typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;


	protected:

		MetalEdgemap(const Blueprint& bp);
		~MetalEdgemap();

		float *	_allocBuffer( size_t bytes ) override;
		void	_commitBuffer() override;

		id<MTLBuffer>   m_buffer = nil;			// What the shaders read.
		id<MTLBuffer>   m_newBuffer = nil;		// Made by _allocBuffer(), until _commitBuffer().
	};



} // namespace wax
#endif // WAX_METALEDGEMAP_DOT_H
