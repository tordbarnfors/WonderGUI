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
#include <wg_legacyrootcapsule.h>
#include <wg_gfxdevice.h>
#include <wg_msg.h>
#include <wg_msgrouter.h>
#include <wg_base.h>

#include <wg2_eventhandler.h>
#include <wg2_event.h>


#include <algorithm>

namespace wg
{
	using namespace Util;

	const TypeInfo LegacyRootCapsule::TYPEINFO = { "LegacyRootCapsule", &Widget::TYPEINFO };

	static const char	c_hookType[] = {"LegacyRootHook"};


	//____ create() ___________________________________________________________

	LegacyRootCapsule_p LegacyRootCapsule::create()
	{
		return LegacyRootCapsule_p(new LegacyRootCapsule());
	}

	LegacyRootCapsule_p LegacyRootCapsule::create(const Blueprint& blueprint)
	{
		return LegacyRootCapsule_p(new LegacyRootCapsule(blueprint));
	}


	//____ constructor ________________________________________________________

	LegacyRootCapsule::LegacyRootCapsule() : m_hook(this)
	{
		m_pEventHandler = new WgEventHandler(this);

		_routeKeyEvents();

		_startReceiveUpdates();
	}

	//____ _routeKeyEvents _____________________________________________________

	void LegacyRootCapsule::_routeKeyEvents()
	{
		m_keyPressRoute = Base::msgRouter()->addRoute(MsgType::KeyPress, [this](Msg* pMsg)
									{ this->_receive(pMsg); });

		m_keyReleaseRoute = Base::msgRouter()->addRoute(MsgType::KeyRelease, [this](Msg* pMsg)
									{ this->_receive(pMsg); });
	}

	//____ Destructor _________________________________________________________

	LegacyRootCapsule::~LegacyRootCapsule()
	{
		_stopReceiveUpdates();

		Base::msgRouter()->deleteRoute(m_keyPressRoute);
		Base::msgRouter()->deleteRoute(m_keyReleaseRoute);

		delete m_pEventHandler;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& LegacyRootCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ tooltip() __________________________________________________________

	String LegacyRootCapsule::tooltip() const
	{
		//TODO: Implement!!!

		return String();
	}

	//____ pointerStyle() _____________________________________________________

	PointerStyle LegacyRootCapsule::pointerStyle() const
	{
		return m_pEventHandler->PointerStyle();
	}

	//____ _defaultSize() _____________________________________________________

	SizeSPX LegacyRootCapsule::_defaultSize(int scale) const
	{
		//NOTE! This does not take change of scale into account!

		if( m_hook.Widget() )
			return m_hook.Widget()->PreferredPixelSize()*64;
		else
			return {0,0};
	}

	//____ _minSize() _________________________________________________________

	SizeSPX LegacyRootCapsule::_minSize(int scale) const
	{
		//NOTE! This does not take change of scale into account!

		if( m_hook.Widget() )
			return m_hook.Widget()->MinPixelSize()*64;
		else
			return {0,0};
	}

	//____ _maxSize() _________________________________________________________

	SizeSPX LegacyRootCapsule::_maxSize(int scale) const
	{
		//NOTE! This does not take change of scale into account!

		if( m_hook.Widget() )
			return m_hook.Widget()->MaxPixelSize()*64;
		else
			return { spx_max, spx_max };
	}

	//____ _matchingHeight() __________________________________________________

	spx LegacyRootCapsule::_matchingHeight(spx width, int scale) const
	{
		//NOTE! This does not take change of scale into account!

		if( m_hook.Widget() )
			return m_hook.Widget()->MatchingPixelHeight(width/64)*64;
		else
			return 0;
	}

	//____ _matchingWidth() ___________________________________________________

	spx LegacyRootCapsule::_matchingWidth(spx height, int scale) const
	{
		//NOTE! This does not take change of scale into account!

		if( m_hook.Widget() )
			return m_hook.Widget()->MatchingPixelWidth(height/64)*64;
		else
			return 0;

	}

	//____ _preRender() _______________________________________________________

	void LegacyRootCapsule::_preRender()
	{
		for (auto& pWidget : m_preRenderCalls)
		{
			if( pWidget )
				pWidget->_preRender();
		}

		m_preRenderCalls.clear();
	}

	//____ _render() __________________________________________________________

	void LegacyRootCapsule::_render(GfxDevice* pDevice, const RectSPX& canvas, const RectSPX& window)
	{
		Widget::_render(pDevice, canvas, window);

		if (!m_hook.Widget())
			return;

		WgPatches patches;

		const RectSPX * pRects 	= pDevice->clipList();
		int nRects 				= pDevice->clipListSize();

		for( int i = 0 ; i < nRects ; i++ )
		{
			patches.push({pRects[i].x/64, pRects[i].y/64, pRects[i].w/64, pRects[i].h/64} );
		}


		m_hook.Widget()->_renderPatches(pDevice, canvas/64, window/64, &patches);
	}

	//____ _resize() __________________________________________________________

	void LegacyRootCapsule::_resize(const SizeSPX& size, int scale)
	{
		if( scale == -1 )
			scale = m_scale;

		auto oldSize = m_size;
		auto oldScale = m_scale;

		m_size = size;
		m_scale = scale;

		if( m_hook.Widget() )
		{
			m_bBlockRequestResize = true;			// _setScale() might issue requestResize() calls.

			if( scale != oldScale )
				m_hook.Widget()->_setScale(scale*WG_SCALE_BASE/64);

			if( size != oldSize )
				m_hook.Widget()->_onNewSize({size.w/64,size.h/64});

			m_bBlockRequestResize = false;
		}

	}

	//____ _setState() ________________________________________________________

	void LegacyRootCapsule::_setState(State state)
	{
		Widget::_setState(state);

		if( m_hook.Widget() )
		{
			m_hook.Widget()->_setState(state);

			if( m_state.isFocused() && !state.isFocused() )
				m_pEventHandler->SetKeyboardFocus(nullptr);
		}
	}

	//____ _receive() _________________________________________________________

	void LegacyRootCapsule::_receive(Msg* pMsg)
	{
		switch( pMsg->type() )
		{
			case MsgType::MouseEnter:
			{
				m_bMouseInside = true;
				auto pMess = static_cast<InputMsg*>(pMsg);
				CoordSPX localPos = _toLocal( pMess->pointerSpxPos() );
				m_pEventHandler->QueueEvent( new WgEvent::MouseMove( WgCoord( localPos.x/64, localPos.y/64 ) ) );
				break;
			}

			case MsgType::MouseMove:
			{
				auto pMess = static_cast<InputMsg*>(pMsg);
				CoordSPX localPos = _toLocal( pMess->pointerSpxPos() );
				m_pEventHandler->QueueEvent( new WgEvent::MouseMove( WgCoord( localPos.x/64, localPos.y/64 ) ) );
				break;
			}

			case MsgType::MouseLeave:
			{
				m_bMouseInside = false;
				auto pMess = static_cast<InputMsg*>(pMsg);
				CoordSPX localPos = _toLocal( pMess->pointerSpxPos() );
				m_pEventHandler->QueueEvent( new WgEvent::MouseMove( WgCoord( localPos.x/64, localPos.y/64 ) ) );
				break;
			}

			case MsgType::MouseDrag:
			{
				if( !m_bMouseInside )
				{
					auto pMess = static_cast<InputMsg*>(pMsg);
					CoordSPX localPos = _toLocal( pMess->pointerSpxPos() );
					m_pEventHandler->QueueEvent( new WgEvent::MouseMove( WgCoord( localPos.x/64, localPos.y/64 ) ) );
				}
				break;
			}

			case MsgType::MousePress:
			{
				auto pMess = static_cast<MousePressMsg*>(pMsg);
				int button;
				switch( pMess->button() )
				{
					case wg::MouseButton::Left:
						button = 1;
						break;
					case wg::MouseButton::Right:
						button = 2;
						break;
					default:
						button = 0;
						break;
				}

				if( button > 0 )
					m_pEventHandler->QueueEvent( new WgEvent::MouseButtonPress(button) );
				break;
			}

			case MsgType::MouseRelease:
			{
				auto pMess = static_cast<MouseReleaseMsg*>(pMsg);

				int button;
				switch( pMess->button() )
				{
					case wg::MouseButton::Left:
						button = 1;
						break;
					case wg::MouseButton::Right:
						button = 2;
						break;
					default:
						button = 0;
						break;
				}

				if( button > 0 )
					m_pEventHandler->QueueEvent( new WgEvent::MouseButtonRelease(button) );
				break;
			}

			case MsgType::KeyPress:
			{
				auto pMess = static_cast<KeyPressMsg*>(pMsg);
				m_pEventHandler->QueueEvent( new WgEvent::KeyPress( pMess->nativeKeyCode() ) );

				// WG2 wants return as text input.

				if( pMess->translatedKeyCode() == Key::Return )
					m_pEventHandler->QueueEvent( new WgEvent::Character( 13
																		) );

				break;
			}

			case MsgType::KeyRelease:
			{
				auto pMess = static_cast<KeyReleaseMsg*>(pMsg);
				m_pEventHandler->QueueEvent( new WgEvent::KeyRelease( pMess->nativeKeyCode() ) );
				break;
			}

			case MsgType::TextInput:
			{
				auto pMess = static_cast<TextInputMsg*>(pMsg);
				String str = pMess->text();

				int maxSize = str.length()*4;
				char * pDest = (char*) Base::memStackAlloc(maxSize);

				TextTool::getTextUTF8(str.chars(), pDest, maxSize);

				const char * p = pDest;

				while( * p != 0 )
				{
					int chr = wg::TextTool::readChar(p);
					m_pEventHandler->QueueEvent( new WgEvent::Character( chr ) );
				}

				Base::memStackFree(maxSize);
				break;
			}

			case MsgType::WheelRoll:
			{
				WheelRollMsg* pMess = static_cast<WheelRollMsg*>(pMsg);
				m_pEventHandler->QueueEvent( new WgEvent::MouseWheelRoll( pMess->wheel(), pMess->distance(), pMess->invertScroll()) );
				break;
			}

			default:
				break;
		}

		Widget::_receive( pMsg );
	}

	//____ _update() __________________________________________________________

	void LegacyRootCapsule::_update(int microPassed, int64_t microsecTimestamp)
	{
		int passed = microPassed + m_microsecStored;
		m_microsecStored = passed % 1000;
		m_pEventHandler->QueueEvent( new WgEvent::Tick( passed / 1000 ) );
		m_pEventHandler->ProcessEvents();
	}


	//____ _alphaTest() _______________________________________________________

	bool LegacyRootCapsule::_alphaTest(const CoordSPX& ofs)
	{
		return true;
/*
		if (m_pPluginRoot)
			return m_calls.alphaTest(m_pPluginRoot, { ofs.x,ofs.y } );

		return false;
*/
	}

	//____ _childRequestRender() _____________________________________________________

	void LegacyRootCapsule::_childRequestRender()
	{
		if (!m_bBlockRequestResize)
			_requestRender();
	}

	void LegacyRootCapsule::_childRequestRender( const WgRect& rect )
	{
		if (!m_bBlockRequestResize)
			_requestRender({ rect.x*64,rect.y*64,rect.w*64,rect.h*64 });
	}

	//____ _childRequestResize() _______________________________________________

	void LegacyRootCapsule::_childRequestResize()
	{
		if (!m_bBlockRequestResize)
			_requestResize();
	}

	//____ _focusRequested() ___________________________________________________

	bool LegacyRootCapsule::_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting )
	{
		if (!m_state.isFocused())
		{
			if (!grabFocus(true))
				return false;
		}
		return m_pEventHandler->SetKeyboardFocus(pWidgetRequesting);
	}

	//____ _focusReleased() ___________________________________________________

	bool LegacyRootCapsule::_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing )
	{
		bool result = (bool) releaseFocus();

		if( !result )
			return false;

		return m_pEventHandler->SetKeyboardFocus(nullptr);
	}

	//____ _inViewRequested() ___________________________________________________

	void LegacyRootCapsule::_inViewRequested( WgHook * pChild )
	{
		_requestInView({ 0,0,m_size.w,m_size.h },{ 0,0,m_size.w,m_size.h } );
	}

	void LegacyRootCapsule::_inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea )
	{
		_requestInView(	mustHaveArea*64, niceToHaveArea*64 );
	}

	//____ FindWidget() __________________________________________________________

	WgWidget * LegacyRootCapsule::FindWidget( const WgCoord& ofs, WgSearchMode mode )
	{
		if( !PixelGeo().contains(ofs) || !m_hook.Widget() )
			return 0;

		if( m_hook.Widget() && m_hook.Widget()->IsContainer() )
			return m_hook.Widget()->CastToContainer()->FindWidget( ofs, mode );

		return m_hook.Widget();
	}

	//____ DeleteChild() _________________________________________________________

	bool LegacyRootCapsule::DeleteChild( WgWidget * pWidget )
	{
		if( pWidget == m_hook.Widget() )
			return DeleteAllChildren();

		return false;
	}

	//____ ReleaseChild() ________________________________________________________

	WgWidget * LegacyRootCapsule::ReleaseChild( WgWidget * pWidget )
	{
		if( pWidget == m_hook.Widget() )
		{
			SetChild(0);
			return pWidget;
		}

		return nullptr;
	}

	//____ DeleteAllChildren() ___________________________________________________

	bool LegacyRootCapsule::DeleteAllChildren()
	{
		WgWidget * p = m_hook.Widget();
		SetChild(0);
		delete p;
		return true;
	}

	//____ ReleaseAllChildren() __________________________________________________

	bool LegacyRootCapsule::ReleaseAllChildren()
	{
		return SetChild(0);
	}

	//____ CastToRoot() __________________________________________________________

	WgRoot * LegacyRootCapsule::CastToRoot()
	{
		return this;
	}

	const WgRoot * LegacyRootCapsule::CastToRoot() const
	{
		return this;
	}

	//____ IsRoot() ______________________________________________________________

	bool LegacyRootCapsule::IsRoot() const
	{
		return true;
	}

	//____ SetChild() ____________________________________________________________

	bool LegacyRootCapsule::SetChild( WgWidget * pWidget )
	{
		if( !pWidget && m_hook.Widget() )
		{
			m_hook._releaseWidget();

			_requestRender();
			_requestResize();
			return false;
		}

		m_hook._attachWidget(pWidget);
		m_hook.Widget()->_onNewSize(m_size/64);
		_requestRender();

		if(pWidget->PreferredPixelSize()*64 != m_size)
			_requestResize();

		return true;
	}

//____ Hook::~Hook() _______________________________________________________

LegacyRootCapsule::Hook::~Hook()
{
}

//____ Hook::Type() ___________________________________________________________

const char * LegacyRootCapsule::Hook::Type( void ) const
{
	return ClassType();
}

//____ Hook::ClassType() ______________________________________________________

const char * LegacyRootCapsule::Hook::ClassType()
{
	return c_hookType;
}

//____ Hook::PixelPos() _______________________________________________________

WgCoord LegacyRootCapsule::Hook::PixelPos() const
{
	return {0,0};
}

//____ Hook::PixelSize() ______________________________________________________

WgSize LegacyRootCapsule::Hook::PixelSize() const
{
	return m_pRoot->CanvasSize();
}

//____ Hook::PixelGeo() _______________________________________________________

WgRect LegacyRootCapsule::Hook::PixelGeo() const
{
	return m_pRoot->CanvasSize();
}

//____ Hook::ScreenPixelPos() _________________________________________________

WgCoord LegacyRootCapsule::Hook::ScreenPixelPos() const
{
	return {0,0};
}

//____ Hook::ScreenPixelGeo() _________________________________________________

WgRect LegacyRootCapsule::Hook::ScreenPixelGeo() const
{
	return m_pRoot->CanvasSize();
}

//____ Hook::Root() ___________________________________________________________

WgRoot * LegacyRootCapsule::Hook::Root() const
{
	return m_pRoot;
}
//____ Hook::_requestRender() _________________________________________________

void LegacyRootCapsule::Hook::_requestRender()
{
	m_pRoot->_childRequestRender();
}

void LegacyRootCapsule::Hook::_requestRender( const WgRect& rect )
{
	m_pRoot->_childRequestRender(rect);
}


//____ Hook::_requestResize() _________________________________________________

void LegacyRootCapsule::Hook::_requestResize()
{
	m_pRoot->_childRequestResize();
}

//____ Hook::_requestFocus() _________________________________________________________

bool LegacyRootCapsule::Hook::_requestFocus()
{
	return m_pRoot->_focusRequested(this, m_pWidget);
}

//____ Hook::_releaseFocus() _________________________________________________________

bool LegacyRootCapsule::Hook::_releaseFocus()
{
	return m_pRoot->_focusReleased(this, m_pWidget);
}

//____ Hook::_requestInView() ________________________________________________________

void LegacyRootCapsule::Hook::_requestInView()
{
	m_pRoot->_inViewRequested(this);
}

void LegacyRootCapsule::Hook::_requestInView( const WgRect& mustHaveArea, const WgRect& niceToHaveArea )
{
	m_pRoot->_inViewRequested( this, mustHaveArea, niceToHaveArea );
}


//____ Hook::_prevHook() ______________________________________________________

WgHook * LegacyRootCapsule::Hook::_prevHook() const
{
	return nullptr;
}

//____ Hook::_nextHook() ______________________________________________________

WgHook * LegacyRootCapsule::Hook::_nextHook() const
{
	return nullptr;
}

//____ Hook::_holder() ________________________________________________________

WgWidgetHolder * LegacyRootCapsule::Hook::_holder() const
{
	return m_pRoot;
}

//____ Hook::_parent() ________________________________________________________

WgContainer * LegacyRootCapsule::Hook::_parent() const
{
	return nullptr;
}




} // namespace wg
