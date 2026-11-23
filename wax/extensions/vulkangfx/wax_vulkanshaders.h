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
	// spirv/, which CMakeLists.txt turns into wax_vulkanshaders.cpp in the build directory. Named as
	// GpuBackend has them, see GpuBackend::_vertexShaderName() and _pixelShaderName().

	namespace VulkanShaders
	{
		struct Shader
		{
			const char *		pName;
			const uint32_t *	pWords;
			size_t				bytes;
		};

		extern const Shader		shaders[];
		extern const int		nbShaders;
	}
}

#endif // WAX_VULKANSHADERS_DOT_H
