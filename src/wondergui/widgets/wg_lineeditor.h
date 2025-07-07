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

#ifndef	WG_LINEEDITOR_DOT_H
#define	WG_LINEEDITOR_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_editabletext.h>

namespace wg
{

	class LineEditor;
	typedef	StrongPtr<LineEditor>	LineEditor_p;
	typedef	WeakPtr<LineEditor>		LineEditor_wp;

	class LineEditor:public Widget
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			spx				defaultLengthInChars = 20;		// Set to zero for returning default width calculated from actual text in field.
			bool			disabled = false;
			bool			dropTarget = false;
			EditableText::Blueprint	editor;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Ibeam;
			KeyAction		returnKeyAction = KeyAction::ReleaseFocus;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Creation __________________________________________

		static LineEditor_p	create() { return LineEditor_p(new LineEditor()); }
		static LineEditor_p	create(const Blueprint& blueprint) { return LineEditor_p(new LineEditor(blueprint)); }

		//.____ Components _______________________________________

		EditableText		editor;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ______________________________________________________

		void				setDefaultLengthInChars( int whitespaceChars );
		int					defaultLengthInChars() const { return m_defaultLengthInChars; }

		//.____ Behavior ______________________________________________________

		void				setReturnKeyAction(KeyAction action);
		inline KeyAction	returnKeyAction() { return m_returnKeyAction;  }

		//.____ Internal _________________________________________________

		SizeSPX			_defaultSize(int scale) const override;

	protected:
		LineEditor();
		
		template<class BP> LineEditor(const BP& bp) : editor(this), m_textScrollOfs(0), Widget(bp)
		{
			editor._initFromBlueprint(bp.editor);
			editor.setMaxLines(1);
			m_returnKeyAction = bp.returnKeyAction;
			m_defaultLengthInChars = bp.defaultLengthInChars;
		}

		virtual ~LineEditor();


		void			_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) override;
		void			_resize( const SizeSPX& size, int scale ) override;
		void			_receive( Msg * pMsg ) override;
		void			_setState( State state ) override;

		CoordSPX		_componentPos( const Component * pComponent ) const override;
		SizeSPX			_componentSize( const Component * pComponent ) const override;
		RectSPX			_componentGeo( const Component * pComponent ) const override;

		void			_componentRequestRender(const Component * pComponent) override;
		void			_componentRequestRender( const Component * pComponent, const RectSPX& rect ) override;
		void			_componentRequestResize( const Component * pComponent ) override;
		void			_componentRequestInView(const Component * pComponent, const RectSPX& mustHave, const RectSPX& niceToHave) override;

		void			_receiveComponentNotif( Component * pComponent, ComponentNotif notification, int value, void * pData ) override;

		spx				_editorWidth() const;

	private:

		spx				m_textScrollOfs;
		int				m_defaultLengthInChars = 20;
		KeyAction	m_returnKeyAction = KeyAction::ReleaseFocus;
	};






} // namespace wg
#endif // WG_TEXTDISPLAY_DOT_H
