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
#ifndef WG2_LAYER_DOT_H
#define WG2_LAYER_DOT_H

#ifndef WG2_CONTAINER_DOT_H
#	include <wg2_container.h>
#endif


class WgLayerHook : public WgHook
{
	friend class WgLayer;
public:
	WgCoord			PixelPos() const { return m_geo.pos(); }
	WgSize			PixelSize() const { 	return m_geo.size(); }
	WgRect			PixelGeo() const { return m_geo; }

	WgCoord			ScreenPixelPos() const;
	WgRect			ScreenPixelGeo() const;

	WgLayerHook *	Prev() const { return static_cast<WgLayerHook*>(_prevHook()); }
	WgLayerHook *	Next() const { return static_cast<WgLayerHook*>(_nextHook()); }
	WgLayer*		Parent() const;

protected:

	void		_requestRender();
	void		_requestRender( const WgRect& rect );
	WgWidgetHolder* _holder() const { return _parent(); }

	WgRect		m_geo;
};


class WgLayer : public WgContainer
{
	friend class WgLayerHook;

public:
	bool			IsLayer() const override;
	WgLayer *		CastToLayer() override;
	const WgLayer *	CastToLayer() const override;

	WgHook *		SetBase( WgWidget * pWidget );
	WgWidget *		Base();
	bool			DeleteBase();
	WgWidget *		ReleaseBase();


	inline WgLayerHook *	FirstHook() const { return static_cast<WgLayerHook*>(_firstHook()); }
	inline WgLayerHook *	LastHook() const { return static_cast<WgLayerHook*>(_lastHook()); }

	// Overloaded from WgWidget

	int				MatchingPixelHeight( int width ) const override;
	int				MatchingPixelWidth( int height ) const override;

	WgSize			PreferredPixelSize() const override;

protected:
	WgLayer();

	class BaseHook : public WgHook
	{
		friend class WgLayer;

	public:
		BaseHook() : WgHook() {};

		const char *Type( void ) const;
		static const char * ClassType();

		// Standard Hook methods

		WgCoord		PixelPos() const { return m_pParent->PixelPos(); }
		WgSize		PixelSize() const { 	return m_pParent->PixelSize(); }
		WgRect		PixelGeo() const { return m_pParent->PixelGeo(); }

		WgCoord		ScreenPixelPos() const { return m_pParent->ScreenPixelPos(); }
		WgRect		ScreenPixelGeo() const { return m_pParent->ScreenPixelGeo(); }

		WgLayer *		Parent() const { return m_pParent; }

	protected:
		void		_requestRender();
		void		_requestRender( const WgRect& rect );
		void		_requestResize();

		WgHook *	_prevHook() const { return 0; }
		WgHook *	_nextHook() const { return m_pParent->_firstLayerHook(); }
		WgContainer* _parent() const { return m_pParent; }
		WgWidgetHolder* _holder() const { return m_pParent; }

		WgLayer * 	m_pParent;
	};

	void			_onNewSize(const WgSize& size) override;
	virtual	void	_onRequestRender( const WgRect& rect, const WgLayerHook * pHook );	// rect is in our coordinate system.
	virtual WgLayerHook * _firstLayerHook() const = 0;
	virtual void	 _onBaseChanged();


	BaseHook		m_baseHook;
	WgSize			m_size;

};


#endif //WG2_LAYER_DOT_H
