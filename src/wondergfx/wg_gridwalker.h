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
#ifndef WG_GRIDWALKER_DOT_H
#define WG_GRIDWALKER_DOT_H
#pragma once

#include <wg_gfxtypes.h>
#include <wg_surface.h>
#include <cassert>

namespace wg
{
	class GridWalker
	{
	public:
		GridWalker(PixelBuffer& pixelBuffer);

		bool		firstLine();
		bool		nextLine();

		RectI		firstCell();
		RectI		nextCell();

	private:

		inline uint32_t	_readPixel(const uint8_t* pPixel) const;

		PixelBuffer*	m_pBuffer;
		int				m_pixelBytes;
		uint32_t		m_gridPixel;

		RectI			m_cell;

	};

	uint32_t GridWalker::_readPixel(const uint8_t* pPixel) const
	{
		switch (m_pixelBytes)
		{
			case 1:
				return * pPixel;
			case 2:
				return * (reinterpret_cast<const uint16_t*>(pPixel));
			case 3:
				return pPixel[0] + (uint32_t(pPixel[1]) << 8) + (uint32_t(pPixel[2]) << 16);
			case 4:
				return *(reinterpret_cast<const uint32_t*>(pPixel));
			default:
				assert(false);
				return 0;
		}
	}
}
#endif //WG_GRIDWALKER_DOT_H
