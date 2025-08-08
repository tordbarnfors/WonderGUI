/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Bärnfors's WonderGUI Graphics Toolkit
  and copyright (c) Bärnfors Technology AB, Sweden [info@barnfors.com].

							-----------

  The WonderGUI Graphics Toolkit is free software you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact us [info@barnfors.com] for details.

=========================================================================*/

#include <wg_debugfrontend.h>
#include <wg_debugcapsule.h>

#include <wg_basictextlayout.h>
#include <wg_basicnumberlayout.h>
#include <wg_colorskin.h>

namespace wg
{
	using namespace Util;

	const TypeInfo DebugFrontend::TYPEINFO = { "DebugFrontend", &Capsule::TYPEINFO };


	//____ constructor ____________________________________________________________

	DebugFrontend::DebugFrontend(const Blueprint& bp) : Capsule(bp)
	{
		m_pBackend = bp.backend;
		m_pTheme 	= bp.theme;
		m_pIcons	= bp.icons;
		m_pTransparencyGrid = bp.transparencyGrid;

		_createDebuggerBP();
		m_pBackend->setBlueprint(m_debugPanelBP);

		m_pBackend->setObjectSelectedCallback([this](Object* pSelected,Object* pSelectedFrom) {

			auto pWidget = dynamic_cast<Widget*>(pSelected);
			if (pWidget)
			{
				selectWidget(pWidget);
				return;
			}

			auto pSkin = dynamic_cast<Skin*>(pSelected);
			if( pSkin )
			{
				selectSkin(pSkin);
				return;
			}

			if( pSelected )
				selectObject(pSelected, pSelectedFrom);
		});
	}

	//____ Destructor _____________________________________________________________

	DebugFrontend::~DebugFrontend()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DebugFrontend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ addDebugCapsule() _____________________________________________________

	void DebugFrontend::addDebugCapsule( DebugCapsule * pCapsule )
	{
		m_capsules.push_back(pCapsule);
	}

	//____ removeDebugCapsule() __________________________________________________

	void DebugFrontend::removeDebugCapsule( DebugCapsule * pCapsule )
	{
		m_capsules.erase(std::remove(m_capsules.begin(), m_capsules.end(), pCapsule), m_capsules.end());
	}


	//____ selectWidget() _________________________________________________________

	void DebugFrontend::selectWidget(Widget * pWidget)
	{
		for( auto pCapsule : m_capsules )
			pCapsule->_widgetSelected(pWidget);
	}

	//____ selectSkin() _________________________________________________________

	void DebugFrontend::selectSkin(Skin* pSkin)
	{

	}

	//____ selectObject() _______________________________________________________

	void DebugFrontend::selectObject(Object* pSelected, Object * pSelectedFrom)
	{

	}

	//____ _createDebuggerBP() ___________________________________________________

	void DebugFrontend::_createDebuggerBP()
	{
		m_debugPanelBP.theme = m_pTheme;
		m_debugPanelBP.icons = m_pIcons;
		m_debugPanelBP.transparencyGrid = m_pTransparencyGrid;

		auto pListTextLayout = BasicTextLayout::create( WGBP(BasicTextLayout,
														  _.placement = Placement::East ));

		auto pInfoLayout = BasicTextLayout::create( WGBP(BasicTextLayout,
														  _.wrap = true,
														  _.placement = Placement::Center ));

		auto pWrapTextLayout = BasicTextLayout::create(WGBP(BasicTextLayout,
			_.wrap = true,
			_.placement = Placement::NorthWest));

		auto pValueLayout = BasicNumberLayout::create( WGBP(BasicNumberLayout,
			_.style = m_pTheme->defaultStyle(),
			_.decimalMin = 2
		));

		auto pIntegerLayout = BasicNumberLayout::create(WGBP(BasicNumberLayout,
			_.style = m_pTheme->defaultStyle(),
			_.decimalMin = 0
		));


		CharBuffer chrBuff;
		chrBuff.pushBack("0x");
		chrBuff.setStyle(m_pTheme->defaultStyle());

		auto pPointerLayout = BasicNumberLayout::create(WGBP(BasicNumberLayout,
			_.style = m_pTheme->defaultStyle(),
			_.base = 16,
			_.integerGrouping = 0,
			_.prefix = String(&chrBuff)
			));

		auto pPtsLayout = BasicNumberLayout::create(WGBP(BasicNumberLayout,
			_.style = m_pTheme->defaultStyle(),
			_.decimalMin = 2
		));



		m_debugPanelBP.classCapsule = WGBP(LabelCapsule,
			_.skin = ColorSkin::create(HiColor::Transparent, { 10,0,0,8 }),
			_.label.style = m_pTheme->finePrintStyle()
		);


		m_debugPanelBP.listEntryLabel = WGBP(TextDisplay,
											 _.display.style = m_pTheme->strongStyle() );

		m_debugPanelBP.listEntryText = WGBP(TextDisplay,
											 _.display.style = m_pTheme->defaultStyle(),
											 _.display.layout = pListTextLayout );

		m_debugPanelBP.listEntryInteger = WGBP(NumberDisplay,
											 _.display.layout = pIntegerLayout );

		m_debugPanelBP.listEntryBool = WGBP(NumberDisplay,
											_.display.layout = pValueLayout);

		m_debugPanelBP.listEntrySPX = WGBP(NumberDisplay,
											 _.display.layout = pIntegerLayout );

		m_debugPanelBP.listEntryPts = WGBP(NumberDisplay,
											 _.display.layout = pPtsLayout );

		m_debugPanelBP.listEntryDecimal = WGBP(NumberDisplay,
											 _.display.layout = pValueLayout );

//		m_debugPanelBP.listEntryPointer = WGBP(NumberDisplay,
//											 _.display.layout = pPointerLayout );

		m_debugPanelBP.listEntryDrawer = m_pTheme->treeListDrawer();
		m_debugPanelBP.selectableListEntryCapsule = WGOVR( m_pTheme->treeListEntry(), _.selectable = true );

		m_debugPanelBP.textField = WGBP(TextDisplay,
			_.display.style = m_pTheme->defaultStyle(),
			_.display.layout = pWrapTextLayout,
			_.skin = m_pTheme->canvasSkin() );

		m_debugPanelBP.infoDisplay = WGBP(TextDisplay,
											 _.display.style = m_pTheme->emphasisStyle(),
											 _.display.layout = pInfoLayout );

		m_debugPanelBP.table = WGBP(TablePanel,
									_.columnLayout = Base::defaultPackLayout());

	}


} // namespace wg

