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

#include <wg_plugincalls.h>
#include <wg_pluginsurface.h>


namespace wg
{
	const TypeInfo PluginSurface::TYPEINFO = { "PluginSurface", &Surface::TYPEINFO };


	//____ create ______________________________________________________________

	PluginSurface_p PluginSurface::create( wg_obj object)
	{
		return PluginSurface_p(new PluginSurface(object));
	}

	//____ constructor _____________________________________________________________

	PluginSurface::PluginSurface(wg_obj object) : Surface( Blueprint(), PixelFormat::BGRA_8, SampleMethod::Bilinear )
	{
		PluginCalls::object->retain(object);

		m_cSurface = object;

		// We have initialized with an empty Blueprint. Now lets set correct values.

		m_id				= PluginCalls::surface->getSurfaceIdentity(object);
		m_scale				= PluginCalls::surface->surfaceScale(object);
		m_pixelFormat		= (PixelFormat)PluginCalls::surface->surfacePixelFormat(object);
		m_pPixelDescription	= (PixelDescription*)PluginCalls::surface->surfacePixelDescription(object);
		
		wg_sizeI pixSize 	= PluginCalls::surface->surfacePixelSize(object);
		m_size				= * (SizeI*) &pixSize;
		m_sampleMethod		= (SampleMethod)PluginCalls::surface->surfaceSampleMethod(object);
		m_bMipmapped		= PluginCalls::surface->surfaceIsMipmapped(object);
		m_bTiling			= PluginCalls::surface->surfaceIsTiling(object);
		m_bCanvas			= PluginCalls::surface->surfaceCanBeCanvas(object);
		m_pPalette			= (Color8*)PluginCalls::surface->surfacePalette(object);
		m_paletteSize		= PluginCalls::surface->surfacePaletteSize(object);
		m_paletteCapacity	= PluginCalls::surface->surfacePaletteCapacity(object);
		m_pBaggage			= reinterpret_cast<Object*>(PluginCalls::surface->getSurfaceBaggage(object));
	}

	//____ Destructor ______________________________________________________________

	PluginSurface::~PluginSurface()
	{
		if (m_cObserverId != 0)
			PluginCalls::surface->removeSurfaceObserver(m_cSurface,m_cObserverId);

		PluginCalls::object->release(m_cSurface);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PluginSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ scale() ____________________________________________________________

	int PluginSurface::scale() const
	{
		return PluginCalls::surface->surfaceScale(m_cSurface);
	}

	//____ allocPixelBuffer() _________________________________________________

	const PixelBuffer PluginSurface::allocPixelBuffer(const RectI& rect)
	{
		auto pixbuf = PluginCalls::surface->allocPixelBufferFromRect(m_cSurface, (const wg_rectI*)&rect);

		return { (PixelFormat) pixbuf.format, pixbuf.pixels, (Color8*) pixbuf.palette, * (RectI*) &pixbuf.rect, pixbuf.pitch };
	}

	//____ pushPixels() _______________________________________________________

	bool PluginSurface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		wg_pixelBuffer pixbuf = { (wg_pixelFormat)buffer.format, buffer.pixels, (wg_color8*)buffer.palette, *(wg_rectI*)&buffer.rect, buffer.pitch };

		return PluginCalls::surface->pushPixelsFromRect(m_cSurface, &pixbuf, (const wg_rectI*) &bufferRect);
	}

	//____ pullPixels() _______________________________________________________

	void PluginSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		wg_pixelBuffer pixbuf = { (wg_pixelFormat)buffer.format, buffer.pixels, (wg_color8*)buffer.palette, *(wg_rectI*)&buffer.rect, buffer.pitch };

		PluginCalls::surface->pullPixelsFromRect(m_cSurface, &pixbuf, (const wg_rectI*)&bufferRect);
	}

	//____ freePixelBuffer() __________________________________________________

	void PluginSurface::freePixelBuffer(const PixelBuffer& buffer)
	{
		wg_pixelBuffer pixbuf = { (wg_pixelFormat)buffer.format, buffer.pixels, (wg_color8*)buffer.palette, *(wg_rectI*)&buffer.rect, buffer.pitch };

		PluginCalls::surface->freePixelBuffer(m_cSurface, &pixbuf);
	}

	//____ alpha() ____________________________________________________________

	int PluginSurface::alpha( CoordSPX coord )
	{
		return PluginCalls::surface->surfaceAlpha(m_cSurface, { coord.x, coord.y });
	}

	//____ fill() _____________________________________________________________

	bool PluginSurface::fill(HiColor color)
	{
		return PluginCalls::surface->fillSurface(m_cSurface, *(wg_color*)&color);
	}

	bool PluginSurface::fill(const RectI& region, HiColor color )
	{
		return PluginCalls::surface->fillSurfaceRect( m_cSurface, (const wg_rectI*) &region, * (wg_color*)&color );
	}

	//____ addObserver() ______________________________________________________

	int PluginSurface::addObserver(const std::function<void(int nRects, const RectI* pRects)>& func)
	{
		if( m_pObserver == nullptr )
			m_cObserverId = PluginCalls::surface->addSurfaceObserver(m_cSurface, [](int nRects, const wg_rectI* pRects, void* pData, int data) { ((PluginSurface*)pData)->notifyObservers(nRects, (RectI*)pRects); }, this, 0);

		return Surface::addObserver(func);
	}

	//____ removeObserver() ___________________________________________________

	bool PluginSurface::removeObserver(int observerId)
	{
		Surface::removeObserver(observerId);

		if (m_pObserver == nullptr)
			PluginCalls::surface->removeSurfaceObserver(m_cSurface, m_cObserverId);

		return true;
	}



} // namespace wg
