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
#include <wg_debugtheme.h>

#include <wg_base.h>
#include <wg_blockskin.h>
#include <wg_colorskin.h>
#include <wg_plaintextlayout.h>
#include <wg_basicnumberlayout.h>
#include <wax_bitmapreader.h>

namespace wg
{
	// Default icons and transparency grid as .surf, generated from
	// resources/debugger_gfx.png and resources/checkboardtile.png into
	// wg_debuggerresources.cpp. To regenerate, from the WonderGUI root:
	//
	//   scripts/embed_images.rb --image2bitmap=<path to image2bitmap> --output=wondergui/extensions/debugger/core/wg_debuggerresources.cpp
	//                           --namespace=wg _debuggerIconsBitmap=resources/debugger_gfx.png
	//                           _debuggerTransparencyGridBitmap=resources/checkboardtile.png

	extern const char	_debuggerIconsBitmap[];
	extern const char	_debuggerTransparencyGridBitmap[];

	static int		s_widgetKitUsers = 0;
	static bool		s_bOwnsWidgetKit = false;

	//____ acquireWidgetKit() ____________________________________________________

	bool DebugTheme::acquireWidgetKit( Font * pNormal, Font * pBold, Font * pItalic, Font * pMono )
	{
		if( s_widgetKitUsers++ == 0 && !dbgkit::isInitialized() )
		{
			if( !dbgkit::init(pNormal, pBold, pItalic, pMono) )
				return false;

			s_bOwnsWidgetKit = true;
		}

		return dbgkit::isInitialized();
	}

	//____ releaseWidgetKit() ____________________________________________________

	void DebugTheme::releaseWidgetKit()
	{
		if( s_widgetKitUsers == 0 )
			return;

		if( --s_widgetKitUsers == 0 && s_bOwnsWidgetKit )
		{
			dbgkit::exit();
			s_bOwnsWidgetKit = false;
		}
	}

	//____ create() ______________________________________________________________

	DebugTheme DebugTheme::create( Bitmap * pIcons, Bitmap * pTransparencyGrid )
	{
		DebugTheme theme;

		Bitmap_p pDefaultIcons;
		Bitmap_p pDefaultTransparencyGrid;

		if( !pIcons || !pTransparencyGrid )
		{
			auto pReader = BitmapReader::create(WGBP(BitmapReader, _.factory = Base::defaultBitmapFactory()));

			if( !pIcons )
			{
				pDefaultIcons = pReader->readBitmapFromMemory(_debuggerIconsBitmap);
				pIcons = pDefaultIcons;
			}

			if( !pTransparencyGrid )
			{
				pDefaultTransparencyGrid = pReader->readBitmapFromMemory(_debuggerTransparencyGridBitmap, WGBP(Bitmap, _.tiling = true));
				pTransparencyGrid = pDefaultTransparencyGrid;
			}
		}

		theme.icons = pIcons;
		theme.transparencyGrid = pTransparencyGrid;

		// Icons. All 16x16 blocks in the icons bitmap.

		theme.refreshIcon = BlockSkin::create(WGBP(BlockSkin,
			_.bitmap = pIcons,
			_.firstBlock = Rect(0, 0, 16, 16);
		));

		theme.selectIcon = BlockSkin::create(WGBP(BlockSkin,
			_.bitmap = pIcons,
			_.firstBlock = Rect(16, 0, 16, 16);
		));

		theme.expandIcon = BlockSkin::create(WGBP(BlockSkin,
			_.bitmap = pIcons,
			_.firstBlock = Rect(32, 0, 16, 16);
		));

		theme.condenseIcon = BlockSkin::create(WGBP(BlockSkin,
			_.bitmap = pIcons,
			_.firstBlock = Rect(48, 0, 16, 16);
		));

		theme.unselectIcon = BlockSkin::create(WGBP(BlockSkin,
			_.bitmap = pIcons,
			_.firstBlock = Rect(0, 64, 16, 16);
		));

		theme.dirtyRectsIcon = BlockSkin::create(WGBP(BlockSkin,
			_.bitmap = pIcons,
			_.firstBlock = Rect(16, 64, 16, 16);
		));


		// Text and number layouts.

		auto pListTextLayout = PlainTextLayout::create( WGBP(PlainTextLayout,
			_.placement = Placement::East ));

		auto pInfoLayout = PlainTextLayout::create( WGBP(PlainTextLayout,
			_.wrap = true,
			_.placement = Placement::Center ));

		auto pWrapTextLayout = PlainTextLayout::create( WGBP(PlainTextLayout,
			_.wrap = true,
			_.placement = Placement::NorthWest ));

		auto pValueLayout = BasicNumberLayout::create( WGBP(BasicNumberLayout,
			_.style = dbgkit::TextStyles::Default,
			_.decimalMin = 2
		));

		auto pIntegerLayout = BasicNumberLayout::create( WGBP(BasicNumberLayout,
			_.style = dbgkit::TextStyles::Default,
			_.decimalMin = 0
		));

		auto pPtsLayout = BasicNumberLayout::create( WGBP(BasicNumberLayout,
			_.style = dbgkit::TextStyles::Default,
			_.decimalMin = 2
		));

		// Widget blueprints.

		theme.classCapsule = WGBP(LabelCapsule,
			_.skin = ColorSkin::create(Color::Transparent, { 10,0,0,8 }),
			_.label.style = dbgkit::TextStyles::FinePrint
		);

		theme.listEntryLabel = WGBP(TextDisplay,
			_.display.style = dbgkit::TextStyles::Strong );

		theme.listEntryText = WGBP(TextDisplay,
			_.display.style = dbgkit::TextStyles::Default,
			_.display.layout = pListTextLayout );

		theme.listEntryInteger = WGBP(NumberDisplay,
			_.display.layout = pIntegerLayout );

		theme.listEntryBool = WGBP(NumberDisplay,
			_.display.layout = pValueLayout );

		theme.listEntrySPX = WGBP(NumberDisplay,
			_.display.layout = pIntegerLayout );

		theme.listEntryPts = WGBP(NumberDisplay,
			_.display.layout = pPtsLayout );

		theme.listEntryDecimal = WGBP(NumberDisplay,
			_.display.layout = pValueLayout );

		theme.listEntryDrawer = dbgkit::TreeListDrawer::Blueprint();
		theme.selectableListEntryCapsule = WGOVR( dbgkit::TreeListEntry::Blueprint(), _.selectable = true );

		theme.textField = WGBP(TextDisplay,
			_.display.style = dbgkit::TextStyles::Default,
			_.display.layout = pWrapTextLayout,
			_.skin = dbgkit::Skins::Canvas );

		theme.infoDisplay = WGBP(TextDisplay,
			_.display.style = dbgkit::TextStyles::Emphasis,
			_.display.layout = pInfoLayout );

		theme.table = WGBP(TablePanel,
			_.columnLayout = Base::defaultPackLayout());

		return theme;
	}

} // namespace wg
