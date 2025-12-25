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
#include <win32api.h>
#include <win32window.h>

#include <windows.h>
#include <shobjidl.h>
#include <fstream>
#include <iostream>

#include <wg_softsurface.h>
#include <wg_freetypefont.h>
#include <themes/simplistic/wg_simplistic.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace wg;
using namespace wapp;

extern float		g_ticksToMicroseconds;
extern Theme_p		g_pDefaultTheme;


//____ programArguments() _____________________________________________________

std::vector<std::string> Win32API::programArguments() const
{
	//TODO: Implement!!!
	return std::vector<std::string>();
}

//____ time() _________________________________________________________________

int64_t Win32API::time()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return int64_t(counter.QuadPart * g_ticksToMicroseconds);
}

//____ loadBlob() _____________________________________________________________

wg::Blob_p Win32API::loadBlob(const std::string& path, bool bNullTerminate)
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

	pBlob = Blob::create(fileSize, bNullTerminate);

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

wg::Surface_p Win32API::loadSurface(const std::string& path, wg::SurfaceFactory* pFactory, const wg::Surface::Blueprint& bp)
{
	if (path.rfind(".surf") == path.size() - 5 || path.rfind(".srf") == path.size() - 4)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input.good())
			return nullptr;

		auto pReader = SurfaceReader::create({ .factory = Base::defaultSurfaceFactory() });
		Surface_p pSurface = pReader->readSurfaceFromStream(input, bp);
		input.close();
		return pSurface;
	}
/*	else if (path.rfind(".bmp") == path.size() - 4)
	{
		HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, path.c_str(), IMAGE_BITMAP,
			0, 0, LR_LOADFROMFILE);

		if (hBitmap) {
			BITMAP bm;
			GetObject(hBitmap, sizeof(BITMAP), &bm);

			PixelFormat pixelFormat;
			switch (bm.bmBitsPixel)
			{
				case 24:
					pixelFormat = PixelFormat::BGR_8;
					break;
				case 32:
					pixelFormat = PixelFormat::BGRA_8;
					break;
				default:
					DeleteObject(hBitmap);
					return nullptr;
			}

			Surface_p pSurface = wg::SoftSurface::create({
				.canvas = false,
				.format = pixelFormat,
				.size = { (int)bm.bmWidth, (int)bm.bmHeight }
				}, 
				(uint8_t*)bm.bmBits,
				pixelFormat,
				0, nullptr, 0);

			DeleteObject(hBitmap);
			return pSurface;
		}
	} */
	else
	{
		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (data)
		{
			PixelFormat destFormat;
			PixelDescription pixelDesc;

			if (channels == 3)
			{
				pixelDesc.bits = 24;
				pixelDesc.type = PixelType::Chunky;
				pixelDesc.colorSpace = ColorSpace::sRGB;
				pixelDesc.R_mask = 0x000000FF;
				pixelDesc.G_mask = 0x0000FF00;
				pixelDesc.B_mask = 0x00FF0000;
				pixelDesc.A_mask = 0x00000000;

				destFormat = PixelFormat::BGR_8;
			}
			else if (channels == 4)
			{
				pixelDesc.bits = 32;
				pixelDesc.type = PixelType::Chunky;
				pixelDesc.colorSpace = ColorSpace::sRGB;
				pixelDesc.R_mask = 0x000000FF;
				pixelDesc.G_mask = 0x0000FF00;
				pixelDesc.B_mask = 0x00FF0000;
				pixelDesc.A_mask = 0xFF000000;

				destFormat = PixelFormat::BGRA_8;
			}
			else
			{
				stbi_image_free(data);
				return nullptr;
			}

			Surface_p pSurface = wg::SoftSurface::create({
				.canvas = false,
				.format = destFormat,
				.size = { width, height }
				},
				(uint8_t*)data,
				pixelDesc,
				0, nullptr, 0);

			stbi_image_free(data);
			return pSurface;
		}
	}
	return nullptr;
}

//____ initDefaultTheme() _____________________________________________________

wg::Theme_p Win32API::initDefaultTheme()
{
	if (g_pDefaultTheme)
		return g_pDefaultTheme;

	// Create the default theme, which is a simplistic theme.

	auto pFont1Blob = loadBlob("resources/NotoSans-Regular.ttf");
	auto pFont2Blob = loadBlob("resources/NotoSans-Bold.ttf");
	auto pFont3Blob = loadBlob("resources/NotoSans-Italic.ttf");
	auto pFont4Blob = loadBlob("resources/DroidSansMono.ttf");

	auto pFont1 = FreeTypeFont::create(pFont1Blob);
	auto pFont2 = FreeTypeFont::create(pFont2Blob);
	auto pFont3 = FreeTypeFont::create(pFont3Blob);
	auto pFont4 = FreeTypeFont::create(pFont4Blob);

	auto pThemeSurface = loadSurface("resources/skin_widgets.png");

	auto pTheme = Simplistic::create(pFont1, pFont2, pFont3, pFont4, pThemeSurface);
	if (!pTheme)
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to create default theme", nullptr, nullptr, __func__, __FILE__, __LINE__);
		return nullptr;
	}
	Base::setDefaultTheme(pTheme);
	Base::setDefaultStyle(pTheme->defaultStyle());

	g_pDefaultTheme = pTheme;

	return pTheme;
}

//____ notifyPopup() __________________________________________________________

bool Win32API::notifyPopup(const std::string& title, const std::string& message, IconType iconType)
{
	//TODO: Implement!!!
	return false;
}

//____ messageBox() ___________________________________________________________

DialogButton Win32API::messageBox(const std::string& title, const std::string& message, DialogType dialogType,
	IconType iconType, DialogButton defaultButton)
{
	//TODO: Implement!!!
	return DialogButton::Undefined;
}

//____ inputBox() _____________________________________________________________

std::string Win32API::inputBox(const std::string& title, const std::string& message, const std::string& defaultInput)
{
	//TODO: Implement!!!
	return "";
}

//____ saveFileDialog() _______________________________________________________

std::string Win32API::saveFileDialog(const std::string& title, const std::string& defaultPath, 
	const std::string& defaultFilename, const std::vector<std::string>& filterPatterns, 
	const std::string& singleFilterDescription)
{
	//TODO: Implement!!!
	return "";
}

//____ openFileDialog() _______________________________________________________

std::string Win32API::openFileDialog(const std::string& title, const std::string& defaultPath, 
	const std::string& defaultFilename, const std::vector<std::string>& filterPatterns, 
	const std::string& singleFilterDescription)
{
	std::string result;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, (void**)&pFileOpen);

		if (SUCCEEDED(hr))
		{
			pFileOpen->SetTitle(_stringToWString(title).c_str());
			pFileOpen->SetFileName(_stringToWString(defaultFilename).c_str());

			IShellItem* psiFolder;
			hr = SHCreateItemFromParsingName(_stringToWString(defaultPath).c_str(),
				NULL, IID_PPV_ARGS(&psiFolder));
			if (SUCCEEDED(hr))
			{
				pFileOpen->SetFolder(psiFolder);
				psiFolder->Release();
			}

			// Set up file type filters

			if (!filterPatterns.empty())
			{
				std::string	combinedPatterns;
				for (const auto& pattern : filterPatterns)
				{
					if (!combinedPatterns.empty())
						combinedPatterns += ";";
					combinedPatterns += pattern;
				}

				auto patternDesc = _stringToWString(singleFilterDescription);
				auto patterns = _stringToWString(combinedPatterns);

				COMDLG_FILTERSPEC fileTypes[2] = {
					{ patternDesc.c_str(), patterns.c_str()},
					{ L"All Files", L"*.*"}
				};

				pFileOpen->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);
				pFileOpen->SetFileTypeIndex(1); // 1-based index (1 = first filter)
			}


			// Show the dialog
			hr = pFileOpen->Show(NULL);

			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr))
					{
						// Get the required buffer size
						int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1,
							nullptr, 0, nullptr, nullptr);
						if (sizeNeeded > 0)
						{
							// Convert
							result.resize(sizeNeeded - 1, 0); // -1 to exclude null terminator
							WideCharToMultiByte(CP_UTF8, 0, pszFilePath, -1,
								&result[0], sizeNeeded, nullptr, nullptr);
						}

						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}

	return result;
}

//____ openMultiFileDialog() __________________________________________________

std::vector<std::string> Win32API::openMultiFileDialog(const std::string& title, 
	const std::string& defaultPath, const std::string& defaultFilename,	
	const std::vector<std::string>& filterPatterns, const std::string& singleFilterDescription)
{
	//TODO: Implement!!!
	return std::vector<std::string>();
}

//____ selectFolderDialog() ___________________________________________________

std::string Win32API::selectFolderDialog(const std::string& title, const std::string& defaultPath)
{
	//TODO: Implement!!!
	return "";
}

//____ createWindow() _________________________________________________________

WindowAPI::Result Win32API::_createWindow(Window* pAPI, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	WindowAPI::Result	result;

	auto pWindow = new Win32Window(pAPI, origin, pos, size, title, resizable, open);

	result.pSysCalls = pWindow;
	result.success = (pWindow != nullptr);
	result.geo = { pos, size };
	result.errorMsg = result.success ? "" : "Failed to create Win32 window.";
	result.root = pWindow->rootPanel();
	return result;
}

//____ openLibrary() __________________________________________________________

LibId Win32API::openLibrary(const std::string& path)
{
	//TODO: Implement!!!
	return nullptr;
}

//____ loadSymbol() ___________________________________________________________

void* Win32API::loadSymbol(LibId lib, const std::string& symbol)
{
	//TODO: Implement!!!
	return nullptr;
}

//____ closeLibrary() _________________________________________________________

bool Win32API::closeLibrary(LibId lib)
{
	//TODO: Implement!!!
	return false;
}

//____ resourceDirectory() ____________________________________________________

std::string Win32API::resourceDirectory()
{
	return "resources/";
}

//____ _stringToWString() _____________________________________________________

std::wstring Win32API::_stringToWString(const std::string& str)
{
	if (str.empty())
		return std::wstring();

	// Get the required buffer size
	int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1,
		nullptr, 0);
	if (sizeNeeded <= 0)
		return std::wstring();

	// Convert
	std::wstring result(sizeNeeded - 1, 0); // -1 to exclude null terminator
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1,
		&result[0], sizeNeeded);

	return result;
}