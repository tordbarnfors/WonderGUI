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
#pragma once

#include <wonderapp.h>
#include <wondergui.h>


//____ Win32API ________________________________________________________

class Win32API : public wapp::API
{
public:

	std::vector<std::string>	programArguments() const override;

	int64_t						time() override;

	wg::Blob_p 					loadBlob(const std::string& path) override;
	wg::Surface_p 				loadSurface(const std::string& path, wg::SurfaceFactory* pFactory = nullptr, 
											const wg::Surface::Blueprint& bp = wg::Surface::Blueprint()) override;

	wg::Theme_p					initDefaultTheme() override;

	bool						notifyPopup(const std::string& title, const std::string& message, wapp::IconType iconType) override;

	wapp::DialogButton			messageBox(	const std::string& title, const std::string& message, wapp::DialogType dialogType,
											wapp::IconType iconType, wapp::DialogButton defaultButton = wapp::DialogButton::Undefined) override;

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


	wapp::LibId					openLibrary(const std::string& path) override;
	void*						loadSymbol(wapp::LibId lib, const std::string& symbol) override;
	bool						closeLibrary(wapp::LibId lib) override;

	std::string					resourceDirectory() override;

	wapp::WindowAPI::Result		_createWindow(wapp::Window* pAPI, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open) override;

private:


};