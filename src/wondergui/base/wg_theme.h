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

#ifndef	WG_THEME_DOT_H
#define WG_THEME_DOT_H
#pragma once

#include <wg_pointers.h>

#include <wg_togglebutton.h>
#include <wg_labelcapsule.h>

#include <wg_button.h>
#include <wg_scrollpanel.h>
#include <wg_textdisplay.h>
#include <wg_drawerpanel.h>
#include <wg_paddingcapsule.h>
#include <wg_texteditor.h>


namespace wg
{

	class Theme;
	typedef	StrongPtr<Theme>	Theme_p;
	typedef	WeakPtr<Theme>		Theme_wp;


	class Theme : public Object
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _______________________________________________________

		// Text sizes

		virtual pts			textSizeSmallest() const = 0;
		virtual pts			textSizeSmall() const = 0;
		virtual pts			textSizeNormal() const = 0;
		virtual pts			textSizeLarge() const = 0;
		virtual pts			textSizeLargest() const = 0;

		// Fonts

		virtual Font_p		normalFont() const = 0;
		virtual Font_p		monospaceFont() const = 0;
		virtual Font_p		boldFont() const = 0;
		virtual Font_p		italicFont() const = 0;

		// Text Styles

		virtual TextStyle_p heading1Style() const = 0;
		virtual TextStyle_p heading2Style() const = 0;
		virtual TextStyle_p heading3Style() const = 0;
		virtual TextStyle_p heading4Style() const = 0;
		virtual TextStyle_p heading5Style() const = 0;
		virtual TextStyle_p heading6Style() const = 0;

		virtual TextStyle_p subheadStyle1() const = 0;		// Special paragraph or section below heading
		virtual TextStyle_p subheadStyle2() const = 0;
		virtual TextStyle_p subheadStyle3() const = 0;
		virtual TextStyle_p subheadStyle4() const = 0;
		virtual TextStyle_p subheadStyle5() const = 0;
		virtual TextStyle_p subheadStyle6() const = 0;

		virtual TextStyle_p paragraphStyle() const = 0;		// Paragraph of text

		virtual TextStyle_p strongStyle() const = 0;		// Usually bold
		virtual TextStyle_p emphasisStyle() const = 0;		// Usually italic
		virtual TextStyle_p codeStyle() const = 0;			// For snippets of code, should be monospaced.
		virtual TextStyle_p linkStyle() const = 0;			// For hyperlinks
		virtual TextStyle_p monospaceStyle() const = 0;		// For monospaced text


		virtual TextStyle_p quoteStyle() const = 0;			// Usually italic
		virtual TextStyle_p captionStyle() const = 0;		// Text under image or figure.
		virtual TextStyle_p calloutStyle() const = 0;		// Separate text section in magazine or such which highligt key passages.

		virtual TextStyle_p defaultStyle() const = 0;		// Default style for UI elements
		virtual TextStyle_p pressableStyle() const = 0;		// Default text style for buttons etc.

		// Useful skins

		virtual Skin_p		plateSkin() const = 0;			// Main backgroud element, with frame or bevel or such.
		virtual Skin_p		canvasSkin() const = 0;			// Background for texteditors and other work-fields.
		virtual Skin_p		windowSkin() const = 0;			// Background for window.


		// Standard widgets

		virtual const LabelCapsule::Blueprint&		labeledBox() const = 0;		// Section with label and a box around it
		virtual const LabelCapsule::Blueprint&		labeledSection() const = 0;	// Section with label and possibly a horizontal divider

		virtual const Button::Blueprint& 			pushButton() const = 0;

		virtual const ToggleButton::Blueprint& 		toggleButton() const = 0;
		virtual const ToggleButton::Blueprint& 		radioButton() const = 0;
		virtual const ToggleButton::Blueprint& 		checkbox() const = 0;

		virtual const ScrollPanel::Blueprint& 		scrollPanelX() const = 0;
		virtual const ScrollPanel::Blueprint& 		scrollPanelY() const = 0;
		virtual const ScrollPanel::Blueprint& 		scrollPanelXY() const = 0;

		virtual const DrawerPanel::Blueprint&		treeListDrawer() const = 0;
		virtual const PaddingCapsule::Blueprint&	treeListEntry() const = 0;

		virtual const TextEditor::Blueprint&		textEditor() const = 0;

		//

		virtual const TextDisplay::Blueprint& 		windowTitleBar() const = 0;


	protected:
		Theme() {};
		~Theme() {}


	};


} // namespace wg
#endif //WG_THEME_DOT_H
