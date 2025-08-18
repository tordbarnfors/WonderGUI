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
#ifndef WG_DEBUGCAPSULE_DOT_H
#define WG_DEBUGCAPSULE_DOT_H
#pragma once

#include <wg_capsule.h>
#include <wg_boxskin.h>
#include <wg_debugfrontend.h>


namespace wg
{
	class DebugFrontend;
	typedef	StrongPtr<DebugFrontend>	DebugFrontend_p;
	typedef	WeakPtr<DebugFrontend>		DebugFrontend_wp;


	class DebugCapsule;
	typedef	StrongPtr<DebugCapsule>	DebugCapsule_p;
	typedef	WeakPtr<DebugCapsule>		DebugCapsule_wp;

	//____ DebugCapsule ______________________________________________________

	class DebugCapsule : public Capsule
	{
	public:
		//____ Blueprint __________________________________________

		struct Blueprint
		{

			Object_p		baggage;
			Widget_p		child;
			bool			disabled		= false;
			bool			dropTarget		= false;
			Finalizer_p		finalizer		= nullptr;
			DebugFrontend_p	frontend;									// Mandatory!!!
			int				id				= 0;
			MarkPolicy		markPolicy		= MarkPolicy::AlphaTest;
			std::string		name;
			bool			pickable		= false;
			uint8_t			pickCategory	= 0;
			bool			pickHandle		= false;
			PointerStyle	pointer			= PointerStyle::Undefined;
			bool			selectable		= false;
			Skin_p			selectionSkin;
			Skin_p			skin;
			bool			stickyFocus		= false;
			bool			tabLock			= false;
			String			tooltip;
			bool			usePickHandles	= false;
		};


		//.____ Creation __________________________________________

//		static DebugCapsule_p	create();
		static DebugCapsule_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		const std::string& name() const { return m_name; }

		//.____ Appearance __________________________________________________________

		PointerStyle 	pointerStyle() const override;

		//.____ Internal ______________________________________________________

		void			_widgetSelected(Widget * pWidget);		// Called by DebugFrontend.
		void			_setSelectMode(bool bSelectMode);		// Called by DebugFrontend.


	protected:
//		DebugCapsule();

		template<class BP> DebugCapsule( const BP& bp ) : Capsule(bp)
		{
			m_pFrontend = bp.frontend;

			m_name = bp.name;

			 if (bp.child)
				 slot.setWidget(bp.child);

			if( bp.selectionSkin )
				m_pSelectionSkin = bp.selectionSkin;
			else
				m_pSelectionSkin = BoxSkin::create( WGBP(BoxSkin,
														 _.color = HiColor(Color::LightCyan).withAlpha(2048);
														 _.outlineThickness = 1,
														 _.outlineColor = Color::DarkCyan;
													) );
			_startReceiveUpdates();

			m_pFrontend->_addDebugCapsule(this);
		}

		virtual ~DebugCapsule();

		void	_receive(Msg* pMsg) override;

		void 	_update(int microPassed, int64_t microsecTimestamp) override;
		void	_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;

		Widget * _findWidget(const CoordSPX& ofs, SearchMode mode) override;

		RectSPX		_selectionArea();


		DebugFrontend_p		m_pFrontend;

		bool		m_bInSelectMode = false;

		Widget_p	m_pSelectedWidget;

		RectSPX		m_renderedSelectionArea;

		Skin_p		m_pSelectionSkin;

		std::string	m_name;
		
	};


} // namespace wg
#endif //WG_DEBUGCAPSULE_DOT_H

