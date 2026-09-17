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
#ifndef	WG_THEME_GLOSSYBLUE_DOT_H
#define	WG_THEME_GLOSSYBLUE_DOT_H
#pragma once

// GENERATED FILE -- do not hand-edit the two marked generated blocks (the
// colours in namespace Colors, and the skins further down); both are
// overwritten by `widgetkit-src/common/build.py pack glossyblue`. The colours
// come from the exported_colors section of palette.yaml -- some are measured
// straight off the rendered bitmaps, so they cannot drift from the art.
// Everything else in this file is hand-maintained; edit it directly.
//
// Source of truth: widgetkit-src/glossyblue/ (specs/, parts/, palette.yaml,
// header_shell.h -- this file's un-generated content is copied verbatim
// from header_shell.h). Companion image: resources/glossyblue_skinblocks.png,
// rendered at 2x pt density (see common/theme_lib.py) -- load it with
// Surface::Blueprint.scale = 128 for correct hidpi sizing.

#include <wg_base.h>				// Base::setDefaultStyle() / setDefaultTextLayout()
#include <wg_gfxbase.h>			// GfxBase::throwError() for the scale check in init()
#include <wg_font.h>
#include <wg_textstyle.h>
#include <wg_basictextlayout.h>
#include <wg_blockskin.h>
#include <wg_boxskin.h>
#include <wg_colorskin.h>
#include <wg_doubleskin.h>

#include <wg_labelcapsule.h>
#include <wg_paddingcapsule.h>
#include <wg_scrollcapsule.h>

#include <wg_splitpanel.h>
#include <wg_tablepanel.h>
#include <wg_drawerpanel.h>

#include <wg_button.h>
#include <wg_togglebutton.h>
#include <wg_lineeditor.h>
#include <wg_texteditor.h>
#include <wg_textdisplay.h>
#include <wg_selectbox.h>


namespace wg::glossyblue
{
	namespace Colors
	{
		// >>> BEGIN GENERATED COLORS <<<
		// Mid-tones of the corresponding gradient bitmaps, measured at pack time,
		// so a flat surface (the capsule label skins, app code) sits flush against
		// a real Plate or Canvas. Border needs no such treatment: a bitmap's
		// outline is a flat colour already.
		inline const Color	Plate = Color(208,215,224);		// measured: centre pixel of parts/Plate.png
		inline const Color	Border = Color(140,148,160);		// palette: panel.border
		inline const Color	Canvas = Color(241,244,247);		// measured: centre pixel of parts/Canvas.png

		// The channel a scrollbar handle runs in: midway between Plate and the old
		// hardcoded Color(150,156,166). Plate alone made the track vanish into the
		// panel; the old value read as a dark trench cut into it.
		inline const Color	ScrollbarTrack = Color(186,192,201);
		inline const Color	Titlebar = Color(196,214,236);
		inline const Color	TitlebarBorder = Color(110,140,175);
		inline const Color	TitlebarSelected = Color(99,168,235);		// palette: raised.top
		inline const Color	TitlebarBorderSelected = Color(20,62,110);		// palette: raised.border

		// The theme's blue accent, for app code that wants to match it.
		inline const Color	Accent = Color(33,100,173);		// palette: raised.bottom
		inline const Color	AccentHovered = Color(48,124,200);		// palette: hover.bottom

		// Selected text: white on a saturated theme blue (between Accent and
		// AccentHovered). 5.6:1 contrast, and both colours are painted with
		// BlendMode::Replace so they do not depend on the widget's tint.
		inline const Color	TextSelection = Color(40,105,180);
		inline const Color	TextSelectionText = Color(255,255,255);
		// >>> END GENERATED COLORS <<<
	}

	namespace TextSizes
	{
		inline const int	Smallest = 8;
		inline const int	Small = 10;
		inline const int	Normal = 12;
		inline const int	Large = 16;
		inline const int	Largest = 20;
	}

	namespace Fonts
	{
		inline Font_p		Normal;
		inline Font_p		Bold;
		inline Font_p		Italic;
		inline Font_p		Mono;
	}

	namespace TextStyles
	{
		inline TextStyle_p	Heading1;
		inline TextStyle_p	Heading2;
		inline TextStyle_p	Heading3;
		inline TextStyle_p	Heading4;
		inline TextStyle_p	Heading5;
		inline TextStyle_p	Heading6;

		inline TextStyle_p	Default;
		inline TextStyle_p	Strong;
		inline TextStyle_p	Emphasis;
		inline TextStyle_p	Code;
		inline TextStyle_p	Mono;
		inline TextStyle_p	FinePrint;

		inline TextStyle_p	NormalDark;
		inline TextStyle_p	NormalBright;

	}

	namespace TextLayouts
	{
		inline TextLayout_p	LeftNoWrap;
		inline TextLayout_p	CenteredNoWrap;
	}

	// Which skins carry an outer margin (BlockSkin _.spacing), so the widget
	// keeps its own distance from its neighbours without every caller setting
	// panel spacing. Set per-spec in widgetkit-src/glossyblue/specs/.
	//
	//   1pt on every side -- Button, ToggleButton, SelectBox, Field.
	//     Free-standing controls that get lined up in a PackPanel; two
	//     neighbours end up 2pts apart. A caller who wants them welded into a
	//     segmented strip can still override the skin.
	//
	//   none -- everything else, on purpose:
	//     Plate / Canvas / Window / Titlebar   backdrops, must reach their edge
	//     PlateNoBevel                         table corner, abuts the scrollbars
	//     SelectBoxEntry                       list rows must touch
	//     ScrollbarTrack / Handle* / Button*   the Scroller lays these out
	//                                          adjacent; a margin would leave
	//                                          gaps in the bar
	//     SplitHandleX / Y                     fills the drag gap exactly
	//     Checkbox / RadioButton               these are ICON skins, not widget
	//                                          skins; Icon::spacing sets the gap
	//                                          to the label
	//
	// Spacing STACKS with the padding of the panel a widget sits in, so the
	// container paddings came down by 1 when spacing was introduced: Plate
	// 5 -> 4, Window 6 -> 5, PlateNoBevel 4 -> 3, and the capsule skins'
	// content sides 4 -> 3. A control is therefore the same distance from its
	// panel's edge as it was before spacing existed, and 2pts from its
	// neighbour instead of 0.
	//
	// Skins::Field is Skins::Canvas plus that margin -- literally the same
	// atlas block (see `blocks_from` in specs/Field.yaml). They are separate
	// entries because Canvas is also the SelectBox drop-down's background,
	// where a margin would inset the list inside the popup.
	namespace Skins
	{
		inline Skin_p		Plate;
		inline Skin_p		PlateNoBevel;
		inline Skin_p		Canvas;
		inline Skin_p		Field;		// Canvas with an outer margin, for LineEditor/TextEditor
		inline Skin_p		Window;
		inline Skin_p		Titlebar;
		inline Skin_p		Button;
		inline Skin_p		ToggleButton;
		inline Skin_p		Checkbox;
		inline Skin_p		RadioButton;
		inline Skin_p		SelectBox;
		inline Skin_p		SelectBoxEntry;
		inline Skin_p		ScrollbarTrack;
		inline Skin_p		ScrollbarHandleX;
		inline Skin_p		ScrollbarHandleY;
		inline Skin_p		ScrollbarButtonUp;
		inline Skin_p		ScrollbarButtonDown;
		inline Skin_p		ScrollbarButtonLeft;
		inline Skin_p		ScrollbarButtonRight;
		inline Skin_p		SplitHandleX;		// vertical bar   (SplitPanelX, children side by side)
		inline Skin_p		SplitHandleY;		// horizontal bar (SplitPanelY, children stacked)
	}

	namespace Transitions
	{
		inline ValueTransition_p	openClose;
	}

	inline Skin_p		_pLabelCapsuleSkin;
	inline Skin_p		_pCapsuleLabelSkin;
	inline Skin_p		_pCapsuleLabelSkin2;
	inline Skin_p		_pInvisibleBoxSkin;
	inline Skin_p		_pPlusMinusToggleSkin;
	inline Skin_p		_pSelectableEntrySkin;

	// The two layers of each split handle. Skins::SplitHandleX/Y are
	// DoubleSkins built from these just after the generated block below.
	inline Skin_p		_pSplitHandleXBarSkin;
	inline Skin_p		_pSplitHandleXDotsSkin;
	inline Skin_p		_pSplitHandleYBarSkin;
	inline Skin_p		_pSplitHandleYDotsSkin;


	inline bool isInitialized()
	{
		return Fonts::Normal != nullptr;
	}


	//____ init() ______________________________________________________________
	//
	// IMPORTANT: pSkinBlocks MUST have been created with a scale of 128.
	//
	// glossyblue_skinblocks.png is a 2x-density image: every coordinate in the
	// generated Skins:: section below is in pts at 1pt = 2px. BlockSkin resolves
	// those pts against the SURFACE's scale (wg_blockskin.cpp does
	// roundToPixels(ptsToSpx(_firstBlock, pSurface->scale()))), and
	// Surface::Blueprint.scale defaults to 0, which Surface resolves to 64 (1x).
	// A PNG carries no density information, so nothing can infer this for you.
	//
	// Loaded at the default scale, every block is read at half its size from
	// half its offset -- the widgets do not fail, they just render as garbage
	// sampled from the wrong part of the atlas. That is a miserable thing to
	// debug, so refuse it here instead:
	//
	//     auto pSkinBlocks = SomeSurface::create( WGBP(Surface,
	//         _.size  = atlasSize,        // whatever the PNG actually is
	//         _.scale = 128,              // <-- REQUIRED, 2x density atlas
	//         _.format = PixelFormat::BGRA_8 ), pPixels, ... );
	//
	inline bool init(Font* pNormal, Font* pBold, Font* pItalic, Font* pMonospace, Surface* pSkinBlocks )
	{
		if( !pNormal || !pBold || !pItalic || !pMonospace || !pSkinBlocks )
			return false;

		if( pSkinBlocks->scale() != 128 )
		{
			GfxBase::throwError( ErrorLevel::Error, ErrorCode::InvalidParam,
				"glossyblue: skinblocks surface must be created with scale = 128. "
				"It is a 2x-density atlas, and at any other scale every block is "
				"sampled from the wrong place. Add _.scale = 128 to its Surface blueprint.",
				nullptr, nullptr, __func__, __FILE__, __LINE__ );
			return false;
		}


		Fonts::Normal	= pNormal;
		Fonts::Bold		= pBold;
		Fonts::Italic	= pItalic;
		Fonts::Mono		= pMonospace;

		TextStyles::Strong		= TextStyle::create(WGBP(TextStyle, _.color = HiColor::Black, _.font = Fonts::Bold, _.size = TextSizes::Normal ));
		TextStyles::Emphasis	= TextStyle::create(WGBP(TextStyle, _.color = HiColor::Black, _.font = Fonts::Italic, _.size = TextSizes::Normal ));
		TextStyles::Code		= TextStyle::create(WGBP(TextStyle, _.font = Fonts::Mono, _.color = HiColor::Black, _.size = TextSizes::Normal));
		TextStyles::Mono		= TextStyle::create(WGBP(TextStyle, _.font = Fonts::Mono, _.color = HiColor::Black, _.size = TextSizes::Normal));
		TextStyles::FinePrint	= TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::Black, _.size = 9));

		TextStyles::NormalDark	= TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::Black, _.size = TextSizes::Normal,
									_.states = { {State::Disabled, Color8::DarkGrey} }));

		TextStyles::NormalBright = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::White, _.size = TextSizes::Normal,
								_.states = { {State::Disabled, Color8::LightGrey} }));

		TextStyles::Default = TextStyles::NormalDark;

		TextStyles::Heading1 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::Black, _.size = 20));
		TextStyles::Heading2 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Bold, _.color = HiColor::Black, _.size = 20));
		TextStyles::Heading3 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::Black, _.size = 16));
		TextStyles::Heading4 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Bold, _.color = HiColor::Black, _.size = 16));
		TextStyles::Heading5 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Bold, _.color = HiColor::Black, _.size = 14));
		TextStyles::Heading6 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::Black, _.size = 14));

		// BasicTextLayout's selection defaults are selectionBackBlend =
		// BlendMode::Invert and selectionCharBlend = BlendMode::Invert, which
		// literally inverts whatever is behind the selection -- on a light
		// canvas that comes out a muddy dark grey.
		//
		// Both are Replace, deliberately. Replace is "completely opaque,
		// ignoring alpha of source and TINT-COLOR", so the selection fill
		// lands as exactly TextSelection and the glyphs as exactly
		// TextSelectionText, whatever tint the editor happens to have set.
		// BlendMode::Blend would modulate the fill by that tint, and
		// BlendMode::Ignore on the chars would hand back the text's own
		// colour (HiColor::blend returns baseColor for it) -- leaving
		// selected glyphs rendered identically to unselected ones.
		TextLayouts::LeftNoWrap = BasicTextLayout::create(WGBP(BasicTextLayout,
			_.autoEllipsis = true,
			_.placement = Placement::West,
			_.selectionBackBlend = BlendMode::Replace,
			_.selectionBackColor = Colors::TextSelection,
			_.selectionCharBlend = BlendMode::Replace,
			_.selectionCharColor = Colors::TextSelectionText,
			_.wrap = false));

		TextLayouts::CenteredNoWrap = BasicTextLayout::create(WGBP(BasicTextLayout,
			_.autoEllipsis = true,
			_.placement = Placement::Center,
			_.selectionBackBlend = BlendMode::Replace,
			_.selectionBackColor = Colors::TextSelection,
			_.selectionCharBlend = BlendMode::Replace,
			_.selectionCharColor = Colors::TextSelectionText,
			_.wrap = false));

		// Give StaticText's fallback something to fall back TO.
		//
		// StaticText::_style() already reads
		//     if (m_pStyle) return m_pStyle; return Base::defaultStyle();
		// and _layout() does the same with Base::defaultTextLayout() -- but
		// nothing in WonderGUI ever calls the setters, so those defaults are
		// null. A text component that ends up without a style therefore has no
		// FONT, and draws nothing at all.
		//
		// That is a miserable failure mode, because the usual way to end up
		// without a style is not to omit the text but to OVERRIDE it: writing
		//     .label = { .text = "Save" }
		// builds a whole fresh DynamicText::Blueprint, silently discarding the
		// wrapper's default `.style` and `.layout` along with it. C++ has no
		// way to merge into an aggregate, so it is easy to do by accident and
		// gives no clue when you do -- the caption simply is not there.
		//
		// With these two lines the same mistake renders in the theme's default
		// style instead: visibly wrong rather than invisible. The wrappers
		// further down go on to restore their OWN intended style, so this is
		// the net beneath that -- and it also catches plain wg:: widgets built
		// without the kit's wrappers.
		Base::setDefaultStyle(TextStyles::Default);
		Base::setDefaultTextLayout(TextLayouts::LeftNoWrap);

		Transitions::openClose = ValueTransition::create(250000);

		// --- hand-maintained skins: not block-based, so not part of the
		//     generated atlas/pipeline. Edit these directly. -----------------

		Skins::Titlebar = BoxSkin::create(WGBP(BoxSkin,
			_.color = Colors::Titlebar,
			_.outlineColor = Colors::TitlebarBorder,
			_.outlineThickness = 1,
			_.padding = 4,
			_.states = { {State::Flagged,Colors::TitlebarSelected,Colors::TitlebarBorderSelected} }));

		_pLabelCapsuleSkin = BoxSkin::create(WGBP(BoxSkin,
			_.color = HiColor::Transparent,
			_.outlineColor = Colors::Border,
			_.outlineThickness = 1,
			_.spacing = { 8,2,2,2 },
			// 4 -> 3 on the content sides: a capsule holds controls that now
			// carry 1pt of their own spacing, and the two stack. The top stays
			// at 10 -- that is clearance for the capsule's label, not a
			// content margin, so nothing doubles up there.
			_.padding = { 10, 3, 3, 3 }));

		_pCapsuleLabelSkin = ColorSkin::create(WGBP(ColorSkin,
			_.color = Colors::Plate,
			_.padding = { 2,2,2,2 }
		));

		_pCapsuleLabelSkin2 = ColorSkin::create(WGBP(ColorSkin,
			_.color = HiColor::Transparent,
			_.padding = { 2,2,2,2 }
		));

		_pInvisibleBoxSkin = ColorSkin::create(WGBP(ColorSkin,
			_.color = HiColor::Transparent,
			_.spacing = { 6,2,2,2 },
			// 4 -> 3 on the content sides, as above; 16 is label clearance.
			_.padding = { 16, 3, 3, 3 }));

		_pSelectableEntrySkin = BoxSkin::create(WGBP(BoxSkin,
			_.markAlpha = 0,
			_.states = { {State::Default, Color::Transparent,Color::Transparent},
						 {State::Hovered, HiColor(Colors::Accent).withAlpha(700), HiColor(Colors::Accent).withAlpha(1400)},
						 {State::Selekted, HiColor(Colors::Accent).withAlpha(1400),HiColor(Colors::Accent).withAlpha(2400) }
			}
		));

		Skins::SelectBoxEntry = BoxSkin::create(WGBP(BoxSkin,
			// 5pts left, to line an entry's text up with the closed box's own.
			// The two are measured from the same origin -- the popup attaches to
			// the widget's geo, so its left edge and the widget's coincide -- but
			// they were reached by different routes: the closed box insets its
			// text by Skins::SelectBox's spacing (1) plus its padding (5) = 6pts,
			// while an entry only had Skins::Canvas's padding (1). Hence 1 + 5 = 6
			// here, and the text of the selected entry no longer jumps left by
			// 5pts as the list opens.
			//
			// Padding moves the TEXT only: BoxSkin fills and outlines the rect it
			// is given, and SelectBox gives it the list canvas's full content
			// width, so the hover/selection highlight still reaches both edges.
			// 5 on the right too, for symmetry -- the closed box's 18 is arrow
			// clearance, which the list has no use for. Top and bottom stay 0 so
			// row height is unchanged; the engine folds this padding into the
			// list's default width (entryDefault.w + listPadding.w), so nothing
			// gets truncated.
			_.padding = { 0, 5, 0, 5 },
			_.states = { {State::Default, Color::Transparent, Color::Transparent},
						 {State::Hovered, HiColor(Colors::Accent).withAlpha(700), HiColor(Colors::Accent).withAlpha(1400)},
						 {State::Selekted, HiColor(Colors::Accent).withAlpha(1400),HiColor(Colors::Accent).withAlpha(2400) }
			}
		));

		// Colors::ScrollbarTrack sits between Plate and the old hardcoded
		// Color(150,156,166), which had a near-black outline and read as a
		// dark trench rather than part of the panel it sits in. The outline is
		// Colors::Border like the other hand-written skins here -- this was
		// the only one not using the named constants, so it never followed the
		// theme as the palette changed.
		Skins::ScrollbarTrack = BoxSkin::create(WGBP(BoxSkin,
			_.color = Colors::ScrollbarTrack,
			_.outlineColor = Colors::Border,
			_.outlineThickness = 1,
			_.padding = 0));

		// --- generated skins: one BlockSkin::create() per widgetkit-src/glossyblue/parts/*
		//     entry, rebuilt every `build.py pack glossyblue`. Do not hand-edit
		//     below this point until the END marker -- change the spec in
		//     specs/, or hand-touch-up the rendered part in parts/, instead. ---

		// >>> BEGIN GENERATED SKINS <<<
		Skins::Button = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 133,22,14,20 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = 6,
			_.padding = 6,
			_.spacing = 1,
			_.states = { State::Default, State::Hovered, {State::Pressed, Coord(1,1)}, State::Disabled }
		));

		Skins::Canvas = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 195,37,12,12 },
			_.frame = 4,
			_.padding = 1
		));

		Skins::Checkbox = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 45,80,14,14 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Checked, State::CheckedHovered, State::CheckedPressed, State::Disabled, State::DisabledChecked }
		));

		Skins::Plate = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 115,27,14,14 },
			_.frame = 5,
			_.padding = 4
		));

		Skins::PlateNoBevel = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 208,1,4,14 },
			_.frame = { 5,1,5,1 },
			_.padding = 3
		));

		_pPlusMinusToggleSkin = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 123,95,12,12 },
			_.axis = Axis::X,
			_.blockSpacing = 2,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Checked, State::CheckedHovered, State::CheckedPressed }
		));

		Skins::RadioButton = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 18,95,14,14 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Checked, State::CheckedHovered, State::CheckedPressed, State::Disabled, State::DisabledChecked }
		));

		Skins::ScrollbarButtonDown = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 45,1,16,16 },
			_.axis = Axis::Y,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::ScrollbarButtonLeft = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 79,63,16,16 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::ScrollbarButtonRight = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 147,63,16,16 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::ScrollbarButtonUp = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 62,1,16,16 },
			_.axis = Axis::Y,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::ScrollbarHandleX = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 79,46,28,16 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = { 8,7,8,7 },
			_.padding = 6,
			_.rigidPartX = {8,12,YSections::All},
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::ScrollbarHandleY = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 1,1,16,28 },
			_.axis = Axis::Y,
			_.blockSpacing = 1,
			_.frame = { 7,8,7,8 },
			_.padding = 6,
			_.rigidPartY = {8,12,XSections::All},
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::SelectBox = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 18,1,26,20 },
			_.axis = Axis::Y,
			_.blockSpacing = 1,
			_.frame = { 5,18,5,5 },
			_.padding = { 3,18,3,5 },
			_.spacing = 1,
			_.rigidPartY = {5.5,9,XSections::Right},
			_.states = { State::Default, State::Hovered, {State::Pressed, Coord(1,1)}, State::Disabled }
		));

		_pSplitHandleXBarSkin = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 79,1,8,44 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = { 8,2,8,2 },
			_.padding = 2,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		_pSplitHandleXDotsSkin = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 115,1,8,25 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = 0,
			_.padding = 0,
			_.rigidPartX = {1,6,YSections::All},
			_.rigidPartY = {1,23,XSections::All},
			_.states = { State::Default, State::Disabled }
		));

		_pSplitHandleYBarSkin = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 18,110,44,8 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = { 2,8,2,8 },
			_.padding = 2,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		_pSplitHandleYDotsSkin = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 165,80,25,8 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = 0,
			_.padding = 0,
			_.rigidPartY = {1,6,XSections::All},
			_.rigidPartX = {1,23,YSections::All},
			_.states = { State::Default, State::Disabled }
		));

		Skins::ToggleButton = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 133,1,14,20 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = 6,
			_.padding = 6,
			_.spacing = 1,
			_.states = { State::Default, State::Hovered, {State::Checked, Coord(1,1)}, {State::CheckedHovered, Coord(1,1)}, State::Disabled }
		));

		Skins::Window = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 193,22,14,14 },
			_.frame = 5,
			_.padding = 5
		));

		Skins::Field = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 195,37,12,12 },
			_.frame = 4,
			_.padding = 2,
			_.spacing = 1
		));
		// >>> END GENERATED SKINS <<<

		// --- split handles: grey bar + dot overlay ------------------------
		// The dots are a separate, fully transparent BlockSkin laid over the
		// bar rather than part of its bitmap. Baked into the bar they had to
		// fit inside a rigid part carved out of the bar's own middle band --
		// on an 8pt bar that band is 4pts and a rigid run may use at most 2 of
		// them, which capped the dots at 2pts. The overlay has no border and no
		// corners, so it needs no frame: its whole block is the centre section,
		// its rigid parts pin the dots on both axes, and they are now 3.5pts.
		//
		// skinInSkin is false so the front layer gets the whole canvas rather
		// than the back layer's content rect, and the padding is stated here
		// instead of being inherited from whichever layer happens to be front.
		Skins::SplitHandleX = DoubleSkin::create(WGBP(DoubleSkin,
			_.skins[0] = _pSplitHandleXDotsSkin,		// front
			_.skins[1] = _pSplitHandleXBarSkin,			// back
			_.skinInSkin = false,
			_.padding = 2 ));

		Skins::SplitHandleY = DoubleSkin::create(WGBP(DoubleSkin,
			_.skins[0] = _pSplitHandleYDotsSkin,		// front
			_.skins[1] = _pSplitHandleYBarSkin,			// back
			_.skinInSkin = false,
			_.padding = 2 ));

		return true;
	}

	inline bool exit()
	{
		Fonts::Normal = nullptr;
		Fonts::Bold = nullptr;
		Fonts::Italic = nullptr;
		Fonts::Mono = nullptr;

		TextStyles::Heading1 = nullptr;
		TextStyles::Heading2 = nullptr;
		TextStyles::Heading3 = nullptr;
		TextStyles::Heading4 = nullptr;
		TextStyles::Heading5 = nullptr;
		TextStyles::Heading6 = nullptr;
		TextStyles::Default = nullptr;
		TextStyles::Strong = nullptr;
		TextStyles::Emphasis = nullptr;
		TextStyles::Code = nullptr;
		TextStyles::Mono = nullptr;
		TextStyles::FinePrint = nullptr;
		TextStyles::NormalDark = nullptr;
		TextStyles::NormalBright = nullptr;


		Base::setDefaultStyle(nullptr);
		Base::setDefaultTextLayout(nullptr);

		TextLayouts::LeftNoWrap = nullptr;
		TextLayouts::CenteredNoWrap = nullptr;

		Skins::Plate = nullptr;
		Skins::PlateNoBevel = nullptr;
		Skins::Canvas = nullptr;
		Skins::Field = nullptr;
		Skins::Window = nullptr;
		Skins::Titlebar = nullptr;
		Skins::Button = nullptr;
		Skins::ToggleButton = nullptr;
		Skins::Checkbox = nullptr;
		Skins::RadioButton = nullptr;
		Skins::SelectBox = nullptr;
		Skins::SelectBoxEntry = nullptr;
		Skins::ScrollbarTrack = nullptr;
		Skins::ScrollbarHandleX = nullptr;
		Skins::ScrollbarHandleY = nullptr;
		Skins::ScrollbarButtonUp = nullptr;
		Skins::ScrollbarButtonDown = nullptr;
		Skins::ScrollbarButtonLeft = nullptr;
		Skins::ScrollbarButtonRight = nullptr;
		Skins::SplitHandleX = nullptr;
		Skins::SplitHandleY = nullptr;

		Transitions::openClose = nullptr;

		_pLabelCapsuleSkin = nullptr;
		_pCapsuleLabelSkin = nullptr;
		_pCapsuleLabelSkin2 = nullptr;
		_pInvisibleBoxSkin = nullptr;
		_pPlusMinusToggleSkin = nullptr;
		_pSelectableEntrySkin = nullptr;

		_pSplitHandleXBarSkin = nullptr;
		_pSplitHandleXDotsSkin = nullptr;
		_pSplitHandleYBarSkin = nullptr;
		_pSplitHandleYDotsSkin = nullptr;

		return true;
	}

	// --- A note on the constructors below -------------------------------
	//
	// Each fills in any text style or layout the caller left null with the
	// value its own Blueprint declares as the default. That looks redundant --
	// the Blueprint already says `_.style = TextStyles::NormalBright` -- but it
	// is not, because a nested Blueprint is replaced wholesale, never merged:
	//
	//     wkit::Button::create({ .label = { .text = "Save" } })
	//
	// constructs a fresh DynamicText::Blueprint whose `style` and `layout` are
	// null, and the default is gone. Nothing warns; the button just has no
	// caption, because a text component with no style has no font.
	//
	// Filling the gaps here turns that from "replace" into "merge", so naming
	// one field of a nested blueprint keeps the rest. A caller who genuinely
	// wants a different style still passes one, and it is respected.
	// --------------------------------------------------------------------

	//____ Button _______________________________________________________
	// NOTE: label defaults to NormalBright (white text) since the Button skin
	// is a saturated blue fill -- NormalDark (black text) would be low-contrast.

	class Button : public wg::Button
	{
	public:
		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			Icon::Blueprint	icon;
			int				id = 0;
			DynamicText::Blueprint label = WGBP(DynamicText, _.layout = TextLayouts::CenteredNoWrap, _.style = TextStyles::NormalBright );
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			bool			selectOnPress = false;
			Skin_p			skin = Skins::Button;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::Button_p	create() { return new Button(Blueprint()); }
		inline static wg::Button_p	create(const Blueprint& blueprint) { return new Button(blueprint); }

	protected:

		Button(const Blueprint& bp) : wg::Button(bp)
		{
			if( !label.style() )
				label.setStyle(TextStyles::NormalBright);
			if( !label.layout() )
				label.setLayout(TextLayouts::CenteredNoWrap);
		}

	};

	//____ ToggleButton _______________________________________________________

	class ToggleButton : public wg::ToggleButton
	{
	public:

		struct Blueprint
		{
			Object_p		baggage;
			bool			checked = false;
			ClickArea		clickArea = ClickArea::Default;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			bool			flipOnRelease = false;
			Icon::Blueprint	icon;
			int				id = 0;
			DynamicText::Blueprint label = WGBP(DynamicText, _.layout = TextLayouts::CenteredNoWrap, _.style = TextStyles::NormalBright );
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin = Skins::ToggleButton;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::ToggleButton_p	create() { return new ToggleButton(Blueprint()); }
		inline static wg::ToggleButton_p	create(const Blueprint& blueprint) { return new ToggleButton(blueprint); }

	protected:

		ToggleButton(const Blueprint& bp) : wg::ToggleButton(bp)
		{
			if( !label.style() )
				label.setStyle(TextStyles::NormalBright);
			if( !label.layout() )
				label.setLayout(TextLayouts::CenteredNoWrap);
		}
	};

	//____ Checkbox ______________________________________________________

	class Checkbox : public wg::ToggleButton
	{
	public:

		struct Blueprint
		{
			Object_p		baggage;
			bool			checked = false;
			ClickArea		clickArea = ClickArea::Default;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			bool			flipOnRelease = false;
			Icon::Blueprint	icon = WGBP(Icon, _.skin = Skins::Checkbox, _.spacing = 4 );
			int				id = 0;
			DynamicText::Blueprint label = WGBP(DynamicText, _.layout = TextLayouts::LeftNoWrap, _.style = TextStyles::NormalDark );
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::ToggleButton_p	create() { return new Checkbox(Blueprint()); }
		inline static wg::ToggleButton_p	create(const Blueprint& blueprint) { return new Checkbox(blueprint); }

	protected:

		Checkbox(const Blueprint& bp) : wg::ToggleButton(bp)
		{
			if( !label.style() )
				label.setStyle(TextStyles::NormalDark);
			if( !label.layout() )
				label.setLayout(TextLayouts::LeftNoWrap);
		}

	};

	//____ RadioButton ______________________________________________________

	class RadioButton : public wg::ToggleButton
	{
	public:
		struct Blueprint
		{
			Object_p		baggage;
			bool			checked = false;
			ClickArea		clickArea = ClickArea::Default;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			bool			flipOnRelease = false;
			// Skins::RadioButton is the ICON -- the little dot -- exactly as
			// Skins::Checkbox is for Checkbox above. It used to be set as the
			// widget's `skin` instead, which stretched a 14x14 frameless
			// ellipse across the whole widget: a squashed oval sitting behind
			// the label rather than a radio button beside it.
			Icon::Blueprint	icon = WGBP(Icon, _.skin = Skins::RadioButton, _.spacing = 4 );
			int				id = 0;
			DynamicText::Blueprint label = WGBP(DynamicText, _.layout = TextLayouts::LeftNoWrap, _.style = TextStyles::NormalDark );
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::ToggleButton_p	create(const Blueprint& blueprint) { return new RadioButton(blueprint); }

	protected:

		RadioButton(const Blueprint& bp) : wg::ToggleButton(bp)
		{
			if( !label.style() )
				label.setStyle(TextStyles::NormalDark);
			if( !label.layout() )
				label.setLayout(TextLayouts::LeftNoWrap);
		}

	};

	//____ LabelAndFrameCapsule ______________________________________________________

	class LabelAndFrameCapsule : public wg::LabelCapsule
	{
	public:

		struct Blueprint
		{
			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = true;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			DynamicText::Blueprint	label = WGBP(DynamicText, _.layout = TextLayouts::LeftNoWrap, _.style = TextStyles::NormalDark );
			Placement		labelPlacement = Placement::North;
			Skin_p			labelSkin = _pCapsuleLabelSkin;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = true;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin = _pLabelCapsuleSkin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			bool			takesFocusFromChild = true;
			String			tooltip;
			bool			usePickHandles = false;

		};

		inline static wg::LabelCapsule_p	create() { return new LabelAndFrameCapsule(Blueprint()); }
		inline static wg::LabelCapsule_p	create(const Blueprint& blueprint) { return new LabelAndFrameCapsule(blueprint); }

	protected:

		LabelAndFrameCapsule(const Blueprint& bp) : wg::LabelCapsule(bp)
		{
			if( !label.style() )
				label.setStyle(TextStyles::NormalDark);
			if( !label.layout() )
				label.setLayout(TextLayouts::LeftNoWrap);
		}

	};

	//____ LabeledSectionCapsule ______________________________________________________

	class LabeledSectionCapsule : public wg::LabelCapsule
	{
	public:

		struct Blueprint
		{
			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = true;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			DynamicText::Blueprint	label = WGBP(DynamicText, _.layout = TextLayouts::LeftNoWrap, _.style = TextStyles::NormalDark );
			Placement		labelPlacement = Placement::North;
			Skin_p			labelSkin = _pCapsuleLabelSkin2;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = true;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin = _pInvisibleBoxSkin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			bool			takesFocusFromChild = true;
			String			tooltip;
			bool			usePickHandles = false;

		};

		inline static wg::LabelCapsule_p	create() { return new LabeledSectionCapsule(Blueprint()); }
		inline static wg::LabelCapsule_p	create(const Blueprint& blueprint) { return new LabeledSectionCapsule(blueprint); }

	protected:

		LabeledSectionCapsule(const Blueprint& bp) : wg::LabelCapsule(bp)
		{
			if( !label.style() )
				label.setStyle(TextStyles::NormalDark);
			if( !label.layout() )
				label.setLayout(TextLayouts::LeftNoWrap);
		}

	};

	//____ ScrollCapsuleX _____________________________________________________

	class ScrollCapsuleX : public wg::ScrollCapsule
	{
	public:

		struct Blueprint
		{
			bool				autoHideScrollbars = true;
			Axis				autoScrollAxis = Axis::Undefined;
			Object_p			baggage;
			Widget_p			child;
			Skin_p				cornerSkin = Skins::PlateNoBevel;
			bool				disabled = false;
			bool				dropTarget = false;
			Finalizer_p			finalizer = nullptr;
			int					id = 0;
			MarkPolicy			markPolicy = MarkPolicy::AlphaTest;
			bool				overlayScrollbars = false;
			pts					pageOverlapX = 8;
			pts					pageOverlapY = 8;
			bool				pickable = false;
			uint8_t				pickCategory = 0;
			bool				pickHandle = false;
			PointerStyle		pointer = PointerStyle::Undefined;
			Scroller::Blueprint	scrollbarX = WGBP(Scroller, _.back = Skins::ScrollbarTrack,
															_.backwardButton = Skins::ScrollbarButtonLeft,
															_.bar = Skins::ScrollbarHandleX,
															_.forwardButton = Skins::ScrollbarButtonRight);
			Scroller::Blueprint	scrollbarY = WGBP(Scroller, _.back = Skins::ScrollbarTrack,
															_.backwardButton = Skins::ScrollbarButtonUp,
															_.bar = Skins::ScrollbarHandleY,
															_.forwardButton = Skins::ScrollbarButtonDown );
			bool				scrollX = true;
			bool				scrollY = false;
			bool				selectable = false;
			Skin_p				skin;
			pts					stepSizeX = 8;
			pts					stepSizeY = 8;
			bool				stickyFocus = false;
			bool				tabLock = false;
			bool				takesFocusFromChild = true;
			String				tooltip;
			CoordTransition_p	transition;
			bool				usePickHandles = false;

			Axis				wheelAxis = Axis::Y;						// Scroll direction of primary mouse wheel. Secondary mouse wheel is the oposite.
			ModKeys				wheelAxisModifier = ModKeys::Shift;		// Flips the scroll direction of primary mouse wheel.
			ModKeys				wheelAccelerator = ModKeys::Alt;
			int					wheelAccelFactor = 5;

			bool				wheelFollowsScrollbar = true;

			pts					wheelStepSizeX = 16;
			pts					wheelStepSizeY = 16;
		};

		inline static wg::ScrollCapsule_p	create() { return new ScrollCapsuleX(Blueprint()); }
		inline static wg::ScrollCapsule_p	create(const Blueprint& blueprint) { return new ScrollCapsuleX(blueprint); }

	protected:

		ScrollCapsuleX(const Blueprint& bp) : wg::ScrollCapsule(bp) {}
	};

	//____ ScrollCapsuleY ____________________________________________________________

	class ScrollCapsuleY : public wg::ScrollCapsule
	{
	public:

		//____ Blueprint ____________________________________________________________

		struct Blueprint
		{
			bool				autoHideScrollbars = true;
			Axis				autoScrollAxis = Axis::Undefined;
			Object_p			baggage;
			Widget_p			child;
			Skin_p				cornerSkin = Skins::PlateNoBevel;
			bool				disabled = false;
			bool				dropTarget = false;
			Finalizer_p			finalizer = nullptr;
			int					id = 0;
			MarkPolicy			markPolicy = MarkPolicy::AlphaTest;
			bool				overlayScrollbars = false;
			pts					pageOverlapX = 8;
			pts					pageOverlapY = 8;
			bool				pickable = false;
			uint8_t				pickCategory = 0;
			bool				pickHandle = false;
			PointerStyle		pointer = PointerStyle::Undefined;
			Scroller::Blueprint	scrollbarX = WGBP(Scroller, _.back = Skins::ScrollbarTrack,
															_.backwardButton = Skins::ScrollbarButtonLeft,
															_.bar = Skins::ScrollbarHandleX,
															_.forwardButton = Skins::ScrollbarButtonRight);
			Scroller::Blueprint	scrollbarY = WGBP(Scroller, _.back = Skins::ScrollbarTrack,
															_.backwardButton = Skins::ScrollbarButtonUp,
															_.bar = Skins::ScrollbarHandleY,
															_.forwardButton = Skins::ScrollbarButtonDown );
			bool				scrollX = false;
			bool				scrollY = true;
			bool				selectable = false;
			Skin_p				skin;
			pts					stepSizeX = 8;
			pts					stepSizeY = 8;
			bool				stickyFocus = false;
			bool				tabLock = false;
			bool				takesFocusFromChild = true;
			String				tooltip;
			CoordTransition_p	transition;
			bool				usePickHandles = false;

			Axis				wheelAxis = Axis::Y;						// Scroll direction of primary mouse wheel. Secondary mouse wheel is the oposite.
			ModKeys				wheelAxisModifier = ModKeys::Shift;		// Flips the scroll direction of primary mouse wheel.
			ModKeys				wheelAccelerator = ModKeys::Alt;
			int					wheelAccelFactor = 5;

			bool				wheelFollowsScrollbar = true;

			pts					wheelStepSizeX = 16;
			pts					wheelStepSizeY = 16;
		};

		inline static wg::ScrollCapsule_p	create() { return new ScrollCapsuleY(Blueprint()); }
		inline static wg::ScrollCapsule_p	create(const Blueprint& blueprint) { return new ScrollCapsuleY(blueprint); }

	protected:

		ScrollCapsuleY(const Blueprint& bp) : wg::ScrollCapsule(bp) {}


	};

	//____ ScrollCapsuleXY ______________________________________________________

	class ScrollCapsuleXY : public wg::ScrollCapsule
	{
	public:

		struct Blueprint
		{
			bool				autoHideScrollbars = true;
			Axis				autoScrollAxis = Axis::Undefined;
			Object_p			baggage;
			Widget_p			child;
			Skin_p				cornerSkin = Skins::PlateNoBevel;
			bool				disabled = false;
			bool				dropTarget = false;
			Finalizer_p			finalizer = nullptr;
			int					id = 0;
			MarkPolicy			markPolicy = MarkPolicy::AlphaTest;
			bool				overlayScrollbars = false;
			pts					pageOverlapX = 8;
			pts					pageOverlapY = 8;
			bool				pickable = false;
			uint8_t				pickCategory = 0;
			bool				pickHandle = false;
			PointerStyle		pointer = PointerStyle::Undefined;
			Scroller::Blueprint	scrollbarX = WGBP(Scroller, _.back = Skins::ScrollbarTrack,
															_.backwardButton = Skins::ScrollbarButtonLeft,
															_.bar = Skins::ScrollbarHandleX,
															_.forwardButton = Skins::ScrollbarButtonRight);
			Scroller::Blueprint	scrollbarY = WGBP(Scroller, _.back = Skins::ScrollbarTrack,
															_.backwardButton = Skins::ScrollbarButtonUp,
															_.bar = Skins::ScrollbarHandleY,
															_.forwardButton = Skins::ScrollbarButtonDown );
			bool				scrollX = true;
			bool				scrollY = true;
			bool				selectable = false;
			Skin_p				skin;
			pts					stepSizeX = 8;
			pts					stepSizeY = 8;
			bool				stickyFocus = false;
			bool				tabLock = false;
			bool				takesFocusFromChild = true;
			String				tooltip;
			CoordTransition_p	transition;
			bool				usePickHandles = false;

			Axis				wheelAxis = Axis::Y;						// Scroll direction of primary mouse wheel. Secondary mouse wheel is the oposite.
			ModKeys				wheelAxisModifier = ModKeys::Shift;		// Flips the scroll direction of primary mouse wheel.
			ModKeys				wheelAccelerator = ModKeys::Alt;
			int					wheelAccelFactor = 5;

			bool				wheelFollowsScrollbar = true;

			pts					wheelStepSizeX = 16;
			pts					wheelStepSizeY = 16;
		};

		inline static wg::ScrollCapsule_p	create() { return new ScrollCapsuleXY(Blueprint()); }
		inline static wg::ScrollCapsule_p	create(const Blueprint& blueprint) { return new ScrollCapsuleXY(blueprint); }

	protected:

		ScrollCapsuleXY(const Blueprint& bp) : wg::ScrollCapsule(bp) {}


	};

	//____ SplitPanelX ____________________________________________________________

	class SplitPanelX : public wg::SplitPanel
	{
	public:
		struct Blueprint
		{
			Axis			axis = Axis::X;
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			Skin_p			handleSkin = Skins::SplitHandleX;
			pts				handleThickness = 0;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			MaskOp			maskOp = MaskOp::Recurse;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			float			resizeRatio = 0.5f;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			bool			takesFocusFromChild = false;
			String			tooltip;
			bool			usePickHandles = false;

		};

		inline static wg::SplitPanel_p	create() { return new SplitPanelX(Blueprint()); }
		inline static wg::SplitPanel_p	create(const Blueprint& blueprint) { return new SplitPanelX(blueprint); }

	protected:

		SplitPanelX(const Blueprint& bp) : wg::SplitPanel(bp)
		{}
	};

	//____ SplitPanelY ______________________________________________________

	class SplitPanelY : public wg::SplitPanel
	{
	public:
		struct Blueprint
		{
			Axis			axis = Axis::Y;
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			Skin_p			handleSkin = Skins::SplitHandleY;
			pts				handleThickness = 0;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			MaskOp			maskOp = MaskOp::Recurse;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			float			resizeRatio = 0.5f;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			bool			takesFocusFromChild = false;
			String			tooltip;
			bool			usePickHandles = false;

		};

		inline static wg::SplitPanel_p	create() { return new SplitPanelY(Blueprint()); }
		inline static wg::SplitPanel_p	create(const Blueprint& blueprint) { return new SplitPanelY(blueprint); }

	protected:

		SplitPanelY(const Blueprint& bp) : wg::SplitPanel(bp) {}
	};

	//____ TreeListDrawer ______________________________________________________

	class TreeListDrawer : public wg::DrawerPanel
	{
	public:

		struct Blueprint
		{
			Object_p			baggage;
			Coord				buttonOfs;
			Placement			buttonPlacement = Placement::West;
			Size				buttonSize = Size{ 14, 14 };
			Skin_p				buttonSkin = _pPlusMinusToggleSkin;
			bool				disabled = false;
			Direction			direction = Direction::Down;
			bool				dropTarget = false;
			Finalizer_p			finalizer = nullptr;
			int					id = 0;
			MarkPolicy			markPolicy = MarkPolicy::AlphaTest;
			MaskOp				maskOp = MaskOp::Recurse;
			bool				pickable = false;
			uint8_t				pickCategory = 0;
			bool				pickHandle = false;
			PointerStyle		pointer = PointerStyle::Undefined;
			bool				selectable = true;
			Skin_p				skin;
			bool				stickyFocus = false;
			bool				tabLock = false;
			bool				takesFocusFromChild = true;
			String				tooltip;
			ValueTransition_p	transition = Transitions::openClose;
			bool				usePickHandles = false;
		};

		inline static wg::DrawerPanel_p	create() { return new TreeListDrawer(Blueprint()); }
		inline static wg::DrawerPanel_p	create(const Blueprint& blueprint) { return new TreeListDrawer(blueprint); }

	protected:

		TreeListDrawer(const Blueprint& bp) : wg::DrawerPanel(bp) {}
	};

	//____ TreeListEntry ______________________________________________________

	class TreeListEntry : public wg::PaddingCapsule
	{
	public:

		struct Blueprint
		{

			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			Border			padding;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin = _pSelectableEntrySkin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			bool			takesFocusFromChild = true;
			String			tooltip;
			bool			usePickHandles = false;
		};

		inline static wg::PaddingCapsule_p	create() { return new TreeListEntry(Blueprint()); }
		inline static wg::PaddingCapsule_p	create(const Blueprint& blueprint) { return new TreeListEntry(blueprint); }

	protected:

		TreeListEntry(const Blueprint& bp) : wg::PaddingCapsule(bp) {}
	};

	//____ ListTable ______________________________________________________

	class ListTable : public wg::TablePanel
	{
	public:
		struct Blueprint
		{
			Object_p		baggage;

			PackLayout_p	columnLayout;
			int				columns = 2;

			pts				columnSpacing = 4;
			pts				columnSpacingAfter = 0;
			pts				columnSpacingBefore = 0;

			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::Undefined;
			MaskOp			maskOp = MaskOp::Recurse;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;

			PackLayout_p	rowLayout;
			int				rows = 2;

			Skin_p			rowSkin;
			Skin_p			rowSkin2;

			pts				rowSpacing = 1;
			pts				rowSpacingAfter = 0;
			pts				rowSpacingBefore = 0;

			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			bool			takesFocusFromChild = true;
			String			tooltip;
			bool			usePickHandles = false;

		};

		inline static wg::TablePanel_p	create() { return new ListTable(Blueprint()); }
		inline static wg::TablePanel_p	create(const Blueprint& blueprint) { return new ListTable(blueprint); }

	protected:

		ListTable(const Blueprint& bp) : wg::TablePanel(bp) {}
	};

	//____ TextEditor ______________________________________________________


	class TextEditor : public wg::TextEditor
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			EditableText::Blueprint	editor = WGBP(EditableText, _.style = TextStyles::NormalDark );
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			KeyAction		returnKeyAction = KeyAction::Insert;
			bool			selectable = false;
			Skin_p			skin = Skins::Field;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::TextEditor_p	create() { return new TextEditor(Blueprint()); }
		inline static wg::TextEditor_p	create(const Blueprint& blueprint) { return new TextEditor(blueprint); }

	protected:

		TextEditor(const Blueprint& bp) : wg::TextEditor(bp)
		{
			if( !editor.style() )
				editor.setStyle(TextStyles::NormalDark);
		}


	};


	//____ LineEditor ______________________________________________________

	class LineEditor : public wg::LineEditor
	{
	public:

		struct Blueprint
		{
			Object_p		baggage;
			spx				defaultLengthInChars = 20;		// Set to zero for returning default width calculated from actual text in field.
			bool			disabled = false;
			bool			dropTarget = false;
			EditableText::Blueprint	editor = WGBP(EditableText, _.style = TextStyles::NormalDark );
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Ibeam;
			KeyAction		returnKeyAction = KeyAction::ReleaseFocus;
			bool			selectable = false;
			Skin_p			skin = Skins::Field;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::LineEditor_p	create() { return new LineEditor(Blueprint()); }
		inline static wg::LineEditor_p	create(const Blueprint& blueprint) { return new LineEditor(blueprint); }

	protected:

		LineEditor(const Blueprint& bp) : wg::LineEditor(bp)
		{
			if( !editor.style() )
				editor.setStyle(TextStyles::NormalDark);
		}
	};

	//____ SelectBox ______________________________________________________

	class SelectBox : public wg::SelectBox
	{
	public:

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;

			// The CLOSED box. Separate from `entryTextStyle` below on purpose, and
			// NOT the same style: the closed box is a raised control in the blue
			// accent colour -- the same surface as a Button -- so its text is
			// white, while the drop-down list is drawn on Skins::Canvas and stays
			// dark. Two backgrounds, two styles. Sharing one, which the kit did
			// while it was working around the engine, left the closed box with
			// black text on blue.
			DynamicText::Blueprint display = WGBP(DynamicText, _.layout = TextLayouts::LeftNoWrap, _.style = TextStyles::NormalBright );

			bool			dropTarget = false;
			Skin_p			entrySkin = Skins::SelectBoxEntry;
			TextStyle_p		entryTextStyle = TextStyles::NormalDark;
			TextLayout_p	entryTextLayout;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			Skin_p			listSkin = Skins::Canvas;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin = Skins::SelectBox;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::SelectBox_p	create() { return new SelectBox(Blueprint()); }
		inline static wg::SelectBox_p	create(const Blueprint& blueprint) { return new SelectBox(blueprint); }

	protected:

		SelectBox(const Blueprint& bp) : wg::SelectBox(bp)
		{
			// Ordinary gap-filling now, exactly like the other wrappers: the base
			// class applies `bp.display` itself, so this only has to cover the
			// case where a caller replaced the nested blueprint with a designated
			// initializer and lost the kit's defaults with it.
			//
			// This used to be a workaround. wg::SelectBox had no way at all to
			// reach its own text component -- no Blueprint field, no init -- so
			// the closed box drew nothing however many entries it held, and the
			// kit had to add its own `textStyle` and apply it unconditionally.
			// The engine now carries `display`, so that is all gone.
			if( !display.style() )
				display.setStyle(TextStyles::NormalBright);
			if( !display.layout() )
				display.setLayout(TextLayouts::LeftNoWrap);
		}
	};

	//____ WindowTitleBar ______________________________________________________

	class WindowTitleBar : public wg::TextDisplay
	{
	public:

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			DynamicText::Blueprint	display = WGBP(DynamicText, _.layout = TextLayouts::CenteredNoWrap, _.style = TextStyles::Heading5 );
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin = Skins::Titlebar;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::TextDisplay_p	create() { return new WindowTitleBar(Blueprint()); }
		inline static wg::TextDisplay_p	create(const Blueprint& blueprint) { return new WindowTitleBar(blueprint); }

	protected:
		WindowTitleBar(const Blueprint& bp) : wg::TextDisplay(bp)
		{
			if( !display.style() )
				display.setStyle(TextStyles::Heading5);
			if( !display.layout() )
				display.setLayout(TextLayouts::CenteredNoWrap);
		}
	};

} // namespace wg::glossyblue
#endif //WG_THEME_GLOSSYBLUE_DOT_H
