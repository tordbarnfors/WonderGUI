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
#ifndef WG_COLUMNHEADER_DOT_H
#define WG_COLUMNHEADER_DOT_H
#pragma once

#include <wg_component.h>
#include <wg_textlayout.h>
#include <wg_base.h>

namespace wg
{
	 
	//____ ColumnHeader ___________________________________________________

	class ColumnHeader : public Component, protected TextItem, protected SkinSlot::Holder
	{
	public:
		ColumnHeader(Widget * pWidget);
		virtual ~ColumnHeader();

		//.____ Blueprint _____________________________________________
		
		struct Blueprint
		{
			TextLayout_p	layout;
			Skin_p			skin;
			TextStyle_p		style;
			String			text;
		};
		
		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance ____________________________________________

		void			setSkin(Skin * pSkin);
		inline Skin_p	skin() const { return m_skin.get(); }

		void			setText(const char * str) { setText(CharSeq(str));}
		void			setText(const CharSeq& seq);
		void			setText(const String& str);
		inline String	text() const { return m_text; }

		void			setStyle(TextStyle* pStyle);
		void			clearStyle();
		TextStyle_p		style() const { return m_pStyle; }

		void			setLayout(TextLayout* pLayout);
		void			clearLayout();
		TextLayout_p	layout() const { return m_pLayout; }

		//.____ Internal ______________________________________________________

		void			_initFromBlueprint(const Blueprint& blueprint);

	protected:
		void			_setSortOrder(SortOrder order);
		inline SortOrder _sortOrder() const { return m_sortOrder; }


		void			_setState(State state);
		inline State	_state() const { return m_state; }

		void			_setSize(SizeSPX size, int scale);
		inline SizeSPX	_size() const { return m_size; }

		SizeSPX			_defaultSize(int scale) const;
		spx				_matchingWidth(spx height, int scale) const;
		spx				_matchingHeight(spx width, int scale) const;

		bool			_receive(Msg * pMsg);
		void			_render(GfxDevice * pDevice, const RectSPX& _canvas);

		TextLayout* _layout() const { return m_pLayout ? m_pLayout.rawPtr() : Base::defaultTextLayout().rawPtr(); }
		TextStyle* _style() const { if (m_pStyle) return m_pStyle.rawPtr(); return Base::defaultStyle().rawPtr(); }


		//

		SizeSPX		_textSize() const override;
		State		_textState() const override;
		TextStyle* _textStyle() const override;
		const Char* _textBegin() const override;
		int 		_textLength() const override;
		bool		_caretVisible() const override;
		int			_caretOffset() const override;
		std::tuple<int, int>	_selectedText() const override;		// Begin/end of selection
		void		_mapperRequestRender() override;
		void		_mapperRequestRender(const RectSPX& rect) override;
		void		_mapperRequestResize() override;
		int			_scale() const override;

		float		_skinValue(const SkinSlot* pSlot) const override;
		float		_skinValue2(const SkinSlot* pSlot) const override;
		State		_skinState(const SkinSlot* pSlot) const override;
		SizeSPX		_skinSize(const SkinSlot* pSlot) const override;
		void		_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect) override;

	private:
		SkinSlot		m_skin;
		String			m_text;
		TextLayout_p	m_pLayout;
		TextStyle_p		m_pStyle;

		State			m_state;
		SizeSPX			m_size;
		int				m_scale = 64;

		bool			m_bPressed;
		SortOrder		m_sortOrder;
	};



} // namespace wg
#endif //WG_COLUMNHEADER_DOT_H
