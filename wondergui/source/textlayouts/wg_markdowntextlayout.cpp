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
#include <wg_markdowntextlayout.h>
#include <wg_textlink.h>
#include <wax_utf8.h>

#include "md4c/md4c.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace wg
{
	const TypeInfo MarkdownTextLayout::TYPEINFO = { "MarkdownTextLayout", &FlowTextLayout::TYPEINFO };

	//____ _decodeEntity() ____________________________________________________
	//
	// Decodes numeric and the most common named entities. Returns 0 if unknown.

	static char32_t _decodeEntity( std::string_view entity )
	{
		if( entity.size() < 3 || entity.front() != '&' || entity.back() != ';' )
			return 0;

		std::string_view name = entity.substr(1, entity.size() - 2);

		if( name[0] == '#' )
		{
			std::string number( name.substr(1) );
			int base = 10;

			if( !number.empty() && (number[0] == 'x' || number[0] == 'X') )
			{
				number.erase(0,1);
				base = 16;
			}

			char * pEnd = nullptr;
			unsigned long value = std::strtoul(number.c_str(), &pEnd, base);
			if( number.empty() || *pEnd != 0 || value == 0 || value > Utf8::MaxCodepoint )
				return Utf8::ReplacementChar;

			return char32_t(value);
		}

		static const struct { const char * pName; char32_t code; } entities[] =
		{
			{ "amp", '&' }, { "lt", '<' }, { "gt", '>' }, { "quot", '"' }, { "apos", '\'' },
			{ "nbsp", 0xA0 }, { "copy", 0xA9 }, { "reg", 0xAE }, { "trade", 0x2122 },
			{ "hellip", 0x2026 }, { "mdash", 0x2014 }, { "ndash", 0x2013 }, { "deg", 0xB0 },
			{ "times", 0xD7 }, { "euro", 0x20AC }, { "bull", 0x2022 }, { "middot", 0xB7 }
		};

		for( auto& e : entities )
			if( name == e.pName )
				return e.code;

		return 0;
	}

	//____ constructor _____________________________________________________________

	MarkdownTextLayout::MarkdownTextLayout(const Blueprint& bp) : FlowTextLayout( _settings(bp) ), m_blueprint(bp)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MarkdownTextLayout::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ escape() ___________________________________________________________

	std::string MarkdownTextLayout::escape( std::string_view text )
	{
		std::string out;
		out.reserve( text.size() + text.size()/8 );

		for( char c : text )
		{
			if( c == '\n' )
				out += "\\\n";								// Backslash before newline is a hard line break.
			else
			{
				if( std::strchr("\\`*_{}[]()<>#+-.!|~&", c) != nullptr && c != 0 )
					out.push_back('\\');
				out.push_back(c);
			}
		}
		return out;
	}

	//____ _settings() ________________________________________________________

	FlowTextLayout::Settings MarkdownTextLayout::_settings( const Blueprint& bp )
	{
		Settings s;
		s.autoEllipsis			= bp.autoEllipsis;
		s.caret					= bp.caret;
		s.lineSpacing			= bp.lineSpacing;
		s.paragraphSpacing		= bp.paragraphSpacing;
		s.placement				= bp.placement;
		s.selectionBackBlend	= bp.selectionBackBlend;
		s.selectionBackColor	= bp.selectionBackColor;
		s.selectionCharBlend	= bp.selectionCharBlend;
		s.selectionCharColor	= bp.selectionCharColor;
		s.tabWidth				= bp.tabWidth;
		s.wrap					= bp.wrap;
		return s;
	}

	//____ _parse() ___________________________________________________________
	/*
		md4c reports the text to display with pointers into the source, but not the
		markers. We collect the visible pieces of text with the styles in effect and
		then treat everything between them as markup, which is hidden or (with
		showMarkup set) displayed in the markup style.
	*/

	void MarkdownTextLayout::_parse( const std::string& text, Markup& markup ) const
	{
		struct Piece
		{
			int					begin;
			int					end;
			const TextStyle *	styles[c_maxRunStyles];
			int					nbStyles;
			TextLink_p			pLink;
			char32_t			substitute;
		};

		struct LineIndent
		{
			int		lineBegin;
			pts		indent;
			pts		wrapIndent;
		};

		struct Context
		{
			Context( const Blueprint& bp, const std::string& text ) : bp(bp), text(text) {}

			const Blueprint&				bp;
			const std::string&				text;

			std::vector<const TextStyle*>	blockStyles;
			std::vector<const TextStyle*>	spanStyles;
			std::vector<TextLink_p>			links;
			std::vector<bool>				lists;				// True for ordered lists.

			int		quoteDepth = 0;
			bool	bInCodeBlock = false;
			int		pendingCodeIndent = 0;					// Indentation md4c reported without pointer into source.
			bool	bPendingListMarker = false;
			int		lastTextEnd = 0;

			std::vector<Piece>		pieces;
			std::vector<LineIndent>	lineIndents;

			void addPiece( int begin, int end, char32_t substitute = 0 )
			{
				Piece piece;
				piece.begin = begin;
				piece.end = end;
				piece.nbStyles = 0;
				piece.substitute = substitute;
				piece.pLink = links.empty() ? nullptr : links.back();

				for( auto p : blockStyles )
					if( p && piece.nbStyles < c_maxRunStyles )
						piece.styles[piece.nbStyles++] = p;

				for( auto p : spanStyles )
					if( p && piece.nbStyles < c_maxRunStyles )
						piece.styles[piece.nbStyles++] = p;

				pieces.push_back(piece);
			}

			void recordLineIndent( int ofs, bool bMarkerLine )
			{
				if( bp.showMarkup )
					return;

				size_t nl = ofs > 0 ? text.rfind('\n', ofs - 1) : std::string::npos;
				int lineBegin = nl == std::string::npos ? 0 : int(nl) + 1;

				if( !lineIndents.empty() && lineIndents.back().lineBegin >= lineBegin )
					return;

				// List items: line with the marker is indented to the marker, its wrapped
				// lines and lines without marker to the text after the marker.

				int listDepth = int(lists.size());
				pts step = bp.indentation;

				pts indent = quoteDepth * step + (listDepth > 0 ? (listDepth - 1) * step : 0);
				pts wrapIndent = 0;

				if( listDepth > 0 )
				{
					if( bMarkerLine )
						wrapIndent = step;
					else
						indent += step;
				}

				lineIndents.push_back( { lineBegin, indent, wrapIndent } );
			}

			void addListMarker( int textBegin )
			{
				int m = textBegin - 1;
				while( m >= 0 && (text[m] == ' ' || text[m] == '\t') )
					m--;

				if( m < 0 || lists.empty() )
					return;

				int markerBegin = m;
				char32_t substitute = 0;

				if( lists.back() )
				{
					if( text[m] != '.' && text[m] != ')' )
						return;

					while( markerBegin > 0 && text[markerBegin-1] >= '0' && text[markerBegin-1] <= '9' )
						markerBegin--;
				}
				else
				{
					if( text[m] != '-' && text[m] != '+' && text[m] != '*' )
						return;

					substitute = bp.bullet;
				}

				recordLineIndent( markerBegin, true );

				addPiece( markerBegin, m + 1, substitute );
				if( m + 1 < textBegin )
					addPiece( m + 1, m + 2 );						// Keep one space between marker and text.
			}
		};

		Context c( m_blueprint, text );

		MD_PARSER parser = {};

		parser.flags = MD_FLAG_NOHTML | MD_FLAG_STRIKETHROUGH;

		parser.enter_block = [](MD_BLOCKTYPE type, void* pDetail, void* pUserdata) -> int
		{
			auto& c = * static_cast<Context*>(pUserdata);
			const TextStyle * pStyle = nullptr;

			switch( type )
			{
				case MD_BLOCK_H:
				{
					unsigned level = static_cast<MD_BLOCK_H_DETAIL*>(pDetail)->level;
					if( level >= 1 && level <= 6 )
						pStyle = c.bp.heading[level-1];
					break;
				}
				case MD_BLOCK_QUOTE:
					c.quoteDepth++;
					pStyle = c.bp.quote;
					break;
				case MD_BLOCK_UL:
					c.lists.push_back(false);
					break;
				case MD_BLOCK_OL:
					c.lists.push_back(true);
					break;
				case MD_BLOCK_LI:
					c.bPendingListMarker = !c.bp.showMarkup;
					pStyle = c.bp.listItem;
					break;
				case MD_BLOCK_CODE:
					c.bInCodeBlock = true;
					pStyle = c.bp.codeBlock;
					break;
				default:
					break;
			}

			c.blockStyles.push_back(pStyle);
			return 0;
		};

		parser.leave_block = [](MD_BLOCKTYPE type, void* pDetail, void* pUserdata) -> int
		{
			auto& c = * static_cast<Context*>(pUserdata);

			switch( type )
			{
				case MD_BLOCK_QUOTE:
					c.quoteDepth--;
					break;
				case MD_BLOCK_UL:
				case MD_BLOCK_OL:
					c.lists.pop_back();
					break;
				case MD_BLOCK_LI:
					c.bPendingListMarker = false;
					break;
				case MD_BLOCK_CODE:
					c.bInCodeBlock = false;
					c.pendingCodeIndent = 0;
					break;
				default:
					break;
			}

			c.blockStyles.pop_back();
			return 0;
		};

		parser.enter_span = [](MD_SPANTYPE type, void* pDetail, void* pUserdata) -> int
		{
			auto& c = * static_cast<Context*>(pUserdata);
			const TextStyle * pStyle = nullptr;

			switch( type )
			{
				case MD_SPAN_EM:
					pStyle = c.bp.emphasis;
					break;
				case MD_SPAN_STRONG:
					pStyle = c.bp.strong;
					break;
				case MD_SPAN_CODE:
					pStyle = c.bp.code;
					break;
				case MD_SPAN_DEL:
					pStyle = c.bp.strikethrough;
					break;
				case MD_SPAN_A:
				{
					auto pA = static_cast<MD_SPAN_A_DETAIL*>(pDetail);

					TextLink::Blueprint bp;
					bp.link = std::string( pA->href.text, pA->href.size );
					bp.tooltip = std::string( pA->title.text, pA->title.size );

					c.links.push_back( TextLink::create(bp) );
					pStyle = c.bp.link;
					break;
				}
				default:
					break;
			}

			c.spanStyles.push_back(pStyle);
			return 0;
		};

		parser.leave_span = [](MD_SPANTYPE type, void* pDetail, void* pUserdata) -> int
		{
			auto& c = * static_cast<Context*>(pUserdata);

			if( type == MD_SPAN_A )
				c.links.pop_back();

			c.spanStyles.pop_back();
			return 0;
		};

		parser.text = [](MD_TEXTTYPE type, const MD_CHAR* pText, MD_SIZE size, void* pUserdata) -> int
		{
			auto& c = * static_cast<Context*>(pUserdata);
			const std::string& text = c.text;

			// Soft line breaks are displayed as spaces when rendering. md4c doesn't
			// say where they are, but they are the first newline after last text.

			if( type == MD_TEXT_SOFTBR )
			{
				if( !c.bp.showMarkup )
				{
					size_t pos = text.find('\n', c.lastTextEnd);
					if( pos != std::string::npos )
					{
						c.addPiece( int(pos), int(pos) + 1, ' ' );
						c.lastTextEnd = int(pos) + 1;
					}
				}
				return 0;
			}

			// Text not in our source is generated, like newlines and indentation in code blocks.

			const char * pBase = text.data();
			if( pText < pBase || pText + size > pBase + text.size() )
			{
				if( type == MD_TEXT_CODE && c.bInCodeBlock && !(size == 1 && pText[0] == '\n') )
					c.pendingCodeIndent += int(size);
				return 0;
			}

			int begin = int(pText - pBase);
			int end = begin + int(size);

			// Indentation in code blocks is displayed as it is.

			if( c.bInCodeBlock && c.pendingCodeIndent > 0 )
			{
				int n = c.pendingCodeIndent;
				while( n > 0 && begin > 0 && (text[begin-1] == ' ' || text[begin-1] == '\t') )
				{
					begin--;
					n--;
				}
				c.pendingCodeIndent = 0;
			}

			if( c.bPendingListMarker )
			{
				c.bPendingListMarker = false;
				c.addListMarker(begin);
			}

			c.recordLineIndent( begin, false );

			char32_t substitute = 0;
			if( type == MD_TEXT_ENTITY && !c.bp.showMarkup )
				substitute = _decodeEntity( std::string_view(text).substr(begin, size) );

			c.addPiece( begin, end, substitute );
			c.lastTextEnd = end;
			return 0;
		};

		md_parse( text.data(), MD_SIZE(text.size()), &parser, &c );

		// Turn pieces and the markup between them into runs.

		std::sort( c.pieces.begin(), c.pieces.end(), [](const Piece& a, const Piece& b) { return a.begin < b.begin; } );

		auto& runs = markup.runs;
		int textLen = int(text.size());

		auto addRuns = [&]( int begin, int end, const Piece * pPiece )
		{
			// Substituted pieces are displayed as one character, may contain newline.

			if( pPiece && pPiece->substitute )
			{
				Run run;
				run.begin = begin;
				run.end = end;
				run.substitute = pPiece->substitute;
				run.pLink = pPiece->pLink;
				for( int i = 0 ; i < pPiece->nbStyles ; i++ )
					run.addStyle( pPiece->styles[i] );
				runs.push_back(run);
				return;
			}

			// Other runs never span a newline, which we leave out.

			while( begin < end )
			{
				size_t nl = text.find('\n', begin);
				int lineEnd = (nl == std::string::npos || int(nl) >= end) ? end : int(nl);

				if( lineEnd > begin )
				{
					Run run;
					run.begin = begin;
					run.end = lineEnd;

					if( pPiece )
					{
						run.pLink = pPiece->pLink;
						for( int i = 0 ; i < pPiece->nbStyles ; i++ )
							run.addStyle( pPiece->styles[i] );
					}
					else if( m_blueprint.showMarkup )
						run.addStyle( m_blueprint.markup );
					else
						run.bHidden = true;

					if( run.bHidden || run.nbStyles > 0 || run.pLink )
						runs.push_back(run);
				}

				begin = lineEnd + 1;
			}
		};

		int pos = 0;
		for( auto& piece : c.pieces )
		{
			if( piece.end <= pos )
				continue;									// Overlaps previous piece completely.

			int begin = std::max(piece.begin, pos);

			if( begin > pos )
				addRuns( pos, begin, nullptr );

			addRuns( begin, piece.end, &piece );
			pos = piece.end;
		}

		if( pos < textLen )
			addRuns( pos, textLen, nullptr );

		// Turn line indentations into blocks.

		for( auto& line : c.lineIndents )
		{
			if( line.indent == 0 && line.wrapIndent == 0 )
				continue;

			size_t nl = text.find('\n', line.lineBegin);

			Block block;
			block.begin = line.lineBegin;
			block.end = nl == std::string::npos ? textLen + 1 : int(nl) + 1;
			block.indent = line.indent;
			block.wrapIndent = line.wrapIndent;

			markup.blocks.push_back(block);
		}
	}

} // namespace wg
