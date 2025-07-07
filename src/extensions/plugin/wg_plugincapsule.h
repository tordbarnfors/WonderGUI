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
#ifndef WG_PLUGINCAPSULE_DOT_H
#define WG_PLUGINCAPSULE_DOT_H
#pragma once

#include <wg_widget.h>
#include <wg_plugininterface.h>

#include <cstring>

//TODO: Need to support skin padding.

namespace wg
{

	class PluginCapsule;
	typedef	StrongPtr<PluginCapsule>	PluginCapsule_p;
	typedef	WeakPtr<PluginCapsule>		PluginCapsule_wp;

	//____ PluginCapsule ________________________________________________________
	/**
	*/

	class PluginCapsule : public Widget
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{

			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool 			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Arrow;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Creation __________________________________________

		static PluginCapsule_p	create( );
		static PluginCapsule_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance ____________________________________________________

		String			tooltip() const override;
		PointerStyle	pointerStyle() const override;

		//.____ Internal _________________________________________________

		spx			_matchingHeight(spx width, int scale = -1) const override;
		spx			_matchingWidth(spx height, int scale = -1) const override;

		SizeSPX		_defaultSize(int scale = -1) const override;
		SizeSPX		_minSize(int scale = -1) const override;
		SizeSPX		_maxSize(int scale = -1) const override;

		void    	_preRender() override;
		void		_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;

		void		_resize(const SizeSPX& size, int scale = -1) override;
		void		_setState(State state) override;

		void		_receive(Msg* pMsg) override;
		bool		_alphaTest(const CoordSPX& ofs) override;

		bool        _requestPreRenderCall() { return Widget::_requestPreRenderCall(); };

		bool		_isPluginVisible() const;
		RectSPX		_pluginWindowSection() const;		// Returns the window section within the childs canvas.
		
		void		_pluginRequestRender(const RectSPX& rect);
		void		_pluginRequestResize();

		bool		_pluginRequestFocus(bool bRaiseWindow = false);
		bool		_pluginReleaseFocus();

		void		_pluginRequestInView(const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea);

		void		_connectPlugin(void * pPluginRoot, wg_pluginroot_calls* pCalls);
		void		_disconnectPlugin();

		void		_receiveRoutedKeyMsg(Msg* pMsg);


	protected:
		PluginCapsule();

		template<class BP> PluginCapsule(const BP& bp) : Widget(bp)
		{
			std::memset(&m_calls, 0, sizeof(m_calls));
		}

		 ~PluginCapsule();

		void	_update(int microPassed, int64_t microsecTimestamp) override;

		void *		m_pPluginRoot = nullptr;
		
		wg_pluginroot_calls	m_calls;

		bool m_bMouseInside = false;

		RouteId		m_keyPressRoute;
		RouteId		m_keyReleaseRoute;		
	};


} // namespace wg
#endif //WG_PLUGINCAPSULE_DOT_H
