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
#ifndef	WG_PLAINTEXTLAYOUT_DOT_H
#define	WG_PLAINTEXTLAYOUT_DOT_H
#pragma once

#include <wg_flowtextlayout.h>

namespace wg
{
	class PlainTextLayout;
	typedef	StrongPtr<PlainTextLayout>	PlainTextLayout_p;
	typedef	WeakPtr<PlainTextLayout>	PlainTextLayout_wp;

	//____ PlainTextLayout __________________________________________________________
	/**
		TextLayout displaying the text as it is, in the base style of the text.
		No markup is interpreted.
	*/

	class PlainTextLayout : public FlowTextLayout
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			bool		autoEllipsis		= true;
			Caret_p		caret				= nullptr;
			Finalizer_p	finalizer			= nullptr;
			float		lineSpacing			= 1.f;
			float		paragraphSpacing	= 0.f;					// Add to lineSpacing at hard line endings.
			Placement	placement			= Placement::NorthWest;
			BlendMode	selectionBackBlend	= BlendMode::Invert;
			HiColor		selectionBackColor	= HiColor::White;
			BlendMode	selectionCharBlend	= BlendMode::Invert;
			HiColor		selectionCharColor	= HiColor::White;
			int			tabWidth			= 8;					// Number of whitespace characters a tab corresonds to.
			bool		wrap				= false;
		};

		//.____ Creation __________________________________________

		static PlainTextLayout_p create() { return PlainTextLayout_p(new PlainTextLayout(Blueprint())); }
		static PlainTextLayout_p create( const Blueprint& blueprint ) { return PlainTextLayout_p(new PlainTextLayout(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		PlainTextLayout( const Blueprint& blueprint );
		virtual ~PlainTextLayout() {}

		static Settings		_settings( const Blueprint& bp );
	};

} // namespace wg
#endif //WG_PLAINTEXTLAYOUT_DOT_H
