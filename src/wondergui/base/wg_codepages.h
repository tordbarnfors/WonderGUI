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
#ifndef WG_CODEPAGES_DOT_H
#define WG_CODEPAGES_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_enumextras.h>

namespace wg
{

	/**
		Static class containing code pages for tanslating 8-bit characters from various
		encodings to unicode.

		These code pages are used by CharSeq8, which provides a convenient way of reading
		text in various legacy encodings, but can also be accessed directly.

		The code pages are simply static arrays of 256 16-bit entries each, where each
		entry contains the unicode number for the 8-bit character corresponding
		to the array index.
	**/

	class CodePages /** @private */
	{
	public:
		//.____ Misc ___________________________________________________________

		static inline const uint16_t* getCodePage( CodePage codePage ) { return LookupTable[(int)codePage]; }

	private:
		const static uint16_t* LookupTable[CodePage_size];
	};


} // namespace wg
#endif //WG_CODEPAGES_DOT_H
