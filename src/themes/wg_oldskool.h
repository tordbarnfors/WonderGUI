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

#include <wg_button.h>


namespace wg::Oldskool
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
	}




	inline ValueTransition_p	openCloseTransition;


	inline Skin_p		_pLabelCapsuleSkin;
	inline Skin_p		_pCapsuleLabelSkin;
	inline Skin_p		_pCapsuleLabelSkin2;
	inline Skin_p		_pInvisibleBoxSkin;
	inline Skin_p		_pPlusMinusToggleSkin;
	inline Skin_p		_pSelectableEntrySkin;





	inline bool init(Font* pNormal, Font* pBold, Font* pItalic, Font* pMonospace, Surface* pWidgets ) 
	{ 
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

		openCloseTransition = ValueTransition::create(250000);

		Skins::Plate = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pWidgets,
			_.firstBlock = { 0,60,10,10 },
			_.padding = 3,
			_.frame = 3));

		Skins::PlateNoBevel = ColorSkin::create(WGBP(ColorSkin,
			_.color = Colors::Plate,
			_.padding = 3));

		Skins::Canvas = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pWidgets,
			_.firstBlock = { 24,60,10,10 },
			_.padding = 3,
			_.frame = 3));

		Skins::Window = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pWidgets,
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
			_.surface = pWidgets,
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


		Skins::Button = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pWidgets,
			_.firstBlock = { 0,15,10,10 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = 4,
			_.padding = 4,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));


		Skins::ToggleButton = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pWidgets,
			_.firstBlock = { 0,26,10,10 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = 4,
			_.padding = 4,
			_.states = { State::Default, State::Hovered, State::Checked, State::CheckedHovered, State::Disabled }
		));

		Skins::Checkbox = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pWidgets,
			_.firstBlock = { 0,37,10,10 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Checked, State::Disabled, State::DisabledChecked }
		));

		Skins::RadioButton = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pWidgets,
			_.firstBlock = { 0,48,10,10 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Checked, State::Disabled }
		));

		Skins::SelectBox = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pWidgets,
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

		return true; 
	}

	inline bool exit() 
	{
		return true; 
	}



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
			Skin_p			skin = wg::Oldskool::Skins::Button;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		inline static wg::Button_p	create() { return new wg::Oldskool::Button(Blueprint()); }
		inline static wg::Button_p	create(const Blueprint& blueprint) { return new wg::Oldskool::Button(blueprint); }

	protected:

		Button(const Blueprint& bp) : wg::Button(bp)
		{
		}

	};



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

		inline static wg::ToggleButton_p	create() { return new wg::Oldskool::ToggleButton(Blueprint()); }
		inline static wg::ToggleButton_p	create(const Blueprint& blueprint) { return new wg::Oldskool::ToggleButton(blueprint); }

	protected:

		ToggleButton(const Blueprint& bp) : wg::ToggleButton(bp)
		{
		}
	};

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

		inline static wg::ToggleButton_p	create() { return new wg::Oldskool::Checkbox(Blueprint()); }
		inline static wg::ToggleButton_p	create(const Blueprint& blueprint) { return new wg::Oldskool::Checkbox(blueprint); }

	protected:

		Checkbox(const Blueprint& bp) : wg::ToggleButton(bp)
		{
		}

	};

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

		inline static wg::ToggleButton_p	create(const Blueprint& blueprint) { return new wg::Oldskool::RadioButton(blueprint); }

	protected:

		RadioButton(const Blueprint& bp) : wg::ToggleButton(bp)
		{
		}

	};

	class LabelCapsule : public wg::LabelCapsule
	{
	public:
		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = true;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			DynamicText::Blueprint	label;
			Placement		labelPlacement = Placement::NorthWest;
			Skin_p			labelSkin;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = true;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			bool			takesFocusFromChild = true;
			String			tooltip;
			bool			usePickHandles = false;

		};

		//.____ Creation ______________________________________________________


		inline static wg::LabelCapsule_p	create() { return new wg::Oldskool::LabelCapsule(Blueprint()); }
		inline static wg::LabelCapsule_p	create(const Blueprint& blueprint) { return new wg::Oldskool::LabelCapsule(blueprint); }

	protected:

		LabelCapsule(const Blueprint& bp) : wg::LabelCapsule(bp)
		{
		}

	};


} // namespace wg
#endif //WG_THEME_OLDSKOOL_DOT_H