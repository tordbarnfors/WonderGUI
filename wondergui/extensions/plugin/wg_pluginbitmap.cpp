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
#include <wg_plugincalls.h>
#include <wg_pluginbitmap.h>


namespace wg
{
	const TypeInfo PluginBitmap::TYPEINFO = { "PluginBitmap", &Bitmap::TYPEINFO };


	//____ create ______________________________________________________________

	PluginBitmap_p PluginBitmap::create( wax_obj object)
	{
		PluginBitmap_p pBitmap = new PluginBitmap(nullptr);
		pBitmap->_attach(object);
		return pBitmap;
	}

	//____ constructor _____________________________________________________________

	PluginBitmap::PluginBitmap( const Restorer& restorer ) : Bitmap( Blueprint(), PixelFormat::ARGB_8, SampleMethod::Bilinear )
	{
		m_restorer = restorer;
	}

	//____ _attach() _______________________________________________________________

	void PluginBitmap::_attach(wax_obj object)
	{
		PluginCalls::object->retain(object);

		m_cBitmap = object;

		// We have initialized with an empty Blueprint. Now lets set correct values.

		m_id				= PluginCalls::bitmap->getBitmapIdentity(object);
		m_scale				= PluginCalls::bitmap->bitmapScale(object);
		m_pixelFormat		= (PixelFormat)PluginCalls::bitmap->bitmapPixelFormat(object);
		m_colorSpace		= (ColorSpace)PluginCalls::bitmap->bitmapColorSpace(object);
		m_bBigEndian		= PluginCalls::bitmap->bitmapIsBigEndian(object) != 0;
		m_pixelDescription	= * (const PixelDescription*)PluginCalls::bitmap->bitmapPixelDescription(object);

		wax_sizeI pixSize 	= PluginCalls::bitmap->bitmapPixelSize(object);
		m_size				= * (SizeI*) &pixSize;
		m_sampleMethod		= (SampleMethod)PluginCalls::bitmap->bitmapSampleMethod(object);
		m_bMipmapped		= PluginCalls::bitmap->bitmapIsMipmapped(object);
		m_bTiling			= PluginCalls::bitmap->bitmapIsTiling(object);
		m_bCanvas			= PluginCalls::bitmap->bitmapCanBeCanvas(object);
		m_pPalette			= (Color8*)PluginCalls::bitmap->bitmapPalette(object);
		m_paletteSize		= PluginCalls::bitmap->bitmapPaletteSize(object);
		m_paletteCapacity	= PluginCalls::bitmap->bitmapPaletteCapacity(object);
		m_pUserData			= reinterpret_cast<Object*>(PluginCalls::bitmap->getBitmapUserData(object));
	}

	//____ Destructor ______________________________________________________________

	PluginBitmap::~PluginBitmap()
	{
		if (!m_cBitmap)
			return;									// The host never created its bitmap.

		if (m_cObserverId != 0)
			PluginCalls::bitmap->removeBitmapObserver(m_cBitmap,m_cObserverId);

		// The bitmap of the host may outlive us, held by a font of the host or by what is
		// about to be rendered, and it calls _restoreFromHost() with a pointer to us.

		if (m_restorer)
			PluginCalls::bitmap->dropBitmapRestorer(m_cBitmap);

		PluginCalls::object->release(m_cBitmap);
	}

	//____ _restoreFromHost() ______________________________________________________

	// The restorer the host gets for a bitmap of ours that has one. It runs when the host
	// has replaced the device of its backend, on the thread the host does that on.

	uint8_t PluginBitmap::_restoreFromHost( wax_obj bitmap, void * pData )
	{
		PluginBitmap_p pBitmap = static_cast<PluginBitmap*>(pData);		// Kept while its restorer runs.

		return pBitmap->m_restorer && pBitmap->m_restorer(pBitmap);
	}

	//____ isContentLost() _________________________________________________________

	bool PluginBitmap::isContentLost() const
	{
		return PluginCalls::bitmap->bitmapIsContentLost(m_cBitmap) != 0;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginBitmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ scale() ____________________________________________________________

	int PluginBitmap::scale() const
	{
		return PluginCalls::bitmap->bitmapScale(m_cBitmap);
	}

	//____ allocPixelBuffer() _________________________________________________

	const PixelBuffer PluginBitmap::allocPixelBuffer(const RectI& rect)
	{
		auto pixbuf = PluginCalls::bitmap->allocPixelBufferFromRect(m_cBitmap, (const wax_rectI*)&rect);

		return { (PixelFormat) pixbuf.format, pixbuf.pixels, (Color8*) pixbuf.palette, * (RectI*) &pixbuf.rect, pixbuf.pitch,
				 (ColorSpace) pixbuf.colorSpace, pixbuf.bigEndian != 0 };
	}

	//____ pushPixels() _______________________________________________________

	bool PluginBitmap::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		wax_pixelBuffer pixbuf = { (wax_pixelFormat)buffer.format, buffer.pixels, (wax_color8*)buffer.palette, *(wax_rectI*)&buffer.rect, buffer.pitch,
								  (wax_colorSpace)buffer.colorSpace, (uint8_t) buffer.bigEndian };

		return PluginCalls::bitmap->pushPixelsFromRect(m_cBitmap, &pixbuf, (const wax_rectI*) &bufferRect);
	}

	//____ pullPixels() _______________________________________________________

	void PluginBitmap::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		//TODO: Support bAutoNotify when we are ready to change the C-API.

		wax_pixelBuffer pixbuf = { (wax_pixelFormat)buffer.format, buffer.pixels, (wax_color8*)buffer.palette, *(wax_rectI*)&buffer.rect, buffer.pitch,
								  (wax_colorSpace)buffer.colorSpace, (uint8_t) buffer.bigEndian };

		PluginCalls::bitmap->pullPixelsFromRect(m_cBitmap, &pixbuf, (const wax_rectI*)&bufferRect);

		if( bAutoNotify )
		{
			notifyObservers(1, &bufferRect);
		}
	}

	//____ freePixelBuffer() __________________________________________________

	void PluginBitmap::freePixelBuffer(const PixelBuffer& buffer)
	{
		wax_pixelBuffer pixbuf = { (wax_pixelFormat)buffer.format, buffer.pixels, (wax_color8*)buffer.palette, *(wax_rectI*)&buffer.rect, buffer.pitch,
								  (wax_colorSpace)buffer.colorSpace, (uint8_t) buffer.bigEndian };

		PluginCalls::bitmap->freePixelBuffer(m_cBitmap, &pixbuf);
	}

	//____ alpha() ____________________________________________________________

	int PluginBitmap::alpha( CoordSPX coord )
	{
		return PluginCalls::bitmap->bitmapAlpha(m_cBitmap, { coord.x, coord.y });
	}

	//____ fill() _____________________________________________________________

	bool PluginBitmap::fill(Color color)
	{
		return PluginCalls::bitmap->fillBitmap(m_cBitmap, *(wax_color*)&color);
	}

	bool PluginBitmap::fill(const RectI& region, Color color )
	{
		return PluginCalls::bitmap->fillBitmapRect( m_cBitmap, (const wax_rectI*) &region, * (wax_color*)&color );
	}

	//____ addObserver() ______________________________________________________

	int PluginBitmap::addObserver(const std::function<void(int nRects, const RectI* pRects)>& func)
	{
		if( m_pObserver == nullptr )
			m_cObserverId = PluginCalls::bitmap->addBitmapObserver(m_cBitmap, [](int nRects, const wax_rectI* pRects, void* pData, int data) { ((PluginBitmap*)pData)->notifyObservers(nRects, (RectI*)pRects); }, this, 0);

		return Bitmap::addObserver(func);
	}

	//____ removeObserver() ___________________________________________________

	bool PluginBitmap::removeObserver(int observerId)
	{
		Bitmap::removeObserver(observerId);

		if (m_pObserver == nullptr)
			PluginCalls::bitmap->removeBitmapObserver(m_cBitmap, m_cObserverId);

		return true;
	}



} // namespace wg
