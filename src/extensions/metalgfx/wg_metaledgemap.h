/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Metalware Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

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

		int		_whiteColorOfs() const { return m_whiteColorOfs/4; }
		int		_flatColorsOfs() const { return m_paletteOfs/4;  }
		int		_colorstripXOfs() const { return m_paletteOfs / 4 + int(m_pColorstripsX - m_pPalette); }
		int		_colorstripYOfs() const { return m_paletteOfs / 4 + int(m_pColorstripsY - m_pPalette); }


	protected:

		id<MTLBuffer>   m_bufferId = nil;
		float *      	m_pBuffer = nullptr;

		int			m_paletteOfs;			// Offset to palette in buffer, measured in floats.
		int			m_whiteColorOfs;		// Offset to a white, default color, measured in floats.

	};



} // namespace wg
#endif // WG_METALEDGEMAP_DOT_H
