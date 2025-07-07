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

#ifndef	WG_TEXTEDITOR_DOT_H
#define	WG_TEXTEDITOR_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_editabletext.h>

namespace wg
{

	class TextEditor;
	typedef	StrongPtr<TextEditor>		TextEditor_p;
	typedef	WeakPtr<TextEditor>	TextEditor_wp;

	/**
	* @brief Editable text widget.
	*/
	class TextEditor:public Widget
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			EditableText::Blueprint	editor;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			KeyAction		returnKeyAction = KeyAction::Insert;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Creation __________________________________________

		static TextEditor_p	create() { return TextEditor_p(new TextEditor()); }
		static TextEditor_p	create(const Blueprint& blueprint) { return TextEditor_p(new TextEditor(blueprint)); }

		//.____ Components ____________________________________

		EditableText		editor;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Behavior ______________________________________________________

		void				setReturnKeyAction(KeyAction action);
		inline KeyAction	returnKeyAction() { return m_returnKeyAction; }

		void				setTabKeyAction(KeyAction action);
		inline KeyAction	tabKeyAction() { return m_tabKeyAction; }


		//.____ Internal _________________________________________________

		spx				_matchingWidth(spx width, int scale) const override;
		spx				_matchingHeight(spx width, int scale) const override;
		SizeSPX			_defaultSize(int scale) const override;

	protected:
		TextEditor() : editor(this) {}
		template<class BP> TextEditor(const BP& bp) : editor(this), Widget(bp)
		{
			editor._initFromBlueprint(bp.editor);
			m_returnKeyAction = bp.returnKeyAction;
		}
		
		virtual ~TextEditor() {};

		void			_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) override;
		void			_resize( const SizeSPX& size, int scale ) override;
		void			_receive( Msg * pMsg ) override;
		void			_setState( State state ) override;

		bool			_markTest(const CoordSPX& ofs) override;

		void			_receiveComponentNotif( Component * pComponent, ComponentNotif notification, int value, void * pData ) override;

		
		KeyAction	m_returnKeyAction = KeyAction::Insert;
		KeyAction	m_tabKeyAction = KeyAction::Insert;
	};

} // namespace wg
#endif // WG_TEXTEDITOR_DOT_H
