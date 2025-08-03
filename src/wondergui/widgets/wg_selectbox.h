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
#ifndef WG_SELECTBOX_DOT_H
#define WG_SELECTBOX_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_sidecanvas.h>
#include <wg_dynamicvector.h>
#include <wg_dynamictext.h>

namespace wg
{

	class SelectBox;
	
	//____ SelectBoxEntry ________________________________________________________

	class SelectBoxEntry : private TextItem
	{
		friend class SelectBox;
	public:
		
		struct Blueprint
		{
			int		id;
			String	text;
		};
		
		SelectBoxEntry( const Blueprint& bp ) : m_id(bp.id), m_string(bp.text) {};
		virtual ~SelectBoxEntry() = default;
		
		inline int		id() const { return m_id; }
		inline String	text() const { return m_string; }

	protected:

		// Overloaded from Text

		SizeSPX		_textSize() const override;
		State		_textState() const override;
		TextStyle * _textStyle() const override;

		const Char *_textBegin() const override;
		int 		_textLength() const override;

		bool		_caretVisible() const override;
		int			_caretOffset() const override;
		std::tuple<int, int>	_selectedText() const override;

		void		_mapperRequestRender() override;
		void		_mapperRequestRender(const RectSPX& rect) override;
		void		_mapperRequestResize() override;

		int			_scale() const override;

		int			m_id;
		spx			m_height;
		String 		m_string;
		SelectBox * m_pParent = nullptr;
		State		m_state = State::Default;
	};
		

	//____ SelectBox __________________________________________________________

	class SelectBox;
	typedef	StrongPtr<SelectBox>		SelectBox_p;
	typedef	WeakPtr<SelectBox>		SelectBox_wp;


	class SelectBox : public Widget, protected DynamicVector<SelectBoxEntry>::Holder, protected SideCanvas::Holder
	{
		friend class SelectBoxEntry;
	public:

		//.____ Blueprint ___________________________________________________________
		
		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Skin_p			entrySkin;
			TextStyle_p		entryTextStyle;
			TextLayout_p	entryTextLayout;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			Skin_p			listSkin;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Creation ____________________________________________________________

		static SelectBox_p	create() { return SelectBox_p(new SelectBox()); }
		static SelectBox_p	create( const Blueprint& blueprint ) { return SelectBox_p(new SelectBox(blueprint)); }

		//.____ Components __________________________________________________________

		DynamicText			text;
		DynamicVector<SelectBoxEntry>	entries;
		
		//.____ Identification ______________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance __________________________________________________________

		void			setEntrySkin(Skin * pSkin);
		Skin_p			entrySkin() const { return m_pEntrySkin;  }

		void			setEntryStyle(TextStyle * pStyle);
		TextStyle_p		entryStyle() const { return m_pEntryStyle; }

		void			setEntryTextLayout(TextLayout * pTextLayout);
		TextLayout_p	entryTextLayout() const { return m_pEntryTextLayout; }

		void			setListSkin(Skin * pSkin);
		Skin_p			listSkin() const { return m_pListCanvas->skin(); }

		//.___ Control _____________________________________________________________

		void			selectEntry(const DynamicVector<SelectBoxEntry>::iterator& it);
		bool			selectEntryById(int id);
		void			selectEntryByIndex(int index);

		int				selectedEntryId() const;
		inline int		selectedEntryIndex() const { return m_selectedEntryIndex; }

		//.____ Internal ______________________________________________________

		SizeSPX		_defaultSize(int scale) const override;
		spx			_matchingHeight(spx width, int scale) const override;

	protected:
		SelectBox();
		template<class BP> SelectBox(const BP& bp) : text(this), entries(this), Widget(bp)
		{
			m_pListCanvas = new MySideCanvas(this);

			m_pEntryStyle 		= bp.entryTextStyle;
			m_pEntryTextLayout 	= bp.entryTextLayout;

			if( bp.listSkin )
			{
				m_listCanvasDefaultSize = bp.listSkin->_contentBorderSize(m_scale);
				m_pListCanvas->m_skin.set(bp.listSkin);
			}
			
			if( bp.entrySkin )
			{
				SizeSPX padding = bp.entrySkin->_contentBorderSize(m_scale);

				m_pEntrySkin = bp.entrySkin;
				m_listCanvasDefaultSize.w += padding.w;
				m_entryContentPaddingSize = padding;
			}
		}


		virtual ~SelectBox();
		void 			_receive(Msg* pMsg) override;

		void		_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) override;
		void		_resize(const SizeSPX& size, int scale) override;
		void		_setState(State state) override;

		void		_open();
		void		_close();

		class MySideCanvas : public SideCanvas 
		{ 
			friend class SelectBox; 
			MySideCanvas(SideCanvas::Holder* pHolder) : SideCanvas(pHolder) {}
		};
		typedef	StrongPtr<MySideCanvas>	MySideCanvas_p;
		typedef	WeakPtr<MySideCanvas>	MySideCanvas_wp;

		void		_requestRenderEntry(SelectBoxEntry* pEntry);
		void		_markEntry(int idx);
		void		_selectEntry(int idx);
		int			_findEntry(const CoordSPX& ofsInListPanel, CoordSPX * pOfsOut = nullptr);

		inline TextLayout * _entryTextLayout() { return m_pEntryTextLayout ? m_pEntryTextLayout : Base::defaultTextLayout(); }
		inline const TextLayout * _entryTextLayout() const { return m_pEntryTextLayout ? m_pEntryTextLayout: Base::defaultTextLayout();  }

		//
		
		void		_didAddEntries(SelectBoxEntry * pEntry, int nb) override;
		void		_didMoveEntries(SelectBoxEntry * pFrom, SelectBoxEntry * pTo, int nb) override;
		void		_willEraseEntries(SelectBoxEntry * pEntry, int nb) override;

		//
		
		spx			_sideCanvasMatchingHeight( const SideCanvas * pCanvas, spx width, int scale ) const override;
		spx			_sideCanvasMatchingWidth( const SideCanvas * pCanvas, spx height, int scale ) const override;

		SizeSPX		_sideCanvasDefaultSize( const SideCanvas * pCanvas, int scale ) const  override;
		void		_sideCanvasRender( SideCanvas * pCanvas, GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) override;
		void		_sideCanvasResize( SideCanvas * pCanvas, const SizeSPX& size, int scale ) override;

		void		_sideCanvasReceive( SideCanvas * pCanvas,  Msg * pMsg ) override;
		bool		_sideCanvasAlphaTest( SideCanvas * pCanvas,  const CoordSPX& ofs ) override;			
	
		void		_sideCanvasSetSkin(SideCanvas * pCanvas, Skin * pSkin) override;
		void		_sideCanvasSetState(SideCanvas * pCanvas, State state) override;

		

	private:

		MySideCanvas_p	m_pListCanvas;
		int				m_selectedEntryIndex = -1;
		int				m_markedEntryIndex = -1;		// Marked through mouse hover or keyboard input.

		Skin_p			m_pEntrySkin;
		TextStyle_p		m_pEntryStyle;
		TextLayout_p	m_pEntryTextLayout;

		spx				m_entryContentWidth;			// Width of content of an entry in the list.
		SizeSPX			m_entryContentPaddingSize;		// Size of padding of content of an entry in the list.

		SizeSPX			m_defaultSize;
		spx				m_matchingHeight = 0;			// Matching height for current width.

		spx				m_listCanvasMatchingHeight = 0;
		SizeSPX			m_listCanvasDefaultSize;

		bool			m_bOpen = false;
		State			m_closeState;


	};




} // namespace wg
#endif //WG_SELECTBOX_DOT_H
