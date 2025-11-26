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

#include <win32api.h>
#include <win32window.h>

#include <windows.h>
#include <fstream>
#include <iostream>

#include <wg_softsurface.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace wg;
using namespace wapp;

extern float		g_ticksToMicroseconds;



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

wg::Blob_p Win32API::loadBlob(const std::string& path)
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
	//TODO: Implement!!!
	return nullptr;
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

std::string Win32API::saveFileDialog(const std::string& title, const std::string& defaultPathAndFile,
	const std::vector<std::string>& filterPatterns, const std::string& singleFilterDescription)
{
	//TODO: Implement!!!
	return "";
}

//____ openFileDialog() _______________________________________________________

std::string Win32API::openFileDialog(const std::string& title, const std::string& defaultPathAndFile,
	const std::vector<std::string>& filterPatterns, const std::string& singleFilterDescription)
{
	//TODO: Implement!!!
	return "";
}

//____ openMultiFileDialog() __________________________________________________

std::vector<std::string> Win32API::openMultiFileDialog(const std::string& title, const std::string& defaultPathAndFile,
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
	//TODO: Implement!!!
	return "";
}

