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
#include <limits>
#include <memory.h>
#include <wax_bitmap.h>
#include <wax_dataset.h>
#include <wax_util.h>
#include <wax_base.h>
#include <wax_pixeltools.h>

namespace wax
{
	using namespace Util;


	const TypeInfo Bitmap::TYPEINFO = { "Bitmap", &Object::TYPEINFO };


	//____ ~Bitmap() ____________________________________________________________

	Bitmap::~Bitmap()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Bitmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ scale() ____________________________________________________________

	int Bitmap::scale() const
	{
		return m_scale;
	}

	//____ fill() _________________________________________________________________
	/**
	 * Fill the bitmap with the specified color.
	 *
	 * @param col	Color value with which to fill the bitmap.
	 *
	 * The fill instruction replaces the bitmap content with the pixel values so
	 * no blending operations will take place. If the specified color has an alpha
	 * value it will replace the alpha channel of the bitmap (if it has any), not blend the color
	 * onto the bitmap.
	 *
	 * If the bitmap has less than 24-bits or color information, it will be filled with the
	 * pixel value most closely resembling the one specified.
	 *
	 **/

	bool Bitmap::fill( Color color )
	{
		return fill(RectI(0, 0, pixelSize()), color );
	}

	/**
	 * Fill the specied region of the bitmap with the specified color.
	 *
	 * @param col		Color value with which to fill the region.
	 * @param region	Region of bitmap to fill.
	 *
	 * The fill instruction replaces the bitmap content with the pixel values so
	 * no blending operations will take place. If the specified color has an alpha
	 * value it will replace the alpha channel of the bitmap (if it has any), not blend the color
	 * onto the bitmap.
	 *
	 * If the bitmap has less than 24-bits or color information, it will be filled with the
	 * pixel value most closely resembling the one specified.
	 *
	 **/
	bool Bitmap::fill(const RectI& region, Color color  )
	{
		auto pixbuf = allocPixelBuffer(region);

		PixelTools::fillBitmap(pixbuf.pixels, pixbuf.format, pixbuf.colorSpace, pixbuf.bigEndian, pixbuf.pitch, RectI(pixbuf.rect.size()), color,
							   pixbuf.palette, m_paletteSize);
		
		pullPixels(pixbuf);
		freePixelBuffer(pixbuf);

		return true;
	}

	//____ pullPixels() _______________________________________________________
	/**
	 * @brief Partial copy of pixels from PixelBuffer to Bitmap.
	 *
	 * Partial copy of pixels from PixelBuffer to Bitmap.
	 *
	 * @param buffer		PixelBuffer to copy pixels from.
	 * @param bufferRect	The source rectangle withing the PixelBuffer.
	 * @param bAutoNotify	Notify any bitmap observers that pulled area has been updated.
	 * @return 				True if operation could be performed.
	 *
	 * Only the specified rectangle within the PixelBuffer is copied to the Bitmap,
	 * the rest of the bitmap is unaffected. To copy the whole PixelBuffer to Bitmap, use pullPixels(const PixelBuffer& buffer) instead.
	 *
	 * Please note that the rectangle specified is within the PixelBuffer, not the Bitmap. You should therefore not add
	 * the offset of the PixelBuffer to bufferRect.
	 *
	 * If bAutoNotify is set to false, a manual call to NotifyObservers might be needed.
	 *
	 */
	void Bitmap::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify)
	{
		if( bAutoNotify )
		{
			RectSPX rect = (bufferRect + buffer.rect.pos());

			notifyObservers(1, &rect);
		}
	}


	//____ addObserver() ______________________________________________________

	int Bitmap::addObserver(const std::function<void(int nRects, const RectI* pRects)>& func)
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

	bool Bitmap::removeObserver(int observerId)
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

	//____ notifyObservers() _________________________________________________

	void Bitmap::notifyObservers(int nRects, const RectI* pRects)
	{
		Observer* p = m_pObserver;
		while (p)
		{
			p->func(nRects, pRects);
			p = p->pNext;
		}
	}

	//____ declineRestore() ___________________________________________________

	bool Bitmap::declineRestore( Bitmap * pBitmap )
	{
		return false;
	}

	//____ dropRestorer() _____________________________________________________

	void Bitmap::dropRestorer()
	{
		if( m_restorer )
			m_restorer = declineRestore;
	}

	//____ blueprint() ________________________________________________________

	Bitmap::Blueprint Bitmap::blueprint() const
	{
		Blueprint bp;

		bp.userData = m_pUserData;
		bp.bigEndian = m_bBigEndian;
		bp.canvas = m_bCanvas;
		bp.colorSpace = m_colorSpace;
		bp.palette = m_pPalette;
		bp.dynamic = m_bDynamic;
		bp.format = m_pixelFormat;
		bp.identity = m_id;
		bp.mipmap = m_bMipmapped;
		bp.restorer = m_restorer;
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
	 * Copy the content of the specified bitmap to given coordinate of this bitmap
	 *
	 * @param	pSrcBitmap		The bitmap from which to copy the graphics
	 * @param	dst				Destination coordinate for top-left corner of copied graphics.
	 *
	 * The entire content of pSrcBitmap is considered a rectangular graphics block that
	 * will be copied to the coordinate specified. The content is copied as is, with color
	 * values and alpha channel, no blend operation is applied. Pixel format is converted
	 * as needed. If the source bitmap doesn't have an alpha channel, the alpha values of
	 * copied pixels are set to 255. Clipping is applied if needed. The destination coordinate
	 * must be within the bounds of the destination area.
	 *
	 * @return True if successful, otherwise false.
	 **/
	bool Bitmap::copy(CoordI dest, Bitmap * pSrcBitmap )
	{
		if( !pSrcBitmap )
			return false;

		return copy( dest, pSrcBitmap, RectI(0,0,pSrcBitmap->pixelSize()) );
	}

	/**
	 * Copy the specified rectangular area of the source bitmap to given coordinate of this bitmap.
	 *
	 * @param	pSrcBitmap		The bitmap from which to copy the graphics block
	 * @param 	srcRect			Rectangular section of source bitmap to copy. Must be fully within
	 * 							the bounds of pSrcBitmap.
	 * @param	dst				Destination coordinate for top-left corner of copied graphics.
	 *
	 * A rectangular graphics block is copied from the source bitmap to the coordinate specified.
	 * The content is copied as is, with color values and alpha channel, no blend operation is applied.
	 * Pixel format is converted as needed. If the source bitmap doesn't have an alpha channel, the alpha values of
	 * copied pixels are set to 255. Clipping is applied if needed. The destination coordinate must be within the
	 * bounds of the destination bitmap.
	 *
	 * @return True if successful, otherwise false.
	 **/
	bool Bitmap::copy(CoordI _dest, Bitmap * pSrcBitmap, const RectI& _srcRect )
	{
		if( !pSrcBitmap || pSrcBitmap->pixelFormat() == PixelFormat::Undefined || m_pixelFormat == PixelFormat::Undefined )
			return false;

		// Clip as needed to get source and dest rectangles.
		
		RectI srcRect = RectI::overlap(_srcRect, pSrcBitmap->m_size );
		if( srcRect.isEmpty() )
			return false;
			
		RectI destRect = RectI::overlap( { _dest, srcRect.size() }, m_size );
		if( destRect.isEmpty() )
			return false;
		
		srcRect.setSize( destRect.size() );
		
		// Allocate our buffers, push source pixels
			
		auto srcbuf = pSrcBitmap->allocPixelBuffer(srcRect);
		bool bPushed = pSrcBitmap->pushPixels(srcbuf);
		if (!bPushed)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Failed to access pixels of source bitmap.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			//TODO: Error handling.
		}
		
		auto dstbuf = allocPixelBuffer( destRect );
		
		//
		
		auto srcDesc = Util::pixelFormatToDescription(srcbuf.format, srcbuf.bigEndian);
		auto dstDesc = Util::pixelFormatToDescription(dstbuf.format, dstbuf.bigEndian);

		bool retVal = PixelTools::copyPixels(srcbuf.rect.w, srcbuf.rect.h,
											 srcbuf.pixels, srcDesc, srcbuf.colorSpace, srcbuf.pitch - PixelTools::bytesPerLine(srcDesc, srcbuf.rect.w),
											 srcbuf.palette, pSrcBitmap->paletteSize(),
											 dstbuf.pixels, dstbuf.format, dstbuf.colorSpace, dstbuf.bigEndian, dstbuf.pitch - PixelTools::bytesPerLine(dstDesc, dstbuf.rect.w),
											 m_pPalette, m_paletteSize, m_paletteCapacity);


		pullPixels(dstbuf);
		freePixelBuffer(dstbuf);
		
		pSrcBitmap->freePixelBuffer(srcbuf);

		return retVal;
	}

	//____ convert() _________________________________________________________

	Bitmap_p Bitmap::convert( const Bitmap::Blueprint& _bp, BitmapFactory* pFactory)
	{
		if (!pFactory)
			pFactory = Base::defaultBitmapFactory();

		if (!pFactory)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "No bitmap factory available for bitmap conversion.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			return Bitmap_p();
		}

		auto bp = _bp;
		
		bp.size = m_size;
		
		if( bp.scale == 0 )
			bp.scale = m_scale;
		
		if( bp.sampleMethod == SampleMethod::Undefined )
			bp.sampleMethod = m_sampleMethod;
		
		if( bp.format == PixelFormat::Undefined )
			bp.format = m_pixelFormat;

		if( bp.colorSpace == ColorSpace::Undefined )
			bp.colorSpace = m_colorSpace;
		
		auto pBitmap = pFactory->createBitmap(bp);
		if( pBitmap->copy({ 0,0 }, this) )
			return pBitmap;
		
		return nullptr;
	}


	//____ _alpha() ___________________________________________________________

	int Bitmap::_alpha(CoordSPX _coord, const PixelBuffer& buffer)
	{
		//TODO: Take advantage of subpixel precision and interpolate alpha value if bitmap set to interpolate.

		CoordI coord(((_coord.x + 32) / 64) % m_size.w, ((_coord.y + 32) / 64) % m_size.h);

		uint8_t alpha;
		if( PixelTools::extractAlphaChannel(buffer.format, buffer.bigEndian, buffer.pixels, buffer.pitch, { coord, 1, 1 }, &alpha, 1, buffer.palette) )
			return Color::unpackLinearTab[alpha];

		return 4096;
	}

    //____ _isBlueprintValid() ________________________________________________

    bool Bitmap::_isBlueprintValid(const Blueprint& bp, SizeI maxSize)
    {
        SizeI size = bp.size;

        PixelFormat format = bp.format;
        if( format == PixelFormat::Undefined )
            format = PixelFormat::ARGB_8;

		if( bp.colorSpace < ColorSpace_min || bp.colorSpace > ColorSpace_max )
			return false;

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

	void Bitmap::_fixSrcParam( PixelFormat& format, const Color8*& pPalette, int& paletteSize )
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

	void Bitmap::_fixSrcParam( const PixelDescription& pixelDesc, const Color8*& pPalette, int& paletteSize )
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
	 * @fn virtual uint32_t	Bitmap::pixel( CoordI coord ) const
	 *
	 * Get the raw pixel value from the specified coordinate of the bitmap.
	 *
	 * @param coord Coordinate of pixel to get.
	 *
	 * The coordinate must be within the bitmap boundaries. A coordinate outside
	 * the bitmap will result in undefined behavior.
	 *
	 * If pixel is less than 32 bits in size, it is packed into the lower
	 * bits of the return value with the rest of the bits cleared.
	 *
	 * @return Pixel value in bitmap's native format.
	 *
	 */


	/**
	 * @fn virtual uint8_t Bitmap::alpha(CoordI coord) const
	 *
	 * Get the alpha value from the specified coordinate of the bitmap.
	 *
	 * @param coord Coordinate of pixel.
	 *
	 * Gets the alpha value of the pixel at the specified coordinate in the range of 0-255.
	 * The alpha value is by default used for opacity level,
	 * where 0 is a fully transparent pixel and 255 is opaque.
	 *
	 * The coordinate specified must be within the bitmap boundaries. A coordinate outside
	 * the bitmap will result in undefined behavior.
	 *
	 * @return Alpha value of pixel at coordinate.
	 */

	/**
	 * @fn virtual bool Bitmap::isOpaque() const
	 *
	 * Check if bitmap is entirely opaque.
	 *
	 * A bitmap is opaque if it contains no transparent or semi-transparent
	 * pixels at all and therefore is guaranteed to completely cover any graphics
	 * behind it. This method is called regularly in time-critical sections of
	 * the code to optimize rendering operations and therefore should not do any
	 * time consuming operations.
	 *
	 * @return True if bitmap is guaranteed to only contain completely opaque pixels. False if
	 * if it does contain (semi)transparent pixels or we simply just don't know.
	 */


	/**
	 * @fn virtual const PixelBuffer Bitmap::allocPixelBuffer(const RectI& rect)
	 * 
	 * @brief 	Allocate a buffer to access part of the bitmap
	 * 
	 * @param rect	A rectangular area (measured in pixels) of the Bitmap that should be
	 * 				mapped to this PixelBuffer.
	 * 
	 * @return 	A PixelBuffer providing info about the buffer, as well as working as a handle for
	 * 			future references.
	 * 
	 * Allocates a buffer that provides access to a rectangular part of the bitmap. First pixel
	 * in the buffer is the topmost pixel of the mapped rectangle. Mapping only part of the bitmap
	 * and not the whole bitmap can allow for significantly faster push/pull operations, as well as
	 * save memory.
	 * 
	 * The buffer should be expected to be uninitialized, but might already contain the
	 * content of the Bitmap rectangle depending on Bitmap implementation. For example is the
	 * buffer of a SoftBitmap just a pointer into the pixels of the bitmap, providing
	 * direct access, while the OpenGL and Metal bitmaps are uninitialized since we don't
	 * have direct access to the texture pixels.
	 * 
	 * To fill the buffer with the content of the bitmap rectangle, call pushPixels(). To copy the
	 * content of the buffer to the bitmap rectangle, use pullPixels(). These calls will do the actual
	 * pixel copying for an OpenGL and Metal bitmap, while just immediately return for a SoftBitmap.
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
	 * @fn virtual bool	Bitmap::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	 * 
	 * @brief Partial copy of pixels from Bitmap to PixelBuffer.
	 * 
	 * Partial copy of pixels from Bitmap to PixelBuffer.
	 * 
	 * @param buffer		PixelBuffer to copy pixels to.
	 * @param bufferRect	The destination rectangle withing the PixelBuffer. 
	 * @return 				True if operation could be performed.
	 * 
	 * Only the specified rectangle within the PixelBuffer is updated with pixels from the Bitmap,
	 * the rest is unaffected. To update the whole PixelBuffer, you can use pushPixels(const PixelBuffer& buffer) instead.
	 * 
	 * Please note that the rectangle specified is within the PixelBuffer, not the Bitmap. You should therefore not add
	 * the offset of the PixelBuffers mapping to bufferRect.
	 * 
	 */

	 //____ alpha() ____________________________________________________________
	 /**
	  * @fn virtual int	Bitmap::alpha( CoordSPX coord )
	  *
	  * Get the alpha value from the specified coordinate of the bitmap.
	  *
	  * @param coord Coordinate, specified in subpixels.
	  *
	  * Gets the alpha value of the pixel at the specified coordinate in the range of 0-4096.
	  * The alpha value is by default used for opacity level,
	  * where 0 is a fully transparent pixel and 4096 is opaque.
	  * The coordinate specified must be within the bitmap boundaries. A coordinate outside
	  * the bitmap will result in undefined behavior.
	  *
	  * @return Alpha value of pixel at coordinate.
	  */


} // namespace wax
