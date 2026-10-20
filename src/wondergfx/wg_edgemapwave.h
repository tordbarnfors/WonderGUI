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
#ifndef WG_EDGEMAPWAVE_DOT_H
#define WG_EDGEMAPWAVE_DOT_H
#pragma once

#include <wg_edgemapshape.h>
#include <wg_color.h>

namespace wg
{
	class EdgemapWave;
	typedef	StrongPtr<EdgemapWave>	EdgemapWave_p;
	typedef	WeakPtr<EdgemapWave>	EdgemapWave_wp;

	//____ EdgemapWave ________________________________________________________
	/**
	 * @brief A wave, like the area of a chart, made into an edgemap.
	 *
	 * The wave is filled between a top and a bottom line, given as samples at each pixel
	 * along x, and the lines can have outlines. It is drawn as one edgemap, flipped as
	 * set, which lets it run along y instead or in any direction.
	 *
	 * Samples are set as needed and the edgemap updated by refresh(), only where samples
	 * have changed.
	 */

	class EdgemapWave : public EdgemapShape
	{
	public:

		struct Blueprint
		{
			spx					bottomOutlineThickness = 64;
			HiColor				color = Color8::LightGrey;
			EdgemapFactory_p	factory;								// Default edgemap factory of GfxBase if not set.
			Finalizer_p			finalizer = nullptr;
			GfxFlip				flip = GfxFlip::None;					// Flip of the wave, which runs along x before it.
			bool				optimize = true;						// Check for re-setting of same values to minimize updated regions.
			SampleOrigo			origo = SampleOrigo::Top;
			HiColor				outlineColor = Color8::DarkGrey;
			Tint_p				outlineTint;							// Overrides outlineColor when set.
			CoordSPX			pos;									// Top-left corner on canvas, after flip. Rounded to whole pixels.
			SizeSPX				size;									// Mandatory. Size before flip. Rounded to whole pixels.
			Tint_p				tint;									// Overrides color when set. Placed in the rect of the wave.
			spx					topOutlineThickness = 64;
		};

		//.____ Creation __________________________________________

		static EdgemapWave_p	create( const Blueprint& blueprint );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________________

		bool		setSize( SizeSPX size );						// Destroys all samples. Will need to be set again.
		SizeSPX		size() const { return m_size * 64; }

		void		setPos( CoordSPX pos );
		CoordSPX	pos() const { return m_pieces[0].pos; }

		void		setFlip( GfxFlip flip );
		GfxFlip		flip() const { return m_pieces[0].flip; }

		//.____ Appearance ____________________________________________________

		bool		setColor( HiColor col );
		HiColor		color() const { return m_color; }

		void		setTint( Tint * pTint );
		void		clearTint() { setTint(nullptr); }
		Tint_p		tint() const { return m_pTint; }

		bool		setOutlineColor( HiColor col );
		HiColor		outlineColor() const { return m_outlineColor; }

		void		setOutlineTint( Tint * pTint );
		void		clearOutlineTint() { setOutlineTint(nullptr); }
		Tint_p		outlineTint() const { return m_pOutlineTint; }

		//.____ Content __________________________________________________

		int			nbSamples() const { return m_nbSamples;  }

		void		setSamples( int sampleBegin, int sampleEnd, spx * pTopSamples, spx * pBottomSamples );
		void		setSamples( int sampleBegin, int sampleEnd, float * pTopSamples, float * pBottomSamples );

		void		setFlatTopLine( int sampleBegin, int sampleEnd, spx sample );
		void		setFlatTopLine( int sampleBegin, int sampleEnd, float sample );

		void		setFlatBottomLine( int sampleBegin, int sampleEnd, spx sample );
		void		setFlatBottomLine( int sampleBegin, int sampleEnd, float sample );

		void		refresh();											// Updates the edgemap with samples set since last refresh.

		Edgemap_p	edgemap() const { return m_pieces[0].pEdgemap; }

	protected:

		EdgemapWave( const Blueprint& bp );
		virtual ~EdgemapWave();

		struct Brush
		{
			spx		thickness;
			spx		influenceSlope[32*4+4];			// Max 64 pixels wide brush with 4x oversampling + room for 4 zeroes.
		};

		void 		_setFlatLine( spx * pSamples, int sampleBegin, int sampleEnd, spx sample );
		void 		_drawLine(spx * pSrc, spx * pDestTop, spx * pDestBottom, int nPoints, Brush& brush);
		void 		_generateInfluenceSlope( Brush& brush );

		void 		_updateEdgemapColors();
		void 		_regenEdgemap();


		void		_optimizeRangeSetSamples( spx * pDestBegin, spx * pDestEnd, const spx * pSource, spx * pSampleStart );
		void		_optimizeRangeSetSamples( spx * pDestBegin, spx * pDestEnd, const float * pSource, spx * pSampleStart );

		int			_generateColorPalette( HiColor * pDest );
		int			_generateTintPalette( Tint_p * pDest );

		void		_setupBuffer(int nbSamples, bool bClearSamples);


		uint8_t *	m_pBuffer = nullptr;

		SizeI		m_size;

		SampleOrigo	m_origo;

		bool		m_bHasOutlines;

		HiColor		m_color = HiColor::Undefined;
		HiColor		m_outlineColor = HiColor::Undefined;

		Tint_p		m_pTint;
		Tint_p		m_pOutlineTint;

		Brush		m_topBrush;
		Brush		m_bottomBrush;

		EdgemapFactory_p	m_pFactory;


		int			m_nbSamples;
		spx *		m_pTopSamples;
		spx *		m_pBottomSamples;
		int			m_topSamplesPadding;		// Extra space before/after samples for top and bottom.
		int			m_bottomSamplesPadding;


		int			m_dirtBegin;
		int			m_dirtEnd;

		bool		m_bOptimizeDirtRange = true;
	};

}

#endif //WG_EDGEMAPWAVE_DOT_H
