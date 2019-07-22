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

#ifndef WG_STACKPANEL_DOT_H
#define WG_STACKPANEL_DOT_H
#pragma once

#include <wg_panel.h>
#include <wg_paddedslot.h>
#include <wg_ipaddedchildren.h>


namespace wg
{

	class StackPanel;
	typedef	StrongPtr<StackPanel>	StackPanel_p;
	typedef	WeakPtr<StackPanel>		StackPanel_wp;



	//____ StackPanelSlot ____________________________________________________________

	class StackPanelSlot : public PaddedSlot		/** @private */
	{
	public:
		StackPanelSlot() : origo(Origo::Center), SizeQolicy(SizeQolicy2D::Original) {}

		Origo			origo;
		SizeQolicy2D	SizeQolicy;
	};


	class IStackPanelChildren;
	typedef	StrongInterfacePtr<IStackPanelChildren>	IStackPanelChildren_p;
	typedef	WeakInterfacePtr<IStackPanelChildren>	IStackPanelChildren_wp;


	//____ StackPanelChildrenHolder __________________________________________________

	class StackPanelChildrenHolder : public PaddedChildrenHolder			/** @private */
	{
	public:
		virtual void		_childRequestRender(Slot * pSlot) = 0;
	};

	//____ IStackPanelChildren ________________________________________________________

	class IStackPanelChildren : public IPaddedChildren<StackPanelSlot,StackPanelChildrenHolder>
	{
	public:
		/** @private */

		IStackPanelChildren( SlotArray<StackPanelSlot> * pSlotArray, StackPanelChildrenHolder * pHolder ) : IPaddedChildren<StackPanelSlot,StackPanelChildrenHolder>(pSlotArray,pHolder) {}

		//.____ Misc __________________________________________________________

		inline IStackPanelChildren_p	ptr() { return IStackPanelChildren_p(this); }

		//.____ Geometry ______________________________________________________

		void		setSizeQolicy( int index, SizeQolicy2D policy );
		void		setSizeQolicy( iterator it, SizeQolicy2D policy );

		SizeQolicy2D SizeQolicy( int index ) const;
		SizeQolicy2D SizeQolicy( iterator it ) const;

		void		setOrigo( int index, Origo origo );
		void		setOrigo( iterator it, Origo origo );

		Origo		origo( int index ) const;
		Origo		origo( iterator it ) const;


	protected:
		void		_setSizeQolicy( StackPanelSlot * pSlot, SizeQolicy2D policy );
		void		_setOrigo( StackPanelSlot * pSlot, Origo origo );
	};



	//____ StackPanel ___________________________________________________________

	/**
	*/

	class StackPanel : public Panel, protected StackPanelChildrenHolder
	{
		friend class IStackPanelChildren;

	public:

		//.____ Creation __________________________________________

		static StackPanel_p	create() { return StackPanel_p(new StackPanel()); }

		//.____ Interfaces _______________________________________

		IStackPanelChildren	children;

		//.____ Identification __________________________________________

		bool				isInstanceOf( const char * pClassName ) const;
		const char *		className( void ) const;
		static const char	CLASSNAME[];
		static StackPanel_p	cast( Object * pObject );

		//.____ Geometry ____________________________________________

		int			matchingHeight( int width ) const;
		int			matchingWidth( int height ) const;

		Size		preferredSize() const;


	protected:
		StackPanel();
		virtual ~StackPanel();
		virtual Widget* _newOfMyType() const { return new StackPanel(); };

		// Overloaded from Widget

		void		_cloneContent( const Widget * _pOrg );
		void		_setSize( const Size& size );

		// Overloaded from Container

		Widget *	_firstChild() const;
		Widget *	_lastChild() const;

		void		_firstSlotWithGeo( SlotWithGeo& package ) const;
		void		_nextSlotWithGeo( SlotWithGeo& package ) const;

		// Overloaded from PaddedChildrenHolder

		Slot *		_incSlot(Slot * pSlot) const;
		Slot *		_decSlot(Slot * pSlot) const;
		void		_didAddSlots( Slot * pSlot, int nb );
		void		_didMoveSlots(Slot * pFrom, Slot * pTo, int nb);
		void		_willRemoveSlots( Slot * pSlot, int nb );
		void		_hideSlots( Slot *, int nb );
		void		_unhideSlots( Slot *, int nb );
		void		_repadSlots( Slot *, int nb, Border padding );
		void		_repadSlots(Slot *, int nb, const Border * pPaddings);
		Object *	_object() { return this; }
		WidgetHolder *	_widgetHolder() { return this; }


		// Overloaded from WidgetHolder

		Coord		_childPos( Slot * pSlot ) const;
		Size		_childSize( Slot * pSlot ) const;

		void		_childRequestRender( Slot * pSlot );
		void		_childRequestRender( Slot * pSlot, const Rect& rect );
		void		_childRequestResize( Slot * pSlot );

		Widget *	_prevChild( const Slot * pSlot ) const;
		Widget *	_nextChild( const Slot * pSlot ) const;

		void		_releaseChild(Slot * pSlot);

		// Internal to StackPanel

		void 	_refreshPreferredSize();
		void	_adaptChildrenToSize();

		void	_hideChildren( StackPanelSlot * pSlot, int nb );
		void	_unhideChildren( StackPanelSlot * pSlot, int nb );


		Rect	_childGeo( const StackPanelSlot * pSlot ) const;

		Size	m_preferredSize;

		SlotArray<StackPanelSlot>	m_children;
	};


} // namespace wg
#endif //WG_STACKPANEL_DOT_H
