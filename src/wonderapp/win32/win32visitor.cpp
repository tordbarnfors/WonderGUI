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

#include <win32visitor.h>
#include <win32window.h>

#include <windows.h>

using namespace wg;

//____ programArguments() _____________________________________________________

std::vector<std::string> Win32Visitor::programArguments() const
{
	//TODO: Implement!!!
	return std::vector<std::string>();
}

//____ time() _________________________________________________________________

int64_t Win32Visitor::time()
{
	//TODO: Implement!!!
	return 0;
}

//____ loadBlob() _____________________________________________________________

wg::Blob_p Win32Visitor::loadBlob(const std::string& path)
{
	Blob_p	pBlob;
	DWORD	bytesRead = 0;

	HANDLE hFile = CreateFile(
		path.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
		return nullptr;

	auto fileSize = GetFileSize(hFile, NULL);
	if (fileSize == INVALID_FILE_SIZE)
		goto cleanup;

	if (fileSize == 0)
		goto cleanup;

	pBlob = Blob::create(fileSize);

	if (!ReadFile(hFile, pBlob->data(), fileSize, &bytesRead, NULL))
	{
		pBlob = nullptr;
		goto cleanup;
	}

	if (bytesRead != fileSize)
		pBlob = nullptr;

cleanup:
	CloseHandle(hFile);
	return pBlob;
}

//____ loadSurface() __________________________________________________________

wg::Surface_p Win32Visitor::loadSurface(const std::string& path, wg::SurfaceFactory* pFactory, const wg::Surface::Blueprint& bp)
{
	//TODO: Implement!!!
	return nullptr;
}

//____ initDefaultTheme() _____________________________________________________

wg::Theme_p Win32Visitor::initDefaultTheme()
{
	//TODO: Implement!!!
	return nullptr;
}

//____ notifyPopup() __________________________________________________________

bool Win32Visitor::notifyPopup(const std::string& title, const std::string& message, WonderApp::IconType iconType)
{
	//TODO: Implement!!!
	return false;
}

//____ messageBox() ___________________________________________________________

WonderApp::DialogButton Win32Visitor::messageBox(const std::string& title, const std::string& message, WonderApp::DialogType dialogType,
	WonderApp::IconType iconType, WonderApp::DialogButton defaultButton)
{
	//TODO: Implement!!!
	return WonderApp::DialogButton::Undefined;
}

//____ inputBox() _____________________________________________________________

std::string Win32Visitor::inputBox(const std::string& title, const std::string& message, const std::string& defaultInput)
{
	//TODO: Implement!!!
	return "";
}

//____ saveFileDialog() _______________________________________________________

std::string Win32Visitor::saveFileDialog(const std::string& title, const std::string& defaultPathAndFile,
	const std::vector<std::string>& filterPatterns, const std::string& singleFilterDescription)
{
	//TODO: Implement!!!
	return "";
}

//____ openFileDialog() _______________________________________________________

std::string Win32Visitor::openFileDialog(const std::string& title, const std::string& defaultPathAndFile,
	const std::vector<std::string>& filterPatterns, const std::string& singleFilterDescription)
{
	//TODO: Implement!!!
	return "";
}

//____ openMultiFileDialog() __________________________________________________

std::vector<std::string> Win32Visitor::openMultiFileDialog(const std::string& title, const std::string& defaultPathAndFile,
	const std::vector<std::string>& filterPatterns, const std::string& singleFilterDescription)
{
	//TODO: Implement!!!
	return std::vector<std::string>();
}

//____ selectFolderDialog() ___________________________________________________

std::string Win32Visitor::selectFolderDialog(const std::string& title, const std::string& defaultPath)
{
	//TODO: Implement!!!
	return "";
}

//____ createWindow() _________________________________________________________

Window_p Win32Visitor::createWindow(const Window::Blueprint& blueprint)
{
	return Win32Window::create(blueprint);
}

//____ openLibrary() __________________________________________________________

WonderApp::LibId Win32Visitor::openLibrary(const std::string& path)
{
	//TODO: Implement!!!
	return nullptr;
}

//____ loadSymbol() ___________________________________________________________

void* Win32Visitor::loadSymbol(WonderApp::LibId lib, const std::string& symbol)
{
	//TODO: Implement!!!
	return nullptr;
}

//____ closeLibrary() _________________________________________________________

bool Win32Visitor::closeLibrary(WonderApp::LibId lib)
{
	//TODO: Implement!!!
	return false;
}

//____ resourceDirectory() ____________________________________________________

std::string Win32Visitor::resourceDirectory()
{
	//TODO: Implement!!!
	return "";
}

