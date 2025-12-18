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
#include	<stdio.h>
#include	<wg_fpsdisplay.h>
#include	<wg_texttool.h>
#include	<wg_gfxdevice.h>
#include	<wg_color.h>
#include	<wg_char.h>
#include	<wg_base.h>
#include	<wg_basictextlayout.h>
#include	<wg_msgrouter.h>

namespace wg
{

	const TypeInfo FpsDisplay::TYPEINFO = { "FpsDisplay", &Widget::TYPEINFO };



	//____ FpsDisplay() _________________________________________________________________

	FpsDisplay::FpsDisplay( void ) : labels(this), values(this)
	{
		m_pTickBuffer		= new int[c_tickBuffer];
		for( int i = 0 ; i < c_tickBuffer ; i++ )
			m_pTickBuffer[i] = 1;

		m_tickBufferOfs		= 0;

		labels.setText( "Now:\nMin:\nAvg:\nMax:" );


		// We set our own default text textMappers since we rely on text placement
		// to present the information in a readable way.

		labels.setLayout(BasicTextLayout::create({}));

		BasicTextLayout::Blueprint bp;
		bp.placement = Placement::NorthEast;

		BasicTextLayout_p pValueTextLayout = BasicTextLayout::create( bp );
		values.setLayout(pValueTextLayout);

		_startReceiveUpdates();
	}

	//____ ~FpsDisplay() __________________________________________________________

	FpsDisplay::~FpsDisplay( void )
	{
		if( m_pTickBuffer )
		{
			delete [] m_pTickBuffer;
			m_pTickBuffer = 0;
		}
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& FpsDisplay::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _defaultSize() __________________________________________________________

	SizeSPX FpsDisplay::_defaultSize(int scale) const
	{
		SizeSPX contentSize = labels._defaultSize(scale);

		TextAttr attr;
		values._style()->exportAttr( State(State::Default), &attr, scale );

		int prevSize = attr.pFont->size();
		attr.pFont->setSize(attr.size);
		contentSize.w += attr.pFont->maxAdvance() * 7;			// Reserve space for: ' 999.99' after longest label.
		attr.pFont->setSize(prevSize);

		int valueHeight = values._defaultSize(scale).h;
		if( valueHeight > contentSize.h )
			contentSize.h = valueHeight;

		return m_skin.sizeForContent(contentSize, scale);
	}

	//____ _render() ________________________________________________________

	void FpsDisplay::_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window )
	{
		Widget::_render(pDevice,_canvas,_window);

		RectSPX content = m_skin.contentRect( _canvas, m_scale, m_state );

		labels._render( pDevice, content );
		values._render( pDevice, content );
	}

	//____ _receive() _____________________________________________________________

	void FpsDisplay::_update(int microPassed, int64_t microsecTimestamp)
	{
		// Update tick buffer

		m_tickBufferOfs = (++m_tickBufferOfs) % c_tickBuffer;

		int msDiff = microPassed / 1000;
		if( msDiff > 0 )
			m_pTickBuffer[m_tickBufferOfs] = msDiff;
		else
			m_pTickBuffer[m_tickBufferOfs] = 1;

		// Update valueTexts from tick-buffer

		const int	cCurrentFrames = 10;
		int currOfs = ((int)m_tickBufferOfs) - cCurrentFrames;
		if( currOfs < 0 )
			currOfs += c_tickBuffer;

		int	currTotal = 0;
		for( int i = 0 ; i < cCurrentFrames ; i++ )
		{
			currTotal += m_pTickBuffer[currOfs++];
			currOfs %= c_tickBuffer;
		}
		float	fpsCurrent = 1000.f / (currTotal / (float) cCurrentFrames);

		//____

		int	avg = 0;
		for( int i = 0 ; i < c_tickBuffer ; i++ )
			avg += m_pTickBuffer[i];
		float fpsAvg = 1000.f / (((float)avg)/c_tickBuffer);

		//____

		int	min = 1000000000;
		for( int i = 0 ; i < c_tickBuffer ; i++ )
			if( min > m_pTickBuffer[i] )
				min = m_pTickBuffer[i];
		float fpsMax = 1000.f / min;

		//____

		int	max = 0;
		for( int i = 0 ; i < c_tickBuffer ; i++ )
			if( max < m_pTickBuffer[i] )
				max = m_pTickBuffer[i];
		float fpsMin = 1000.f / max;

		//____

		char	temp[100];
		snprintf( temp, 100, "%.2f\n%.2f\n%.2f\n%.2f", fpsCurrent, fpsMin, fpsAvg, fpsMax );
		values._setText(CharSeq(temp));

		_requestRender();
	}

	//____ _setState() ______________________________________________________

	void FpsDisplay::_setState( State state )
	{

		labels._setState(state);
		values._setState(state);
		_requestRender();							//TODO: Check if there has been changes to text appearance.

		if (!state.isDisabled() && m_state.isDisabled())
			_startReceiveUpdates();

		if (state.isDisabled() && !m_state.isDisabled())
			_stopReceiveUpdates();

		Widget::_setState(state);
	}

} // namespace wg
