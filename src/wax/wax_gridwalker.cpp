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
#include <wg_gridwalker.h>
#include <wg_gfxutil.h>

namespace wg
{


	GridWalker::GridWalker(PixelBuffer& pixelBuffer)
	{
		m_pBuffer = &pixelBuffer;

		PixelDescription pixelDescription = Util::pixelFormatToDescription(m_pBuffer->format);

		m_pixelBytes = pixelDescription.bits / 8;

		m_gridPixel = _readPixel(m_pBuffer->pixels);
	}

	bool GridWalker::firstLine()
	{
		m_cell.y = 0;
		m_cell.h = 0;
		return nextLine();
	}

	bool GridWalker::nextLine()
	{
		const uint8_t* pPixels = m_pBuffer->pixels + (m_cell.y + m_cell.h) * m_pBuffer->pitch;
		const uint8_t* pPixelsEnd = m_pBuffer->pixels + m_pBuffer->rect.h * m_pBuffer->pitch;

		while (pPixels < pPixelsEnd && _readPixel(pPixels) == m_gridPixel)
			pPixels += m_pBuffer->pitch;

		if (pPixels >= pPixelsEnd)
			return false;

		auto pBeg = pPixels;
		while (pPixels < pPixelsEnd && _readPixel(pPixels) != m_gridPixel)
			pPixels += m_pBuffer->pitch;

		if (pPixels >= pPixelsEnd)
			return false;

		m_cell.x = 0;
		m_cell.y = int(pBeg - m_pBuffer->pixels) / m_pBuffer->pitch;
		m_cell.h = int(pPixels - pBeg) / m_pBuffer->pitch;
		return true;
	}

	RectI GridWalker::firstCell()
	{
		m_cell.x = 0;
		const uint8_t* pPixels = m_pBuffer->pixels + m_cell.y * m_pBuffer->pitch + m_cell.x * m_pixelBytes;
		const uint8_t* pPixelsEnd = m_pBuffer->pixels + m_cell.y * m_pBuffer->pitch + m_pBuffer->rect.w * m_pixelBytes;


		auto pBeg = pPixels;
		while (pPixels < pPixelsEnd && _readPixel(pPixels) != m_gridPixel)
			pPixels += m_pixelBytes;

		if (pPixels >= pPixelsEnd)
			return RectI();

		m_cell.w = int(pPixels - pBeg) / m_pixelBytes;

		return m_cell;
	}

	RectI GridWalker::nextCell()
	{
		const uint8_t* pPixels = m_pBuffer->pixels + m_cell.y * m_pBuffer->pitch + (m_cell.x + m_cell.w) * m_pixelBytes;
		const uint8_t* pPixelsEnd = m_pBuffer->pixels + m_cell.y * m_pBuffer->pitch + m_pBuffer->rect.w * m_pixelBytes;

		while (pPixels < pPixelsEnd && _readPixel(pPixels) == m_gridPixel)
			pPixels += m_pixelBytes;

		auto pBeg = pPixels;
		while (pPixels < pPixelsEnd && _readPixel(pPixels) != m_gridPixel)
			pPixels += m_pixelBytes;

		if (pPixels >= pPixelsEnd)
			return RectI();

		m_cell.x = int(pBeg - (m_pBuffer->pixels + m_cell.y * m_pBuffer->pitch)) / m_pixelBytes;
		m_cell.w = int(pPixels - pBeg) / m_pixelBytes;

		return m_cell;
	}
}
