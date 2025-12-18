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
#include <wg_plugincapsule.h>
#include <wg_gfxdevice.h>
#include <wg_msg.h>
#include <wg_base.h>
#include <wg_msgrouter.h>

#include <algorithm>

namespace wg
{
	using namespace Util;

	const TypeInfo PluginCapsule::TYPEINFO = { "PluginCapsule", &Widget::TYPEINFO };

	//____ create() ___________________________________________________________

	PluginCapsule_p PluginCapsule::create()
	{
		return PluginCapsule_p(new PluginCapsule());
	}

	PluginCapsule_p PluginCapsule::create(const Blueprint& blueprint)
	{
		return PluginCapsule_p(new PluginCapsule(blueprint));
	}


	//____ constructor ________________________________________________________

	PluginCapsule::PluginCapsule()
	{
		m_keyPressRoute = Base::msgRouter()->addRoute( MsgType::KeyPress, [this](Msg* pMsg)
		{
			this->_receiveRoutedKeyMsg(pMsg);
		} );

		m_keyReleaseRoute = Base::msgRouter()->addRoute( MsgType::KeyRelease, [this](Msg* pMsg)
		{
			this->_receiveRoutedKeyMsg(pMsg);
		} );

		std::memset(&m_calls, 0, sizeof(m_calls));
	}

	//____ Destructor _________________________________________________________

	PluginCapsule::~PluginCapsule()
	{
		Base::msgRouter()->deleteRoute(m_keyPressRoute);
		Base::msgRouter()->deleteRoute(m_keyReleaseRoute);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ tooltip() __________________________________________________________

	String PluginCapsule::tooltip() const
	{
		//TODO: Implement!!!

		return String();
	}

	//____ pointerStyle() _____________________________________________________

	PointerStyle PluginCapsule::pointerStyle() const
	{
		if (m_pPluginRoot)
			return (PointerStyle)m_calls.pointerStyle(m_pPluginRoot);
		else
			return m_pointerStyle;
	}

	//____ _defaultSize() _____________________________________________________

	SizeSPX PluginCapsule::_defaultSize(int scale) const
	{
		if (m_pPluginRoot)
		{
			auto size = m_calls.defaultSize(m_pPluginRoot, scale);
			return { size.w, size.h };
		}

		return SizeSPX();
	}

	//____ _minSize() _________________________________________________________

	SizeSPX PluginCapsule::_minSize(int scale) const
	{
		if (m_pPluginRoot)
		{
			auto size = m_calls.minSize(m_pPluginRoot, scale);
			return { size.w, size.h };
		}
		return SizeSPX();
	}

	//____ _maxSize() _________________________________________________________

	SizeSPX PluginCapsule::_maxSize(int scale) const
	{
		if (m_pPluginRoot)
		{
			auto size = m_calls.maxSize(m_pPluginRoot, scale);
			return { size.w, size.h };
		}

		return SizeSPX(spx_max,spx_max);
	}

	//____ _matchingHeight() __________________________________________________

	spx PluginCapsule::_matchingHeight(spx width, int scale) const
	{
		if (m_pPluginRoot)
			return  m_calls.matchingHeight(m_pPluginRoot, width, scale);

		return 0;
	}

	//____ _matchingWidth() ___________________________________________________

	spx PluginCapsule::_matchingWidth(spx height, int scale) const
	{
		if (m_pPluginRoot)
			return  m_calls.matchingWidth(m_pPluginRoot, height, scale);

		return 0;
	}

	//____ _preRender() _______________________________________________________

	void PluginCapsule::_preRender()
	{
		if (m_pPluginRoot)
			m_calls.preRender(m_pPluginRoot);
	}

	//____ _render() __________________________________________________________

	void PluginCapsule::_render(GfxDevice* pDevice, const RectSPX& canvas, const RectSPX& window)
	{
		if (m_pPluginRoot)
			m_calls.render(m_pPluginRoot, static_cast<Object*>(pDevice), {canvas.x,canvas.y,canvas.w,canvas.h}, { window.x,window.y,window.w,window.h });
	}

	//____ _resize() __________________________________________________________

	void PluginCapsule::_resize(const SizeSPX& size, int scale)
	{
		Widget::_resize(size, scale);

		if (m_pPluginRoot)
			m_calls.resize(m_pPluginRoot, {size.w,size.h}, scale);
	}

	//____ _setState() ________________________________________________________

	void PluginCapsule::_setState(State state)
	{
		if (m_pPluginRoot)
			m_calls.setState(m_pPluginRoot, * (wg_state*) &state);

		Widget::_setState(state);
	}

	//____ _receiveRoutedKeyMsg() ___________________________________________

	void PluginCapsule::_receiveRoutedKeyMsg(Msg* pMsg)
	{
		if(!m_pPluginRoot)
			return;

		if( !pMsg->hasSource() )
		{
			if( pMsg->type() == MsgType::KeyPress )
			{
				auto pMess = static_cast<KeyPressMsg*>(pMsg);
				m_calls.setKeyState(m_pPluginRoot, pMess->nativeKeyCode(), 1, pMess->timestamp() );
				return;
			}

			if( pMsg->type() == MsgType::KeyRelease )
			{
				auto pMess = static_cast<KeyReleaseMsg*>(pMsg);
				m_calls.setKeyState(m_pPluginRoot, pMess->nativeKeyCode(), 0, pMess->timestamp() );
				return;
			}
		}
	}


	//____ _receive() _________________________________________________________

	void PluginCapsule::_receive(Msg* pMsg)
	{

		if(!m_pPluginRoot)
		{
			Widget::_receive(pMsg);
			return;
		}

		switch( pMsg->type() )
		{
			case MsgType::MouseEnter:
			{
				m_bMouseInside = true;
				auto pMess = static_cast<InputMsg*>(pMsg);
				CoordSPX localPos = _toLocal( pMess->pointerSpxPos() );
				m_calls.setPointerPos(m_pPluginRoot, { localPos.x, localPos.y }, pMess->timestamp() );

				break;
			}

			case MsgType::MouseMove:
			{
				auto pMess = static_cast<InputMsg*>(pMsg);
				CoordSPX localPos = _toLocal( pMess->pointerSpxPos() );
				m_calls.setPointerPos(m_pPluginRoot, { localPos.x, localPos.y }, pMess->timestamp() );

				break;
			}

			case MsgType::MouseLeave:
			{
				m_bMouseInside = false;
				auto pMess = static_cast<InputMsg*>(pMsg);
				CoordSPX localPos = _toLocal( pMess->pointerSpxPos() );
				m_calls.setPointerPos(m_pPluginRoot, { localPos.x, localPos.y }, pMess->timestamp() );

				break;
			}

			case MsgType::MouseDrag:
			{
				if( !m_bMouseInside )
				{
					auto pMess = static_cast<InputMsg*>(pMsg);
					CoordSPX localPos = _toLocal( pMess->pointerSpxPos() );
					m_calls.setPointerPos(m_pPluginRoot, { localPos.x, localPos.y }, pMess->timestamp() );
				}
				break;
			}


			case MsgType::MousePress:
			{
				auto pMess = static_cast<MousePressMsg*>(pMsg);
				m_calls.setButtonState(m_pPluginRoot, (int) pMess->button(), 1, pMess->timestamp() );
				break;
			}

			case MsgType::MouseRelease:
			{
				auto pMess = static_cast<MouseReleaseMsg*>(pMsg);
				m_calls.setButtonState(m_pPluginRoot, (int) pMess->button(), 0, pMess->timestamp() );
				break;
			}

			case MsgType::KeyPress:
			{
				auto pMess = static_cast<KeyPressMsg*>(pMsg);
				m_calls.setKeyState(m_pPluginRoot, pMess->nativeKeyCode(), 1, pMess->timestamp() );
				break;
			}

			case MsgType::KeyRelease:
			{
				auto pMess = static_cast<KeyReleaseMsg*>(pMsg);
				m_calls.setKeyState(m_pPluginRoot, pMess->nativeKeyCode(), 0, pMess->timestamp() );
				break;
			}

			case MsgType::TextInput:
			{
				auto pMess = static_cast<TextInputMsg*>(pMsg);
				String str = pMess->text();

				int maxSize = str.length()*4;
				char * pDest = (char*) Base::memStackAlloc(maxSize);

				TextTool::getTextUTF8(str.chars(), pDest, maxSize);

				m_calls.putText(m_pPluginRoot,pDest);
				Base::memStackFree(maxSize);
				break;
			}

			case MsgType::WheelRoll:
			{
				auto pMess = static_cast<WheelRollMsg*>(pMsg);
				m_calls.wheelRoll(m_pPluginRoot, pMess->wheel(), pMess->distance(), pMess->invertScroll(), pMess->timestamp() );
				break;
			}

			default:
				break;
		}

		Widget::_receive( pMsg );
	}

	//____ _update() __________________________________________________________

	void PluginCapsule::_update(int microPassed, int64_t microsecTimestamp)
	{
		if(m_pPluginRoot)
			m_calls.onUpdate(m_pPluginRoot, microPassed, microsecTimestamp);
		else
			_stopReceiveUpdates();
	}


	//____ _alphaTest() _______________________________________________________

	bool PluginCapsule::_alphaTest(const CoordSPX& ofs)
	{
		return true;
/*
		if (m_pPluginRoot)
			return m_calls.alphaTest(m_pPluginRoot, { ofs.x,ofs.y } );

		return false;
*/
	}

	//____ _isPluginVisible() ___________________________________________________

	bool PluginCapsule::_isPluginVisible() const
	{
		return _holder()->_isChildVisible(_slot());
	}

	//____ _pluginWindowSection() _______________________________________________

	RectSPX PluginCapsule::_pluginWindowSection() const
	{
		auto rect = _windowSection();
		return { rect.x, rect.y, rect.w, rect.h };
	}

	//____ _pluginRequestRender() _______________________________________________

	void PluginCapsule::_pluginRequestRender(const RectSPX& rect)
	{
		_requestRender(rect);
	}

	//____ _pluginRequestResize() _______________________________________________

	void PluginCapsule::_pluginRequestResize()
	{
		_requestResize();
	}

	//____ _pluginRequestFocus() ________________________________________________

	bool PluginCapsule::_pluginRequestFocus(bool bRaiseWindow)
	{
		return grabFocus(bRaiseWindow);
	}

	//____ _pluginReleaseFocus() ________________________________________________

	bool PluginCapsule::_pluginReleaseFocus()
	{
		return releaseFocus();
	}

	//____ _pluginRequestInView() _______________________________________________

	void PluginCapsule::_pluginRequestInView(const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea)
	{
		_requestInView(mustHaveArea, niceToHaveArea);
	}

	//____ _connectPlugin() _____________________________________________________

	void PluginCapsule::_connectPlugin(void * pPluginRoot, wg_pluginroot_calls* pCalls)
	{
		m_pPluginRoot = pPluginRoot;
		m_calls = *pCalls;
		_startReceiveUpdates();

		if(pPluginRoot)
			m_calls.resize(pPluginRoot, {m_size.w,m_size.h}, m_scale);
	}

	//___ _disconnectPlugin() ____________________________________________________

	void PluginCapsule::_disconnectPlugin()
	{
		std::memset(&m_calls, 0, sizeof(m_calls));
		m_pPluginRoot = nullptr;
	}




} // namespace wg
