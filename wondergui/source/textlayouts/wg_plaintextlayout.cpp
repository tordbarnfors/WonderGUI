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
#include <wg_plaintextlayout.h>

namespace wg
{
	const TypeInfo PlainTextLayout::TYPEINFO = { "PlainTextLayout", &FlowTextLayout::TYPEINFO };

	//____ constructor _____________________________________________________________

	PlainTextLayout::PlainTextLayout(const Blueprint& bp) : FlowTextLayout( _settings(bp) )
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PlainTextLayout::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _settings() ________________________________________________________

	FlowTextLayout::Settings PlainTextLayout::_settings( const Blueprint& bp )
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

} // namespace wg
