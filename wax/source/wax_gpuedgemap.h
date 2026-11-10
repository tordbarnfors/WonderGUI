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
#ifndef WAX_GPUEDGEMAP_DOT_H
#define WAX_GPUEDGEMAP_DOT_H
#pragma once

#include <wax_edgemap.h>

#include <cstddef>

namespace wax
{

	class GpuEdgemap;
	typedef StrongPtr<GpuEdgemap> GpuEdgemap_p;
	typedef WeakPtr<GpuEdgemap> GpuEdgemap_wp;

	//____ GpuEdgemap __________________________________________________________
	/**
	 * @brief Base class of the edgemaps of GpuBackends.
	 *
	 * Everything the segments and warp shaders read of an edgemap is in one buffer of float4
	 * entries, which this class fills in. Subclasses only create the buffer, see _allocBuffer().
	 *
	 *   Edge strips, one per pixel column and edge: where the edge begins in pixels, how much
	 *   coverage it gains per pixel down the column, and what the first and last pixel it
	 *   crosses add.
	 *   Raw samples, the left and right side of each column and edge in pixels, for warps.
	 *   Flat colors, one per segment.
	 *   Tint table, one per segment. x is the offset of the segment's tint block, -1 for a
	 *   flat colored segment.
	 *   Tint blocks, a slot per segment big enough for any tint. Only there once a segment has
	 *   had a tint, most edgemaps never do.
	 *
	 * DX12Edgemap and VulkanEdgemap are built on it.
	 */

	class GpuEdgemap : public Edgemap
	{
		friend class GpuBackend;

	public:

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

	protected:

		GpuEdgemap(const Blueprint& bp);
		~GpuEdgemap();

		// Subclasses call this when constructed, and when they have got a new buffer for the
		// content they had, for example on a new device. Fills it in from what Edgemap keeps in
		// memory. Returns false if there is no buffer.

		bool	_createContent();

		// Creates a buffer of the given size, mapped for writing, without letting go of the one
		// we have, which may be read from while the new one is filled in. Returns null on failure.

		virtual float *	_allocBuffer( size_t bytes ) = 0;

		// Lets go of the buffer we had, for the one _allocBuffer() made. It may still be in use by
		// the GPU, which the subclass has to wait for or keep it around until it isn't.

		virtual void	_commitBuffer() = 0;

		void	_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) override;
		void	_colorsUpdated(int beginColor, int endColor) override;

		// Where the content is in the buffer, in float4 entries.

		int		_rawSamplesOfs() const { return m_samplesSize / 2; }
		int		_flatColorsOfs() const { return m_samplesSize; }
		int		_tintTableOfs() const { return m_samplesSize + m_nbSegments; }
		int		_tintSlotOfs(int segment) const { return m_samplesSize + m_nbSegments * 2 + segment * c_tintSlotSize; }

		bool	_replaceBuffer( bool bWithTintSlots );
		void	_writeColors( int beginSegment, int endSegment );

		static const int c_tintSlotSize;		// In float4 entries.

		float *	m_pGpuData = nullptr;			// Mapped content of the buffer, null if there is none.
		int		m_samplesSize = 0;				// Size of the edge strips and raw samples, in float4 entries.
		bool	m_bHasTintSlots = false;
	};

} // namespace wax
#endif // WAX_GPUEDGEMAP_DOT_H
