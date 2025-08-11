
#include "fontgenerator.h"

#include <wg_freetypefont.h>
#include <string>
#include <fstream>

#ifdef WIN32
#    include <SDL_image.h>
#elif __APPLE__
#    include <SDL2_image/SDL_image.h>
#else
#    include <SDL2/SDL_image.h>
#endif


using namespace wg;
using namespace std;

//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(Visitor* pVisitor)
{
	m_pAppVisitor = pVisitor;

	if (!_setupGUI(pVisitor))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}
	
	loadTTF("resources/DroidSans.ttf");
	
	return true;
}

//____ update() _______________________________________________________________

bool MyApp::update()
{
	return true;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{
}


//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(Visitor* pVisitor)
{
	m_pWindow = pVisitor->createWindow({ .size = {800,700}, .title = "Font Generator" });

	//

	auto pFontBlob = pVisitor->loadBlob("resources/DroidSans.ttf");
	auto pFont = FreeTypeFont::create(pFontBlob);

	m_pTextStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 14,
									_.color = Color8::Black,
									_.states = {{State::Disabled, Color8::DarkGrey}
	} ));

	m_pLabelStyle = TextStyle::create(WGBP(TextStyle,
									_.font = pFont,
									_.size = 16,
									_.color = Color8::Black));


	Base::setDefaultStyle(m_pTextStyle);

	m_pTextLayoutCentered = BasicTextLayout::create(WGBP(BasicTextLayout,
		_.placement = Placement::Center));


	//

	if (!_loadSkins(pVisitor))
		return false;

	m_pLayout = PackLayout::create({ .wantedSize = PackLayout::WantedSize::Default,
	.expandFactor = PackLayout::Factor::Weight, .shrinkFactor = PackLayout::Factor::Zero });

	
	auto pBasePanel = PackPanel::create( { .axis = Axis::Y, .layout = m_pLayout });
	
	auto pSplitPanel = SplitPanel::create( { .axis = Axis::Y, .handleSkin = m_pButtonSkin, .resizeRatio = 0.f });
	
	pSplitPanel->slots[0] = createCharsPanel();
	pSplitPanel->slots[1] = createOutputPanel();

	pBasePanel->slots << createInputPanel();
	pBasePanel->slots << pSplitPanel;

	pBasePanel->setSlotWeight(0, 2, {0.f,1.f});

	m_pWindow->setContent(pBasePanel);

	pSplitPanel->setSplit(0.3f);		// Needs to be done once it has the right size.
	return true;
}

//____ selectAndLoadTTF() __________________________________________________________

bool MyApp::selectAndLoadTTF()
{
	auto selectedFile = m_pAppVisitor->openFileDialog("Select Font", "", {"*.ttf", "*.ttc", "*.otf"}, "Font Files");

	if (selectedFile.empty())
		return false;

	bool bLoaded = loadTTF(selectedFile.c_str());
	return bLoaded;	
}


//____ loadTTF() _________________________________________________________________

bool MyApp::loadTTF( const string& path)
{
	auto pBlob = m_pAppVisitor->loadBlob(path);
	
	if( pBlob )
	{
		m_pLoadedFontBlob = pBlob;
		m_pTTFPathDisplay->display.setText(path);
		return true;
	}
	
	return false;
}

//____ saveBitmapFont() _________________________________________________________________

bool MyApp::saveBitmapFont()
{
//	static const char * filters[3] = { "*.*", "*.ttc", "*.otf" };
	
	std::string outputPath = m_pAppVisitor->saveFileDialog("Enter output name without extension", "", {}, "");

	if( !outputPath.empty() )
	{
		// Save image

		int saveFormat = m_pImageFormatSelector->selectedEntryId();

		switch (saveFormat)
		{
			case 0:				// 8-bit SURF (uncompressed)
			{
				auto pCopy = m_pBitmapFontSurface->convert( { .format = PixelFormat::BGRA_8 } );
				
				auto pixbuf = pCopy->allocPixelBuffer();
				pCopy->pushPixels(pixbuf);
				 
				for( int y = 0 ; y < pixbuf.rect.h ; y++ )
				{
					uint8_t * pLine = pixbuf.pixels + pixbuf.pitch*y;
					for( int x = 0 ; x < pixbuf.rect.w ; x++ )
					{
						uint8_t alpha = pLine[x*4+3];
						if( alpha < 3 )
							pLine[x*4+3] = 0;
					}
				}
				
				pCopy->pullPixels(pixbuf);
				pCopy->freePixelBuffer(pixbuf);
				
				auto pIndexedSurface = pCopy->convert( { .format = PixelFormat::Index_8_linear } );
				if( pIndexedSurface )
				{
					std::string path = outputPath + ".surf";

					std::ofstream out(path, std::ios::binary);
					auto pWriter = SurfaceWriter::create({});
					pWriter->writeSurfaceToStream(out, pIndexedSurface);
					out.close();
				}
				else
				{
					int x = 0;
				}
			}


			break;
			case 1:				// 32-bit SURF (uncompressed)
			{
				std::string path = outputPath + ".surf";

				std::ofstream out(path, std::ios::binary);
				auto pWriter = SurfaceWriter::create({});
				pWriter->writeSurfaceToStream(out, m_pBitmapFontSurface);
				out.close();
			}
			break;
			
			case 2:				// 32-bit PNG (compressed)
			{
				std::string pngPath = outputPath + ".png";

				// Save PNG

				auto pixbuf = m_pBitmapFontSurface->allocPixelBuffer();
				m_pBitmapFontSurface->pushPixels(pixbuf);

				PixelDescription pixdesc = Util::pixelFormatToDescription(pixbuf.format);

				auto pSDLSurf = SDL_CreateRGBSurfaceFrom(pixbuf.pixels, pixbuf.rect.w, pixbuf.rect.h, pixdesc.bits, pixbuf.pitch, (Uint32) pixdesc.R_mask, (Uint32) pixdesc.G_mask, (Uint32) pixdesc.B_mask, (Uint32) pixdesc.A_mask);

				IMG_SavePNG(pSDLSurf, pngPath.c_str());
				m_pBitmapFontSurface->freePixelBuffer(pixbuf);
			}
			break;
		}

				
		// Save spec
		
		std::string specPath = outputPath + ".fnt";

		std::ofstream out( specPath );
		out << m_bitmapFontSpec;
		out.close();
		
		return true;
	}
	
	return false;
}

//____ generateBitmapFont() _________________________________________________________

bool MyApp::generateBitmapFont()
{
	if( !m_pLoadedFontBlob )
		return false;
	
	if( m_pCharsEditor->editor.isEmpty() )
		return false;
	
	spx size = m_pSizeSelector->selectedEntryId() * 64;
	FreeTypeFont::RenderMode renderMode = (FreeTypeFont::RenderMode) m_pModeSelector->selectedEntryId();
	bool bStemDarkening = (m_pStemDarkeningSelector->selectedEntryId() == 1);
			
	auto pFont = FreeTypeFont::create( { .blob = m_pLoadedFontBlob, .renderMode = renderMode, .stemDarkening = bStemDarkening });
	if( !pFont )
		return false;

	pFont->setSize(size);

	String charmap = m_pCharsEditor->editor.text();
	
	generateFontSurface(pFont, charmap);
	generateFontSpec(pFont, charmap);
	
	return true;
}

//____ generateFontSpec() _____________________________________________________

bool MyApp::generateFontSpec( FreeTypeFont * pFont, String& charmap )
{
	int size = pFont->size()/64;
	int whitespace = pFont->whitespaceAdvance()/64;
	int monochrome = pFont->isMonochrome();
	int baseline = pFont->maxAscend()/64;
	int linegap = pFont->lineGap()/64;
	
	
	char buffer[1024];
	
	snprintf( buffer, 1024, "[INFO BEGIN]\n\nSIZE: %d\nWHITESPACE: %d\nMONOCHROME: %d\nBASELINE: %d\nLINEGAP: %d\n\n[INFO END]\n\n",
			size, whitespace, monochrome, baseline, linegap );
	
	m_bitmapFontSpec = buffer;
	
	m_bitmapFontSpec += "\n[CHARMAP BEGIN]\n\n";
	m_bitmapFontSpec += CharSeq(charmap).getStdString();
	m_bitmapFontSpec += "\n[CHARMAP END]\n\n";
	
	m_bitmapFontSpec += "\n[KERNING BEGIN]\n\n";
	
	for( int i = 0 ; i < charmap.length() ; i++ )
	{
		uint16_t code1 = charmap.chars()[i].code();
		Glyph glyph1;
		pFont->getGlyphWithoutBitmap(code1, glyph1);
		
		for( int j = 0 ; j < charmap.length() ; j++ )
		{
			uint16_t code2 = charmap.chars()[j].code();
			Glyph glyph2;
			pFont->getGlyphWithoutBitmap(code2, glyph2);
			
			spx kerningSPX = pFont->kerning(glyph1, glyph2);

			if( glyph2.bearingX < 0 )
				kerningSPX += glyph2.bearingX;
			
			int kerning = (kerningSPX - 32) / 64;
			
			if( kerning != 0 )
			{
				char buff1[4] = {0,0,0,0};
				TextTool::uint16ToUtf8(code1, buff1, 4);

				char buff2[4] = {0,0,0,0};
				TextTool::uint16ToUtf8(code2, buff2, 4);

				snprintf( buffer, 1024, "%s%s %d ", buff1, buff2, kerning );
				m_bitmapFontSpec += buffer;
			}
		}
	}
	
	m_bitmapFontSpec += "\n[KERNING END]\n";

	
	return true;
}

//____ generateFontSurface() ___________________________________________________

bool MyApp::generateFontSurface( FreeTypeFont * pFont, String& chars )
{
	bool bUseSRGB = (m_pStemDarkeningSelector->selectedEntryId() == 1);
		
	PixelFormat	outputFormat = bUseSRGB ? PixelFormat::BGRA_8_sRGB : PixelFormat::BGRA_8_linear;

	
	// Calculate size of surface needed.
		
	spx maxWidth = 4096*64;
	spx ofsX = 0;
	int nRows = 1;

	for( int i = 0 ; i < chars.length() ; i++ )
	{
		Glyph glyph;
		pFont->getGlyphWithBitmap(chars.chars()[i].code(), glyph);

		if( glyph.pSurface )
		{
			spx advance = glyph.advance;
			if( glyph.bearingX < 0 )
				advance -= glyph.bearingX;

			ofsX += advance+64;
		
			if( ofsX > maxWidth )
			{
				nRows++;
				ofsX = advance+64;
			}
		}
	}
	
	SizeI surfaceSize;
	
	int rowPixelHeight = (pFont->maxAscend() + pFont->maxDescend()) / 64;
	surfaceSize.h = 1 + (rowPixelHeight+1) * nRows;
	
	if( nRows == 1 )
		surfaceSize.w = ofsX / 64;
	else
		surfaceSize.w = maxWidth / 64;
	
	// Generate and fill in surface
	
	auto pSurface = Base::defaultSurfaceFactory()->createSurface( { .canvas = true, .format = outputFormat, .size = surfaceSize });
	auto pDevice = Base::defaultGfxDevice();

	HiColor lineColor(1024,0,0,4096);
	
	pDevice->beginRender();
	
	pDevice->beginCanvasUpdate(pSurface);
	pDevice->setBlendMode(BlendMode::Replace);
	pDevice->fill(Color::Transparent);
	pDevice->setBlendMode(BlendMode::Replace);

	CoordSPX pos;
	int rowHeight = (pFont->maxAscend() + pFont->maxDescend()) / 64;
	spx baselineOfs = pFont->maxAscend();
	
	pDevice->setTintColor(Color::White);
	pDevice->drawLine(pos, Direction::Right, surfaceSize.w*64, lineColor);
	pos.y += 64;
	
	for( int i = 0 ; i < chars.length() ; i++ )
	{
		Glyph glyph;
		pFont->getGlyphWithBitmap(chars.chars()[i].code(), glyph);
		
		spx bearingX = glyph.bearingX;
		spx advance = glyph.advance;
		if( bearingX < 0 )
		{
			advance -= bearingX;
			bearingX = 0;
		}
		
		if( pos.x + advance > surfaceSize.w*64  )
		{
			pos.x = 0;
			pos.y += rowHeight*64;
			pDevice->drawLine(pos, Direction::Right, surfaceSize.w*64, lineColor);
			pos.y += 64;
		}

		if( glyph.pSurface )
		{
			pDevice->setBlitSource(glyph.pSurface);
			pDevice->blit(pos + CoordSPX(bearingX,baselineOfs + glyph.bearingY), glyph.rect);
			pos.x += advance;
			pDevice->drawLine(pos, Direction::Down, rowHeight*64, lineColor );
			pos.x += 64;
		}		
	}

	pos.x = 0;
	pos.y += rowHeight*64;
	pDevice->drawLine(pos, Direction::Right, surfaceSize.w*64, lineColor);

	pDevice->endCanvasUpdate();
	pDevice->endRender();
	
	m_pBitmapFontSurface = pSurface;
	
	m_pFontDisplaySurface = Base::defaultSurfaceFactory()->createSurface( { .canvas = true, .format = PixelFormat::BGRA_8, .size = surfaceSize });

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(m_pFontDisplaySurface);
	pDevice->fill( HiColor::White );
	pDevice->setBlitSource(pSurface);
	pDevice->setTintColor(HiColor::Black);
	pDevice->blit({0,0});
	pDevice->endCanvasUpdate();
	pDevice->endRender();
	
	
	return true;
}

//____ displayBitmapFont() ___________________________________________________

void MyApp::displayBitmapFont()
{
	m_pBitmapDisplay->setImage(m_pFontDisplaySurface);
}

//____ createInputPanel() ________________________________________________________

Widget_p MyApp::createInputPanel()
{
	auto pBase = PackPanel::create();
	pBase->setAxis(Axis::Y);
	pBase->setLayout(m_pLayout);
	pBase->setSkin(m_pPlateSkin);
	pBase->setId(1);
	
	auto pTopRow = PackPanel::create();
	pTopRow->setAxis(Axis::X);
	pTopRow->setLayout(m_pLayout);
	pTopRow->setId( 2 );
	
	auto pLabel = TextDisplay::create( WGBP(TextDisplay,
											_.display = WGBP(DynamicText,
															 _.style = m_pTextStyle,
															 _.text = "Source: ",
															 _.layout = m_pTextLayoutCentered )
											) );

	auto pPath = TextDisplay::create( WGBP(TextDisplay,
											 _.skin = m_pSectionSkin,
											 _.display = WGBP(DynamicText, _.style = m_pTextStyle, _.layout = m_pTextLayoutCentered)
										   ) );
	
	auto pLoadButton = Button::create( WGBP(Button,
											_.skin = m_pButtonSkin,
											_.label = WGBP(DynamicText, _.layout = m_pTextLayoutCentered, _.style = m_pTextStyle, _.text = "Select" )
											));
	
	pTopRow->slots << pLabel;
	pTopRow->slots << pPath;
	pTopRow->slots << pLoadButton;
	
	pTopRow->setSlotWeight( 0, 3, {0.f, 1.f, 0.f});
	
	auto pBottomRow = PackPanel::create();
	pBottomRow->setAxis(Axis::X);
	pBottomRow->setId(3);
//	pBottomRow->setSizeBroker(m_pSizeBroker);

	auto pSizeLabel = TextDisplay::create( WGBP(TextDisplay,
												_.display = WGBP(DynamicText,
																 _.style = m_pTextStyle,
																 _.text = "Size:",
																 _.layout = m_pTextLayoutCentered )
												));
	
	auto pSizeSelector = SelectBox::create();
	pSizeSelector->setSkin(m_pButtonSkin);
	pSizeSelector->setListSkin(m_pPlateSkin);
	
	char tmp[8];
	for( int i = 6 ; i <= 32 ; i++ )
	{
		snprintf( tmp, 8, "%i", i);
//		pSizeSelector->entries.pushBack(SelectBoxEntry(i, String(tmp)));
		pSizeSelector->entries.pushBack({ i, String(tmp) } );

	}
	
	auto pModeLabel = TextDisplay::create( WGBP(TextDisplay,
												_.display = WGBP(DynamicText,
																 _.style = m_pTextStyle,
																 _.text = "Render mode:",
																 _.layout = m_pTextLayoutCentered )
												));

	auto pModeSelector = SelectBox::create();
	pModeSelector->setSkin(m_pButtonSkin);
	pModeSelector->setListSkin(m_pPlateSkin);
	pModeSelector->entries.pushBack({ 	{ int(FreeTypeFont::RenderMode::Monochrome), String("Monochrome") },
										{ int(FreeTypeFont::RenderMode::CrispEdges), String("CrispEdges") },
										{ int(FreeTypeFont::RenderMode::BestShapes), String("BestShapes")} });
	
	auto pSRGBLabel = TextDisplay::create( WGBP(TextDisplay,
												_.display = WGBP(DynamicText,
																 _.style = m_pTextStyle,
																 _.text = "Stemdarkening:",
																 _.layout = m_pTextLayoutCentered )
												));

	auto pStemDarkeningSelector = SelectBox::create();
	pStemDarkeningSelector->setSkin(m_pButtonSkin);
	pStemDarkeningSelector->setListSkin(m_pPlateSkin);
	pStemDarkeningSelector->entries.pushBack({ 	{ 0, String("Off") },
											{ 1, String("On") } });

	pBottomRow->slots << pSizeLabel;
	pBottomRow->slots << pSizeSelector;
	pBottomRow->slots << pModeLabel;
	pBottomRow->slots << pModeSelector;
	pBottomRow->slots << pSRGBLabel;
	pBottomRow->slots << pStemDarkeningSelector;

	pBottomRow->setSpacing(6);
	
	
	pBase->slots << pTopRow;
	pBase->slots << pBottomRow;

	pBase->setSpacing(4,8,4);

	m_pTTFPathDisplay = pPath;
	m_pSizeSelector = pSizeSelector;
	m_pModeSelector = pModeSelector;
	m_pStemDarkeningSelector = pStemDarkeningSelector;

	m_pSizeSelector->selectEntryById(10);
	m_pModeSelector->selectEntryById(int(FreeTypeFont::RenderMode::BestShapes));
	m_pStemDarkeningSelector->selectEntryByIndex(0);

	Base::msgRouter()->addRoute( pLoadButton, MsgType::Select, [this](Msg*pMsg){this->selectAndLoadTTF();});

	return pBase;
}

//____ createCharsPanel() _____________________________________________________

Widget_p MyApp::createCharsPanel()
{
	auto pBase = PackPanel::create();
	pBase->setAxis(Axis::Y);
	pBase->setLayout(m_pLayout);
	pBase->setSkin(m_pPlateSkin);
	
	auto pLabel = TextDisplay::create( { .display = { .text = String("Characters to include") } } );
	
	auto pWindow = ScrollPanel::create();

	pWindow->scrollbarX.setBackSkin(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color8::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black)));
	pWindow->scrollbarX.setBarSkin(m_pPlateSkin);

	pWindow->scrollbarY.setBackSkin(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color8::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black)));
	pWindow->scrollbarY.setBarSkin(m_pPlateSkin);

	pWindow->setAutohideScrollbars(true, true);
	pWindow->setSizeConstraints(SizeConstraint::GreaterOrEqual, SizeConstraint::GreaterOrEqual);

	m_pCharsEditor = TextEditor::create( WGBP(TextEditor,
											_.skin = m_pSectionSkin
											));
	
	pWindow->slot = m_pCharsEditor;
	
	
	pBase->slots << pLabel;
	pBase->slots << pWindow;
	pBase->setSpacing(4);

	pBase->setSlotWeight(0, 2, {0.f, 1.f} );

	return pBase;
}

//____ createOutputPanel() _____________________________________________________

Widget_p MyApp::createOutputPanel()
{
	auto pBase = PackPanel::create();
	pBase->setAxis(Axis::Y);
	pBase->setLayout(m_pLayout);
	pBase->setSkin(m_pPlateSkin);

	
	auto pTopPanel = PackPanel::create();
	pTopPanel->setAxis(Axis::X);
	pTopPanel->setLayout(m_pLayout);
	
	auto pLeftBottomLayout = BasicTextLayout::create( { .placement = Placement::SouthWest } );
	
	auto pLabel = TextDisplay::create( { .display = {.layout = pLeftBottomLayout, .text = String("Preview") } } );
	auto pFiller = Filler::create();
	auto pGenerateButton = Button::create( { .label = { .text = String("Generate")}, .skin = m_pButtonSkin } );
	
	auto pImageFormatSelector = SelectBox::create();
	pImageFormatSelector->setSkin(m_pButtonSkin);
	pImageFormatSelector->setListSkin(m_pPlateSkin);
	pImageFormatSelector->entries.pushBack({ 	{ 0, String("Uncompressed 8-bit indexed SURF") },
												{ 1, String("Uncompressed 32-bit SURF") },
												{ 2, String("Compressed 32-bit PNG")} });
	pImageFormatSelector->selectEntryByIndex(1);
	m_pImageFormatSelector = pImageFormatSelector;
	
	auto pSaveButton = Button::create( { .label = { .text = String("Save")}, .skin = m_pButtonSkin } );

	Base::msgRouter()->addRoute(pGenerateButton, MsgType::Select, [this](Msg*pMsg){this->generateBitmapFont();this->displayBitmapFont();} );
	
	
	pTopPanel->slots << pLabel;
	pTopPanel->slots << pFiller;
	pTopPanel->slots << pGenerateButton;
	pTopPanel->slots << pImageFormatSelector;
	pTopPanel->slots << pSaveButton;

	pTopPanel->setSlotWeight(0, 5, {0.f,1.f,0.f,0.f,0.f});
	pTopPanel->setSpacing(0,8,4 );
	
	
	auto pWindow = ScrollPanel::create();
	pWindow->setSkin(m_pSectionSkin);
	
	pWindow->scrollbarX.setBackSkin(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color8::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black)));
	pWindow->scrollbarX.setBarSkin(m_pPlateSkin);
	
	pWindow->scrollbarY.setBackSkin(BoxSkin::create(WGBP(BoxSkin,
		_.color = Color8::DarkOliveGreen,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black)));
	pWindow->scrollbarY.setBarSkin(m_pPlateSkin);
	
	pWindow->setAutohideScrollbars(true, false);
	pWindow->setSizeConstraints(SizeConstraint::None, SizeConstraint::None);
	
	pBase->slots << pTopPanel;
	pBase->slots << pWindow;

	pBase->setSlotWeight(0, 2, {0.f, 1.f} );
	pBase->setSpacing(4);
	
	m_pBitmapDisplay = Image::create();
	pWindow->slot = m_pBitmapDisplay;
	
	
	Base::msgRouter()->addRoute( pSaveButton, MsgType::Select, [this](Msg*pMsg){this->saveBitmapFont();});


	
	return pBase;
}


//____ _loadSkins() ___________________________________________________________

bool MyApp::_loadSkins(Visitor * pVisitor)
{
	string path = "resources/greyskin/";

	auto pPlateSurf = pVisitor->loadSurface(path + "plate.bmp");
	auto pButtonSurf = pVisitor->loadSurface(path + "button.bmp");
	auto pStateButtonSurf = pVisitor->loadSurface(path + "state_button.bmp");
	auto pCheckBoxSurf = pVisitor->loadSurface(path + "checkbox.png");

	if (!pPlateSurf || !pButtonSurf || !pStateButtonSurf)
		return false;

	m_pPlateSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pPlateSurf,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = 4
	));

	m_pButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pButtonSurf,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = { 4,10,4,10 },
		_.states = { State::Default, State::Hovered, State::Pressed, State::Disabled }
	));

	m_pToggleButtonSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pStateButtonSurf,
		_.axis = Axis::X,
		_.frame = 4,
		_.padding = 4,
		_.states = { State::Default, State::Hovered, State::Checked, State::Checked + State::Hovered, State::Disabled }
	));

	m_pCheckBoxSkin = BlockSkin::create(WGBP(BlockSkin,
		_.surface = pCheckBoxSurf,
		_.axis = Axis::Y,
		_.frame = 3,
//		_.defaultSize = { 12,12 },
		_.states = { State::Default, State::Checked }
	));

	m_pSectionSkin = BoxSkin::create(WGBP(BoxSkin,
		_.color = HiColor::White,
		_.outlineThickness = 1,
		_.outlineColor = Color8::Black,
		_.padding = { 2,2,2,2 }
	));


	return true;
}
