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

#include <wg_widget.h>
#include <wg_staticnumber.h>

namespace wg
{

	const TypeInfo	StaticNumber::TYPEINFO = { "StaticNumber", &Component::TYPEINFO };

	//____ constructor ________________________________________________________

	StaticNumber::StaticNumber(Widget * pWidget) : Component(pWidget)
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& StaticNumber::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setLayout() _____________________________________________________

	void StaticNumber::setLayout(NumberLayout* pLayout)
	{
		if (m_pLayout.rawPtr() != pLayout)
		{
			m_pLayout = pLayout;
			_refresh();
		}
	}

	//____ clearLayout() ___________________________________________________

	void StaticNumber::clearLayout()
	{
		if (m_pLayout)
		{
			m_pLayout = 0;
			_refresh();
		}
	}

	//____ _set() _____________________________________________________________

	bool StaticNumber::_set( double value )
	{
		if( m_value != value )
		{
			m_value = value;
			_refresh();
			return true;
		}
		return false;
	}

	//____ _render() __________________________________________________________

	void StaticNumber::_render(GfxDevice* pDevice, const RectSPX& canvas, int scale, State state)
	{
		NumberLayout* pLayout = m_pLayout ? m_pLayout.rawPtr() : Base::defaultNumberLayout().rawPtr();

		pLayout->render(pDevice, canvas, m_value, scale, state);

	}


	//____ _defaultSize() ___________________________________________________

	SizeSPX StaticNumber::_defaultSize(int scale) const
	{
		NumberLayout* pLayout = m_pLayout ? m_pLayout.rawPtr() : Base::defaultNumberLayout().rawPtr();

		return pLayout->defaultSize(m_value, scale, State::Default);
	}


	//____ _refresh() _________________________________________________________

	void StaticNumber::_refresh()
	{
		NumberLayout* pLayout = m_pLayout ? m_pLayout.rawPtr() : Base::defaultNumberLayout().rawPtr();

//		if (m_pLayout)
//			m_charBuffer = pLayout->format(m_value, m_scale);

		_requestRender();

		//TODO: Conditional call to _requestResize();
	}

	//____ _stateChangeNeedsRender() __________________________________________

	bool StaticNumber::_stateChangeNeedsRender(State newState, State oldState) const
	{
		NumberLayout* pLayout = m_pLayout ? m_pLayout.rawPtr() : Base::defaultNumberLayout().rawPtr();

		return pLayout->stateChangeNeedsRender(newState, oldState);
	}

	//____ _initFromBlueprint() _______________________________________________

	void StaticNumber::_initFromBlueprint(const Blueprint& blueprint)
	{
		m_pLayout = blueprint.layout;
	}


} // namespace wg
