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

#ifndef	WG_THEME_SIMPLISTIC_DOT_H
#define WG_THEME_SIMPLISTIC_DOT_H
#pragma once

#include <wg_theme.h>

#include <wg_skin.h>
#include <wg_surfacefactory.h>


namespace wg
{

	class Simplistic;
	typedef	StrongPtr<Simplistic>	Simplistic_p;
	typedef	WeakPtr<Simplistic>		Simplistic_wp;


	class Simplistic : public Theme
	{
	public:

		//.____ Creation __________________________________________

		static Simplistic_p	create(Font * pNormal, Font * pBold, Font * pItalic, Font * pMonospace, Surface * pWidgets );

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _______________________________________________________

		// Text sizes

		pts			textSizeSmallest() const override;
		pts			textSizeSmall() const override;
		pts			textSizeNormal() const override;
		pts			textSizeLarge() const override;
		pts			textSizeLargest() const override;

		// Fonts

		Font_p		normalFont() const override;
		Font_p		monospaceFont() const override;
		Font_p		boldFont() const override;
		Font_p		italicFont() const override;

		// Text Styles

		TextStyle_p heading1Style() const override;
		TextStyle_p heading2Style() const override;
		TextStyle_p heading3Style() const override;
		TextStyle_p heading4Style() const override;
		TextStyle_p heading5Style() const override;
		TextStyle_p heading6Style() const override;

		TextStyle_p subheadStyle1() const override;		// Special paragraph or section below heading
		TextStyle_p subheadStyle2() const override;
		TextStyle_p subheadStyle3() const override;
		TextStyle_p subheadStyle4() const override;
		TextStyle_p subheadStyle5() const override;
		TextStyle_p subheadStyle6() const override;

		TextStyle_p paragraphStyle() const override;		// Paragraph of text

		TextStyle_p strongStyle() const override;		// Usually bold
		TextStyle_p emphasisStyle() const override;		// Usually italic
		TextStyle_p codeStyle() const override;			// For snippets of code, should be monospaced.
		TextStyle_p linkStyle() const override;			// For hyperlinks
		TextStyle_p monospaceStyle() const override;		// For monospaced text


		TextStyle_p quoteStyle() const override;		// Usually italic
		TextStyle_p captionStyle() const override;		// Text under image or figure.
		TextStyle_p calloutStyle() const override;		// Separate text section in magazine or such which highligt key passages.
		TextStyle_p finePrintStyle() const override;	// Style for fine print text. As small as possible while remaining readable

		TextStyle_p defaultStyle() const override;		// Default style for UI elements
		TextStyle_p pressableStyle() const override;	// Default text style for buttons etc.



		//

		Skin_p		plateSkin() const override;
		Skin_p		canvasSkin() const override;
		Skin_p		windowSkin() const override;

		//

		const LabelCapsule::Blueprint&		labeledBox() const override;		// Section with label and a box around it
		const LabelCapsule::Blueprint&		labeledSection() const override;	// Section with label and possibly a horizontal divider

		const Button::Blueprint& 			pushButton() const override;

		const ToggleButton::Blueprint& 		toggleButton() const override;
		const ToggleButton::Blueprint& 		radioButton() const override;
		const ToggleButton::Blueprint& 		checkbox() const override;

		const ScrollPanel::Blueprint& 		scrollPanelX() const override;
		const ScrollPanel::Blueprint& 		scrollPanelY() const override;
		const ScrollPanel::Blueprint& 		scrollPanelXY() const override;

		const DrawerPanel::Blueprint&		treeListDrawer() const override;
		const PaddingCapsule::Blueprint&	treeListEntry() const override;

		const TextEditor::Blueprint&		textEditor() const override;


		const TextDisplay::Blueprint& 		windowTitleBar() const override;



	protected:
		Simplistic( Font * pNormal, Font * pBold, Font * pItalic, Font * pMonospace, Surface * pWidgets );
		~Simplistic() {}

		const static int	c_textSizeSmallest = 8;
		const static int	c_textSizeSmall = 10;
		const static int	c_textSizeNormal = 12;
		const static int	c_textSizeLarge = 16;
		const static int	c_textSizeLargest = 20;



		Font_p		m_pFontNormal;
		Font_p		m_pFontBold;
		Font_p		m_pFontItalic;
		Font_p		m_pFontMonospace;

		TextStyle_p	m_heading1Style;
		TextStyle_p	m_heading2Style;
		TextStyle_p	m_heading3Style;
		TextStyle_p	m_heading4Style;
		TextStyle_p	m_heading5Style;
		TextStyle_p	m_heading6Style;

		TextStyle_p m_pStrongStyle;
		TextStyle_p m_pEmphasisStyle;
		TextStyle_p m_pCodeStyle;
		TextStyle_p m_pMonospaceStyle;
		TextStyle_p m_pFinePrintStyle;


		TextStyle_p	m_pBlackStyle;
		TextStyle_p	m_pWhiteStyle;


		Skin_p		m_pPlateSkin;
		Skin_p		m_pCanvasSkin;
		Skin_p		m_pWindowSkin;

		ValueTransition_p		m_pOpenCloseTransition;

		LabelCapsule::Blueprint m_labeledBoxBP;
		LabelCapsule::Blueprint m_labeledSectionBP;

		Button::Blueprint		m_pushButtonBP;

		ToggleButton::Blueprint	m_toggleButtonBP;
		ToggleButton::Blueprint	m_radioButtonBP;
		ToggleButton::Blueprint	m_checkboxBP;

		ScrollPanel::Blueprint	m_scrollPanelXBP;
		ScrollPanel::Blueprint	m_scrollPanelYBP;
		ScrollPanel::Blueprint	m_scrollPanelXYBP;

		DrawerPanel::Blueprint	m_treeListDrawer;
		PaddingCapsule::Blueprint m_treeListEntry;

		TextEditor::Blueprint	m_textEditor;

		TextDisplay::Blueprint	m_windowTitleBar;
	};


} // namespace wg
#endif //WG_THEME_STEEL_N_RED_DOT_H
