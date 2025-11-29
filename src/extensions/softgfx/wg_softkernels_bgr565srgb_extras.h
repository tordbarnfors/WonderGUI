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
/**
 *  A handful of extra hand-tuned kernels for drawing/blitting on
 *  BGR565_bigendian canvas for some improved performance.
 *  
 */


#ifndef WG_SOFTKERNELS_BGR565SRGB_EXTRAS_DOT_H
#define WG_SOFTKERNELS_BGR565SRGB_EXTRAS_DOT_H
#pragma once

#include <wg_softbackend.h>

namespace wg
{
	bool addExtraSoftKernelsForBGR565sRGBCanvas( SoftBackend * pBackend );
};

#endif //WG_SOFTKERNELS_BGR565SRGB_EXTRAS_DOT_H
