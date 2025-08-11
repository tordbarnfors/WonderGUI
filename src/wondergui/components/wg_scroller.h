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

#ifndef	WG_SCROLLER_DOT_H
#define WG_SCROLLER_DOT_H
#pragma once

#include <wg_component.h>
#include <wg_pointers.h>
#include <wg_skin.h>
#include <wg_msg.h>

namespace wg
{

	//____ Scroller __________________________________________________________________

	class Scroller : public Component
	{
	public:

		class Holder
		{
		public:
			virtual void	_scrollbarStep(const Scroller* pComponent, int dir) = 0;
			virtual void	_scrollbarPage(const Scroller* pComponent, int dir) = 0;
			virtual void	_scrollbarWheel(const Scroller* pComponent, int dir) = 0;
			virtual spx		_scrollbarMove(const Scroller* pComponent, spx pos) = 0;
			virtual std::tuple<spx, spx, spx> _scrollbarOfsLenContent(const Scroller* pComponent) = 0;
		};

		Scroller(Widget* pWidget, Scroller::Holder * pHolderInterface, Axis axis);
		virtual ~Scroller();


		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Skin_p	back;
			Skin_p	backwardButton;
			Skin_p	bar;
			Skin_p	forwardButton;
			bool	jumpToPress = false;
			bool	visible = true;
		};

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance _____________________________________________

		void		setSkins(Skin* pBack, Skin* pBar, Skin* pBackwardButton, Skin* pForwardButton);
		void		setBackSkin(Skin * pSkin);
		Skin_p		backSkin() const { return m_skins[Back]; }
		void		setBarSkin(Skin * pSkin);
		Skin_p		barSkin() const { return m_skins[Bar]; }
		void		setForwardButtonSkin(Skin * pSkin);
		Skin_p		forwardButtonSkin() const { return m_skins[Next]; }
		void		setBackwardButtonSkin(Skin* pSkin);
		Skin_p		backwardButtonSkin() const { return m_skins[Prev]; }

		void		setVisible(bool bVisible);
		bool		isVisible() const { return m_bVisible; }
		
		//.____ Behavior ______________________________________________________

		void		setJumpToPress(bool bJump);
		bool		jumpToPress() const { return m_bJumpToPress; }

		//.____ Misc __________________________________________________

		inline bool				isDisplayable() const { return m_skins[Part::Bar] != nullptr || m_skins[Part::Prev] != nullptr || m_skins[Part::Next] != nullptr; }

		//.____ Internal ______________________________________________

		void		_initFromBlueprint(const Blueprint& bp);

		SizeSPX		_defaultSize(int scale) const;
		void		_render(GfxDevice* pDevice, const RectSPX& _canvas, int scale);
		bool		_receive(Msg* pMsg);
		void		_setState(State state);
		void		_setAxis(Axis axis);
		Axis		_axis() const { return m_axis; }
		bool		_alphaTest(CoordSPX ofs, SizeSPX canvas);
		void		_update(spx newViewPos, spx oldViewPos, spx newViewLen, spx oldViewLen, spx newContentLen, spx oldContentLen );

		State		_backState() { return m_states[Back]; }
		State		_barState() { return m_states[Bar]; }
		State		_backwardButtonState() { return m_states[Prev]; }
		State		_forwardButtonState() { return m_states[Next]; }

	protected:

		enum Part
		{
			None = -1,
			Back = 0,
			Bar = 1,
			Prev = 2,
			Next = 3
		};

		const static int Part_size = 4;			// None not included.

		inline Widget* _holder() const { return m_pWidget; }

		void		_pointerMoved(CoordSPX pointerPos);
		Part		_getMarkedPart(const RectSPX& canvas, CoordSPX pos) const;
		RectSPX		_partGeo(Part part, const RectSPX& canvas, int scale) const;
		void		_setPartState(Part part, State newState, State oldState, const RectSPX& fullCanvas, int scale);
		void		_replaceSkin(Skin* pNew, Skin_p& pOld);

		spx			_buttonLen(Part button, const RectSPX& canvas, int scale) const;
		RectSPX		_dragbarArea(const RectSPX& slideArea, int scale) const;
		RectSPX		_dragbarArea(const RectSPX& slideArea, int scale, spx viewOfs, spx viewLen, spx contentLen) const;

		Holder*		m_pHolderInterface;

		Axis		m_axis;
		bool		m_bJumpToPress = false;
		bool		m_bVisible = true;

		Skin_p		m_skins[Part_size];
		State		m_states[Part_size];

		Part		m_markedPart = Part::None;
		Part		m_pressedPart = Part::None;
		spx			m_barPressOfs = -1;				// press offset from beginning of bar when press on dragbar och middle of bar when press on background.
	};


} // namespace wg

#endif //WG_SCROLLER_DOT_H
