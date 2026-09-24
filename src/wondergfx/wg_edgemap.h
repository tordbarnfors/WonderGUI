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
#ifndef WG_EDGEMAP_DOT_H
#define WG_EDGEMAP_DOT_H
#pragma once

#include <bitset>

#include <wg_object.h>
#include <wg_gfxtypes.h>
#include <wg_color.h>
#include <wg_geo.h>
#include <wg_tint.h>

namespace wg
{

	class Edgemap;
	typedef StrongPtr<Edgemap> Edgemap_p;
	typedef WeakPtr<Edgemap> Edgemap_wp;

	class Edgemap : public Object
	{
	public:

		const static int maxSegments = 16;			// We keep this global for now. Might never need to restrict it more.

		// Each segment is colored either by a flat color or by a Tint.
		//
		// A segment Tint is placed in the rectangle of the Edgemap itself, e.g. (0,0) -> (size.w, size.h)
		// before any flip or rotation, so it follows the Edgemap when drawn flipped or rotated.
		// Any Tint set on the GfxDevice is applied on top of that, in canvas space.
		//
		// In the blueprint, 'tints' take precedence over 'colors' for segments that have a
		// Tint. Segments without either are transparent.

		struct Blueprint
		{
			const HiColor*		colors 		= nullptr;			// Flat colors, one color for each segment.
			Finalizer_p			finalizer	= nullptr;
			int					segments	= 0;				// Mandatory.
			SizeI				size;							// Mandatory.
			const Tint_p * 		tints		= nullptr;			// One Tint per segment. Nullptr entries use colors.
		};


    	//.____ Identification __________________________________________

		const TypeInfo&     typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

		//.____ Geometry ______________________________________________________

		inline SizeI	pixelSize() const { return m_size; }

		//.____ Appearance ____________________________________________________

		virtual bool	setRenderSegments(int nSegments);
		inline int		renderSegments() const { return m_nbRenderSegments; }

		virtual bool	setColors( int begin, int end, const HiColor * pColors );
		virtual bool	setColors( int begin, int end, const Tint_p * pTints );

		inline const HiColor*	flatColors() const { return m_pFlatColors; }	// Flat color of each segment, for segments without Tint.
		inline const Tint_p*	tints() const { return m_pTints; }				// Tint of each segment, nullptr for flat colored segments.
		inline bool				hasTints() const { return m_nbTints > 0; }

		//.____ Content _______________________________________________________

		inline int		segments() const { return m_nbSegments; }

		inline int		samples() const { return m_size.w + 1; }

		bool 			importSamples( SampleOrigo origo, const spx * pSource, int edgeBegin, int edgeEnd,
									  int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0 );

		bool 			importSamples( SampleOrigo origo, const float * pSource, int edgeBegin, int edgeEnd,
									  int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0 );

		bool 			exportSamples( SampleOrigo origo, spx * pDestination, int edgeBegin, int edgeEnd,
									  int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0 );

		bool 			exportSamples( SampleOrigo origo, float * pDestination, int edgeBegin, int edgeEnd,
									  int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0 );

		virtual void	exportBounds( spx * pMinMaxOutput, int nSections, int sectionWidth,
									  int topEdge, int bottomEdge, int mapOffset = 0, int minMaxPitch = 2 );

	protected:

		Edgemap() {};
		Edgemap(const Blueprint& bp);
		virtual ~Edgemap();

		static bool	_validateBlueprint(const Blueprint& bp);

		virtual void 	_importSamples(SampleOrigo origo, const spx* pSource, int edgeBegin, int edgeEnd,
									   int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);

		virtual void 	_importSamples(SampleOrigo origo, const float* pSource, int edgeBegin, int edgeEnd,
									   int sampleBegin, int sampleEnd, int edgePitch, int samplePitch);

		// Typically these two methods is all that you need to override when subclassing.

		virtual void	_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) = 0;
		virtual void	_colorsUpdated(int beginSegment, int endSegment) = 0;		// Flat colors and/or tints of segments have changed.

		SizeI       m_size;
		int			m_nbSegments;
		int			m_nbRenderSegments;

		//


		char* m_pBuffer = nullptr;
		spx* m_pSamples = nullptr;						// Stored vertically, e.g. samples for first column for all edges before samples for second column etc

		HiColor*	m_pFlatColors = nullptr;	// Flat color for each segment.
		Tint_p*		m_pTints = nullptr;			// Tint for each segment, nullptr for flat.
		int			m_nbTints = 0;				// Number of segments with tints.

		bool		m_bConstructed = false;
};


} // namespace wg
#endif // WG_EDGEMAP_DOT_H
