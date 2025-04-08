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

#include <limits>
#include <memory.h>
#include <wg_surface.h>
#include <wg_dataset.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>
#include <wg_pixeltools.h>

namespace wg
{
	using namespace Util;


	const TypeInfo Surface::TYPEINFO = { "Surface", &Object::TYPEINFO };


	//____ ~Surface() ____________________________________________________________

	Surface::~Surface()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Surface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ scale() ____________________________________________________________

	int Surface::scale() const
	{
		return m_scale;
	}

	//____ fill() _________________________________________________________________
	/**
	 * Fill the surface with the specified color.
	 *
	 * @param col	Color value with which to fill the surface.
	 *
	 * The fill instruction replaces the surface content with the pixel values so
	 * no blending operations will take place. If the specified color has an alpha
	 * value it will replace the alpha channel of the surface (if it has any), not blend the color
	 * onto the surface.
	 *
	 * If the surface has less than 24-bits or color information, it will be filled with the
	 * pixel value most closely resembling the one specified.
	 *
	 **/

	bool Surface::fill( HiColor color )
	{
		return fill(RectI(0, 0, pixelSize()), color );
	}

	/**
	 * Fill the specied region of the surface with the specified color.
	 *
	 * @param col		Color value with which to fill the region.
	 * @param region	Region of surface to fill.
	 *
	 * The fill instruction replaces the surface content with the pixel values so
	 * no blending operations will take place. If the specified color has an alpha
	 * value it will replace the alpha channel of the surface (if it has any), not blend the color
	 * onto the surface.
	 *
	 * If the surface has less than 24-bits or color information, it will be filled with the
	 * pixel value most closely resembling the one specified.
	 *
	 **/
	bool Surface::fill(const RectI& region, HiColor color  )
	{
		auto pixbuf = allocPixelBuffer(region);

		PixelTools::fillBitmap(pixbuf.pixels, pixbuf.format, pixbuf.pitch, pixbuf.rect.size(), color);
		
		pullPixels(pixbuf);
		freePixelBuffer(pixbuf);

		return true;
	}

	//____ pullPixels() _______________________________________________________
	/**
	 * @brief Partial copy of pixels from PixelBuffer to Surface.
	 *
	 * Partial copy of pixels from PixelBuffer to Surface.
	 *
	 * @param buffer		PixelBuffer to copy pixels from.
	 * @param bufferRect	The source rectangle withing the PixelBuffer.
	 * @return 				True if operation could be performed.
	 *
	 * Only the specified rectangle within the PixelBuffer is copied to the Surface,
	 * the rest of the surface is unaffected. To copy the whole PixelBuffer to Surface, use pullPixels(const PixelBuffer& buffer) instead.
	 *
	 * Please note that the rectangle specified is within the PixelBuffer, not the Surface. You should therefore not add
	 * the offset of the PixelBuffer to bufferRect.
	 *
	 */
	void Surface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		RectSPX rect = (bufferRect + buffer.rect.pos())*64;

		_notifyObservers(1, &rect);
	}


	//____ addObserver() ______________________________________________________

	int Surface::addObserver(const std::function<void(int nRects, const RectSPX* pRects)>& func)
	{
		int id = m_pObserver ? m_pObserver->id + 1 : 1;
		auto p = new Observer();

		p->id = id;
		p->func = func;
		p->pNext = m_pObserver;

		m_pObserver = p;
		return id;
	}

	//____ removeObserver() ___________________________________________________

	bool Surface::removeObserver(int observerId)
	{
		Observer** pPointer = &m_pObserver;

		while (*pPointer != nullptr)
		{
			if ((*pPointer)->id == observerId)
			{
				auto p = (*pPointer);
				(*pPointer) = p->pNext;
				delete p;
				return true;
			}

			pPointer = &(*pPointer)->pNext;
		}

		return false;
	}

	//____ _notifyObservers() _________________________________________________

	void Surface::_notifyObservers(int nRects, const RectSPX* pRects)
	{
		Observer* p = m_pObserver;
		while (p)
		{
			p->func(nRects, pRects);
			p = p->pNext;
		}
	}

	//____ blueprint() ________________________________________________________

	Surface::Blueprint Surface::blueprint() const
	{
		Blueprint bp;

		bp.baggage = m_pBaggage;
		bp.buffered = m_bBuffered;
		bp.canvas = m_bCanvas;
		bp.palette = m_pPalette;
		bp.dynamic = m_bDynamic;
		bp.format = m_pixelFormat;
		bp.identity = m_id;
		bp.mipmap = m_bMipmapped;
		bp.sampleMethod = m_sampleMethod;
		bp.scale = m_scale;
		bp.size = m_size;
		bp.tiling = m_bTiling;
		bp.paletteSize = m_paletteSize;
		bp.paletteCapacity = m_paletteCapacity;

		return bp;
	}


	//_____ copy() _____________________________________________________________
	/**
	 * Copy the content of the specified surface to given coordinate of this surface
	 *
	 * @param	pSrcSurface		The surface from which to copy the graphics
	 * @param	dst				Destination coordinate for top-left corner of copied graphics.
	 *
	 * The entire content of pSrcSurface is considered a rectangular graphics block that
	 * will be copied to the coordinate specified. The content is copied as is, with color
	 * values and alpha channel, no blend operation is applied. Pixel format is converted
	 * as needed. If the source surface doesn't have an alpha channel, the alpha values of
	 * copied pixels are set to 255. Clipping is applied if needed. The destination coordinate
	 * must be within the bounds of the destination area.
	 *
	 * @return True if successful, otherwise false.
	 **/
	bool Surface::copy(CoordI dest, Surface * pSrcSurface )
	{
		if( !pSrcSurface )
			return false;

		return copy( dest, pSrcSurface, RectI(0,0,pSrcSurface->pixelSize()) );
	}

	/**
	 * Copy the specified rectangular area of the source surface to given coordinate of this surface.
	 *
	 * @param	pSrcSurface		The surface from which to copy the graphics block
	 * @param 	srcRect			Rectangular section of source surface to copy. Must be fully within
	 * 							the bounds of pSrcSurface.
	 * @param	dst				Destination coordinate for top-left corner of copied graphics.
	 *
	 * A rectangular graphics block is copied from the source surface to the coordinate specified.
	 * The content is copied as is, with color values and alpha channel, no blend operation is applied.
	 * Pixel format is converted as needed. If the source surface doesn't have an alpha channel, the alpha values of
	 * copied pixels are set to 255. Clipping is applied if needed. The destination coordinate must be within the
	 * bounds of the destination surface.
	 *
	 * @return True if successful, otherwise false.
	 **/
	bool Surface::copy(CoordI _dest, Surface * pSrcSurface, const RectI& _srcRect )
	{
		if( !pSrcSurface || pSrcSurface->pixelFormat() == PixelFormat::Undefined || m_pixelFormat == PixelFormat::Undefined )
			return false;

		// Clip as needed to get source and dest rectangles.
		
		RectI srcRect = RectI::overlap(_srcRect, pSrcSurface->m_size );
		if( srcRect.isEmpty() )
			return false;
			
		RectI destRect = RectI::overlap( { _dest, srcRect.size() }, m_size );
		if( destRect.isEmpty() )
			return false;
		
		srcRect.setSize( destRect.size() );
		
		// Allocate our buffers, push source pixels
			
		auto srcbuf = pSrcSurface->allocPixelBuffer(srcRect);
		bool bPushed = pSrcSurface->pushPixels(srcbuf);
		if (!bPushed)
		{
			//TODO: Error handling.
		}
		
		auto dstbuf = allocPixelBuffer( destRect );
		
		//
		
		auto& srcDesc = Util::pixelFormatToDescription(srcbuf.format);
		auto& dstDesc = Util::pixelFormatToDescription(m_pixelFormat);

		bool retVal = PixelTools::copyPixels(srcbuf.rect.w, srcbuf.rect.h, srcbuf.pixels, srcbuf.format, srcbuf.pitch - srcbuf.rect.w*srcDesc.bits/8,
								dstbuf.pixels, dstbuf.format, dstbuf.pitch - dstbuf.rect.w*dstDesc.bits/8, pSrcSurface->palette(),
								m_pPalette, pSrcSurface->paletteSize(), m_paletteSize, m_paletteCapacity);


		pullPixels(dstbuf);
		freePixelBuffer(dstbuf);
		
		pSrcSurface->freePixelBuffer(srcbuf);

		return retVal;
	}

	//____ convert() _________________________________________________________

	Surface_p Surface::convert( const Surface::Blueprint& _bp, SurfaceFactory* pFactory)
	{
		if (!pFactory)
			pFactory = GfxBase::defaultSurfaceFactory();

		if (!pFactory)
		{
			//TODO: Error handling!
			return Surface_p();
		}

		auto bp = _bp;
		
		bp.size = m_size;
		
		if( bp.scale == 0 )
			bp.scale = m_scale;
		
		if( bp.sampleMethod == SampleMethod::Undefined )
			bp.sampleMethod = m_sampleMethod;
		
		if( bp.format == PixelFormat::Undefined )
			bp.format = m_pixelFormat;
		
		auto pSurface = pFactory->createSurface(bp);
		if( pSurface->copy({ 0,0 }, this) )
			return pSurface;
		
		return nullptr;
	}


	//____ _alpha() ___________________________________________________________

	int Surface::_alpha(CoordSPX _coord, const PixelBuffer& buffer)
	{
		//TODO: Take endianess into account.
		//TODO: Take advantage of subpixel precision and interpolate alpha value if surface set to interpolate.

		CoordI coord(((_coord.x + 32) / 64) % m_size.w, ((_coord.y + 32) / 64) % m_size.h);

		switch (buffer.format)
		{
		case PixelFormat::Index_8_sRGB:
		case PixelFormat::Index_8_linear:
		{
			uint8_t index = buffer.pixels[buffer.pitch * coord.y + coord.x];
			return HiColor::unpackLinearTab[buffer.palette[index].a];
		}
		case PixelFormat::Alpha_8:
		{
			uint8_t* pPixel = buffer.pixels + buffer.pitch * coord.y + coord.x;
			return HiColor::unpackLinearTab[pPixel[0]];
		}
		case PixelFormat::BGRA_4_linear:
		{
			uint16_t alpha = (*(uint16_t*)(buffer.pixels + buffer.pitch * coord.y + coord.x * 2)) >> 12;
			return HiColor::unpackLinearTab[alpha | (alpha << 4)];
		}
		case PixelFormat::BGRA_8_sRGB:
		case PixelFormat::BGRA_8_linear:
		{
			uint8_t* pPixel = buffer.pixels + buffer.pitch * coord.y + coord.x * 4;
			return HiColor::unpackLinearTab[pPixel[3]];
		}
		default:
			return 4096;
		}
	}

    //____ _isBlueprintValid() ________________________________________________

    bool Surface::_isBlueprintValid(const Blueprint& bp, SizeI maxSize)
    {
        SizeI size = bp.size;

        PixelFormat format = bp.format;
        if( format == PixelFormat::Undefined )
            format = PixelFormat::BGRA_8;

		PixelDescription desc = Util::pixelFormatToDescription(format);
		
		
		if( size.isEmpty() )
			return false;
		
        if (size.w > maxSize.w || size.h > maxSize.h)
            return false;

        if (format < PixelFormat_min || format > PixelFormat_max)
            return false;

        if( desc.type == PixelType::Index )
        {
			if (bp.canvas)
				return false;			// Indexed can't be canvas.

			if( bp.mipmap )
                return false;            // Indexed can't be mipmapped.

			int paletteSize = 0;
			if( bp.palette )
				paletteSize = desc.bits == 8 ? 256 : 65536;
			
			if( bp.paletteSize != 0 )
			{
				if( bp.paletteSize < 0 )
					return false;

				if( bp.palette == nullptr )
					return false;			// paletteSize specified, but no palette pointer.
				
				if( bp.paletteSize > paletteSize )
					return false;			// Too large capacity specified.
				
				paletteSize = bp.paletteSize;
			}

			int paletteCapacity = paletteSize;
			if( bp.paletteCapacity != 0 )
			{
				if( bp.paletteCapacity < 0 )
					return false;
				
				if( bp.paletteCapacity < paletteSize )
					return false;			// Capacity specified to less than size.

				if( bp.paletteCapacity > 65536 || (desc.bits == 8 && bp.paletteCapacity > 256) )
					return false;			// Too large capacity specified.
			}
        }
                
        return true;
    }

	//____ _fixSrcParam() ________________________________________________________

	void Surface::_fixSrcParam( PixelFormat& format, const Color8*& pPalette, int& paletteSize )
	{
		if( format == PixelFormat::Undefined )
			format = m_pixelFormat;
		
		auto srcDesc = Util::pixelFormatToDescription(format);
		if( srcDesc.type == PixelType::Index || srcDesc.type == PixelType::Bitplanes )
		{
			if( pPalette == nullptr )
			{
				pPalette = m_pPalette;
				paletteSize = m_paletteSize;
			}			
		}
	}

	void Surface::_fixSrcParam( const PixelDescription& pixelDesc, const Color8*& pPalette, int& paletteSize )
	{
		if( pixelDesc.type == PixelType::Index || pixelDesc.type == PixelType::Bitplanes )
		{
			if( pPalette == nullptr )
			{
				pPalette = m_pPalette;
				paletteSize = m_paletteSize;
			}
		}
	}


	/**
	 * @fn virtual uint32_t	Surface::pixel( CoordI coord ) const
	 *
	 * Get the raw pixel value from the specified coordinate of the surface.
	 *
	 * @param coord Coordinate of pixel to get.
	 *
	 * The coordinate must be within the surface boundaries. A coordinate outside
	 * the surface will result in undefined behavior.
	 *
	 * If pixel is less than 32 bits in size, it is packed into the lower
	 * bits of the return value with the rest of the bits cleared.
	 *
	 * @return Pixel value in surface's native format.
	 *
	 */


	/**
	 * @fn virtual uint8_t Surface::alpha(CoordI coord) const
	 *
	 * Get the alpha value from the specified coordinate of the surface.
	 *
	 * @param coord Coordinate of pixel.
	 *
	 * Gets the alpha value of the pixel at the specified coordinate in the range of 0-255.
	 * The alpha value is by default used for opacity level,
	 * where 0 is a fully transparent pixel and 255 is opaque.
	 *
	 * The coordinate specified must be within the surface boundaries. A coordinate outside
	 * the surface will result in undefined behavior.
	 *
	 * @return Alpha value of pixel at coordinate.
	 */

	/**
	 * @fn virtual bool Surface::isOpaque() const
	 *
	 * Check if surface is entirely opaque.
	 *
	 * A surface is opaque if it contains no transparent or semi-transparent
	 * pixels at all and therefore is guaranteed to completely cover any graphics
	 * behind it. This method is called regularly in time-critical sections of
	 * the code to optimize rendering operations and therefore should not do any
	 * time consuming operations.
	 *
	 * @return True if surface is guaranteed to only contain completely opaque pixels. False if
	 * if it does contain (semi)transparent pixels or we simply just don't know.
	 */


	/**
	 * @fn virtual const PixelBuffer Surface::allocPixelBuffer(const RectI& rect)
	 * 
	 * @brief 	Allocate a buffer to access part of the surface
	 * 
	 * @param rect	A rectangular area (measured in pixels) of the Surface that should be
	 * 				mapped to this PixelBuffer.
	 * 
	 * @return 	A PixelBuffer providing info about the buffer, as well as working as a handle for
	 * 			future references.
	 * 
	 * Allocates a buffer that provides access to a rectangular part of the surface. First pixel
	 * in the buffer is the topmost pixel of the mapped rectangle. Mapping only part of the surface
	 * and not the whole surface can allow for significantly faster push/pull operations, as well as
	 * save memory.
	 * 
	 * The buffer should be expected to be uninitialized, but might already contain the
	 * content of the Surface rectangle depending on Surface implementation. For example is the
	 * buffer of a SoftSurface just a pointer into the pixels of the surface, providing
	 * direct access, while the OpenGL and Metal surfaces are uninitialized since we don't
	 * have direct access to the texture pixels.
	 * 
	 * To fill the buffer with the content of the surface rectangle, call pushPixels(). To copy the
	 * content of the buffer to the surface rectangle, use pullPixels(). These calls will do the actual
	 * pixel copying for an OpenGL and Metal surface, while just immediately return for a SoftSurface.
	 * 
	 * Once you are done with your buffer, call freePixelBuffer() to return the memory allocated.
	 */

	/**
	 * @fn virtual void freePixelBuffer(const PixelBuffer& buffer)
	 * 
	 * @brief 	Free a previously allocated PixelBuffer.
	 * 
	 * Free a previously allocated PixelBuffer.
	 * 
	 */

	/**
	 * @fn virtual bool	Surface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	 * 
	 * @brief Partial copy of pixels from Surface to PixelBuffer.
	 * 
	 * Partial copy of pixels from Surface to PixelBuffer.
	 * 
	 * @param buffer		PixelBuffer to copy pixels to.
	 * @param bufferRect	The destination rectangle withing the PixelBuffer. 
	 * @return 				True if operation could be performed.
	 * 
	 * Only the specified rectangle within the PixelBuffer is updated with pixels from the Surface,
	 * the rest is unaffected. To update the whole PixelBuffer, you can use pushPixels(const PixelBuffer& buffer) instead.
	 * 
	 * Please note that the rectangle specified is within the PixelBuffer, not the Surface. You should therefore not add
	 * the offset of the PixelBuffers mapping to bufferRect.
	 * 
	 */

	 //____ alpha() ____________________________________________________________
	 /**
	  * @fn virtual int	Surface::alpha( CoordSPX coord )
	  *
	  * Get the alpha value from the specified coordinate of the surface.
	  *
	  * @param coord Coordinate, specified in subpixels.
	  *
	  * Gets the alpha value of the pixel at the specified coordinate in the range of 0-4096.
	  * The alpha value is by default used for opacity level,
	  * where 0 is a fully transparent pixel and 4096 is opaque.
	  * The coordinate specified must be within the surface boundaries. A coordinate outside
	  * the surface will result in undefined behavior.
	  *
	  * @return Alpha value of pixel at coordinate.
	  */


} // namespace wg
