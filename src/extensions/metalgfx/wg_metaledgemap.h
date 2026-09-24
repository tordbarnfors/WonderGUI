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
#ifndef WG_METALEDGEMAP_DOT_H
#define WG_METALEDGEMAP_DOT_H
#pragma once

#import "MetalKit/MetalKit.h"

#include <wg_edgemap.h>


namespace wg
{

	class MetalEdgemap;
	typedef StrongPtr<MetalEdgemap> MetalEdgemap_p;
	typedef WeakPtr<MetalEdgemap> MetalEdgemap_wp;

	class MetalEdgemap : public Edgemap
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

		void	_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) override;
		void	_colorsUpdated(int beginColor, int endColor) override;

		// Buffer layout, in float4 entries:
		//
		//   Edge strips, one per pixel column.
		//   Flat colors, one per segment.
		//   Tint table, one per segment. X is the offset of the segment's tint
		//   block, -1 for a flat colored segment.
		//   Tint blocks, a slot per segment big enough for any tint. Only there
		//   once a segment has had a tint, most edgemaps never do.

		int		_flatColorsOfs() const { return m_samplesSize; }
		int		_tintTableOfs() const { return m_samplesSize + m_nbSegments; }
		int		_tintSlotOfs(int segment) const { return m_samplesSize + m_nbSegments * 2 + segment * c_tintSlotSize; }

		void	_createBuffer(bool bWithTintSlots);
		void	_writeColors(int beginSegment, int endSegment);

		static const int c_tintSlotSize;		// In float4 entries.

	protected:

		id<MTLBuffer>   m_bufferId = nil;
		float *      	m_pBuffer = nullptr;

		int			m_samplesSize = 0;			// Size of the edge strips, in float4 entries.
		bool		m_bHasTintSlots = false;

	};



} // namespace wg
#endif // WG_METALEDGEMAP_DOT_H
