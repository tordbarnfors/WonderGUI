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
#ifndef WAX_SDLGPUSHADERS_DOT_H
#define WAX_SDLGPUSHADERS_DOT_H
#pragma once

#include <cstdint>
#include <cstddef>

namespace wax
{
	// The shaders of SdlGpuBackend, compiled from ../hlsl by compile_shaders.py into shaders/, as
	// SPIR-V, DXIL and MSL. Named as GpuBackend has them, see GpuBackend::_vertexShaderName() and
	// _pixelShaderName().
	//
	// What each binds is in shaders[], which compile_shaders.py writes to wax_sdlgpushaders.cpp.
	// The code is in spirv[], dxil[] and msl[], which CMakeLists.txt fills in the build directory
	// from the files in shaders/, for the formats SDL_GPU has devices for on the platform: SPIR-V
	// everywhere but on Apple's, which only have MSL, and DXIL on Windows. The others have no
	// shaders.

	namespace SdlGpuShaders
	{
		enum class Stage : uint8_t
		{
			Vertex,
			Pixel
		};

		// The storage buffers the shaders read, see ../hlsl/README.md.

		enum class Resource : uint8_t
		{
			None,
			Colors,					// t0
			Extras,					// t1
			EdgemapOrPalette,		// t3
			WarpVertices			// t4
		};

		const int c_maxStorageBuffers = 4;

		struct Shader
		{
			const char *	pName;
			Stage			stage;
			uint8_t			nSamplers;				// 1 for the blit source, sampled texture and sampler 0.
			uint8_t			nStorageBuffers;
			Resource		storageBuffers[c_maxStorageBuffers];	// What each storage buffer slot is.
			uint8_t			nUniformBuffers;		// CanvasInfo, PixelInfo, WarpInfo, and SdlOutputInfo for Dual pixel shaders.
		};

		extern const Shader		shaders[];
		extern const int		nbShaders;

		struct Code
		{
			const char *		pName;
			const uint8_t *		pData;
			size_t				bytes;
		};

		extern const Code		spirv[];
		extern const int		nbSpirv;

		extern const Code		dxil[];
		extern const int		nbDxil;

		extern const Code		msl[];
		extern const int		nbMsl;
	}
}

#endif // WAX_SDLGPUSHADERS_DOT_H
