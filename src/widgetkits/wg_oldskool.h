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
#ifndef	WG_THEME_OLDSKOOL_DOT_H
#define	WG_THEME_OLDSKOOL_DOT_H
#pragma once

#include <wg_font.h>
#include <wg_textstyle.h>
#include <wg_basictextlayout.h>
#include <wg_blockskin.h>
#include <wg_boxskin.h>
#include <wg_colorskin.h>

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


namespace wg::oldskool
{
	namespace Colors
	{
		inline const Color	Plate = Color::LightGrey;
		inline const Color	Border = Color::DarkGrey;

		inline const Color	Titlebar = Color::LightGrey;
		inline const Color	TitlebarBorder = Color::DarkGrey;

		inline const Color	TitlebarSelected = Color::LightCyan;
		inline const Color	TitlebarBorderSelected = Color::DarkCyan;
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

	namespace Skins
	{
		inline Skin_p		Plate;
		inline Skin_p		PlateNoBevel;
		inline Skin_p		Canvas;
		inline Skin_p		Window;
		inline Skin_p		Titlebar;
		inline Skin_p		Button;
		inline Skin_p		ToggleButton;
		inline Skin_p		Checkbox;
		inline Skin_p		RadioButton;
		inline Skin_p		SelectBox;
		inline Skin_p		SelectBoxEntry;
		inline Skin_p		ScrollbarTrack;
		inline Skin_p		ScrollbarHandle;
		inline Skin_p		SplitHandle;
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


	inline bool isInitialized()
	{
		return Fonts::Normal != nullptr;
	}


	inline bool init(Font* pNormal, Font* pBold, Font* pItalic, Font* pMonospace, Surface* pSkinBlocks ) 
	{ 
		if( !pNormal || !pBold || !pItalic || !pMonospace || !pSkinBlocks )
			return false;


		Fonts::Normal	= pNormal;
		Fonts::Bold		= pBold;
		Fonts::Italic	= pItalic;
		Fonts::Mono		= pMonospace;

		TextStyles::Strong		= TextStyle::create({ .color = HiColor::Black, .font = Fonts::Bold, .size = TextSizes::Normal });
		TextStyles::Emphasis	= TextStyle::create({ .color = HiColor::Black, .font = Fonts::Italic, .size = TextSizes::Normal });
		TextStyles::Code		= TextStyle::create(WGBP(TextStyle, _.font = Fonts::Mono, _.color = HiColor::Black, _.size = TextSizes::Normal));
		TextStyles::Mono		= TextStyle::create(WGBP(TextStyle, _.font = Fonts::Mono, _.color = HiColor::Black, _.size = TextSizes::Normal));
		TextStyles::FinePrint	= TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::Black, _.size = 9));

		TextStyles::NormalDark	= TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::Black, _.size = TextSizes::Normal,
									_.states = { {State::Disabled, Color8::DarkGrey} }));

		TextStyles::NormalBright = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::White, _.size = TextSizes::Normal,
								_.states = { {State::Disabled, Color8::LightGrey} }));

		TextStyles::Heading1 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::Black, _.size = 20));
		TextStyles::Heading2 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Bold, _.color = HiColor::Black, _.size = 20));
		TextStyles::Heading3 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::Black, _.size = 16));
		TextStyles::Heading4 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Bold, _.color = HiColor::Black, _.size = 16));
		TextStyles::Heading5 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Bold, _.color = HiColor::Black, _.size = 14));
		TextStyles::Heading6 = TextStyle::create(WGBP(TextStyle, _.font = Fonts::Normal, _.color = HiColor::Black, _.size = 14));

		TextLayouts::LeftNoWrap = BasicTextLayout::create(WGBP(BasicTextLayout,
			_.autoEllipsis = true,
			_.placement = Placement::West,
			_.wrap = false));

		TextLayouts::CenteredNoWrap = BasicTextLayout::create(WGBP(BasicTextLayout,
			_.autoEllipsis = true,
			_.placement = Placement::Center,
			_.wrap = false));

		Transitions::openClose = ValueTransition::create(250000);

		Skins::Plate = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 0,60,10,10 },
			_.padding = 3,
			_.frame = 3));

		Skins::PlateNoBevel = ColorSkin::create(WGBP(ColorSkin,
			_.color = Colors::Plate,
			_.padding = 3));

		Skins::Canvas = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 24,60,10,10 },
			_.padding = 3,
			_.frame = 3));

		Skins::Window = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 36,60,10,10 },
			_.padding = 5,
			_.frame = 3));


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
			_.padding = { 10, 4, 4, 4 }));

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
			_.padding = { 16, 4, 4, 4 }));

		_pPlusMinusToggleSkin = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 0,0,14,14 },
			_.axis = Axis::X,
			_.blockSpacing = 2,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Checked, State::CheckedHovered, State::CheckedPressed }));

		_pSelectableEntrySkin = BoxSkin::create(WGBP(BoxSkin,
			_.markAlpha = 0,
			_.states = { {State::Default, Color::Transparent,Color::Transparent},
						 {State::Hovered, HiColor(Color::LightCyan).withAlpha(1024), HiColor(Color::DarkCyan).withAlpha(1024)},
						 {State::Selekted, Color::LightCyan,Color::DarkCyan }
			}
		));

		Skins::SplitHandle = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 0,15,10,10 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = 4,
			_.padding = 4,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));


		Skins::Button = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 0,15,10,10 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = 4,
			_.padding = 4,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));


		Skins::ToggleButton = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 0,26,10,10 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = 4,
			_.padding = 4,
			_.states = { State::Default, State::Hovered, State::Checked, State::CheckedHovered, State::Disabled }
		));

		Skins::Checkbox = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 0,37,10,10 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Checked, State::Disabled, State::DisabledChecked }
		));

		Skins::RadioButton = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 0,48,10,10 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Checked, State::Disabled }
		));

		Skins::SelectBox = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 96,0,34,22 },
			_.frame = { 4,25,4,4 },
			_.padding = { 3, 25, 3, 4, },
			_.axis = Axis::Y,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::SelectBoxEntry = BoxSkin::create(WGBP(BoxSkin,
			_.states = { {State::Default, Color::Transparent, Color::Transparent},
						 {State::Hovered, HiColor(Color::LightCyan).withAlpha(2048), HiColor(Color::DarkCyan).withAlpha(2048)},
						 {State::Selekted, Color::LightCyan,Color::DarkCyan }
			}
		));

		Skins::ScrollbarTrack = BoxSkin::create(WGBP(BoxSkin,
			_.color = Color::DarkGray,
			_.outlineColor = Color::Black,
			_.outlineThickness = 1,
			_.padding = 2));

		Skins::ScrollbarHandle = BoxSkin::create(WGBP(BoxSkin,
			_.color = Color::LightGray,
			_.outlineColor = Color::Black,
			_.outlineThickness = 1,
			_.padding = 6));



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
		TextStyles::Strong = nullptr;
		TextStyles::Emphasis = nullptr;
		TextStyles::Code = nullptr;
		TextStyles::Mono = nullptr;
		TextStyles::FinePrint = nullptr;
		TextStyles::NormalDark = nullptr;
		TextStyles::NormalBright = nullptr;


		TextLayouts::LeftNoWrap = nullptr;
		TextLayouts::CenteredNoWrap = nullptr;

		Skins::Plate = nullptr;
		Skins::PlateNoBevel = nullptr;
		Skins::Canvas = nullptr;
		Skins::Window = nullptr;
		Skins::Titlebar = nullptr;
		Skins::Button = nullptr;
		Skins::ToggleButton = nullptr;
		Skins::Checkbox = nullptr;
		Skins::RadioButton = nullptr;
		Skins::SelectBox = nullptr;
		Skins::SelectBoxEntry = nullptr;

		Transitions::openClose = nullptr;

		_pLabelCapsuleSkin = nullptr;
		_pCapsuleLabelSkin = nullptr;
		_pCapsuleLabelSkin2 = nullptr;
		_pInvisibleBoxSkin = nullptr;
		_pPlusMinusToggleSkin = nullptr;
		_pSelectableEntrySkin = nullptr;

		return true; 
	}

	//____ Button _______________________________________________________

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
			DynamicText::Blueprint label = { .layout = TextLayouts::CenteredNoWrap, .style = TextStyles::NormalDark };
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
			DynamicText::Blueprint label = { .layout = TextLayouts::CenteredNoWrap, .style = TextStyles::NormalDark };
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
			Icon::Blueprint	icon = { .skin = Skins::Checkbox, .spacing = 4 };
			int				id = 0;
			DynamicText::Blueprint label = { .layout = TextLayouts::CenteredNoWrap, .style = TextStyles::NormalDark };
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
			Icon::Blueprint	icon;
			int				id = 0;
			DynamicText::Blueprint label = { .layout = TextLayouts::CenteredNoWrap, .style = TextStyles::NormalDark };
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin = Skins::RadioButton;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::ToggleButton_p	create(const Blueprint& blueprint) { return new RadioButton(blueprint); }

	protected:

		RadioButton(const Blueprint& bp) : wg::ToggleButton(bp)
		{
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
			DynamicText::Blueprint	label = { .layout = TextLayouts::LeftNoWrap, .style = TextStyles::NormalDark };
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

		LabelAndFrameCapsule(const Blueprint& bp) : wg::LabelCapsule(bp) {}

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
			DynamicText::Blueprint	label = { .layout = TextLayouts::LeftNoWrap, .style = TextStyles::NormalDark };
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

		LabeledSectionCapsule(const Blueprint& bp) : wg::LabelCapsule(bp) {}

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
			Skin_p				cornerSkin;
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
			Scroller::Blueprint	scrollbarX = { .back = Skins::ScrollbarTrack, .bar = Skins::ScrollbarHandle };
			Scroller::Blueprint	scrollbarY = { .back = Skins::ScrollbarTrack, .bar = Skins::ScrollbarHandle };
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
			Skin_p				cornerSkin;
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
			Scroller::Blueprint	scrollbarX = { .back = Skins::ScrollbarTrack, .bar = Skins::ScrollbarHandle };
			Scroller::Blueprint	scrollbarY = { .back = Skins::ScrollbarTrack, .bar = Skins::ScrollbarHandle };
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
			Skin_p				cornerSkin;
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
			Scroller::Blueprint	scrollbarX = { .back = Skins::ScrollbarTrack, .bar = Skins::ScrollbarHandle };
			Scroller::Blueprint	scrollbarY = { .back = Skins::ScrollbarTrack, .bar = Skins::ScrollbarHandle };
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
			Skin_p			handleSkin = Skins::SplitHandle;
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
			Skin_p			handleSkin = Skins::SplitHandle;
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
			EditableText::Blueprint	editor = { .style = TextStyles::NormalDark };
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			KeyAction		returnKeyAction = KeyAction::Insert;
			bool			selectable = false;
			Skin_p			skin = Skins::Canvas;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::TextEditor_p	create() { return new TextEditor(Blueprint()); }
		inline static wg::TextEditor_p	create(const Blueprint& blueprint) { return new TextEditor(blueprint); }

	protected:

		TextEditor(const Blueprint& bp) : wg::TextEditor(bp) {}


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
			EditableText::Blueprint	editor = { .style = TextStyles::NormalDark };
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Ibeam;
			KeyAction		returnKeyAction = KeyAction::ReleaseFocus;
			bool			selectable = false;
			Skin_p			skin = Skins::Canvas;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::LineEditor_p	create() { return new LineEditor(Blueprint()); }
		inline static wg::LineEditor_p	create(const Blueprint& blueprint) { return new LineEditor(blueprint); }

	protected:

		LineEditor(const Blueprint& bp) : wg::LineEditor(bp) {}
	};

	//____ SelectBox ______________________________________________________

	class SelectBox : public wg::SelectBox
	{
	public:

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Skin_p			entrySkin = Skins::SelectBoxEntry;
			TextStyle_p		entryTextStyle = TextStyles::NormalDark;
			TextLayout_p	entryTextLayout;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			Skin_p			listSkin = Skins::SelectBox;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin = Skins::Canvas;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::SelectBox_p	create() { return new SelectBox(Blueprint()); }
		inline static wg::SelectBox_p	create(const Blueprint& blueprint) { return new SelectBox(blueprint); }

	protected:

		SelectBox(const Blueprint& bp) : wg::SelectBox(bp) {}
	};

	//____ WindowTitleBar ______________________________________________________

	class WindowTitleBar : public wg::TextDisplay
	{
	public:

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			DynamicText::Blueprint	display = { .layout = TextLayouts::CenteredNoWrap, .style = TextStyles::Heading5 };
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
		WindowTitleBar(const Blueprint& bp) : wg::TextDisplay(bp) {}
	};

} // namespace wg
#endif //WG_THEME_OLDSKOOL_DOT_H
