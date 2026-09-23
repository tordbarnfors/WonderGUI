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
#ifndef	WG_TEXTSTYLEINSPECTOR_DOT_H
#define WG_TEXTSTYLEINSPECTOR_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_inspectorview.h>

namespace wg
{
	class TextStyleInspector;
	typedef	StrongPtr<TextStyleInspector>	TextStyleInspector_p;
	typedef	WeakPtr<TextStyleInspector>	TextStyleInspector_wp;



	class TextStyleInspector : public InspectorView
	{
	public:

		//.____ Creation __________________________________________

		static TextStyleInspector_p		create(const DebugTheme& theme, IDebugContext* pContext, TextStyle* pStyle) { return TextStyleInspector_p(new TextStyleInspector(theme, pContext, pStyle)); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		TextStyleInspector(const DebugTheme& theme, IDebugContext* pContext, TextStyle* pStyle);
		~TextStyleInspector() {}

		TextStyle_p	m_pStyle;			// Keep our style alive as long as we are, so refresh won't fail.
	};

} // namespace wg
#endif //WG_TEXTSTYLEINSPECTOR_DOT_H


