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

#include <cassert>
#include <cstring>
#include <memory.h>
#include <wg_softsurface.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>
#include <wg_pixeltools.h>

namespace wg
{

	using namespace std;

	const TypeInfo SoftSurface::TYPEINFO = { "SoftSurface", &Surface::TYPEINFO };

	//____ maxSize() _______________________________________________________________

	SizeI SoftSurface::maxSize()
	{
#if WG_IS_64_BITS
		return SizeI(1048576,1048576);
#else
		return SizeI(65534, 65534);
#endif
	}

	//____ Create ______________________________________________________________

	SoftSurface_p SoftSurface::create(const Blueprint& blueprint)
	{
		return SoftSurface_p(new SoftSurface(blueprint));
	}

	SoftSurface_p SoftSurface::create(const Blueprint& blueprint, Blob* pBlob, int pitch)
	{
		return SoftSurface_p(new SoftSurface(blueprint, pBlob, pitch));
	}

	SoftSurface_p SoftSurface::create(const Blueprint& blueprint, const uint8_t* pPixels, const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize )
	{
		return SoftSurface_p(new SoftSurface(blueprint, pPixels, pixelDescription, pitch, pPalette, paletteSize));
	}

	SoftSurface_p SoftSurface::create(const Blueprint& blueprint, const uint8_t* pPixels, PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize )
	{
		return SoftSurface_p(new SoftSurface(blueprint, pPixels, format, pitch, pPalette, paletteSize));
	}


	SoftSurface_p SoftSurface::createInPlace(const Blueprint& blueprint, uint8_t* pPixels, int pitch)
	{
		return SoftSurface_p(new SoftSurface(blueprint, pPixels, pitch));
	}

	//____ constructor ________________________________________________________________

	SoftSurface::SoftSurface( const Blueprint& bp ) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest )
	{
		m_pitch = ((bp.size.w+3)&0xFFFFFFFC)*m_pPixelDescription->bits/8;
		m_pBlob = Blob::create( m_pitch*bp.size.h + m_paletteCapacity*sizeof(Color8) );
		m_pData = (uint8_t*) m_pBlob->data();

		if (m_paletteCapacity > 0)
		{
			m_pPalette = (Color8*)(m_pData + m_pitch * bp.size.h);
			memcpy(m_pPalette, bp.palette, m_paletteSize*sizeof(Color8) );
			_makePalette4096();
		}
		else
			m_pPalette = nullptr;

		_initTiling();
	}

	SoftSurface::SoftSurface(const Blueprint& bp, Blob * pBlob, int pitch ) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
		m_pitch = pitch > 0 ? pitch : bp.size.w * m_pPixelDescription->bits/8;
		m_pBlob = pBlob;
		m_pData = (uint8_t*) m_pBlob->data();
		m_pPalette = const_cast<Color8*>(bp.palette);
		if( m_pPalette )
			_makePalette4096();

		_initTiling();
	}

	SoftSurface::SoftSurface(const Blueprint& bp, uint8_t * pPixels, int pitch) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
		// This constructor creates the surface in place, using the pixels (and palette if present in BP) where they are.
		
		if( pitch == 0 )
			pitch = bp.size.w * m_pPixelDescription->bits/8;
		
		m_pitch = pitch;
		m_pBlob = nullptr;
		m_pData = pPixels;

		if (bp.palette)
		{
			m_pPalette = const_cast<Color8*>(bp.palette);
			_makePalette4096();
		}
		else
			m_pPalette = nullptr;

		_initTiling();
	}

	SoftSurface::SoftSurface(const Blueprint& bp, const uint8_t * pPixels,
							 PixelFormat format, int pitch, const Color8 * pPalette, int paletteSize ) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
				
		m_pitch = ((bp.size.w + 3) & 0xFFFFFFFC)*m_pPixelDescription->bits / 8;
		m_pBlob = Blob::create(m_pitch*m_size.h + m_paletteCapacity*sizeof(Color8) );
		m_pData = (uint8_t*)m_pBlob->data();

		if (m_paletteCapacity > 0)
		{
			m_pPalette = (Color8*)(m_pData + m_pitch * bp.size.h);
			memcpy(m_pPalette, bp.palette, m_paletteSize*sizeof(Color8) );
			_makePalette4096();
		}
		else
			m_pPalette = nullptr;

		_initTiling();
		
		// Copy pixels

		_fixSrcParam(format, pPalette, paletteSize);
		
		int srcPitchAdd = (pitch == 0) ? 0 : pitch - Util::pixelFormatToDescription(format).bits/8 * m_size.w;
		
		PixelTools::copyPixels(m_size.w, m_size.h, pPixels, format, srcPitchAdd,
							   m_pData, m_pixelFormat, m_pitch - m_pPixelDescription->bits/8 * m_size.w, pPalette,
							   m_pPalette, paletteSize, m_paletteSize, m_paletteCapacity);
	}


	SoftSurface::SoftSurface(const Blueprint& bp, const uint8_t * pPixels,
							 const PixelDescription& pixelDescription, int pitch, const Color8 * pPalette, int paletteSize ) : Surface(bp, PixelFormat::BGRA_8, SampleMethod::Nearest)
	{
		
		
		m_pitch = ((bp.size.w + 3) & 0xFFFFFFFC)*m_pPixelDescription->bits / 8;
		m_pBlob = Blob::create(m_pitch*m_size.h + m_paletteCapacity*sizeof(Color8) );
		m_pData = (uint8_t*)m_pBlob->data();

		if (m_paletteCapacity)
		{
			m_pPalette = (Color8*)(m_pData + m_pitch * bp.size.h);
			memcpy(m_pPalette, bp.palette, m_paletteSize*sizeof(Color8) );
			_makePalette4096();
		}
		else
			m_pPalette = nullptr;

		_initTiling();
		
		// Copy pixels
		
		_fixSrcParam(pixelDescription, pPalette, paletteSize);

		int srcPitchAdd = pitch == 0 ? 0 : pitch - pixelDescription.bits/8 * m_size.w;
		
		PixelTools::copyPixels(m_size.w, m_size.h, pPixels, pixelDescription, srcPitchAdd,
							   m_pData, m_pixelFormat, m_pitch - m_pPixelDescription->bits/8 * m_size.w, pPalette,
							   m_pPalette, paletteSize, m_paletteSize, m_paletteCapacity);
	}

	//____ Destructor ______________________________________________________________

	SoftSurface::~SoftSurface()
	{
		delete [] m_pPalette4096;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SoftSurface::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ alpha() _______________________________________________________________

	int SoftSurface::alpha( CoordSPX coord )
	{
		PixelBuffer	buffer;
		buffer.format = m_pixelFormat;
		buffer.palette = m_pPalette;
		buffer.pitch = m_pitch;
		buffer.pixels = m_pData;
		buffer.rect = { 0,0,m_size };

		return _alpha(coord, buffer);
	}

	//____ _initTiling() ________________________________________________________

	void SoftSurface::_initTiling()
	{
		if (m_bTiling)
		{
			int xBits = Util::mostSignificantBit(m_size.w);
			int yBits = Util::mostSignificantBit(m_size.h);

			if (m_size.w != 1 << xBits || m_size.h != 1 << yBits)
			{
				GfxBase::throwError(ErrorLevel::Warning, ErrorCode::FailedPrerequisite, "Surface of non-two-factor size set to tile. Tiling will happen at largest two-factor boundary.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
			}

			m_srcPosMaskX = (1 << xBits) -1;
			m_srcPosMaskY = (1 << yBits) -1;
		}
		else
		{
			m_srcPosMaskX = 0;
			m_srcPosMaskY = 0;
		}
	}

	//____ allocPixelBuffer() _________________________________________________

	const PixelBuffer SoftSurface::allocPixelBuffer(const RectI& rect)
	{
		PixelBuffer	buf;
		buf.pixels = m_pData + m_pitch*rect.y + rect.x*m_pPixelDescription->bits/8;
		buf.palette = m_pPalette;
		buf.format = m_pixelFormat;
		buf.rect = rect;
		buf.pitch = m_pitch;
		return buf;
	}

	//____ pushPixels() _______________________________________________________

	bool SoftSurface::pushPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		return true;
	}

	//____ pullPixels() _______________________________________________________

	void SoftSurface::pullPixels(const PixelBuffer& buffer, const RectI& bufferRect)
	{
		// Nothing to do here.

		Surface::pullPixels(buffer, bufferRect);
	}

	//____ freePixelBuffer() __________________________________________________

	void SoftSurface::freePixelBuffer(const PixelBuffer& buffer)
	{
		// Nothing to do here.
	}

	//____ putPixels() _____________________________________________________________

	#define PCLIP(x,y) (((x)>(y))?(y):(x))

	void SoftSurface::putPixels(const vector<int> &x, const vector<int> &y, const vector<uint32_t> &col, int length, bool replace)
	{
		Color8 color1;
		Color8 color2;
		int ind;

		switch(m_pixelFormat)
		{
			case PixelFormat::BGR_8:
				break;
			case PixelFormat::BGRA_8:
				for(int n=0; n<length; n++)
				{
				  ind = y[n]*m_pitch + x[n]*4;
				  if(!replace) {
					// Get old (1) and new (2) pixel
					color1.argb = *((uint32_t*)&m_pData[ind]);
					color2.argb = col[n];
					// Blend
					color1.r = (uint8_t)(PCLIP((int)color1.r + (int)color2.r,0xFF));
					color1.g = (uint8_t)(PCLIP((int)color1.g + (int)color2.g,0xFF));
					color1.b = (uint8_t)(PCLIP((int)color1.b + (int)color2.b,0xFF));
					color1.a = (uint8_t)(PCLIP((int)color1.a + (int)color2.a,0xFF));
					// Store
					*((uint32_t*)&m_pData[ind]) = color1.argb;
				  } else {
					// Overwrite old pixel with new pixel
					*((uint32_t*)&m_pData[ind]) = col[n];
				  }
				}
				break;
			default:
				break;
		}
	}

	//____ _makePalette4096() ____________________________________________________

	void SoftSurface::_makePalette4096()
	{
		m_pPalette4096 = new HiColor[m_paletteCapacity];

		HiColor * p = m_pPalette4096;
		const int16_t* pUnpackTab = GfxBase::defaultToSRGB() ? HiColor::unpackSRGBTab : HiColor::unpackLinearTab;

		for (int i = 0; i < m_paletteSize; i++)
		{
			p[i].r = pUnpackTab[m_pPalette[i].r];
			p[i].g = pUnpackTab[m_pPalette[i].g];
			p[i].b = pUnpackTab[m_pPalette[i].b];
			p[i].a = HiColor::unpackLinearTab[m_pPalette[i].a];
		}
	}

} // namespace wg
