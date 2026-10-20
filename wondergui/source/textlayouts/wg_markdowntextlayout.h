/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#ifndef	WG_MARKDOWNTEXTLAYOUT_DOT_H
#define	WG_MARKDOWNTEXTLAYOUT_DOT_H
#pragma once

#include <wg_flowtextlayout.h>

namespace wg
{
	class MarkdownTextLayout;
	typedef	StrongPtr<MarkdownTextLayout>	MarkdownTextLayout_p;
	typedef	WeakPtr<MarkdownTextLayout>		MarkdownTextLayout_wp;

	//____ MarkdownTextLayout __________________________________________________________
	/**
		TextLayout displaying text formatted with Markdown (CommonMark).

		Each Markdown element is displayed with its own TextStyle, set in the Blueprint.
		The styles are added on top of the base style of the text, so a style only needs
		to specify what differs from surrounding text, like font for strong text or
		sizeScale for headings. Elements without a style are displayed in the base style.

		By default the Markdown is rendered: markers are hidden, list markers are
		displayed as bullets, lists and quotes are indented and single line breaks
		within paragraphs are displayed as spaces.

		With showMarkup set, the source is displayed as it is with markers visible in
		the markup style, which is what should be used when editing the text.

		Supported are paragraphs, headings, emphasis, strong, strikethrough, code spans,
		code blocks, block quotes, lists and links. Raw HTML is displayed as text.
		Links are reported as TextLinks with the destination as link and title as tooltip.
	*/

	class MarkdownTextLayout : public FlowTextLayout
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			bool		autoEllipsis		= true;
			char32_t	bullet				= 0x2022;				// Character displayed for unordered list items.
			Caret_p		caret				= nullptr;
			TextStyle_p	code;										// `code`
			TextStyle_p	codeBlock;									// Fenced and indented code blocks.
			TextStyle_p	emphasis;									// *emphasis*
			Finalizer_p	finalizer			= nullptr;
			TextStyle_p	heading[6];									// # Heading 1 to ###### Heading 6.
			pts			indentation			= 16;					// Indentation per level of lists and quotes.
			float		lineSpacing			= 1.f;
			TextStyle_p	link;										// [text](destination)
			TextStyle_p	listItem;									// Items of lists, including bullet or number.
			TextStyle_p	markup;										// Markers when showMarkup is set.
			float		paragraphSpacing	= 0.f;					// Add to lineSpacing at hard line endings.
			Placement	placement			= Placement::NorthWest;
			TextStyle_p	quote;										// > Block quote
			BlendMode	selectionBackBlend	= BlendMode::Invert;
			HiColor		selectionBackColor	= HiColor::White;
			BlendMode	selectionCharBlend	= BlendMode::Invert;
			HiColor		selectionCharColor	= HiColor::White;
			bool		showMarkup			= false;				// Display source with visible markers, for editing.
			TextStyle_p	strikethrough;								// ~~strikethrough~~
			TextStyle_p	strong;										// **strong**
			int			tabWidth			= 8;					// Number of whitespace characters a tab corresonds to.
			bool		wrap				= false;
		};

		//.____ Creation __________________________________________

		static MarkdownTextLayout_p create() { return MarkdownTextLayout_p(new MarkdownTextLayout(Blueprint())); }
		static MarkdownTextLayout_p create( const Blueprint& blueprint ) { return MarkdownTextLayout_p(new MarkdownTextLayout(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc __________________________________________

		const Blueprint&	blueprint() const { return m_blueprint; }

		static std::string	escape( std::string_view text );		///< Escape characters in plain text so it is displayed as is.

	protected:
		MarkdownTextLayout( const Blueprint& blueprint );
		virtual ~MarkdownTextLayout() {}

		void				_parse( const std::string& text, Markup& markup ) const override;

		static Settings		_settings( const Blueprint& bp );

		Blueprint			m_blueprint;
	};

} // namespace wg
#endif //WG_MARKDOWNTEXTLAYOUT_DOT_H
