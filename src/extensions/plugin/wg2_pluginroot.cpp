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
#include <wg_plugininterface.h>
#include <wg_plugingfxdevice.h>
#include <wg_plugincalls.h>

#include <wg2_pluginbase.h>
#include <wg2_pluginroot.h>
#include <wg2_eventhandler.h>
#include <wg2_c_pluginroot.h>

using namespace wg;

static const char	c_hookType[] = {"PluginRootHook"};



WgPluginRoot::WgPluginRoot( wg_obj myPluginCapsule, WgPluginContext * pContext )
{
	m_pContext = pContext;
	m_pluginCapsule = myPluginCapsule;
	m_pEventHandler = new WgEventHandler(this);
	m_hook.m_pRoot = this;

	m_interface.structSize		= sizeof(wg_pluginroot_calls);

	m_interface.matchingHeight	= wg2_matchingPluginHeight;
	m_interface.matchingWidth	= wg2_matchingPluginWidth;
	m_interface.defaultSize		= wg2_defaultPluginSize;
	m_interface.minSize			= wg2_minPluginSize;
	m_interface.maxSize			= wg2_maxPluginSize;
	m_interface.markTest		= wg2_markTestPlugin;
	m_interface.preRender		= wg2_preRenderPlugin;
	m_interface.render			= wg2_renderPlugin;
	m_interface.resize			= wg2_resizePlugin;
	m_interface.setState		= wg2_setPluginState;
	m_interface.receive			= wg2_pluginReceive;
	m_interface.pointerStyle	= wg2_pluginPointerStyle;
	m_interface.setPointerPos	= wg2_setPluginPointerPos;
	m_interface.setButtonState	= wg2_setPluginButtonState;
	m_interface.setKeyState		= wg2_setPluginKeyState;
	m_interface.putText			= wg2_putPluginText;
	m_interface.wheelRoll		= wg2_pluginWheelRoll;

	m_interface.onUpdate		= wg2_onPluginUpdate;


	PluginCalls::pluginCapsule->connect(m_pluginCapsule, static_cast<void *>(this), &m_interface);
}

WgPluginRoot::~WgPluginRoot()
{
	if (m_pluginCapsule)
		PluginCalls::pluginCapsule->disconnect(m_pluginCapsule);

	delete m_pEventHandler;
}

//____ FindWidget() ___________________________________________________________

WgWidget * WgPluginRoot::FindWidget( const WgCoord& ofs, WgSearchMode mode )
{
	if( !PixelGeo().contains(ofs) || !m_hook.Widget() )
		return 0;

	if( m_hook.Widget() && m_hook.Widget()->IsContainer() )
		return m_hook.Widget()->CastToContainer()->FindWidget( ofs, mode );

	return m_hook.Widget();
}

//____ DeleteChild() __________________________________________________________

bool WgPluginRoot::DeleteChild( WgWidget * pWidget )
{
	if( pWidget == m_hook.Widget() )
		return SetChild(0);

	return false;
}

//____ ReleaseChild() _________________________________________________________

WgWidget * WgPluginRoot::ReleaseChild()
{
	return m_hook._releaseWidget();
}

WgWidget * WgPluginRoot::ReleaseChild( WgWidget * pWidget )
{
	if( pWidget == m_hook.Widget() )
		return ReleaseChild();

	return nullptr;
}

//____ DeleteAllChildren() ____________________________________________________

bool WgPluginRoot::DeleteAllChildren()
{
	DeleteChild();
	return true;
}

//____ ReleaseAllChildren() ___________________________________________________

bool WgPluginRoot::ReleaseAllChildren()
{
	return ReleaseChild()==0?false:true;
}

//____ CastToRoot() ___________________________________________________________

WgRoot * WgPluginRoot::CastToRoot()
{
	return this;
}

const WgRoot * WgPluginRoot::CastToRoot() const
{
	return this;
}

//____ IsRoot() _______________________________________________________________

bool WgPluginRoot::IsRoot() const
{
	return true;
}

//____ SetChild() _____________________________________________________________

bool WgPluginRoot::SetChild( WgWidget * pWidget )
{
	if( !pWidget )
		return false;

	m_hook._attachWidget(pWidget);
	m_hook.Widget()->_onNewSize(m_size/64);

	if (m_pluginCapsule)
		PluginCalls::pluginCapsule->requestRender(m_pluginCapsule, { 0,0,m_size.w,m_size.h });

	return true;
}

//____ _matchingHeight() ______________________________________________________

spx WgPluginRoot::_matchingHeight(spx width, int scale) const
{
	//NOTE! This does not take change of scale into account!

	if( m_hook.Widget() )
		return m_hook.Widget()->MatchingPixelHeight(width/64)*64*scale/m_scale;	// This is incorrect but works when child is a WgFlexPanel and the best we can do without a lot of rewrite.
	else
		return 0;
}

//____ _matchingWidth() _______________________________________________________

spx WgPluginRoot::_matchingWidth(spx height, int scale) const
{
	//NOTE! This does not take change of scale into account!

	if( m_hook.Widget() )
		return m_hook.Widget()->MatchingPixelWidth(height/64)*64*scale/m_scale;	// This is incorrect but works when child is a WgFlexPanel and the best we can do without a lot of rewrite.
	else
		return 0;

}

//____ _defaultSize() _________________________________________________________

SizeSPX WgPluginRoot::_defaultSize(int scale) const
{
	//NOTE! This does not take change of scale into account!

	if( m_hook.Widget() )
		return m_hook.Widget()->PreferredPointSize()*64*scale/m_scale;
	else
		return {0,0};
}

//____ _minSize() _____________________________________________________________

SizeSPX WgPluginRoot::_minSize(int scale) const
{
	//NOTE! This does not take change of scale into account!

	if( m_hook.Widget() )
		return m_hook.Widget()->MinPixelSize()*64*scale/m_scale;
	else
		return {0,0};
}

//____ _maxSize() _____________________________________________________________

SizeSPX WgPluginRoot::_maxSize(int scale) const
{
	//NOTE! This does not take change of scale into account!

	if( m_hook.Widget() )
	{
		SizeI sz = m_hook.Widget()->MaxPixelSize();

		uint64_t w = uint64_t(sz.w)*64*scale/m_scale;
		uint64_t h = uint64_t(sz.h)*64*scale/m_scale;


		return { int(w), int(h) };
	}
	else
		return { spx_max, spx_max };
}

//____ _markTest() ____________________________________________________________

bool WgPluginRoot::_markTest(const CoordSPX& ofs)
{
	return true;
}

//____ _preRender() ___________________________________________________________

void WgPluginRoot::_preRender()
{
	WgPluginBase::setContext(m_pContext);

	for (auto& pWidget : m_preRenderCalls)
	{
		if( pWidget )
			pWidget->_preRender();
	}

	m_preRenderCalls.clear();
}

//____ _render() ______________________________________________________________

void WgPluginRoot::_render(wg_obj device, const RectSPX& _canvas, const RectSPX& _window)
{
	if (!m_hook.Widget())
		return;

	WgPluginBase::setContext(m_pContext);


	wg_obj hSurfFactory = PluginCalls::gfxDevice->surfaceFactory(device);
	wg_obj hWaveFactory = PluginCalls::gfxDevice->edgemapFactory(device);

	auto pSurfFactory = PluginSurfaceFactory::create(hSurfFactory);
	auto pWaveFactory = PluginEdgemapFactory::create(hWaveFactory);
	auto pDevice = PluginGfxDevice::create(device, pSurfFactory, pWaveFactory);

	WgPatches patches;


	const RectSPX * pRects 	= pDevice->clipList();
	int nRects 				= pDevice->clipListSize();

	for( int i = 0 ; i < nRects ; i++ )
	{
		patches.push({pRects[i].x/64, pRects[i].y/64, pRects[i].w/64, pRects[i].h/64} );
	}


	m_hook.Widget()->_renderPatches(pDevice, { _canvas.x/64, _canvas.y/64, _canvas.w/64, _canvas.h/64 }, { _window.x/64, _window.y/64, _window.w/64, _window.h/64 }, &patches);
}

//____ _resize() ______________________________________________________________

void WgPluginRoot::_resize(const SizeSPX& size, int scale)
{
	WgPluginBase::setContext(m_pContext);

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

//____ _setState() ____________________________________________________________

void WgPluginRoot::_setState(State state)
{
	if( m_hook.Widget() )
	{
		m_hook.Widget()->_setState(state);

		if( m_state.isFocused() && !state.isFocused() )
			m_pEventHandler->SetKeyboardFocus(nullptr);
	}

	m_state = state;
}

//____ _pointerStyle() ________________________________________________________

PointerStyle WgPluginRoot::_pointerStyle() const
{
	return m_pEventHandler->PointerStyle();
}

//____ _setPointerPos() _______________________________________________________

void WgPluginRoot::_setPointerPos( CoordSPX pos, int64_t timestamp )
{
	m_pEventHandler->QueueEvent( new WgEvent::MouseMove( WgCoord( pos.x/64, pos.y/64 ) ) );
}

//____ _setButtonState() ______________________________________________________

void WgPluginRoot::_setButtonState( int button, bool bPressed, int64_t timestamp )
{
	if( bPressed )
		m_pEventHandler->QueueEvent( new WgEvent::MouseButtonPress( button ) );
	else
		m_pEventHandler->QueueEvent( new WgEvent::MouseButtonRelease( button ) );
}

//____ _setKeyState() _________________________________________________________

void WgPluginRoot::_setKeyState( int nativeKeyCode, bool bPressed, int64_t timestamp )
{
	if( bPressed )
	{
		m_pEventHandler->QueueEvent( new WgEvent::KeyPress( nativeKeyCode ) );

		if( WgBase::TranslateKey(nativeKeyCode) == WG_KEY_RETURN )
			m_pEventHandler->QueueEvent( new WgEvent::Character( 13 ) );
	}
	else
		m_pEventHandler->QueueEvent( new WgEvent::KeyRelease( nativeKeyCode ) );
}

//____ _putText() ___________________________________________________

void WgPluginRoot::_putText( const char * pUTF8Text )
{
	const char * p = pUTF8Text;

	while( * p != 0 )
	{
		int chr = wg::TextTool::readChar(p);
		m_pEventHandler->QueueEvent( new WgEvent::Character( chr ) );
	}
}

//____ _wheelRoll() ____________________________________________________________

void WgPluginRoot::_wheelRoll( int wheel, float distance, bool bInvert, int64_t timestamp )
{
	m_pEventHandler->QueueEvent( new WgEvent::MouseWheelRoll( wheel, int(distance), bInvert ) );
}

//____ _update() ______________________________________________________________

void WgPluginRoot::_update(int microPassed, int64_t microsecTimestamp)
{
	WgPluginBase::setContext(m_pContext);

	int passed = microPassed + m_microsecStored;
	m_microsecStored = passed % 1000;
	m_pEventHandler->QueueEvent( new WgEvent::Tick( passed / 1000 ) );
	m_pEventHandler->ProcessEvents();
}

void WgPluginRoot::_addPreRenderCall(WgWidget * pWidget)
{
	m_preRenderCalls.push_back(pWidget);
	if (m_pluginCapsule)
		PluginCalls::pluginCapsule->requestPreRenderCall(m_pluginCapsule);
}

//____ _focusRequested() ___________________________________________________

bool WgPluginRoot::_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting )
{
	if (!m_pluginCapsule)
		return false;

	WgPluginBase::setContext(m_pContext);

	if( m_pEventHandler->KeyboardFocus() )
	{
		return m_pEventHandler->SetKeyboardFocus(pWidgetRequesting);
	}
	else
	{
		bool result = (bool) PluginCalls::pluginCapsule->requestFocus(m_pluginCapsule, true);

		if( !result )
			return false;

		return m_pEventHandler->SetKeyboardFocus(pWidgetRequesting);
	}
}

//____ _focusReleased() ___________________________________________________

bool WgPluginRoot::_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing )
{
	if (!m_pluginCapsule)
		return false;

	WgPluginBase::setContext(m_pContext);

	bool result = (bool) PluginCalls::pluginCapsule->releaseFocus(m_pluginCapsule);

	if( !result )
		return false;

	return m_pEventHandler->SetKeyboardFocus(nullptr);
}

//____ _inViewRequested() ___________________________________________________

void WgPluginRoot::_inViewRequested( WgHook * pChild )
{
	if (m_pluginCapsule)
		PluginCalls::pluginCapsule->requestInView(m_pluginCapsule, { 0,0,m_size.w,m_size.h },{ 0,0,m_size.w,m_size.h } );

}

void WgPluginRoot::_inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea )
{
	WgPluginBase::setContext(m_pContext);

	if (m_pluginCapsule)
		PluginCalls::pluginCapsule->requestInView(m_pluginCapsule,
							{ mustHaveArea.x*64,mustHaveArea.y*64,mustHaveArea.w*64,mustHaveArea.h*64 },
							{ niceToHaveArea.x*64, niceToHaveArea.y*64, niceToHaveArea.h*64, niceToHaveArea.w*64 } );
}

//____ _childRequestResize() _____________________________________________________

void WgPluginRoot::_childRequestResize()
{
	WgPluginBase::setContext(m_pContext);

	if (m_pluginCapsule && !m_bBlockRequestResize)
		PluginCalls::pluginCapsule->requestResize(m_pluginCapsule);
}

//____ _childRequestRender() _____________________________________________________

void WgPluginRoot::_childRequestRender()
{
	WgPluginBase::setContext(m_pContext);

	if (m_pluginCapsule && !m_bBlockRequestResize)
		PluginCalls::pluginCapsule->requestRender(m_pluginCapsule, { 0,0,m_size.w,m_size.h });
}

void WgPluginRoot::_childRequestRender( const WgRect& rect )
{
	WgPluginBase::setContext(m_pContext);

	if (m_pluginCapsule && !m_bBlockRequestResize)
		PluginCalls::pluginCapsule->requestRender(m_pluginCapsule, { rect.x*64,rect.y*64,rect.w*64,rect.h*64 });
}

//____ Hook::~Hook() _______________________________________________________

WgPluginRoot::Hook::~Hook()
{
}

//____ Hook::Type() ___________________________________________________________

const char * WgPluginRoot::Hook::Type( void ) const
{
	return ClassType();
}

//____ Hook::ClassType() ______________________________________________________

const char * WgPluginRoot::Hook::ClassType()
{
	return c_hookType;
}

//____ Hook::PixelPos() _______________________________________________________

WgCoord WgPluginRoot::Hook::PixelPos() const
{
	return {0,0};
}

//____ Hook::PixelSize() ______________________________________________________

WgSize WgPluginRoot::Hook::PixelSize() const
{
	return m_pRoot->CanvasSize();
}

//____ Hook::PixelGeo() _______________________________________________________

WgRect WgPluginRoot::Hook::PixelGeo() const
{
	return m_pRoot->CanvasSize();
}

//____ Hook::ScreenPixelPos() _________________________________________________

WgCoord WgPluginRoot::Hook::ScreenPixelPos() const
{
	return {0,0};
}

//____ Hook::ScreenPixelGeo() _________________________________________________

WgRect WgPluginRoot::Hook::ScreenPixelGeo() const
{
	return m_pRoot->CanvasSize();
}

//____ Hook::Root() ___________________________________________________________

WgRoot * WgPluginRoot::Hook::Root() const
{
	return m_pRoot;
}
//____ Hook::_requestRender() _________________________________________________

void WgPluginRoot::Hook::_requestRender()
{
	m_pRoot->_childRequestRender();
}

void WgPluginRoot::Hook::_requestRender( const WgRect& rect )
{
	m_pRoot->_childRequestRender(rect);
}


//____ Hook::_requestResize() _________________________________________________

void WgPluginRoot::Hook::_requestResize()
{
	m_pRoot->_childRequestResize();
}

//____ Hook::_prevHook() ______________________________________________________

WgHook * WgPluginRoot::Hook::_prevHook() const
{
	return nullptr;
}

//____ Hook::_nextHook() ______________________________________________________

WgHook * WgPluginRoot::Hook::_nextHook() const
{
	return nullptr;
}

//____ Hook::_holder() ________________________________________________________

WgWidgetHolder * WgPluginRoot::Hook::_holder() const
{
	return m_pRoot;
}

//____ Hook::_parent() ________________________________________________________

WgContainer * WgPluginRoot::Hook::_parent() const
{
	return nullptr;
}



