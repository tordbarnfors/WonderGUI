
#include "compressbench.h"

#include <wondergui.h>
#include <wg_freetypefont.h>

#include <string>
#include <filesystem>

#include <cstring>

using namespace wg;
using namespace wapp;
using namespace std;


//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(API * pAPI)
{
	m_pAPI = pAPI;

	m_compressors.push_back( LZCompressor::create() );
	m_compressors.push_back( Q565Compressor::create() );
	m_compressors.push_back( RLECompressor::create({ .primSize = 1}) );
	m_compressors.push_back( RLECompressor::create({ .primSize = 2}) );

	m_pCompressor = m_compressors[0];

	if (!_setupGUI(pAPI))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}

	return true;
}

//____ update() _______________________________________________________________

bool MyApp::update()
{
	return m_bContinue;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{

}

//____ closeWindow() __________________________________________________________

void MyApp::closeWindow(Window* pWindow)
{
	m_pWindow = nullptr;
	m_bContinue = false;
}

//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(API* pAPI)
{
	m_pWindow = Window::create(pAPI, { .size = {600,600}, .title = "Compress Bench" });

	auto pTheme = pAPI->initDefaultTheme();
	m_pTheme = pTheme;

	m_pLayoutRight = BasicTextLayout::create( { .placement = Placement::East } );

	// Create and populate button panel

	auto pButtonPanel = PackPanel::create( { .axis = Axis::X, .skin = pTheme->plateSkin() } );

	auto pLoadButton = WGCREATE( Button, _ = pTheme->pushButton(), _.label.text = "Select test images..." );
	auto pRunButton = WGCREATE( Button, _ = pTheme->pushButton(), _.label.text = "Run tests" );
	auto pSyntheticTestButton = WGCREATE( Button, _ = pTheme->pushButton(), _.label.text = "Run 'synthetic' test" );

	m_pCompressorSelector = SelectBox::create( pTheme->selectBox() );
	m_pCompressorSelector->entries.pushBack( SelectBoxEntry::Blueprint{ .id = 0, .text = "LZWP (Lempel-Ziw based w improvements)" } );
	m_pCompressorSelector->entries.pushBack( SelectBoxEntry::Blueprint{ .id = 1, .text = "Q565 (QOI inspired for RGB565)" } );
	m_pCompressorSelector->entries.pushBack( SelectBoxEntry::Blueprint{ .id = 2, .text = "RLEx-1 (Simple one byte run length encoding)" } );
	m_pCompressorSelector->entries.pushBack( SelectBoxEntry::Blueprint{ .id = 3, .text = "RLEx-2 (Simple two byte run length encoding)" } );
	m_pCompressorSelector->selectEntryByIndex(0);

	pButtonPanel->slots.pushBack({
//		{ WGCREATE( Filler ), { .weight = 1 } },
		{ pLoadButton, { .weight = 0 } },
//		{ WGCREATE( Filler ), { .weight = 0.5f } },
		{ pRunButton, { .weight = 0 } },
		{ pSyntheticTestButton, { .weight = 0 } },
		{ WGCREATE( Filler ), { .weight = 1 } },
		{ m_pCompressorSelector, { .weight = 0 } }
	} );

	Base::msgRouter()->addRoute(pLoadButton, MsgType::Select, [this](Msg * pMsg){ onLoad(); } );
	Base::msgRouter()->addRoute(pRunButton, MsgType::Select, [this](Msg * pMsg){ onRun(); } );
	Base::msgRouter()->addRoute(pSyntheticTestButton, MsgType::Select, [this](Msg * pMsg){ onRunSyntheticTest(); } );
	Base::msgRouter()->addRoute(m_pCompressorSelector, MsgType::Select, [this](Msg * pMsg){ onCompressorSelected(); } );

	// Create main section

	m_pResultTable = WGCREATE( TablePanel, _ = pTheme->listTable(), _.columns = 7, _.rows = 2, _.skin = pTheme->canvasSkin() );

	TextDisplay::Blueprint columnLabelBP( { .skin = pTheme->plateSkin(), .display = { .style = pTheme->strongStyle() }} );

	auto pColumnLabel1 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Filename" );
	auto pColumnLabel2 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Ratio (%)" );
	auto pColumnLabel3 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Comp size" );
	auto pColumnLabel4 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Org size" );
	auto pColumnLabel5 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Comp time (usec)" );
	auto pColumnLabel6 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Decomp time (usec)" );
	auto pColumnLabel7 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Status" );

	refreshList();

	m_pResultTable->slots.replaceRow( 0, { pColumnLabel1, pColumnLabel2, pColumnLabel3, pColumnLabel4, pColumnLabel5, pColumnLabel6, pColumnLabel7 } );




	// Create and populate main panel

	auto pMainPanel = PackPanel::create( { .axis = Axis::Y, .skin = pTheme->windowSkin() } );

	pMainPanel->slots.pushBack( pButtonPanel, { .weight = 0 } );
	pMainPanel->slots.pushBack( m_pResultTable, { .weight = 1 } );

	// Create scrollpanel

	auto pScrollPanel = ScrollPanel::create( pTheme->scrollPanelXY() );

	pScrollPanel->slot = pMainPanel;

	m_pWindow->mainCapsule()->slot = pScrollPanel;
	return true;
}

//____ onLoad() _______________________________________________________________

void MyApp::onLoad()
{
	auto filePaths = m_pAPI->openMultiFileDialog("Select Test Images", "", "", {}, "");

	if( filePaths.size() > 0 )
	{
		m_testSurfaces.clear();

		for( auto& path : filePaths )
		{
			auto pSurface = m_pAPI->loadSurface(path);
			if( pSurface )
			{
				string name = std::filesystem::path(path).stem().string();
				m_testSurfaces.push_back({name, pSurface});
			}
		}

		refreshList();

	}


}

//____ onRun() ________________________________________________________________

void MyApp::onRun()
{
	int srcTotalBytes = 0;
	int compressedTotalBytes = 0;

	int compressTimeTotal = 0;
	int decompressTimeTotal = 0;

	for( int i = 0 ; i < m_testSurfaces.size() ; i++ )
	{
		auto pSurface = m_testSurfaces[i].pSurface;
		auto pixbuf = pSurface->allocPixelBuffer();
		pSurface->pushPixels(pixbuf);

		int srcSize = pixbuf.pitch * pixbuf.rect.h;
		
		// Do compression

		int memNeeded = m_pCompressor->maxCompressedSize(srcSize);
		auto pCompressArea = (uint8_t*) Base::memStackAlloc(memNeeded);

		auto beforeCompTS = m_pAPI->time();
		int compressedSize = m_pCompressor->compress(pCompressArea, pixbuf.pixels, pixbuf.pixels + srcSize );
		auto afterCompTS = m_pAPI->time();


		// Do decompression

		auto pDecompressArea = (uint8_t*) Base::memStackAlloc(srcSize+9);
		strcpy((char*)pDecompressArea+srcSize, "DEADBEEF");

		auto beforeDecompTS = m_pAPI->time();
		int decompressedSize = m_pCompressor->decompress(pDecompressArea, pCompressArea, pCompressArea+compressedSize);
		auto afterDecompTS = m_pAPI->time();

		// Check decompressed result.

		auto pResultDisplay = wg_dynamic_cast<TextDisplay_p>(m_pResultTable->slots[i+1][6].widget());

		bool bEquivalent = true;

		for( int j = 0 ; j < srcSize ; j++ )
		{
			if( pixbuf.pixels[j] != pDecompressArea[j] )
			{
				char temp[64];
				snprintf( temp, 64, "ERROR at offset %d", j );

				pResultDisplay->display.setText( temp );
				bEquivalent = false;
				break;
			}
		}

		if( bEquivalent )
		{
			if( decompressedSize != srcSize )
			{
				pResultDisplay->display.setText( "ERROR: Decompressed size." );
			}
			else if( strcmp( (char*)pDecompressArea+srcSize, "DEADBEEF") != 0 )
			{
				pResultDisplay->display.setText( "ERROR: Decompressed destination overflow." );
			}
			else
			{
				pResultDisplay->display.setText( "SUCCESS" );

				updateListRow( i+1, srcSize, compressedSize, int(afterCompTS-beforeCompTS), int(afterDecompTS-beforeDecompTS) );
			}
		}

		// Cleanup

		Base::memStackFree(srcSize+9);
		Base::memStackFree(memNeeded);
		pSurface->freePixelBuffer(pixbuf);

		// Sum up totals

		srcTotalBytes += srcSize;
		compressedTotalBytes += compressedSize;

		compressTimeTotal += int(afterCompTS - beforeCompTS);
		decompressTimeTotal += int(afterDecompTS - beforeDecompTS);
	}

	updateListRow( int(m_testSurfaces.size()+1), srcTotalBytes, compressedTotalBytes, compressTimeTotal, decompressTimeTotal );
}


//____ onCompressorSelected() _________________________________________________

void MyApp::onCompressorSelected()
{
	int idx = m_pCompressorSelector->selectedEntryId();
	m_pCompressor = m_compressors[idx];
}



//____ updateListRow() ________________________________________________________

void MyApp::updateListRow( int row, int srcSize, int compressedSize, int compressMicroSec, int decompressMicroSec )
{
	char temp[16];

	// Update ratio display

	auto pRatioDisplay = wg_dynamic_cast<TextDisplay_p>(m_pResultTable->slots[row][1].widget());

	snprintf( temp, 16, "%d", compressedSize*100/srcSize );
	pRatioDisplay->display.setText( temp );

	// Update uncompressed size

	auto pCompDisplay = wg_dynamic_cast<TextDisplay_p>(m_pResultTable->slots[row][2].widget());

	snprintf( temp, 16, "%d", compressedSize );
	pCompDisplay->display.setText( temp );

	// Update uncompressed size

	auto pUncompDisplay = wg_dynamic_cast<TextDisplay_p>(m_pResultTable->slots[row][3].widget());

	snprintf( temp, 16, "%d", srcSize );
	pUncompDisplay->display.setText( temp );

	// Update Compress speed

	auto pCompTimeDisplay = wg_dynamic_cast<TextDisplay_p>(m_pResultTable->slots[row][4].widget());

	snprintf( temp, 16, "%d", compressMicroSec );
	pCompTimeDisplay->display.setText( temp );

	// Update Decompress speed

	auto pDecompTimeDisplay = wg_dynamic_cast<TextDisplay_p>(m_pResultTable->slots[row][5].widget());

	snprintf( temp, 16, "%d", decompressMicroSec );
	pDecompTimeDisplay->display.setText( temp );
}



//____ onRunSyntheticTest() ________________________________________________________________

void MyApp::onRunSyntheticTest()
{
	const static char testData[] = "ABBABBACDEFMMMMMMMARABBABOUABasdfdsafasdjwerjaewk dguiaafanlken adsökjj";

	int srcSize = sizeof(testData);

	uint8_t 	compressArea[256];
	char 		decompressArea[256];

	for( int i = 0 ; i < 256 ; i++ )
	{
		compressArea[i] = ' ';
		decompressArea[i] = 0;
	}

	int compressedSize = m_pCompressor->compress(compressArea, testData, testData + srcSize );
	int decompressedSize = m_pCompressor->decompress(decompressArea, compressArea, compressArea+compressedSize);
}


//____ refreshList() __________________________________________________________

void MyApp::refreshList()
{
	m_pResultTable->rows.resize(int(m_testSurfaces.size())+2);

	int index = 1;
	for( auto& test : m_testSurfaces )
	{
		auto pName = TextDisplay::create({ .display = { .text = test.name.c_str() } });
		auto pRatio = TextDisplay::create({ .display = { .text = "---", .layout = m_pLayoutRight } });
		auto pCompSize = TextDisplay::create({ .display = { .text = "---", .layout = m_pLayoutRight } });
		auto pUncompSize = TextDisplay::create({ .display = { .text = "---", .layout = m_pLayoutRight } });
		auto pCompMS = TextDisplay::create({ .display = { .text = "---", .layout = m_pLayoutRight } });
		auto pDecompMS = TextDisplay::create({ .display = { .text = "---", .layout = m_pLayoutRight } });
		auto pCheck = TextDisplay::create({ .display = { .text = "---" } });


		m_pResultTable->slots.replaceRow(index, { pName, pRatio, pCompSize, pUncompSize, pCompMS, pDecompMS, pCheck });
		index++;
	}

	//

	auto pSummaryStyle = m_pTheme->strongStyle();

	auto pName = TextDisplay::create({ .display = { .text = "TOTAL", .style = pSummaryStyle } });
	auto pRatio = TextDisplay::create({ .display = { .text = "---", .layout = m_pLayoutRight, .style = pSummaryStyle } });
	auto pCompSize = TextDisplay::create({ .display = { .text = "---", .layout = m_pLayoutRight, .style = pSummaryStyle  } });
	auto pUncompSize = TextDisplay::create({ .display = { .text = "---", .layout = m_pLayoutRight, .style = pSummaryStyle  } });
	auto pCompMS = TextDisplay::create({ .display = { .text = "---", .layout = m_pLayoutRight, .style = pSummaryStyle } });
	auto pDecompMS = TextDisplay::create({ .display = { .text = "---", .layout = m_pLayoutRight, .style = pSummaryStyle } });
	auto pCheck = TextDisplay::create({ .display = { .text = "---", .style = pSummaryStyle } });

	m_pResultTable->slots.replaceRow(index, { pName, pRatio, pCompSize, pUncompSize, pCompMS, pDecompMS, pCheck });
}

