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
/**
 *  A handful of extra hand-tuned kernels for drawing/blitting on
 *  BGR555_bigendian canvas for some improved performance.
 *  
 */


#ifndef WG_SOFTKERNELS_RGB555BE_EXTRAS_DOT_H
#define WG_SOFTKERNELS_RGB555BE_EXTRAS_DOT_H
#pragma once

#include <wg_softbackend.h>

namespace wg
{
	bool addExtraSoftKernelsForRGB555BECanvas( SoftBackend * pDevice );
};

#endif //WG_SOFTKERNELS_RGB555BE_EXTRAS_DOT_H
