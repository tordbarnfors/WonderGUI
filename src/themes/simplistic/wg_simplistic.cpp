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

#include <wg_simplistic.h>

#include <wg_sysfont.h>

#include <wg_togglegroup.h>
#include <wg_labelcapsule.h>

#include <wg_colorskin.h>
#include <wg_boxskin.h>
#include <wg_blockskin.h>

#include <wg_basictextlayout.h>

namespace wg
{

const TypeInfo Simplistic::TYPEINFO = { "Simplistic", &Theme::TYPEINFO };


//____ create() _______________________________________________________________

Simplistic_p Simplistic::create(Font * pNormal, Font * pBold, Font * pItalic, Font * pMonospace, Surface * pWidgets )
{
	return Simplistic_p(new Simplistic(pNormal, pBold, pItalic, pMonospace, pWidgets));
}

//____ typeInfo() _____________________________________________________________

const TypeInfo& Simplistic::typeInfo(void) const
{
	return TYPEINFO;
}

//____ constructor ____________________________________________________________

Simplistic::Simplistic( Font * pNormal, Font * pBold, Font * pItalic, Font * pMonospace, Surface * pWidgets )
{
	m_pFontNormal = pNormal;
	m_pFontBold = pBold;
	m_pFontItalic = pItalic;
	m_pFontMonospace = pMonospace;


	HiColor plateColor = Color::LightGrey;
	HiColor borderColor = Color::DarkGrey;

	HiColor titlebarColor = Color::LightGreen;
	HiColor titlebarBorderColor = Color::DarkGreen;

	m_pStrongStyle = TextStyle::create(WGBP(TextStyle, _.font = m_pFontBold, _.color = HiColor::Black, _.size = c_textSizeNormal));
	m_pEmphasisStyle = TextStyle::create(WGBP(TextStyle, _.font = m_pFontItalic, _.color = HiColor::Black, _.size = c_textSizeNormal));
	m_pCodeStyle = TextStyle::create(WGBP(TextStyle, _.font = m_pFontMonospace, _.color = HiColor::Black, _.size = c_textSizeNormal));
	m_pMonospaceStyle = TextStyle::create(WGBP(TextStyle, _.font = m_pFontMonospace, _.color = HiColor::Black, _.size = c_textSizeNormal));
	m_pFinePrintStyle = TextStyle::create(WGBP(TextStyle, _.font = m_pFontNormal, _.color = HiColor::Black, _.size = 9));


	m_pBlackStyle = TextStyle::create( WGBP(TextStyle, _.font = m_pFontNormal, _.color = HiColor::Black, _.size = 12 ));
	m_pWhiteStyle = TextStyle::create( WGBP(TextStyle, _.font = m_pFontNormal, _.color = HiColor::White, _.size = 12 ));

	m_heading1Style = TextStyle::create( WGBP(TextStyle, _.font = m_pFontNormal, _.color = HiColor::Black, _.size = 20));
	m_heading2Style = TextStyle::create( WGBP(TextStyle, _.font = m_pFontBold, _.color = HiColor::Black, _.size = 20));
	m_heading3Style = TextStyle::create( WGBP(TextStyle, _.font = m_pFontNormal, _.color = HiColor::Black, _.size = 16));
	m_heading4Style = TextStyle::create( WGBP(TextStyle, _.font = m_pFontBold, _.color = HiColor::Black, _.size = 16));
	m_heading5Style = TextStyle::create( WGBP(TextStyle, _.font = m_pFontBold, _.color = HiColor::Black, _.size = 14));
	m_heading6Style = TextStyle::create( WGBP(TextStyle, _.font = m_pFontNormal, _.color = HiColor::Black, _.size = 14));



	auto pLayoutCenteredNoWrap = BasicTextLayout::create( WGBP(BasicTextLayout,
															   _.autoEllipsis = true,
															   _.placement = Placement::Center,
															   _.wrap = false ));

	m_pOpenCloseTransition = ValueTransition::create(250000);


	m_pPlateSkin = BlockSkin::create( WGBP(BlockSkin,
										_.surface = pWidgets,
										_.firstBlock = { 34,37,10,10 },
										_.padding = 3,
										_.frame = 3 ));

	m_pCanvasSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pWidgets,
		_.firstBlock = { 58,37,10,10 },
		_.padding = 3,
		_.frame = 3));

	m_pWindowSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pWidgets,
		_.firstBlock = { 70,37,10,10 },
		_.padding = 5,
		_.frame = 3));


	auto pTitleBarSkin = BoxSkin::create( WGBP(BoxSkin,
											_.color = titlebarColor,
											_.outlineColor = titlebarBorderColor,
											_.outlineThickness = 1,
											_.padding = 4 ));

	auto pLabelCapsuleSkin = BoxSkin::create( WGBP(BoxSkin,
										  _.color = HiColor::Transparent,
										  _.outlineColor = borderColor,
										  _.outlineThickness = 1,
										  _.spacing = { 8,2,2,2},
										  _.padding = { 10, 4, 4, 4} ));

	auto pCapsuleLabelSkin = ColorSkin::create( WGBP(ColorSkin,
													 _.color = plateColor,
													 _.padding = { 2,2,2,2 }
													 ) );

	auto pCapsuleLabelSkin2 = ColorSkin::create( WGBP(ColorSkin,
													 _.color = HiColor::Transparent,
													 _.padding = { 2,2,2,2 }
													 ) );


	auto pInvisibleBoxSkin = ColorSkin::create( WGBP(ColorSkin,
										  _.color = HiColor::Transparent,
										  _.spacing = { 6,2,2,2},
										  _.padding = { 16, 4, 4, 4} ));

	auto pPlusMinusToggleSkin = BlockSkin::create( WGBP(BlockSkin,
		_.surface = pWidgets,
		_.firstBlock = { 0,0,14,14 },
		_.axis = Axis::X,
		_.blockSpacing = 2,
		_.states = { State::Default, State::Hovered, State::Pressed, State::Checked, State::CheckedHovered, State::CheckedPressed } ));

	auto pSelectableEntrySkin = BoxSkin::create(WGBP(BoxSkin,
		_.markAlpha = 0,
		_.states = { {State::Default, Color::Transparent,Color::Transparent},
					 {State::Hovered, HiColor(Color::LightCyan).withAlpha(1024), HiColor(Color::DarkCyan).withAlpha(1024)},
					 {State::Selected, Color::LightCyan,Color::DarkCyan }
		}
	));

	auto pButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pWidgets,
		_.firstBlock = { 0,15,10,10 },
		_.axis = Axis::X,
		_.blockSpacing = 1,
		_.frame = 4,
		_.padding = 4,
		_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
	));

	auto pToggleButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pWidgets,
		_.firstBlock = { 0,26,10,10 },
		_.axis = Axis::X,
		_.blockSpacing = 1,
		_.frame = 4,
		_.padding = 4,
		_.states = { State::Default, State::Hovered, State::Checked, State::CheckedHovered, State::Disabled }
	));

	auto pCheckboxSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pWidgets,
		_.firstBlock = { 0,37,10,10 },
		_.axis = Axis::X,
		_.blockSpacing = 1,
		_.states = { State::Default, State::Checked, State::Disabled }
	));

	auto pRadioButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pWidgets,
		_.firstBlock = { 0,48,10,10 },
		_.axis = Axis::X,
		_.blockSpacing = 1,
		_.states = { State::Default, State::Checked, State::Disabled }
	));

	auto pSelectBoxSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pWidgets,
		_.firstBlock = { 96,0,34,22},
		_.frame = { 4,25,4,4 },
		_.padding = { 3, 25, 3, 4,},
		_.axis = Axis::Y,
		_.blockSpacing = 1,
		_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
	));

	auto pSelectBoxEntrySkin = BoxSkin::create(WGBP(BoxSkin,
		_.states = { {State::Default, Color::Transparent, Color::Transparent},
					 {State::Hovered, HiColor(Color::LightCyan).withAlpha(2048), HiColor(Color::DarkCyan).withAlpha(2048)},
					 {State::Selected, Color::LightCyan,Color::DarkCyan }
		}
	));


	m_labeledBoxBP = WGBP(LabelCapsule,
						  _.label.style = m_pBlackStyle,
						  _.labelSkin = pCapsuleLabelSkin,
						  _.labelPlacement = Placement::North,
						  _.skin = pLabelCapsuleSkin );

	m_labeledSectionBP = WGBP(LabelCapsule,
						  _.label.style = m_pBlackStyle,
						  _.labelSkin = pCapsuleLabelSkin2,
						  _.labelPlacement = Placement::North,
						  _.skin = pInvisibleBoxSkin );


	auto pScrollbarBgSkin = BoxSkin::create( WGBP(BoxSkin,
											_.color = Color::DarkGray,
											  _.outlineColor = Color::Black,
											  _.outlineThickness = 1,
											  _.padding = 2));

	auto pScrollbarSkin = BoxSkin::create( WGBP(BoxSkin,
											_.color = Color::LightGray,
											  _.outlineColor = Color::Black,
											  _.outlineThickness = 1,
											  _.padding = 6));

	m_scrollPanelYBP = WGBP(ScrollPanel,
						   _.childConstraintX = SizeConstraint::Equal,
						   _.childConstraintY = SizeConstraint::GreaterOrEqual,
						   _.scrollbarY.back = pScrollbarBgSkin,
						   _.scrollbarY.bar = pScrollbarSkin
						   );

	m_scrollPanelXBP = WGBP(ScrollPanel,
						   _.childConstraintX = SizeConstraint::GreaterOrEqual,
						   _.childConstraintY = SizeConstraint::Equal,
						   _.scrollbarX.back = pScrollbarBgSkin,
						   _.scrollbarX.bar = pScrollbarSkin
						   );

	m_scrollPanelXYBP = WGBP(ScrollPanel,
							_.childConstraintX = SizeConstraint::GreaterOrEqual,
							_.childConstraintY = SizeConstraint::GreaterOrEqual,
							_.scrollbarX.back = pScrollbarBgSkin,
							_.scrollbarX.bar = pScrollbarSkin,
							_.scrollbarY.back = pScrollbarBgSkin,
							_.scrollbarY.bar = pScrollbarSkin
							);

	m_splitPanelXBP = WGBP(SplitPanel,
							_.axis = Axis::X,
							_.handleSkin = pButtonSkin
	);

	m_splitPanelYBP = WGBP(SplitPanel,
		_.axis = Axis::Y,
		_.handleSkin = pButtonSkin
	);

	m_treeListDrawer = WGBP(DrawerPanel,
							_.buttonSkin = pPlusMinusToggleSkin,
							_.buttonPlacement = Placement::West,
							_.buttonSize = Size{ 14, 14 },
							_.transition = m_pOpenCloseTransition
	);

	m_treeListEntry = WGBP(PaddingCapsule,
		_.skin = pSelectableEntrySkin
	);

	m_listTable = WGBP(TablePanel,
		_.columnSpacing = 4,
		_.rowSpacing = 1);

	m_textEditor = WGBP(TextEditor,
		_.skin = m_pCanvasSkin,
		_.editor.style = m_pBlackStyle
	);

	m_selectBox = WGBP(SelectBox,
		_.skin = pSelectBoxSkin,
		_.listSkin = m_pCanvasSkin,
		_.entryTextStyle  = m_pBlackStyle,
		_.entrySkin = pSelectBoxEntrySkin
	);

	m_windowTitleBar = WGBP(TextDisplay,
							_.skin = pTitleBarSkin,
							_.display.layout = pLayoutCenteredNoWrap );

	
	m_pushButtonBP = WGBP(Button,
							_.skin = pButtonSkin,
							_.label.style = m_pBlackStyle
	);

	m_toggleButtonBP = WGBP(ToggleButton,
							_.skin = pToggleButtonSkin,
							_.label.style = m_pBlackStyle
	);

	m_checkboxBP = WGBP(ToggleButton,
		_.icon.skin = pCheckboxSkin,
		_.icon.spacing = 4,
		_.label.style = m_pBlackStyle
	);

	m_radioButtonBP = WGBP(ToggleButton,
		_.skin = pRadioButtonSkin,
		_.label.style = m_pBlackStyle
	);
}


pts Simplistic::textSizeSmallest() const
{
	return c_textSizeSmallest;
}

pts Simplistic::textSizeSmall() const
{
	return c_textSizeSmall;
}

pts	Simplistic::textSizeNormal() const
{
	return c_textSizeNormal;
}

pts	Simplistic::textSizeLarge() const
{
	return c_textSizeLarge;
}

pts	Simplistic::textSizeLargest() const
{
	return c_textSizeLargest;
}

Font_p Simplistic::normalFont() const
{
	return m_pFontNormal;
}

Font_p Simplistic::monospaceFont() const
{
	return m_pFontMonospace;
}

Font_p Simplistic::boldFont() const
{
	return m_pFontBold;
}

Font_p Simplistic::italicFont() const
{
	return m_pFontItalic;
}

TextStyle_p Simplistic::heading1Style() const
{
	return m_heading1Style;
}

TextStyle_p Simplistic::heading2Style() const
{
	return m_heading2Style;
}

TextStyle_p Simplistic::heading3Style() const
{
	return m_heading3Style;
}

TextStyle_p Simplistic::heading4Style() const
{
	return m_heading4Style;
}

TextStyle_p Simplistic::heading5Style() const
{
	return m_heading5Style;
}

TextStyle_p Simplistic::heading6Style() const
{
	return m_heading6Style;
}

TextStyle_p Simplistic::subheadStyle1() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::subheadStyle2() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::subheadStyle3() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::subheadStyle4() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::subheadStyle5() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::subheadStyle6() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::paragraphStyle() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::strongStyle() const
{
	return m_pStrongStyle;
}

TextStyle_p Simplistic::emphasisStyle() const
{
	return m_pEmphasisStyle;
}

TextStyle_p Simplistic::codeStyle() const
{
	return m_pCodeStyle;
}

TextStyle_p Simplistic::linkStyle() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::monospaceStyle() const
{
	return m_pMonospaceStyle;
}

TextStyle_p Simplistic::quoteStyle() const
{
	return m_pEmphasisStyle;
}

TextStyle_p Simplistic::captionStyle() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::calloutStyle() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::finePrintStyle() const
{
	return m_pFinePrintStyle;
}


TextStyle_p Simplistic::defaultStyle() const
{
	return m_pBlackStyle;
}

TextStyle_p Simplistic::pressableStyle() const
{
	return m_pBlackStyle;
}

Skin_p Simplistic::plateSkin() const
{
	return m_pPlateSkin;
}

Skin_p Simplistic::canvasSkin() const
{
	return m_pCanvasSkin;
}

Skin_p Simplistic::windowSkin() const
{
	return m_pWindowSkin;
}


const LabelCapsule::Blueprint& Simplistic::labeledBox() const
{
	return m_labeledBoxBP;
}

const LabelCapsule::Blueprint& Simplistic::labeledSection() const
{
	return m_labeledSectionBP;
}

const Button::Blueprint& Simplistic::pushButton() const
{
	return m_pushButtonBP;
}

const ToggleButton::Blueprint& Simplistic::toggleButton() const
{
	return m_toggleButtonBP;
}

const ToggleButton::Blueprint& Simplistic::radioButton() const
{
	return m_radioButtonBP;
}

const ToggleButton::Blueprint& Simplistic::checkbox() const
{
	return m_checkboxBP;
}

const ScrollPanel::Blueprint& Simplistic::scrollPanelX() const
{
	return m_scrollPanelXBP;
}

const ScrollPanel::Blueprint& Simplistic::scrollPanelY() const
{
	return m_scrollPanelYBP;
}

const ScrollPanel::Blueprint& Simplistic::scrollPanelXY() const
{
	return m_scrollPanelXYBP;
}

const SplitPanel::Blueprint& Simplistic::splitPanelX() const
{
	return m_splitPanelXBP;
}

const SplitPanel::Blueprint& Simplistic::splitPanelY() const
{
	return m_splitPanelYBP;
}

const DrawerPanel::Blueprint& Simplistic::treeListDrawer() const
{
	return m_treeListDrawer;
}

const PaddingCapsule::Blueprint& Simplistic::treeListEntry() const
{
	return m_treeListEntry;
}

const TablePanel::Blueprint& Simplistic::listTable() const
{
	return m_listTable;
}


const TextEditor::Blueprint& Simplistic::textEditor() const
{
	return m_textEditor;
}

const SelectBox::Blueprint& Simplistic::selectBox() const
{
	return m_selectBox;
}


const TextDisplay::Blueprint& Simplistic::windowTitleBar() const
{
	return m_windowTitleBar;
}




}
