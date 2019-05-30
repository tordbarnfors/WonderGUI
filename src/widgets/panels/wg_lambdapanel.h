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

#ifndef WG_LAMBDAPANEL_DOT_H
#define WG_LAMBDAPANEL_DOT_H
#pragma once


#include <wg_panel.h>
#include <wg_ihideablechildren.h>

namespace wg
{

	class LambdaPanel;
	typedef	StrongPtr<LambdaPanel>		LambdaPanel_p;
	typedef	WeakPtr<LambdaPanel>		LambdaPanel_wp;

	//____ LambdaPanelSlot ____________________________________________________________

	class LambdaPanelSlot : public Slot		/** @private */
	{
	public:
		LambdaPanelSlot() : pFunc(nullptr), bVisible(false) {}

		const static bool safe_to_relocate = false;

		std::function<Rect(Widget * pWidget, Size parentSize)>	pFunc;
		bool			bVisible;
		Rect			geo;				// Widgets geo relative parent
	};

	//____ LambdaChildrenHolder _________________________________________________

	class LambdaChildrenHolder : public HideableChildrenHolder		/** @private */
	{
	public:
		virtual void		_updateSlotGeo(Slot * pSlot, int nb) = 0;
	};


	class ILambdaPanelChildren;
	typedef	StrongInterfacePtr<ILambdaPanelChildren>	ILambdaPanelChildren_p;
	typedef	WeakInterfacePtr<ILambdaPanelChildren>		ILambdaPanelChildren_wp;

	//____ ILambdaPanelChildren ________________________________________________________

	class ILambdaPanelChildren : public IHideableChildren<LambdaPanelSlot,LambdaChildrenHolder>
	{
	public:
		/** @private */

		ILambdaPanelChildren( SlotArray<LambdaPanelSlot> * pSlotArray, LambdaChildrenHolder * pHolder ) : IHideableChildren<LambdaPanelSlot,LambdaChildrenHolder>(pSlotArray,pHolder) {}

		//.____ Content _______________________________________________________

		iterator	add( Widget * pWidget, std::function<Rect(Widget * pWidget, Size parenSize)> func );

		iterator	insert( int index, Widget * pWidget, std::function<Rect(Widget * pWidget, Size parentSize)> func );
		iterator	insert( iterator pos, Widget * pWidget, std::function<Rect(Widget * pWidget, Size parentSize)> func );

		//.____ Geometry ______________________________________________________

		void		setFunction( int index, std::function<Rect(Widget * pWidget, Size parentSize)> func);
		void		setFunction( iterator pos, std::function<Rect(Widget * pWidget, Size parentSize)> func);

		std::function<Rect(Widget * pWidget, Size parentSize)> function(int index) const;
		std::function<Rect(Widget * pWidget, Size parentSize)> function(iterator pos) const;

		//.____ Misc __________________________________________________________

		inline ILambdaPanelChildren_p	ptr() { return ILambdaPanelChildren_p(this); }
	};


	//____ LambdaPanel _________________________________________________________

	class LambdaPanel : public Panel, protected LambdaChildrenHolder
	{

	public:

		//.____ Creation __________________________________________

		static LambdaPanel_p	create() { return LambdaPanel_p(new LambdaPanel()); }

		//.____ Interfaces _______________________________________

		ILambdaPanelChildren	children;

		//.____ Identification __________________________________________

		bool				isInstanceOf( const char * pClassName ) const override;
		const char *		className( void ) const override;
		static const char	CLASSNAME[];
		static LambdaPanel_p	cast( Object * pObject );

		//.____ Geometry ____________________________________________

		bool				setMinSize(Size min);
		bool				setMaxSize(Size max);
		bool				setSizeLimits( Size min, Size max );
		bool				setPreferredSize(Size pref);

		Size				preferredSize() const override;

	protected:
		LambdaPanel();
		virtual ~LambdaPanel();
		virtual Widget* _newOfMyType() const override { return new LambdaPanel(); };


		// Overloaded from Container

		Widget *	_firstChild() const override;
		Widget *	_lastChild() const override;

		void		_firstSlotWithGeo( SlotWithGeo& package ) const override;
		void		_nextSlotWithGeo( SlotWithGeo& package ) const override;


		// Methods for LambdaPanelChildren

		Slot *		_incSlot(Slot * pSlot) const override;
		Slot *		_decSlot(Slot * pSlot) const override;
		void		_didAddSlots( Slot * pSlot, int nb ) override;
		void		_didMoveSlots(Slot * pFrom, Slot * pTo, int nb) override;
		void		_willRemoveSlots( Slot * pSlot, int nb ) override;
		void		_hideSlots( Slot * pSlot, int nb ) override;
		void		_unhideSlots( Slot * pSlot, int nb ) override;
		void		_updateSlotGeo(Slot * pSlot, int nb) override;
		Object *	_object() override { return this; }
		WidgetHolder *	_widgetHolder() override { return this; }

		// Overloaded from WidgetHolder

		Coord		_childPos( Slot * pSlot ) const override;
		Size		_childSize( Slot * pSlot ) const override;

		void		_childRequestRender( Slot * pSlot ) override;
		void		_childRequestRender( Slot * pSlot, const Rect& rect ) override;
		void		_childRequestResize( Slot * pSlot ) override;

		Widget *	_prevChild( const Slot * pSlot ) const override;
		Widget *	_nextChild( const Slot * pSlot ) const override;

		void		_releaseChild(Slot * pSlot) override;

	private:

		void		_cloneContent( const Widget * _pOrg ) override;
		void		_setSize( const Size& size ) override;
		void		_updateGeo(LambdaPanelSlot * pSlot);

		void		_onRequestRender( const Rect& rect, const LambdaPanelSlot * pSlot );


		SlotArray<LambdaPanelSlot>	m_children;

		Size		m_minSize;
		Size		m_maxSize;
		Size		m_preferredSize;
	};



} // namespace wg
#endif //WG_LAMBDAPANEL_DOT_H
