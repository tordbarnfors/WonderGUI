
#pragma once

#include <wonderapp.h>
#include <wondergui.h>

#include <initializer_list>
#include <string>

class MyApp : public WonderApp
{
public:
	bool		init(wapp::API* pAPI) override;
	bool		update() override;
	void		exit() override;

	void		closeWindow(wapp::Window* pWindow) override;

private:

	bool			_setupGUI(wapp::API* pAPI);
	bool			_loadSkins(wapp::API* pAPI);

	void			_updateLayout();

	wg::Widget_p 	_createControlPanel();
	wg::Widget_p	_createStretchPanel();

	wg::SelectBox_p	_createSelectBox();
	wg::SelectBox_p	_createSizeSourceSelectBox();

	wapp::Window_p		m_pWindow;

	wg::Size			m_dragStartSize;

	wg::Skin_p			m_pPlateSkin;
	wg::Skin_p			m_pButtonSkin;
	wg::Skin_p			m_pTightButtonSkin;
	wg::Skin_p			m_pToggleButtonSkin;
	wg::Skin_p			m_pCheckBoxSkin;
	wg::Skin_p			m_pSectionSkin;

	wg::PackLayout_p	m_pLayout;
	wg::PackPanel_p		m_pPackPanel;
	
	wg::TextStyle_p		m_pTextStyle;
	wg::TextStyle_p		m_pLabelStyle;

	wg::TextLayout_p	m_pTextLayoutCentered;

	wg::FlexPanel_p		m_pBasePanel;

	wg::SelectBox_p		m_pStartSizeSelector;
	wg::SelectBox_p		m_pExpandFactorSelector;
	wg::SelectBox_p		m_pShrinkFactorSelector;
	wg::SelectBox_p		m_pHideSelector;
	wg::SelectBox_p		m_pMinSizeSourceSelector;
	wg::SelectBox_p		m_pDefaultSizeSourceSelector;
	wg::SelectBox_p		m_pMaxSizeSourceSelector;

	wg::TextEditor_p	m_pList1Input;
	wg::TextEditor_p	m_pList2Input;
	wg::TextEditor_p	m_pList3Input;

	wg::TextEditor_p	m_pExpandOrderInput;
	wg::TextEditor_p	m_pShrinkOrderInput;


};


