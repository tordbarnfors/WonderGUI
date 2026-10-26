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
#ifndef WG2_CAPSULE_DOT_H
#define WG2_CAPSULE_DOT_H

#ifndef WG2_CONTAINER_DOT_H
#	include <wg2_container.h>
#endif


class WgCapsule : public WgContainer
{
public:
	virtual ~WgCapsule() {}

	virtual WgHook * SetChild( WgWidget * pWidget );
	WgWidget *		Child() const { return m_hook.Widget(); }
	bool			DeleteChild();
	WgWidget *		ReleaseChild();

	bool			DeleteChild( WgWidget * pWidget );
	WgWidget *		ReleaseChild( WgWidget * pWidget );

	bool			DeleteAllChildren();
	bool			ReleaseAllChildren();


	bool			IsCapsule() const;
	WgCapsule *		CastToCapsule();
	const WgCapsule *	CastToCapsule() const;

	// Overloaded from WgWidget

	int				MatchingPixelHeight( int width ) const;
	int				MatchingPixelWidth( int height ) const;

	WgSize			PreferredPixelSize() const;

protected:
	WgCapsule();

	class CapsuleHook : public WgHook
	{
		friend class WgCapsule;
		friend class WgZoomOutCapsule;
        friend class WgRenderLayerCapsule;

	public:

		const char *Type( void ) const;
		static const char * ClassType();

		// Standard Hook methods

		WgCoord			PixelPos() const;
		WgSize			PixelSize() const;
		WgRect			PixelGeo() const;

		WgCoord			ScreenPixelPos() const;
		WgRect			ScreenPixelGeo() const;

		WgCapsule* 		Parent() const { return m_pParent; }

	protected:
		void			_requestRender() { m_pParent->_onRenderRequested(); }
		void			_requestRender( const WgRect& rect ) { m_pParent->_onRenderRequested(rect); }
		void			_requestResize() { m_pParent->_onResizeRequested(); }

		WgHook *		_prevHook() const { return 0; }
		WgHook *		_nextHook() const { return 0; }
		WgContainer *	_parent() const { return m_pParent; }
		WgWidgetHolder*	_holder() const { return m_pParent; }

		WgCapsule * 	m_pParent;

	};

	WgHook *		_firstHookWithGeo( WgRect& geo ) const;
	WgHook *		_nextHookWithGeo( WgRect& geo, WgHook * pHook ) const;

	WgHook *		_lastHookWithGeo( WgRect& geo ) const;
	WgHook *		_prevHookWithGeo( WgRect& geo, WgHook * pHook ) const;

	//

	virtual WgCoord _childPos();
	virtual WgSize  _childSize();
	virtual WgRect  _childGeo();


	void			_onCollectPatches( WgPatches& container, const WgRect& geo, const WgRect& clip );
	void			_onMaskPatches( WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode );
	void			_onCloneContent( const WgWidget * _pOrg );
	void			_onNewSize( const WgSize& size );

	virtual void	_onRenderRequested();
	virtual void	_onRenderRequested(const WgRect& rect);
	virtual void	_onResizeRequested();

	virtual void	_childAddedRemovedOrReplaced();


	WgHook*			_firstHook() const;
	WgHook*			_lastHook() const;

	CapsuleHook		m_hook;

};

#endif //WG2_CAPSULE_DOT_H
