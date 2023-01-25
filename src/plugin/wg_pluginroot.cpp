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

#include <wg_pluginbase.h>
#include <wg_pluginroot.h>
#include <wg_plugininterface.h>
#include <wg_plugingfxdevice.h>
#include <wg_container.h>
#include <wg_util.h>
#include <wg_base.h>
#include <wg_inputhandler.h>

#include <wg_c_pluginroot.h>


namespace wg
{
	using namespace Util;

	const TypeInfo PluginRoot::TYPEINFO = { "PluginRoot", &Widget::TYPEINFO };

	//____ create() _______________________________________________________________

	PluginRoot_p PluginRoot::create()
	{
		return PluginRoot_p(new PluginRoot());
	}

	PluginRoot_p PluginRoot::create(wg_obj myPluginCapsule)
	{
		return PluginRoot_p(new PluginRoot(myPluginCapsule));
	}


	//____ constructor ____________________________________________________________

	PluginRoot::PluginRoot() : slot(this)
	{
		m_pluginCapsule = 0;
		
		m_interface.structSize		= sizeof(wg_pluginroot_calls);
		
		m_interface.matchingHeight	= wg_matchingPluginHeight;
		m_interface.matchingWidth	= wg_matchingPluginWidth;
		m_interface.defaultSize		= wg_defaultPluginSize;
		m_interface.minSize			= wg_minPluginSize;
		m_interface.maxSize			= wg_maxPluginSize;
		m_interface.markTest		= wg_markTestPlugin;
		m_interface.preRender		= wg_preRenderPlugin;
		m_interface.render			= wg_renderPlugin;
		m_interface.resize			= wg_resizePlugin;
		m_interface.setState		= wg_setPluginState;
		m_interface.receive			= wg_pluginReceive;
		m_interface.pointerStyle	= wg_pluginPointerStyle;
		m_interface.setPointerPos	= wg_setPluginPointerPos;
		m_interface.setButtonState	= wg_setPluginButtonState;
		m_interface.setKeyState		= wg_setPluginKeyState;
		m_interface.putText			= wg_putPluginText;

		m_interface.onUpdate		= wg_onPluginUpdate;
	}

	PluginRoot::PluginRoot(wg_obj myPluginCapsule) : PluginRoot()
	{
		m_pluginCapsule = myPluginCapsule;

		PluginBase::pluginCapsule->connect(m_pluginCapsule, static_cast<Object*>(this), &m_interface);
	}

	//____ Destructor _____________________________________________________________

	PluginRoot::~PluginRoot()
	{
		if (m_pluginCapsule)
			PluginBase::pluginCapsule->disconnect(m_pluginCapsule);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginRoot::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//___ findWidget() ________________________________________________________

	Widget_p PluginRoot::findWidget(const Coord& _ofs, SearchMode mode)
	{
		CoordSPX ofs = Util::ptsToSpx(_ofs, m_scale);


		if (!(m_size.contains(ofs) || !slot._widget()))
			return nullptr;

		if (slot._widget() && slot._widget()->isContainer())
			return static_cast<Container*>(slot._widget())->_findWidget(ofs, mode);

		if (mode == SearchMode::Geometry || slot._widget()->_markTest(ofs))
			return slot._widget();

		return nullptr;
	}

	//____ addDirtyPatch() ____________________________________________________

	void PluginRoot::addDirtyPatch(const RectSPX& rect)
	{
		PluginBase::pluginCapsule->requestRender(m_pluginCapsule, { rect.x,rect.y,rect.w,rect.h });
	}

	//____ addPreRenderCall() _________________________________________________

	bool PluginRoot::addPreRenderCall(Widget* pWidget)
	{
		bool canPreRender;

		if (m_preRenderCalls.empty())
			canPreRender = PluginBase::pluginCapsule->requestPreRenderCall(m_pluginCapsule);
		else
			canPreRender = true;

		if (canPreRender)
			m_preRenderCalls.push_back(pWidget);

		return canPreRender;
	}


	//____ scale() ____________________________________________________________

	int PluginRoot::scale() const
	{
		return m_scale;
	}

	//____ geo() ______________________________________________________________

	Rect PluginRoot::geo() const
	{
		return Util::spxToPts(m_size,m_scale);
	}

	//____ _matchingHeight() __________________________________________________

	spx PluginRoot::_matchingHeight(spx width, int scale) const
	{
		if (slot.isEmpty())
			return 0;
		else
			return slot.widget()->_matchingHeight(width, scale);
	}

	//____ _matchingWidth() ___________________________________________________

	spx PluginRoot::_matchingWidth(spx height, int scale) const
	{
		if (slot.isEmpty())
			return 0;
		else
			return slot.widget()->_matchingWidth(height, scale);
	}

	//____ _defaultSize() _____________________________________________________

	SizeSPX PluginRoot::_defaultSize(int scale) const
	{
		if (slot.isEmpty())
			return SizeSPX();
		else
			return slot.widget()->_defaultSize(scale);
	}

	//____ _minSize() _________________________________________________________

	SizeSPX PluginRoot::_minSize(int scale) const
	{
		if (slot.isEmpty())
			return SizeSPX();
		else
			return slot.widget()->_minSize(scale);
	}

	//____ _maxSize() _________________________________________________________

	SizeSPX PluginRoot::_maxSize(int scale) const
	{
		if (slot.isEmpty())
			return SizeSPX();
		else
			return slot.widget()->_maxSize(scale);
	}

	//____ _markTest() ________________________________________________________

	bool PluginRoot::_markTest(const CoordSPX& ofs)
	{
		//TODO: Support skin with padding.

		return true;
	}

	//____ _preRender() _______________________________________________________

	void PluginRoot::_preRender()
	{
		for (auto& pWidget : m_preRenderCalls)
			pWidget->_preRender();

		m_preRenderCalls.clear();
	}

	//____ _render() __________________________________________________________

	void PluginRoot::_render(wg_obj device, const RectSPX& _canvas, const RectSPX& _window)
	{
		if (slot.isEmpty())
			return;

		wg_obj hFactory = PluginBase::gfxDevice->surfaceFactory(device);

		auto pFactory = PluginSurfaceFactory::create(hFactory);
		auto pDevice = PluginGfxDevice::create(device, pFactory);

		slot._widget()->_render(pDevice, { _canvas.x, _canvas.y, _canvas.w, _canvas.h }, { _window.x, _window.y, _window.w, _window.h });
	}


	//____ _resize() __________________________________________________________

	void PluginRoot::_resize(const SizeSPX& size, int scale)
	{
		m_size = size;

		if (scale >= 0)
			m_scale = scale;

		if (!slot.isEmpty())
			slot._widget()->_resize(size, scale);
	}

	//____ _setState() ________________________________________________________

	void PluginRoot::_setState(State state)
	{
		State oldState = m_state;
		m_state = state;

		if (!slot.isEmpty())
		{
			if (oldState.isEnabled() != state.isEnabled())
				slot._widget()->setEnabled(state.isEnabled());

			if (oldState.isSelected() != state.isSelected())
			{
				bool bSelected = state.isSelected();

				State s = slot._widget()->state();
				s.setSelected(bSelected);
				slot._widget()->_setState(s);
			}
		}

		if( state.isFocused() != oldState.isFocused() )
		{
			if( m_pFocusedChild )
			{
				State s = m_pFocusedChild->state();
				s.setFocused(!s.isFocused());
				m_pFocusedChild->_setState(s);
				
				if( !state.isFocused() )
					m_pFocusedChild = nullptr;
			}
			
			Base::inputHandler()->setFocusedWindow(state.isFocused()?this:nullptr);
			
		}

	}

	//____ _receive() _________________________________________________________

	void PluginRoot::_receive(Msg* pMsg)
	{
		//TODO: Implement!!!
	}

	//____ _pointerStyle() ____________________________________________________

	PointerStyle PluginRoot::_pointerStyle() const
	{
		//TODO: Implement!!!

		return PointerStyle::Default;
	}

	//____ _setPointerPos() ___________________________________________________

	void PluginRoot::_setPointerPos( CoordSPX pos, int64_t timestamp )
	{
		Base::inputHandler()->setPointer(this, Util::spxToPts(pos, m_scale), timestamp );
	}

	//____ _setButtonState() __________________________________________________

	void PluginRoot::_setButtonState( int button, bool bPressed, int64_t timestamp )
	{
		Base::inputHandler()->setButton((MouseButton) button, bPressed, timestamp );
	}

	//____ _setKeyState() __________________________________________________

	void PluginRoot::_setKeyState( int nativeKeyCode, bool bPressed, int64_t timestamp )
	{
		Base::inputHandler()->setKey(nativeKeyCode, bPressed, timestamp );
	}

	//____ _putText() ____________________________________________________________

	void PluginRoot::_putText( const char * pUTF8Text )
	{
		Base::inputHandler()->putText(pUTF8Text);
	}

	//____ _update() __________________________________________________________

	void PluginRoot::_update(int microPassed, int64_t microsecTimestamp)
	{
		Base::msgRouter()->dispatch();
		Base::update(microsecTimestamp);
	}

	//____ _slotTypeInfo() ____________________________________________________

	const TypeInfo& PluginRoot::_slotTypeInfo(const StaticSlot* pSlot) const
	{
		return DynamicSlot::TYPEINFO;
	}

	//____ _container() _______________________________________________________

	Container* PluginRoot::_container()
	{
		return nullptr;
	}

	//____ _root() ____________________________________________________________

	Root* PluginRoot::_root()
	{
		return this;
	}

	//____ _scale() ___________________________________________________________

	int PluginRoot::_scale() const
	{
		return m_scale;
	}

	//____ _child() ___________________________________________________________
	
	Widget*	PluginRoot::_child() const
	{
		return slot._widget();
	}

	//____ _focusedChild() ____________________________________________________

	Widget* PluginRoot::_focusedChild() const
	{
		return m_pFocusedChild.rawPtr();
	}


	//____ _childPos() ________________________________________________________

	CoordSPX PluginRoot::_childPos(const StaticSlot* pSlot) const
	{
		return CoordSPX();
	}

	//____ _childGlobalPos() __________________________________________________

	CoordSPX PluginRoot::_childGlobalPos(const StaticSlot* pSlot) const
	{
		return CoordSPX();
	}

	//____ _isChildVisible() __________________________________________________

	bool PluginRoot::_isChildVisible(const StaticSlot* pSlot) const
	{
		return true;
	}

	//____ _childWindowSection() ______________________________________________

	RectSPX PluginRoot::_childWindowSection(const StaticSlot* pSlot) const
	{
		return m_size;
	}

	//____ _childRequestRender() ______________________________________________

	void PluginRoot::_childRequestRender(StaticSlot* pSlot)
	{
		if (m_pluginCapsule)
			PluginBase::pluginCapsule->requestRender(m_pluginCapsule, { 0,0,m_size.w,m_size.h });
	}

	void PluginRoot::_childRequestRender(StaticSlot* pSlot, const RectSPX& rect)
	{
		if (m_pluginCapsule)
			PluginBase::pluginCapsule->requestRender(m_pluginCapsule, { rect.x,rect.y,rect.w,rect.h });
	}

	//____ _childRequestResize() ______________________________________________
	void PluginRoot::_childRequestResize(StaticSlot* pSlot)
	{
		if (m_pluginCapsule)
			PluginBase::pluginCapsule->requestResize(m_pluginCapsule);
	}

	//____ () ___________________________________________________
	bool PluginRoot::_childRequestFocus(StaticSlot* pSlot, Widget* pWidget)
	{
		if (!m_pluginCapsule)
			return false;

		if( m_pFocusedChild )
		{
			State s = m_pFocusedChild->state();
			s.setFocused(false);
			m_pFocusedChild->_setState(s);

			m_pFocusedChild = pWidget;
			
			s = m_pFocusedChild->state();
			s.setFocused(true);
			m_pFocusedChild->_setState(s);
		}
		else
		{
			bool result = (bool) PluginBase::pluginCapsule->requestFocus(m_pluginCapsule);
			if( result )
				m_pFocusedChild = pWidget;
			
			return result;
		}
	}

	//____ () ___________________________________________________
	bool PluginRoot::_childReleaseFocus(StaticSlot* pSlot, Widget* pWidget)
	{
		if (!m_pluginCapsule)
			return false;

		bool result = (bool) PluginBase::pluginCapsule->releaseFocus(m_pluginCapsule);
		
		return result;
	}

	//____ _childRequestInView() ___________________________________________________

	void PluginRoot::_childRequestInView(StaticSlot* pSlot)
	{
		if (m_pluginCapsule)
			PluginBase::pluginCapsule->requestInView(m_pluginCapsule, { 0,0,m_size.w,m_size.h }, { 0,0,m_size.w,m_size.h } );
	}

	void PluginRoot::_childRequestInView(StaticSlot* pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea)
	{
		if (m_pluginCapsule)
			PluginBase::pluginCapsule->requestInView(m_pluginCapsule, { mustHaveArea.x,mustHaveArea.y,mustHaveArea.w,mustHaveArea.h },
													{ niceToHaveArea.x, niceToHaveArea.y, niceToHaveArea.h, niceToHaveArea.w });
	}

	//____ _prevChild() _______________________________________________________

	Widget* PluginRoot::_prevChild(const StaticSlot* pSlot) const
	{
		return nullptr;
	}

	//____ _nextChild() _______________________________________________________

	Widget* PluginRoot::_nextChild(const StaticSlot* pSlot) const
	{
		return nullptr;
	}

	//____ _releaseChild() ____________________________________________________

	void PluginRoot::_releaseChild(StaticSlot* pSlot)
	{
		slot._setWidget(nullptr);

		if (m_pluginCapsule)
		{
			PluginBase::pluginCapsule->requestRender(m_pluginCapsule, { 0,0, m_size.w, m_size.h });
			PluginBase::pluginCapsule->requestResize(m_pluginCapsule);
		}
	}

	//____ _replaceChild() ____________________________________________________

	void PluginRoot::_replaceChild(StaticSlot* pSlot, Widget* pWidget)
	{
		slot._setWidget(pWidget);

		if (pWidget)
			pWidget->_resize(m_size);

		if (m_pluginCapsule)
		{
			PluginBase::pluginCapsule->requestRender(m_pluginCapsule, { 0,0, m_size.w, m_size.h });
			PluginBase::pluginCapsule->requestResize(m_pluginCapsule);
		}
	}

	//____ _didAddSlots() _____________________________________________________

	void PluginRoot::_didAddSlots(StaticSlot* pSlot, int nb)
	{
		// Nothing to do.
	}

	//____ _didMoveSlots() ____________________________________________________

	void PluginRoot::_didMoveSlots(StaticSlot* pFrom, StaticSlot* pTo, int nb)
	{
		// Nothing to do.
	}

	//____ _willEraseSlots() __________________________________________________

	void PluginRoot::_willEraseSlots(StaticSlot* pSlot, int nb)
	{
		// Nothing to do.
	}

}
