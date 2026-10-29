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
#include <cocoaapi.h>
#include <cocoagfxbackend.h>

#include <wappimageloader.h>
#include <wax_freetypefont.h>
#include <widgetkits/wg_oldskool.h>

#include "tinyfiledialogs.h"

#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <dlfcn.h>
#include <algorithm>

using namespace wg;
using namespace wax;
using namespace wapp;

extern std::vector<CocoaWindow*>	g_windows;
extern wg::DebugFrontend_p			g_pDebugFrontend;
extern int							g_argc;
extern char**						g_argv;

static const char * iconNames[4] = { "info", "warning", "error", "question" };

//____ _nsString() _____________________________________________________________

static NSString * _nsString(const std::string& str)
{
	NSString * p = [NSString stringWithUTF8String:str.c_str()];
	return p ? p : @"";
}

//____ programArguments() _________________________________________________

std::vector<std::string> CocoaAPI::programArguments() const
{
	std::vector<std::string>	args;

	for (int i = 1; i < g_argc; i++)
		args.push_back(g_argv[i]);

	return args;
}

//____ time() _____________________________________________________________

int64_t CocoaAPI::time()
{
	return cocoaTimestamp();
}

//____ loadBlob() _________________________________________________________

Blob_p CocoaAPI::loadBlob(const std::string& path, bool bNullTerminate)
{
	FILE* fp = fopen(path.c_str(), "rb");
	if (!fp)
		return 0;

	fseek(fp, 0, SEEK_END);
	int size = (int)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	Blob_p pBlob = Blob::create(size, bNullTerminate);

	int nRead = (int)fread(pBlob->data(), 1, size, fp);
	fclose(fp);

	if (nRead < size)
		return 0;

	return pBlob;
}

//____ loadSurface() ______________________________________________________

Surface_p CocoaAPI::loadSurface(const std::string& path, SurfaceFactory* pFactory, const Surface::Blueprint& _bp)
{
	return wappLoadSurface(path, pFactory, _bp);
}

//____ initDefaultWidgetKit() ____________________________________________________

bool CocoaAPI::initDefaultWidgetKit()
{
	if( !wkit::isInitialized() )
	{
		auto path = resourceDirectory();

		auto pFont1Blob = loadBlob( path + "NotoSans-Regular.ttf");
		auto pFont2Blob = loadBlob( path + "NotoSans-Bold.ttf");
		auto pFont3Blob = loadBlob( path + "NotoSans-Italic.ttf");
		auto pFont4Blob = loadBlob( path + "DroidSansMono.ttf");

		auto pFont1 = FreeTypeFont::create(pFont1Blob);
		auto pFont2 = FreeTypeFont::create(pFont2Blob);
		auto pFont3 = FreeTypeFont::create(pFont3Blob);
		auto pFont4 = FreeTypeFont::create(pFont4Blob);

		if (!wkit::init(pFont1, pFont2, pFont3, pFont4))
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to init default widget kit", nullptr, nullptr, __func__, __FILE__, __LINE__);
			return false;
		}
	}

	return true;
}

//____ notifyPopup() __________________________________________________________
//
// Native notifications need an app bundle, so we let tinyfiledialogs do this.

bool CocoaAPI::notifyPopup(const std::string& title, const std::string& message, wapp::IconType iconType)
{
	if( iconType == wapp::IconType::Question )
		return false;

	tinyfd_notifyPopup( title.c_str(), message.c_str(), iconNames[int(iconType)]);
	return true;
}

//____ messageBox() ___________________________________________________________

wapp::DialogButton CocoaAPI::messageBox(	const std::string& title, const std::string& message,
											wapp::DialogType dialogType, wapp::IconType iconType,
											wapp::DialogButton defaultButton)
{
	// Buttons in the order AppKit places them, from the right.

	std::vector<wapp::DialogButton> buttons;

	switch( dialogType )
	{
		case wapp::DialogType::Ok:			buttons = { DialogButton::Ok };										break;
		case wapp::DialogType::OkCancel:	buttons = { DialogButton::Ok, DialogButton::Cancel };				break;
		case wapp::DialogType::YesNo:		buttons = { DialogButton::Yes, DialogButton::No };					break;
		case wapp::DialogType::YesNoCancel:	buttons = { DialogButton::Yes, DialogButton::No, DialogButton::Cancel };	break;
	}

	if( defaultButton == DialogButton::Undefined )
		defaultButton = buttons.front();
	else if( std::find(buttons.begin(), buttons.end(), defaultButton) == buttons.end() )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Default button isn't a button of specified dialog type", nullptr, nullptr, __func__, __FILE__, __LINE__);
		return wapp::DialogButton::Undefined;
	}

	NSAlert * pAlert = [[NSAlert alloc] init];
	pAlert.messageText = _nsString(title);
	pAlert.informativeText = _nsString(message);

	switch( iconType )
	{
		case wapp::IconType::Error:		pAlert.alertStyle = NSAlertStyleCritical;		break;
		case wapp::IconType::Warning:	pAlert.alertStyle = NSAlertStyleWarning;		break;
		default:						pAlert.alertStyle = NSAlertStyleInformational;	break;
	}

	for( auto button : buttons )
	{
		NSString * pLabel = @"OK";
		switch( button )
		{
			case DialogButton::Cancel:	pLabel = @"Cancel";	break;
			case DialogButton::Yes:		pLabel = @"Yes";	break;
			case DialogButton::No:		pLabel = @"No";		break;
			default:										break;
		}

		NSButton * pButton = [pAlert addButtonWithTitle:pLabel];

		// AppKit makes the first button the default one, we decide ourselves.

		if( button == defaultButton )
			pButton.keyEquivalent = @"\r";
		else if( button == DialogButton::Cancel )
			pButton.keyEquivalent = @"\033";
		else
			pButton.keyEquivalent = @"";
	}

	NSModalResponse response = [pAlert runModal];
	[pAlert release];

	int index = int(response - NSAlertFirstButtonReturn);
	if( index < 0 || index >= int(buttons.size()) )
		return buttons.back();

	return buttons[index];
}

//____ inputBox() _____________________________________________________________

std::string CocoaAPI::inputBox(const std::string& title, const std::string& message, const std::string& defaultInput)
{
	NSAlert * pAlert = [[NSAlert alloc] init];
	pAlert.messageText = _nsString(title);
	pAlert.informativeText = _nsString(message);
	[pAlert addButtonWithTitle:@"OK"];
	[pAlert addButtonWithTitle:@"Cancel"];

	NSTextField * pField = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
	pField.stringValue = _nsString(defaultInput);
	pAlert.accessoryView = pField;
	pAlert.window.initialFirstResponder = pField;

	std::string result;

	if( [pAlert runModal] == NSAlertFirstButtonReturn )
		result = pField.stringValue.UTF8String;

	[pField release];
	[pAlert release];
	return result;
}

//____ _setupPanel() __________________________________________________________
//
// Patterns such as "*.png" become the allowed file types. A pattern that lets
// anything through, such as "*" or "*.*", removes the filter.

static void _setupPanel( NSSavePanel * pPanel, const std::string& title, const std::string& defaultPath, const std::vector<std::string>& filterPatterns )
{
	pPanel.title = _nsString(title);
	pPanel.message = _nsString(title);

	if( !defaultPath.empty() )
		pPanel.directoryURL = [NSURL fileURLWithPath:_nsString(defaultPath) isDirectory:YES];

	NSMutableArray<UTType*> * pTypes = [NSMutableArray array];

	for( auto& pattern : filterPatterns )
	{
		std::string ext = pattern;
		if( ext.rfind("*.", 0) == 0 )
			ext = ext.substr(2);
		else if( ext.rfind("*", 0) == 0 )
			ext = ext.substr(1);

		if( ext.empty() || ext == "*" )
		{
			[pTypes removeAllObjects];
			break;
		}

		UTType * pType = [UTType typeWithFilenameExtension:_nsString(ext)];
		if( pType )
			[pTypes addObject:pType];
	}

	if( pTypes.count > 0 )
		pPanel.allowedContentTypes = pTypes;
}

//____ saveFileDialog() _______________________________________________________

std::string CocoaAPI::saveFileDialog(	const std::string& title, const std::string& defaultPath, const std::string& defaultFile,
										const std::vector<std::string>& filterPatterns,
										const std::string& singleFilterDescription)
{
	NSSavePanel * pPanel = [NSSavePanel savePanel];
	_setupPanel(pPanel, title, defaultPath, filterPatterns);
	pPanel.canCreateDirectories = YES;

	if( !defaultFile.empty() )
		pPanel.nameFieldStringValue = _nsString(defaultFile);

	if( [pPanel runModal] != NSModalResponseOK || !pPanel.URL )
		return std::string();

	return std::string(pPanel.URL.path.UTF8String);
}

//____ _openFileDialog() ______________________________________________________

std::vector<std::string> CocoaAPI::_openFileDialog(	const std::string& title, const std::string& defaultPath, const std::string& defaultFile,
													const std::vector<std::string>& filterPatterns, bool bMultiple )
{
	NSOpenPanel * pPanel = [NSOpenPanel openPanel];
	_setupPanel(pPanel, title, defaultPath, filterPatterns);
	pPanel.canChooseFiles = YES;
	pPanel.canChooseDirectories = NO;
	pPanel.allowsMultipleSelection = bMultiple ? YES : NO;

	// An open panel can't preselect a file, so defaultFile is not used.

	std::vector<std::string> paths;

	if( [pPanel runModal] == NSModalResponseOK )
	{
		for( NSURL * pURL in pPanel.URLs )
			paths.push_back(pURL.path.UTF8String);
	}

	return paths;
}

//____ openFileDialog() _______________________________________________________

std::string CocoaAPI::openFileDialog(	const std::string& title, const std::string& defaultPath, const std::string& defaultFile,
										const std::vector<std::string>& filterPatterns,
										const std::string& singleFilterDescription)
{
	auto paths = _openFileDialog(title, defaultPath, defaultFile, filterPatterns, false);
	return paths.empty() ? std::string() : paths.front();
}

//____ openMultiFileDialog() __________________________________________________

std::vector<std::string> CocoaAPI::openMultiFileDialog(	const std::string& title,
														const std::string& defaultPath, const std::string& defaultFile,
														const std::vector<std::string>& filterPatterns,
														const std::string& singleFilterDescription)
{
	return _openFileDialog(title, defaultPath, defaultFile, filterPatterns, true);
}

//____ selectFolderDialog() ___________________________________________________

std::string CocoaAPI::selectFolderDialog(const std::string& title, const std::string& defaultPath)
{
	NSOpenPanel * pPanel = [NSOpenPanel openPanel];
	_setupPanel(pPanel, title, defaultPath, {});
	pPanel.canChooseFiles = NO;
	pPanel.canChooseDirectories = YES;
	pPanel.canCreateDirectories = YES;
	pPanel.allowsMultipleSelection = NO;

	if( [pPanel runModal] != NSModalResponseOK || !pPanel.URL )
		return std::string();

	return std::string(pPanel.URL.path.UTF8String);
}

//____ _createWindow() ________________________________________________________

WindowAPI::Result CocoaAPI::_createWindow( Window * pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	WindowAPI::Result result;

	auto pWindow = createBackendWindow(pUserWindow, origin, pos, size, title, resizable, open);

	if( !pWindow->rootPanel() )
	{
		delete pWindow;

		result.success = false;
		result.errorMsg = "Failed to create window";
		result.pSysCalls = nullptr;
		return result;
	}

	g_windows.push_back(pWindow);

	result.pSysCalls = pWindow;
	result.success = true;
	result.geo = {pos,size};
	result.root = pWindow->rootPanel();
	result.debugger = g_pDebugFrontend;

	return result;
}

//____ openLibrary() __________________________________________________________

wapp::LibId CocoaAPI::openLibrary(const std::string& path)
{
	std::string fullPath = "lib" + path + ".dylib";
	return dlopen(fullPath.c_str(), RTLD_LAZY | RTLD_LOCAL);
}

//____ loadSymbol() __________________________________________________________

void* CocoaAPI::loadSymbol(wapp::LibId lib, const std::string& symbol)
{
	return dlsym(lib, symbol.c_str());
}

//____ closeLibrary() _________________________________________________________

bool CocoaAPI::closeLibrary(wapp::LibId lib)
{
	return (dlclose(lib) == 0);
}

//____ resourceDirectory() ____________________________________________________

std::string CocoaAPI::resourceDirectory()
{
	return "resources/";
}
