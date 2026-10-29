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

#include <atomic>
#include <vector>

using namespace wg;
using namespace wax;

//____ CocoaWindowSoftware _____________________________________________________
//
// Renders into a canvas in memory of our own, which is copied to a buffer that
// the layer of the view shows as an image.
//
// Core Animation may read an image as long as it holds on to it, so we never
// write to a buffer that an image still uses. We have up to c_maxBuffers of them
// and keep track of what has changed in the canvas since each of them was last
// updated, so we only need to copy that.

class CocoaWindowSoftware : public CocoaWindow
{
public:
	CocoaWindowSoftware(wapp::Window* pUserWindow, wg::Placement origin, wg::Coord pos, wg::Size size, const std::string& title, bool resizable, bool open);
	~CocoaWindowSoftware();

	void	render() override;
	void 	onWindowSizeUpdated() override;

private:

	// Pixels of a buffer, shared by the buffer and the images made from it. Freed
	// when the last of them lets go, which for an image is when Core Animation is
	// done with it.

	struct Pixels
	{
		std::atomic<int>	refs;
		uint8_t *			pData;
		size_t				size;
	};

	struct Buffer
	{
		Pixels *			pPixels = nullptr;
		std::vector<RectI>	changed;			// Rectangles of the canvas changed since last update.
		bool				bAllChanged = true;

		bool				isInUse() const { return pPixels->refs > 1; }
	};

	static const int	c_maxBuffers = 3;
	static const int	c_maxChangedRects = 64;		// More than this and we copy everything.

	Surface_p		_createCanvas();
	void			_releaseBuffers();
	Pixels *		_createPixels();
	static void		_releasePixels(Pixels * pPixels);

	void			_addChanged(const RectSPX* pRects, int nRects, bool bAll);
	void			_present();
	void			_copyRect(uint8_t* pDest, const RectI& rect);

	CALayer *		m_layer = nil;
	CGColorSpaceRef	m_colorSpace = nullptr;

	Blob_p			m_pCanvasBlob;
	SizeI			m_canvasSize;
	int				m_canvasPitch = 0;

	std::vector<Buffer>	m_buffers;
	bool			m_bPresentPending = false;		// Couldn't present last time, all buffers were in use.
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
	m_layer.contents = nil;
	[m_layer release];

	_releaseBuffers();

	if( m_colorSpace )
		CGColorSpaceRelease(m_colorSpace);
}

//____ render() _______________________________________________________________

void CocoaWindowSoftware::render()
{
	if (!m_pRootPanel || m_bDestroyed)
		return;

	m_pRootPanel->render();

	int nRects = m_pRootPanel->nbUpdatedRects();

	if( nRects > 0 )
		_addChanged(m_pRootPanel->firstUpdatedRect(), nRects, false);

	if( nRects > 0 || m_bPresentPending )
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
	_addChanged(nullptr, 0, true);
	_present();
}

//____ _createCanvas() ________________________________________________________

Surface_p CocoaWindowSoftware::_createCanvas()
{
	SizeI size = _sizeInPixels();
	size.w = std::max(size.w, 1);
	size.h = std::max(size.h, 1);

	// The buffers have the size of the canvas, so we start over with them.

	_releaseBuffers();

	// XRGB_8 is B, G, R, X in memory, which is what Core Graphics calls 32 bit
	// little endian with the alpha byte skipped first.

	m_canvasSize = size;
	m_canvasPitch = size.w * 4;
	m_pCanvasBlob = Blob::create(m_canvasPitch * size.h);

	return SoftSurface::create( WGBP(Surface, _.size = size, _.format = PixelFormat::XRGB_8, _.canvas = true, _.scale = _scale() ), m_pCanvasBlob, m_canvasPitch);
}

//____ _releaseBuffers() ______________________________________________________

void CocoaWindowSoftware::_releaseBuffers()
{
	for( auto& buffer : m_buffers )
		_releasePixels(buffer.pPixels);			// Freed when the images using them are released too.

	m_buffers.clear();
}

//____ _createPixels() ________________________________________________________

CocoaWindowSoftware::Pixels * CocoaWindowSoftware::_createPixels()
{
	size_t size = size_t(m_canvasPitch) * m_canvasSize.h;

	auto pData = (uint8_t*) malloc(size);
	if( !pData )
		return nullptr;

	auto pPixels = new Pixels;
	pPixels->refs = 1;
	pPixels->pData = pData;
	pPixels->size = size;
	return pPixels;
}

//____ _releasePixels() _______________________________________________________

void CocoaWindowSoftware::_releasePixels(Pixels * pPixels)
{
	if( --pPixels->refs == 0 )
	{
		free(pPixels->pData);
		delete pPixels;
	}
}

//____ _addChanged() __________________________________________________________

void CocoaWindowSoftware::_addChanged(const RectSPX* pRects, int nRects, bool bAll)
{
	for( auto& buffer : m_buffers )
	{
		if( buffer.bAllChanged )
			continue;

		if( bAll || buffer.changed.size() + nRects > c_maxChangedRects )
		{
			buffer.bAllChanged = true;
			buffer.changed.clear();
			continue;
		}

		for( int i = 0 ; i < nRects ; i++ )
		{
			// To whole pixels, rounding outwards, within the canvas.

			const RectSPX& r = pRects[i];

			int x1 = std::max(0, r.x / 64);
			int y1 = std::max(0, r.y / 64);
			int x2 = std::min(m_canvasSize.w, (r.x + r.w + 63) / 64);
			int y2 = std::min(m_canvasSize.h, (r.y + r.h + 63) / 64);

			if( x2 > x1 && y2 > y1 )
				buffer.changed.push_back({ x1, y1, x2 - x1, y2 - y1 });
		}
	}
}

//____ _present() _____________________________________________________________

void CocoaWindowSoftware::_present()
{
	// Pick a buffer that no image uses any more. Create one if needed.

	int index = -1;

	for( int i = 0 ; i < (int) m_buffers.size() ; i++ )
	{
		if( !m_buffers[i].isInUse() )
		{
			index = i;
			break;
		}
	}

	if( index < 0 && (int) m_buffers.size() < c_maxBuffers )
	{
		Pixels * pPixels = _createPixels();
		if( pPixels )
		{
			m_buffers.push_back( { pPixels, {}, true } );
			index = int(m_buffers.size()) - 1;
		}
	}

	if( index < 0 )
	{
		m_bPresentPending = true;			// Try again at next render.
		return;
	}

	// Copy what has changed since the buffer was last updated.

	Buffer& buffer = m_buffers[index];

	if( buffer.bAllChanged )
		_copyRect(buffer.pPixels->pData, { 0, 0, m_canvasSize.w, m_canvasSize.h });
	else
	{
		for( auto& rect : buffer.changed )
			_copyRect(buffer.pPixels->pData, rect);
	}

	buffer.changed.clear();
	buffer.bAllChanged = false;

	m_bPresentPending = false;

	// Make an image of the buffer, without copying it. It keeps the pixels in use
	// until Core Animation releases it.

	buffer.pPixels->refs++;

	CGDataProviderRef pProvider = CGDataProviderCreateWithData( buffer.pPixels, buffer.pPixels->pData, buffer.pPixels->size,
																[](void* pInfo, const void*, size_t) { _releasePixels((Pixels*) pInfo); });

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

//____ _copyRect() ____________________________________________________________
//
// The X byte of XRGB_8 is undefined, so we make it opaque while copying.

void CocoaWindowSoftware::_copyRect(uint8_t* pDest, const RectI& rect)
{
	size_t offset = size_t(rect.y) * m_canvasPitch + rect.x * 4;

	const uint8_t * pSrcRow = (const uint8_t*) m_pCanvasBlob->data() + offset;
	uint8_t * pDestRow = pDest + offset;

	for( int y = 0 ; y < rect.h ; y++ )
	{
		auto pSrc = (const uint32_t*) pSrcRow;
		auto pDst = (uint32_t*) pDestRow;

		for( int x = 0 ; x < rect.w ; x++ )
			pDst[x] = pSrc[x] | 0xFF000000;

		pSrcRow += m_canvasPitch;
		pDestRow += m_canvasPitch;
	}
}
