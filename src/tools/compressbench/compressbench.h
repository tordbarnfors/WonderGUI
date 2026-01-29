
#pragma once

#include <wonderapp.h>

#include <wondergui.h>
#include <wg_freetypefont.h>

#include <initializer_list>
#include <string>

using namespace wg;

class MyApp : public WonderApp
{
public:

	bool		init(wapp::API* pAPI) override;
	bool		update() override;
	void		exit() override;

	void		closeWindow(wapp::Window* pWindow) override;


private:

	bool			_setupGUI(wapp::API* pAPI);
	bool			m_bContinue = true;

	wapp::API *		m_pAPI = nullptr;
	wapp::Window_p	m_pWindow = nullptr;

	void			onLoad();
	void			onRun();
	void			onRunSyntheticTest();
	void			onCompressorSelected();

	void			refreshList();
	void 			updateListRow( int row, int srcSize, int compressedSize, int compressMicroSec, int decompressMicroSec );


	struct TestSurface
	{
		std::string 	name;
		Surface_p		pSurface;
	};

	std::vector<TestSurface>	m_testSurfaces;

	Theme_p			m_pTheme;

	TablePanel_p	m_pResultTable;

	std::vector<Compressor_p>	m_compressors;
	Compressor_p	m_pCompressor;

	SelectBox_p		m_pCompressorSelector;

	TextLayout_p	m_pLayoutRight;

};
