/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef WG_EDGEMAP_DOT_H
#define WG_EDGEMAP_DOT_H
#pragma once

#include <bitset>

#include <wg_object.h>
#include <wg_gfxtypes.h>
#include <wg_color.h>
#include <wg_geo.h>
#include <wg_gradient.h>
#include <wg_tintmap.h>

namespace wg
{

	class Edgemap;
	typedef StrongPtr<Edgemap> Edgemap_p;
	typedef WeakPtr<Edgemap> Edgemap_wp;

	class Edgemap : public Object
	{
	public:

		const static int maxSegments = 16;			// We keep this global for now. Might never need to restrict it more.
		
		struct Blueprint
		{
			const HiColor*		colors 		= nullptr;			// Edgemap has either colors, gradients, tintmaps or colorstrips, never more than one of them. Setting one is mandatory.
			Finalizer_p			finalizer	= nullptr;
			const Gradient *	gradients 	= nullptr;			// Edgemap has either colors, gradients, tintmaps or colorstrips, never more than one of them. Setting one is mandatory.
			const Tintmap_p * 	tintmaps	= nullptr;			// Needs to have one tintmap per segement if any. So size() must be 0 or equal to segments.
			int					segments	= 0;				// Mandatory.
			SizeI				size;							// Mandatory.

			const HiColor*		colorstripsX = nullptr;			// One color for each pixel along width for each segment.
			const HiColor*		colorstripsY = nullptr;			// One color for each pixel along height for each segment.
		};
	   	 
		
    	//.____ Identification __________________________________________

		const TypeInfo&     typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

		//.____ Geometry ______________________________________________________

		inline SizeI	pixelSize() const { return m_size; }

		//.____ Appearance ____________________________________________________

		virtual bool	setRenderSegments(int nSegments);
		inline int		renderSegments() const { return m_nbRenderSegments; }

		bool	setColors( int begin, int end, const HiColor * pColors );
		bool	setGradients(int begin, int end, const Gradient* pGradients);
		bool	setTintmaps( int begin, int end, const Tintmap_p * pTintmaps );



		
		//.____ Content _______________________________________________________

		inline int		segments() const { return m_nbSegments; }

		inline int		samples() const { return m_size.w + 1; }

		bool			hasHorizontalTint() const { return m_pColorstripsX; }
		bool			hasVerticalTint() const { return m_pColorstripsY; }

		bool 			importSamples( SampleOrigo origo, const spx * pSource, int edgeBegin, int edgeEnd,
									  int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0 );

		bool 			importSamples( SampleOrigo origo, const float * pSource, int edgeBegin, int edgeEnd,
									  int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0 );

		bool 			exportSamples( SampleOrigo origo, spx * pDestination, int edgeBegin, int edgeEnd,
									  int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0 );

		bool 			exportSamples( SampleOrigo origo, float * pDestination, int edgeBegin, int edgeEnd,
									  int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0 );


	protected:

		Edgemap(const Blueprint& bp);
		virtual ~Edgemap();
		
		static bool	_validateBlueprint(const Blueprint& bp);
	
		void 	_importSamples(SampleOrigo origo, const spx* pSource, int edgeBegin, int edgeEnd,
								int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);

		void 	_importSamples(SampleOrigo origo, const float* pSource, int edgeBegin, int edgeEnd,
								int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);


		virtual void	_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) = 0;
		virtual void	_colorsUpdated(int beginColor, int endColor) = 0;

		SizeI       m_size;
		int			m_nbSegments;
		int			m_nbRenderSegments;

		//
		

		char* m_pBuffer;
		spx* m_pSamples;						// Stored vertically, e.g. samples for first column for all edges before samples for second column etc

		HiColor*	m_pColorstripsX = nullptr;	// Horizontal colorstrips for segment 1, then segment 2 etc. or null if none.
		HiColor*	m_pColorstripsY = nullptr;	// Vertical colorstrips for segment 1, then segment 2 etc. or null if none.
		HiColor*	m_pFlatColors = nullptr;	// Flat colors for when we have no colorstrips.

		HiColor*	m_pPalette = nullptr;		// Pointer at our colors, no matter if they are colorstrips or flat.
		int			m_paletteSize = 0;			// Total number of colors.

		Gradient*	m_pGradients = nullptr;		// Pointer at gradients for our segments, for legacy purposes.
   };


} // namespace wg
#endif // WG_EDGEMAP_DOT_H
