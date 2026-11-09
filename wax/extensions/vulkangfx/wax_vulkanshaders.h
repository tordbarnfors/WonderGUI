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
#ifndef WAX_VULKANSHADERS_DOT_H
#define WAX_VULKANSHADERS_DOT_H
#pragma once

#include <cstdint>
#include <cstddef>

namespace wax
{
	// The shaders of VulkanBackend as SPIR-V, compiled from ../hlsl by compile_shaders.py into
	// wax_vulkanshaders.cpp. Pixel shaders ending in Dual output the second color of dual-source
	// blending.

	namespace VulkanShaders
	{
		struct Code
		{
			const uint32_t *	pWords;
			size_t				bytes;
		};

		extern const Code fillVS;
		extern const Code fillAAVS;
		extern const Code blitVS;
		extern const Code lineVS;
		extern const Code segmentsVS;
		extern const Code shapeVS;
		extern const Code warpVS;

		extern const Code fillPS, fillPSDual;
		extern const Code fillAAPS, fillAAPSDual;
		extern const Code blitPS, blitPSDual;
		extern const Code blurPS, blurPSDual;
		extern const Code paletteBlitPS, paletteBlitPSDual;
		extern const Code paletteBlurPS, paletteBlurPSDual;
		extern const Code linePS, linePSDual;
		extern const Code segmentsPS, segmentsPSDual;
		extern const Code shapePS, shapePSDual;
		extern const Code warpEdgemapPS, warpEdgemapPSDual;
		extern const Code warpBlitPS, warpBlitPSDual;
		extern const Code warpPaletteBlitPS, warpPaletteBlitPSDual;
	}
}

#endif // WAX_VULKANSHADERS_DOT_H
