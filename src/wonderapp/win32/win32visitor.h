/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/
#pragma once

#include <wonderapp.h>
#include <wondergui.h>


//____ Win32Visitor ________________________________________________________

class Win32Visitor : public WonderApp::Visitor
{
public:

	std::vector<std::string>	programArguments() const override;

	int64_t						time() override;

	wg::Blob_p 					loadBlob(const std::string& path) override;
	wg::Surface_p 				loadSurface(const std::string& path, wg::SurfaceFactory* pFactory = nullptr, 
											const wg::Surface::Blueprint& bp = wg::Surface::Blueprint()) override;

	wg::Theme_p					initDefaultTheme() override;

	bool						notifyPopup(const std::string& title, const std::string& message, WonderApp::IconType iconType) override;

	WonderApp::DialogButton		messageBox(	const std::string& title, const std::string& message, WonderApp::DialogType dialogType,
											WonderApp::IconType iconType, WonderApp::DialogButton defaultButton = WonderApp::DialogButton::Undefined) override;

	std::string					inputBox(const std::string& title, const std::string& message, const std::string& defaultInput) override;

	std::string					saveFileDialog( const std::string& title, const std::string& defaultPathAndFile,
												const std::vector<std::string>& filterPatterns,
												const std::string& singleFilterDescription) override;

	std::string					openFileDialog( const std::string& title, const std::string& defaultPathAndFile,
												const std::vector<std::string>& filterPatterns,
												const std::string& singleFilterDescription) override;

	std::vector<std::string>	openMultiFileDialog(const std::string& title, const std::string& defaultPathAndFile,
												const std::vector<std::string>& filterPatterns,
												const std::string& singleFilterDescription) override;

	std::string					selectFolderDialog(const std::string& title, const std::string& defaultPath) override;

	Window_p					createWindow(const Window::Blueprint& blueprint) override;

	WonderApp::LibId			openLibrary(const std::string& path) override;
	void*						loadSymbol(WonderApp::LibId lib, const std::string& symbol) override;
	bool						closeLibrary(WonderApp::LibId lib) override;

	std::string					resourceDirectory() override;

};