
#pragma once

#include <wonderapp.h>
#include <wondergui.h>

#include "kerneldb.h"


#include <initializer_list>
#include <string>

class MyApp : public WonderApp
{
public:

	bool		init(wapp::API* pAPI) override;
	bool		update() override;
	void		exit() override;

	void		closeWindow(wapp::Window* pWindow) override;

	void		tintModeToggled(wg::Msg* pMsg);
	void		blendModeToggled(wg::Msg* pMsg);
	void		sourceFormatToggled(wg::Msg* pMsg);
	void		destFormatToggled(wg::Msg* pMsg);

	void		customBlitTypeToggled(int index, wg::Msg* pMsg);
	void		customBlitTintModeToggled(int index, wg::Msg* pMsg);
	void		customBlitBlendModeToggled(int index, wg::Msg* pMsg);
	void		customBlitSourceFormatToggled(int index, wg::Msg* pMsg);
	void		customBlitDestFormatToggled(int index, wg::Msg* pMsg);


	void		eraseCustomBlitEntry(int index);

	void		addOptimizedBlitEntry();
	bool		exportSource();
	void		clear();
	void		reset();
	void		load();
	void		save();


private:

	bool			_setupGUI(wapp::API* pAPI);
	wg::Widget_p	_buildButtonRow();
	wg::Widget_p	_buildList();

	wg::Widget_p	_buildGlobalSettingsSection();
	wg::Widget_p	_buildOptimizedBlitsSection();
	wg::Widget_p	_buildListSummarySection();
	wg::Widget_p	_buildExportSection();

	wg::Widget_p	_buildLabeledList(int nColumns, std::initializer_list < std::pair<std::string, int>> list);
	wg::PackPanel_p	_buildToggleButtonRow(std::string title, std::vector<KernelDB::BlitType> blitTypes, const bool selected[], std::function<void(wg::Msg*)> pressCallback);
	wg::PackPanel_p	_buildToggleButtonRow(std::string title, std::vector<wg::BlendMode> blendModes, const bool selected[], std::function<void(wg::Msg*)> pressCallback);
	wg::PackPanel_p	_buildToggleButtonRow(std::string title, std::vector<wg::TintMode> tintModes, const bool selected[], std::function<void(wg::Msg*)> pressCallback);
	wg::PackPanel_p	_buildToggleButtonRow(std::string title, std::vector<wg::PixelFormat> pixelFormats, const bool selected[], std::function<void(wg::Msg*)> pressCallback);
	wg::Widget_p	_buildHeaderWithCloseButton(std::string title, std::function<void(wg::Msg*)> pressCallback);

	void			_refreshSummary();
	void			_refreshList();


	KernelDB *			m_pDB = nullptr;
	wapp::API *			m_pAPI = nullptr;
	
	wapp::Window_p		m_pWindow;

	wg::Theme_p			m_pTheme;

	wg::TextStyle_p		m_pTextStyle;
	wg::TextStyle_p		m_pLabelStyle;

	wg::TextLayout_p	m_pTextLayoutCentered;

	wg::ScrollPanel_p	m_pScrollPanel;
	wg::PackPanel_p		m_pList;
	wg::LineEditor_p	m_pKernelFuncNameEditor;

	wg::PackLayout_p	m_pNoExpandLayout;
};
