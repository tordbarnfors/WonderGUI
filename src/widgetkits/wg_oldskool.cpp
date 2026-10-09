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

#include "wg_oldskool.h"

#include <wg_sysfont.h>
#include <wg_surfacereader.h>
#include <wg_base.h>

namespace wg::oldskool
{
	// Skin blocks as a .surf, generated from resources/oldskool_skinblocks.png into
	// wg_oldskool_resources.cpp. To regenerate, from the WonderGUI root:
	//
	//   scripts/embed_images.rb --image2surf=<path to image2surf> --output=src/widgetkits/wg_oldskool_resources.cpp
	//                           --namespace=wg::oldskool _skinBlocksSurf=resources/oldskool_skinblocks.png

	extern const char	_skinBlocksSurf[];
	extern const int	_skinBlocksSurfSize;

	//____ init() _____________________________________________________________

	bool init(Font* pNormal, Font* pBold, Font* pItalic, Font* pMonospace)
	{
		auto pSurfaceFactory = Base::defaultSurfaceFactory();
		if( !pSurfaceFactory )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No default surface factory set", nullptr, nullptr, __func__, __FILE__, __LINE__);
			return false;
		}

		auto pReader = SurfaceReader::create(WGBP(SurfaceReader, _.factory = pSurfaceFactory));
		Surface_p pSkinBlocks = pReader->readSurfaceFromMemory(_skinBlocksSurf);
		if( !pSkinBlocks )
			return false;

		Font_p pNormalFont = pNormal;
		if( !pNormalFont )
			pNormalFont = SysFont::create(pSurfaceFactory);

		Fonts::Normal	= pNormalFont;
		Fonts::Bold		= pBold ? pBold : pNormalFont.rawPtr();
		Fonts::Italic	= pItalic ? pItalic : pNormalFont.rawPtr();
		Fonts::Mono		= pMonospace ? pMonospace : pNormalFont.rawPtr();

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
			_.padding = 1,
			_.frame = 1));

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
			_.padding = 0));

		Skins::ScrollbarHandleX = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 0,114,18,21 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.frame = 3,
			_.padding = 8,
			_.rigidPartX = {4,10,YSections::Center},
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::ScrollbarHandleY = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 131,0,21,18 },
			_.axis = Axis::Y,
			_.blockSpacing = 1,
			_.frame = 3,
			_.padding = 8,
			_.rigidPartY = { 4,10,XSections::Center },
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));


		Skins::ScrollbarButtonUp = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 74,15, 21, 18 },
			_.axis = Axis::Y,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::ScrollbarButtonDown = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 52, 15, 21, 18 },
			_.axis = Axis::Y,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::ScrollbarButtonLeft = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 76,92, 18, 21 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::ScrollbarButtonRight = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 0,92, 18, 21 },
			_.axis = Axis::X,
			_.blockSpacing = 1,
			_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
		));

		Skins::Field = BlockSkin::create(WGBP(BlockSkin,
			_.surface = pSkinBlocks,
			_.firstBlock = { 24,60,10,10 },
			_.padding = 1,
			_.frame = 1,
			_.spacing = 1 ));

		return true;
	}

	//____ exit() _____________________________________________________________

	bool exit()
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
		Skins::SplitHandle = nullptr;

		Transitions::openClose = nullptr;

		_pLabelCapsuleSkin = nullptr;
		_pCapsuleLabelSkin = nullptr;
		_pCapsuleLabelSkin2 = nullptr;
		_pInvisibleBoxSkin = nullptr;
		_pPlusMinusToggleSkin = nullptr;
		_pSelectableEntrySkin = nullptr;

		return true;
	}

} // namespace wg::oldskool
