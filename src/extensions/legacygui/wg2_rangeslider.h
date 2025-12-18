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
#ifndef WG2_RANGESLIDER_DOT_H
#define WG2_RANGESLIDER_DOT_H
#pragma once


#include <wg2_widget.h>
#include <wg_skin.h>


//____ RangeSlider ____________________________________________________________
/**
	* @brief A simple slider widget.
	*
	*
	**/


class WgRangeSlider : public WgWidget
{
public:
	WgRangeSlider();
	virtual ~WgRangeSlider();


	virtual const char* Type(void) const override;
	static const char* GetClass();
	WgWidget* NewOfMyType() const override { return new WgRangeSlider(); };

	//.____ Geometry ____________________________________________

	WgSize			PreferredPixelSize() const override;

	void			SetPreferredSlideLength(int pointLength);
	inline int		PreferredSlideLength() const { return m_preferredSlideLength; }

	//.____ Appearance _____________________________________________

	void			SetSkin(wg::Skin * pSkin) override;

	void			SetBeginHandleSkin(wg::Skin * pSkin);
	wg::Skin_p		BeginHandleSkin() const { return m_pBeginHandleSkin; }

	void			SetEndHandleSkin(wg::Skin * pSkin);
	wg::Skin_p		EndHandleSkin() const { return m_pEndHandleSkin; }

	void			SetAxis(wg::Axis axis);
	wg::Axis		Axis() const { return m_axis; }


	//.____ Control _______________________________________________________

	void			SetSteps(int nbSteps );
	inline int		Steps() const { return m_nbSteps; }

	//.____ Content _______________________________________________________

	void			SetRange(float begin, float end);

	inline float	RangeBegin() const { return m_rangeBegin; }
	inline float	RangeEnd() const { return m_rangeEnd; }


protected:
	void	_onEvent(const WgEvent::Event* pEvent, WgEventHandler* pHandler) override;

	void		_onCloneContent( const WgWidget * _pOrg ) override;
	void		_onRender(wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window) override;
	bool		_onAlphaTest(const WgCoord& ofs) override;

	void		_updatePreferredSize();

	void		_setRange(float begin, float end);
	void		_setHandleState(wg::State state, bool isBeginHandle);
	WgRect		_handleGeo(const WgRect& widgetGeo, bool isbBeginHandle);


private:
	WgSize		m_preferredSize;
	int			m_preferredSlideLength;
	wg::Skin_p	m_pBeginHandleSkin;
	wg::Skin_p	m_pEndHandleSkin;
	float		m_rangeBegin = 0.f;
	float		m_rangeEnd = 1.f;
	float		m_minRange = 0.f;
	float		m_valueAtPress = 0.f;
	int			m_nbSteps = 0;
	wg::Axis		m_axis = wg::Axis::X;
	wg::State		m_beginHandleState;
	wg::State		m_endHandleState;
};

#endif //WG2_RANGESLIDER_DOT_H
