
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
/*
	std::string selected = pAPI->openFileDialog("File to compress", "", "", { "" }, "");

	auto pSource = pAPI->loadBlob(selected);

	auto pInspector = SurfaceFileInspector::create(pSource->data());

	auto pPixels = (uint16_t*) (((char*)pSource->data()) + pInspector->pixelDataOffset());
	auto srcSize = pInspector->pixelDataBytes();

	auto pBlob = Blob::create(pPixels, srcSize, nullptr);
	roundtrip(pBlob);



//	assert(pSource->size() % 2 == 0);

	auto pDest = (uint8_t *) Base::memStackAlloc(srcSize * 2);



	int size = compress(pDest, pPixels, pPixels + srcSize / 2);

	snprintf(m_message, 1024, "Uncompressed size: %d\nCompressed size: %d\nRatio: %d%%", srcSize, size, size * 100 / srcSize);


	Base::memStackFree(srcSize * 2);

*/
	m_pCompressor = LZCompressor::create();
//	m_pCompressor = Q565Compressor::create();


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

	// Create and populate button panel

	auto pButtonPanel = PackPanel::create( { .axis = Axis::X, .skin = pTheme->plateSkin() } );

	auto pLoadButton = WGCREATE( Button, _ = pTheme->pushButton(), _.label.text = "Select test images..." );
	auto pRunButton = WGCREATE( Button, _ = pTheme->pushButton(), _.label.text = "Run tests" );
	auto pSyntheticTestButton = WGCREATE( Button, _ = pTheme->pushButton(), _.label.text = "Run 'synthetic' test" );

	pButtonPanel->slots.pushBack({
//		{ WGCREATE( Filler ), { .weight = 1 } },
		{ pLoadButton, { .weight = 0 } },
//		{ WGCREATE( Filler ), { .weight = 0.5f } },
		{ pRunButton, { .weight = 0 } },
		{ pSyntheticTestButton, { .weight = 0 } },
		{ WGCREATE( Filler ), { .weight = 1 } }
	} );

	Base::msgRouter()->addRoute(pLoadButton, MsgType::Select, [this](Msg * pMsg){ onLoad(); } );
	Base::msgRouter()->addRoute(pRunButton, MsgType::Select, [this](Msg * pMsg){ onRun(); } );
	Base::msgRouter()->addRoute(pSyntheticTestButton, MsgType::Select, [this](Msg * pMsg){ onRunSyntheticTest(); } );

	// Create main section

	m_pResultTable = WGCREATE( TablePanel, _ = pTheme->listTable(), _.columns = 5, _.rows = 1, _.skin = pTheme->canvasSkin() );

	TextDisplay::Blueprint columnLabelBP( { .skin = pTheme->plateSkin(), .display = { .style = pTheme->strongStyle() }} );

	auto pColumnLabel1 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Filename" );
	auto pColumnLabel2 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Ratio (%)" );
	auto pColumnLabel3 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Compress speed (microsec)" );
	auto pColumnLabel4 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Decompress speed (microsec)" );
	auto pColumnLabel5 = WGCREATE( TextDisplay, _ = columnLabelBP, _.display.text = "Check" );



	m_pResultTable->slots.replaceRow( 0, { pColumnLabel1, pColumnLabel2, pColumnLabel3, pColumnLabel4, pColumnLabel5 } );




	// Create and populate main panel

	auto pMainPanel = PackPanel::create( { .axis = Axis::Y, .skin = pTheme->windowSkin() } );

	pMainPanel->slots.pushBack( pButtonPanel, { .weight = 0 } );
	pMainPanel->slots.pushBack( m_pResultTable, { .weight = 1 } );



	m_pWindow->mainCapsule()->slot = pMainPanel;
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

		auto pResultDisplay = wg_dynamic_cast<TextDisplay_p>(m_pResultTable->slots[i+1][4].widget());

		bool bEquivalent = true;

		for( int j = 0 ; j < srcSize ; j++ )
		{
			if( pixbuf.pixels[j] != pDecompressArea[j] )
			{
				char temp[64];
				snprintf( temp, 64, "ERROR at offset %d%", j );

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

				// Update ratio display

				auto pRatioDisplay = wg_dynamic_cast<TextDisplay_p>(m_pResultTable->slots[i+1][1].widget());

				char temp[16];
				snprintf( temp, 16, "%d%", compressedSize*100/srcSize );

				pRatioDisplay->display.setText( temp );

				// Update Compress speed

				auto pCompTimeDisplay = wg_dynamic_cast<TextDisplay_p>(m_pResultTable->slots[i+1][2].widget());

				snprintf( temp, 16, "%d%", int(afterCompTS-beforeCompTS) );
				pCompTimeDisplay->display.setText( temp );

				// Update Decompress speed

				auto pDecompTimeDisplay = wg_dynamic_cast<TextDisplay_p>(m_pResultTable->slots[i+1][3].widget());

				snprintf( temp, 16, "%d%", int(afterDecompTS-beforeDecompTS) );
				pDecompTimeDisplay->display.setText( temp );

			}
		}

		// Cleanup

		Base::memStackFree(srcSize+9);
		Base::memStackFree(memNeeded);
		pSurface->freePixelBuffer(pixbuf);
	}

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
	m_pResultTable->rows.resize(int(m_testSurfaces.size())+1);

	int index = 1;
	for( auto& test : m_testSurfaces )
	{
		auto pName = TextDisplay::create({ .display = { .text = test.name.c_str() } });
		auto pRatio = TextDisplay::create({ .display = { .text = "---" } });
		auto pCompMS = TextDisplay::create({ .display = { .text = "---" } });
		auto pDecompMS = TextDisplay::create({ .display = { .text = "---" } });
		auto pCheck = TextDisplay::create({ .display = { .text = "---" } });


		m_pResultTable->slots.replaceRow(index, { pName, pRatio, pCompMS, pDecompMS, pCheck });
		index++;
	}


}
