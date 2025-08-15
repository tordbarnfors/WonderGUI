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
#ifndef WG_SELECTCAPSULE_DOT_H
#define WG_SELECTCAPSULE_DOT_H
#pragma once

#include <wg_capsule.h>


namespace wg
{

	class SelectCapsule;
	typedef	StrongPtr<SelectCapsule>	SelectCapsule_p;
	typedef	WeakPtr<SelectCapsule>		SelectCapsule_wp;


	class SelectCapsule : public Capsule
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{

			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			recursive = false;
			bool			selectable = false;
			SelectMode		selectMode = SelectMode::SingleEntry;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
			bool			usePickHandles = false;
		};

		//.____ Creation __________________________________________

		static SelectCapsule_p	create();
		static SelectCapsule_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ State _________________________________________________________

		bool		select(Widget* pWidget);
		bool		unselect(Widget* pWidget);

		void		selectAll();
		void		unselectAll();

		Widget_p				findOneSelected() const;
		std::vector<Widget_p>	findAllSelected() const;

		void		setSelectMode(SelectMode mode);
		SelectMode	selectMode() const { return m_selectMode; }

		void		setRecursive(bool bRecursive);
		bool		isRecursive() const { return m_bRecursive; }


		//.____ Internal _________________________________________________



	protected:
		SelectCapsule();

		template<class BP> SelectCapsule(const BP& bp) : Capsule(bp)
		{
			m_selectMode = bp.selectMode;
			m_bRecursive = bp.recursive;
		}

		virtual ~SelectCapsule();

		virtual void	_receive(Msg* pMsg);

		const Container* _topContainer() const;
		Container*		_topContainer();

		void			_setSelModeRecursively(Container* pContainer, bool bSelected, std::vector<Widget_p> * pList, Widget * pException = nullptr);
		bool			_isWidgetUnderOurControl(Widget* pWidget);

		Widget*			_findOneSelectedRecursively(const Container* pContainer) const;
		void			_findAllSelectedRecursively(const Container* pContainer, std::vector<Widget_p>* pList) const;

		SelectMode		m_selectMode = SelectMode::SingleEntry;
		bool			m_bRecursive = false;
	};


} // namespace wg
#endif //WG_SELECTCAPSULE_DOT_H
