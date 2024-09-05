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
#ifndef WAVEFORM_DOT_H
#define WAVEFORM_DOT_H
#pragma once

#include <wg_edgemap.h>
#include <wg_edgemapfactory.h>
#include <wg_edgemaptools.h>
#include <wg_color.h>

namespace wg
{
	class Waveform;
	typedef	StrongPtr<Waveform>	Waveform_p;
	typedef	WeakPtr<Waveform>	Waveform_wp;

	class Waveform : public Object
	{
	public:
		
		struct Blueprint
		{
			spx					bottomOutlineThickness = 64;
			HiColor				color = Color8::LightGrey;
			Finalizer_p			finalizer = nullptr;
			Gradient			gradient;								// Overrides color when set.
			bool				optimize = true;						// Check for re-setting of same values to minimize updated regions.
			SampleOrigo			origo = SampleOrigo::Top;
			HiColor				outlineColor = Color8::DarkGrey;
			Gradient			outlineGradient;						// Overrides outlineColor when set.
			SizeI				size;									// Mandatory
			spx					topOutlineThickness = 64;
		};
		

		//.____ Creation __________________________________________

		static Waveform_p	create( const Blueprint& blueprint, EdgemapFactory * pFactory = nullptr );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________________
		
		bool		setSize( SizeI size );							// Destroys all samples. Will need to be set again.
		SizeI		size() const { return m_size; }
		
		//.____ Appearance ____________________________________________________

		bool		setColor( HiColor col );
		HiColor		color() const { return m_color; }
		
		bool		setGradient( const Gradient& gradient );
		void		clearGradient();
		Gradient	gradient() const { return m_gradient; }

		bool		setOutlineColor( HiColor col );
		HiColor		outlineColor() const { return m_outlineColor; }

		bool		setOutlineGradient( const Gradient& gradient );
		void		clearOutlineGradient();
		Gradient	outlineGradient() const { return m_outlineGradient; }

		//.____ Content __________________________________________________
		
		int			nbSamples() const { return m_nbSamples;  }
		
		void		setSamples( int sampleBegin, int sampleEnd, spx * pTopSamples, spx * pBottomSamples );
		void		setSamples( int sampleBegin, int sampleEnd, float * pTopSamples, float * pBottomSamples );

		void		setFlatTopLine( int sampleBegin, int sampleEnd, spx sample );
		void		setFlatTopLine( int sampleBegin, int sampleEnd, float sample );

		void		setFlatBottomLine( int sampleBegin, int sampleEnd, spx sample );
		void		setFlatBottomLine( int sampleBegin, int sampleEnd, float sample );

		Edgemap_p	refresh();
				
	protected:

		Waveform( const Blueprint& bp, EdgemapFactory * pFactory );
		virtual ~Waveform();

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
		int			_generateGradientPalette( Gradient * pDest );


		
		SizeI		m_size;
		
		SampleOrigo	m_origo;
		
		bool		m_bHasOutlines;
		
		HiColor		m_color = HiColor::Undefined;
		HiColor		m_outlineColor = HiColor::Undefined;

		Gradient	m_gradient;
		Gradient	m_outlineGradient;

		Brush		m_topBrush;
		Brush		m_bottomBrush;

		EdgemapFactory_p	m_pFactory;
		Edgemap_p	m_pEdgemap;

		
		int			m_nbSamples;
		spx *		m_pTopSamples;
		spx *		m_pBottomSamples;
		int			m_topSamplesPadding;		// Extra space before/after samples for top and bottom.
		int			m_bottomSamplesPadding;
		
		spx *		m_pSampleBuffer;
		
		int			m_dirtBegin;
		int			m_dirtEnd;
		
		bool		m_bOptimizeDirtRange = true;
	};

}

#endif //WAVEFORM_DOT_H
