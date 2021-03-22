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

#ifndef WG_TEXTSTYLEMANAGER_DOT_H
#define WG_TEXTSTYLEMANAGER_DOT_H
#pragma once


#include <wg_textstyle.h>

class WgBase;   // Needed by WG2

namespace wg
{
	class TextStyleManager  /** @private */
	{
	friend class Base;
	friend class TextStyle;
	friend class Char;
	friend class CharBuffer;
	friend class TextTool;
	friend class ::WgBase;        // Needed by WG2
	public:

		//.____ Misc ________________________________________________

		static TextStyle_p 	getPointer( TextStyle_h handle );
		static inline int	getNbStyles() { return s_size; }

		const static TypeInfo	TYPEINFO;

	protected:
//		TextStyleManager();
//		~TextStyleManager();

		static const int c_maxCapacity = 65535;			// Max capacity may not exceed 65535.

		static void init();
		static void exit();

		static TextStyle_h			_reserveHandle( TextStyle * p );
		static void					_releaseHandle( TextStyle_h handle );
		static inline TextStyle * 	_getPointer( TextStyle_h handle ) { return s_pLookupTable[handle-1]; }

		static TextStyle **	s_pLookupTable;			// Pointer at handle->pointer lookup table.
		static int			s_capacity;				// SizeI in entries for lookup table.
		static int			s_size;					// SizeI in entries for lookup table.
		static int			s_nextAvailable;		// Offset in table for next available entry.
	};
}

#endif // WG_TEXTSTYLEMANAGER_DOT_H
