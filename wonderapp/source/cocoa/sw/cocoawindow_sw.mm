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
#include <cocoagfxbackend.h>

#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_softedgemapfactory.h>
#include <wax_softbackend.h>
#include <wax_softkernels_default.h>

#import <QuartzCore/QuartzCore.h>

using namespace wg;
using namespace wax;

//____ CocoaWindowSoftware _____________________________________________________
//
// Renders into a pixel buffer of our own, which is handed to the layer of the
// view as an image after each render.

class CocoaWindowSoftware : public CocoaWindow
{
public:
	CocoaWindowSoftware(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	~CocoaWindowSoftware();

	void	render() override;
	void 	onWindowSizeUpdated() override;

private:

	Surface_p		_createCanvas();
	void			_present();

	CALayer *		m_layer = nil;
	CGColorSpaceRef	m_colorSpace = nullptr;

	Blob_p			m_pCanvasBlob;
	SizeI			m_canvasSize;
	int				m_canvasPitch = 0;
};

//____ initGfxBackend() _______________________________________________________

bool initGfxBackend()
{
	auto pBackend = SoftBackend::create();
	addDefaultSoftKernels( pBackend );

	auto pSoftDevice = GfxDeviceGen2::create(pBackend);

	Base::setDefaultDevice(pSoftDevice);

	auto pSurfaceFactory = SoftSurfaceFactory::create();
	Base::setDefaultSurfaceFactory(pSurfaceFactory);

	auto pEdgemapFactory = SoftEdgemapFactory::create();
	Base::setDefaultEdgemapFactory(pEdgemapFactory);

	Base::setDefaultWarpmapFactory(pBackend->warpmapFactory());

	return true;
}

//____ exitGfxBackend() _______________________________________________________

void exitGfxBackend()
{
}

//____ createBackendWindow() __________________________________________________

CocoaWindow * createBackendWindow(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	return new CocoaWindowSoftware(pUserWindow, origin, pos, size, title, resizable, open);
}

//____ CocoaWindowSoftware() __________________________________________________

CocoaWindowSoftware::CocoaWindowSoftware(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open)
{
	m_pUserWindow = pUserWindow;

	if (!_createNSWindow(title, origin, pos, size, resizable, open))
		return;

	m_colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);

	// A layer of our own makes the view layer-hosting, so AppKit leaves its content to us.

	m_layer = [[CALayer alloc] init];
	m_layer.contentsGravity = kCAGravityTopLeft;		// No stretching while waiting for a new image.
	m_layer.contentsScale = m_window.backingScaleFactor;
	m_layer.backgroundColor = CGColorGetConstantColor(kCGColorBlack);

	[m_view setLayer:m_layer];
	[m_view setWantsLayer:YES];

	m_pRootPanel = RootPanel::create( { .canvasSurface = _createCanvas() } );
	m_pRootPanel->setWindowRef(reinterpret_cast<uintptr_t>(this));
}

//____ ~CocoaWindowSoftware() _________________________________________________

CocoaWindowSoftware::~CocoaWindowSoftware()
{
	[m_layer release];

	if( m_colorSpace )
		CGColorSpaceRelease(m_colorSpace);
}

//____ render() _______________________________________________________________

void CocoaWindowSoftware::render()
{
	if (!m_pRootPanel || m_bDestroyed)
		return;

	m_pRootPanel->render();

	if( m_pRootPanel->nbUpdatedRects() > 0 )
		_present();
}

//____ onWindowSizeUpdated() __________________________________________________

void CocoaWindowSoftware::onWindowSizeUpdated()
{
	if (!m_pRootPanel)
		return;

	m_layer.contentsScale = m_window.backingScaleFactor;

	m_pRootPanel->setCanvas(_createCanvas());
	_notifyUserWindowOfResize();

	m_pRootPanel->render();
	_present();
}

//____ _createCanvas() ________________________________________________________

Surface_p CocoaWindowSoftware::_createCanvas()
{
	SizeI size = _sizeInPixels();
	size.w = std::max(size.w, 1);
	size.h = std::max(size.h, 1);

	// XRGB_8 is B, G, R, X in memory, which is what Core Graphics calls 32 bit
	// little endian with the alpha byte skipped first.

	m_canvasSize = size;
	m_canvasPitch = size.w * 4;
	m_pCanvasBlob = Blob::create(m_canvasPitch * size.h);

	return SoftSurface::create( WGBP(Surface, _.size = size, _.format = PixelFormat::XRGB_8, _.canvas = true, _.scale = _scale() ), m_pCanvasBlob, m_canvasPitch);
}

//____ _present() _____________________________________________________________
//
// Core Animation reads the image asynchronously, so it gets a copy of the canvas
// that we don't touch again.
//
//TODO: Only copy the updated rectangles, into a buffer of our own we alternate
// with, instead of copying everything.

void CocoaWindowSoftware::_present()
{
	size_t bytes = size_t(m_canvasPitch) * m_canvasSize.h;

	void * pCopy = malloc(bytes);
	if( !pCopy )
		return;

	memcpy(pCopy, m_pCanvasBlob->data(), bytes);

	CGDataProviderRef pProvider = CGDataProviderCreateWithData(nullptr, pCopy, bytes, [](void*, const void* pData, size_t) { free((void*)pData); });

	CGImageRef image = CGImageCreate( m_canvasSize.w, m_canvasSize.h, 8, 32, m_canvasPitch, m_colorSpace,
									  CGBitmapInfo(kCGBitmapByteOrder32Little) | CGBitmapInfo(kCGImageAlphaNoneSkipFirst),
									  pProvider, nullptr, false, kCGRenderingIntentDefault );

	CGDataProviderRelease(pProvider);

	// An explicit transaction is committed right away. An implicit one would wait
	// for AppKit's run loop, which doesn't run while we are in our own loop.

	[CATransaction begin];
	[CATransaction setDisableActions:YES];
	m_layer.contents = (__bridge id)image;
	[CATransaction commit];

	CGImageRelease(image);
}
