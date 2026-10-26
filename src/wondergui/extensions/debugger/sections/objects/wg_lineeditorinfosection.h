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
#ifndef	WG_LINEEDITORINFOSECTION_DOT_H
#define WG_LINEEDITORINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_lineeditor.h>

namespace wg
{
	class LineEditorInfoSection;
	typedef	StrongPtr<LineEditorInfoSection>	LineEditorInfoSection_p;
	typedef	WeakPtr<LineEditorInfoSection>	LineEditorInfoSection_wp;



	class LineEditorInfoSection : public TypedInfoSection<LineEditor>
	{
	public:

		//.____ Creation __________________________________________

		static LineEditorInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, LineEditor * pInspected) { return LineEditorInfoSection_p(new LineEditorInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		LineEditorInfoSection(const DebugTheme& theme, IDebugContext* pContext, LineEditor * pInspected );
		~LineEditorInfoSection() {}

		DrawerPanel_p	m_pEditorDrawer;
	};

} // namespace wg
#endif //WG_LINEEDITORINFOSECTION_DOT_H
