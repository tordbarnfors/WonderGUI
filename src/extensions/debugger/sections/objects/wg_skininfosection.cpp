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
#include "wg_skininfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo SkinInfoSection::TYPEINFO = { "SkinInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	SkinInfoSection::SkinInfoSection(const DebugTheme& theme, IDebugContext* pContext, Skin * pSkin)
		: TypedInfoSection<Skin>( theme, pContext, Skin::TYPEINFO.className, pSkin )
	{
		auto pBasePanel = WGCREATE( PackPanel, _.axis = Axis::Y );

		pBasePanel->slots << _createRows({
			boolRow( "Opaque:",           [](Skin* s) { return s->isOpaque(); } ),
			boolRow( "Content shifting:", [](Skin* s) { return s->isContentShifting(); } ),
			boolRow( "Ignores value:",    [](Skin* s) { return s->_ignoresValue(); } ),
			boolRow( "Ignores state:",    [](Skin* s) { return s->_ignoresState(); } ),
			boolRow( "Overflows:",        [](Skin* s) { return s->_hasOverflow(); } ),
			intRow ( "Layer:",            [](Skin* s) { return s->layer(); } ),
			intRow ( "Mark alpha:",       [](Skin* s) { return s->markAlpha(); } )
		});

		m_displayedMargin	= pSkin->margin();
		m_displayedPadding	= pSkin->padding();
		m_displayedOverflow	= pSkin->overflow();

		m_pMarginDrawer		= _createBorderDrawer("Margin", m_displayedMargin);
		m_pPaddingDrawer	= _createBorderDrawer("Padding", m_displayedPadding);
		m_pOverflowDrawer	= _createBorderDrawer("Overflow", m_displayedOverflow);

		pBasePanel->slots << m_pMarginDrawer;
		pBasePanel->slots << m_pPaddingDrawer;
		pBasePanel->slots << m_pOverflowDrawer;

		this->slot = pBasePanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SkinInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void SkinInfoSection::refresh()
	{
		TypedInfoSection<Skin>::refresh();

		auto pSkin = inspected();

		_refreshBorderDrawer(m_pMarginDrawer, pSkin->margin(), m_displayedMargin);
		_refreshBorderDrawer(m_pPaddingDrawer, pSkin->padding(), m_displayedPadding);
		_refreshBorderDrawer(m_pOverflowDrawer, pSkin->overflow(), m_displayedOverflow);
	}

} // namespace wg
