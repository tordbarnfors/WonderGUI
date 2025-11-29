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
#ifndef	WG2_PANEL_DOT_H
#define	WG2_PANEL_DOT_H

#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_CONTAINER_DOT_H
#	include <wg2_container.h>
#endif

#include <wg_skin.h>


class WgPanelHook : public WgHook
{
public:
	WgPanelHook *	Prev() const { return static_cast<WgPanelHook*>(_prevHook()); }
	WgPanelHook *	Next() const { return static_cast<WgPanelHook*>(_nextHook()); }
	WgPanel*		Parent() const;

	virtual bool	SetVisible( bool bVisible );
	bool			IsVisible() { return m_bVisible; }

	virtual bool	SetPadding( WgBorders padding );
	WgBorders		Padding() const { return m_padding; }

protected:
	WgPanelHook() : m_bVisible(true) {}
	 virtual ~WgPanelHook() {};

	 WgWidgetHolder* _holder() const override { return _parent(); }

	WgSize		_paddedPreferredPixelSize( int scale ) const;
	WgSize		_paddedMinPixelSize( int scale ) const;
	WgSize		_paddedMaxPixelSize( int scale ) const;
	int			_paddedMatchingPixelWidth( int paddedHeight, int scale ) const;
	int			_paddedMatchingPixelHeight( int paddedWidth, int scale ) const;

	WgSize		_sizeFromPolicy( WgSize specifiedSize, WgSizePolicy widthPolicy, WgSizePolicy heightPolicy, int scale ) const;

	bool		_isDisplayed() const override;

	
	bool			m_bVisible;
	WgBorders		m_padding;
};


class WgPanel : public WgContainer
{
public:
	void		SetFocusGroup( bool bFocusGroup ) { m_bFocusGroup = bFocusGroup; }
	bool		IsFocusGroup() const { return m_bFocusGroup; }

	void		SetRadioGroup( bool bRadioGroup ) { m_bRadioGroup = bRadioGroup; }
	bool		IsRadioGroup() const { return m_bRadioGroup; }

	void		SetTooltipGroup( bool bTooltipGroup ) { m_bTooltipGroup = bTooltipGroup; }
	bool		IsTooltipGroup() const { return m_bTooltipGroup; }

	void		SetMaskOp( WgMaskOp operation );
	WgMaskOp	MaskOp() const { return m_maskOp; }

	inline WgPanelHook *	FirstHook() const { return static_cast<WgPanelHook*>(_firstHook()); }
	inline WgPanelHook *	LastHook() const { return static_cast<WgPanelHook*>(_lastHook()); }

	// Overloaded from WgWidgetHolder

	bool				IsPanel() const;

	WgPanel *			CastToPanel();
	const WgPanel *		CastToPanel() const;

protected:
	WgPanel();
	virtual ~WgPanel() {};

	virtual void	_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode );
	virtual void	_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip );
	virtual void	_onCloneContent( const WgPanel * _pOrg );


	bool		m_bFocusGroup;
	bool		m_bRadioGroup;
	bool		m_bTooltipGroup;	// All descendants belongs to the same tooltip group.
	WgMaskOp	m_maskOp;			// Specifies how container masks background.
};



#endif //WG2_PANEL_DOT_H
