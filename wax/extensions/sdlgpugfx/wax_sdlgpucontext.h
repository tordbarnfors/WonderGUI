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
#ifndef WAX_SDLGPUCONTEXT_DOT_H
#define WAX_SDLGPUCONTEXT_DOT_H
#pragma once

#include <SDL3/SDL_gpu.h>

#include <wax_geo.h>

#include <vector>
#include <cstdint>

namespace wax
{
	class Object;
	struct TypeInfo;

	//____ SdlGpuContext ________________________________________________________
	/**
	 * @brief The SDL_GPU device shared by SdlGpuBackends and their surfaces, edgemaps and Warpmaps.
	 *
	 * Set by SdlGpuBackend::setDevice(). SDL_GPU keeps what the GPU still uses alive until it is
	 * done with it, and gives buffers and textures we upload to that are in use new memory when
	 * asked to cycle them, so there is no bookkeeping of that here.
	 *
	 * Not thread safe, like the rest of Wax.
	 */

	class SdlGpuContext
	{
	public:

		static bool				init( SDL_GPUDevice * pDevice );
		static void				exit();
		static bool				isReady() { return s_pDevice != nullptr; }

		static SDL_GPUDevice *	device() { return s_pDevice; }

		// The format of the shaders for the device, see SdlGpuShaders. INVALID if we have none it
		// takes.

		static SDL_GPUShaderFormat	shaderFormat() { return s_shaderFormat; }

		// True when an upload to a texture must come from rows exactly as long as the region
		// written. The Metal driver of SDL_GPU (3.4.18) ignores pixels_per_row of an upload and
		// takes the length of a row from the width of the region instead, which shears any upload
		// from a wider buffer.

		static bool				tightUploadRows() { return s_bTightUploadRows; }

		// SDL_GPU has no query for it. D3D12 and Metal have at least this, and so do Vulkan
		// drivers for desktop GPUs.

		static int				maxTextureSize() { return 16384; }

		//.____ Errors ____________________________________________________________

		// Reports a failed call through the error handler, with what SDL_GetError() says.
		// Returns bOK.

		static bool				check( bool bOK, const char * pWhat, const Object * pObject, const TypeInfo * pClassType,
									   const char * func, const char * file, int line );

		//.____ Commands __________________________________________________________

		// Submits a command buffer and waits until the GPU is done with it. For reading back.

		static bool				submitAndWait( SDL_GPUCommandBuffer * pCmd );

		//.____ Backends ___________________________________________________________

		static void				addBackend() { s_nBackends++; }
		static void				removeBackend() { s_nBackends--; }
		static int				nbBackends() { return s_nBackends; }

		//.____ Uploads ____________________________________________________________

		// Buffer contents to upload in a copy pass, put in one transfer buffer of their own by
		// record(), which is released when the GPU is done with it. The memory given to add() is
		// read by record().

		class BufferUploads
		{
		public:
			// Cycling gives a buffer that is in use new memory, which only the parts uploaded
			// after are written to. So only an upload of all there is to it, or the first of
			// those that together are, should cycle.

			void		add( SDL_GPUBuffer * pBuffer, uint32_t offset, const void * pData, uint32_t bytes, bool bCycle );
			bool		record( SDL_GPUCopyPass * pCopyPass );

			bool		isEmpty() const { return m_uploads.empty(); }

		private:
			struct Upload
			{
				SDL_GPUBuffer *	pBuffer;
				uint32_t		offset;
				const void *	pData;
				uint32_t		bytes;
				bool			bCycle;
			};

			std::vector<Upload>	m_uploads;
		};

	private:

		static SDL_GPUDevice *		s_pDevice;
		static SDL_GPUShaderFormat	s_shaderFormat;
		static bool					s_bTightUploadRows;
		static int					s_nBackends;
	};

	#define WAX_SDL_CHECK(call, what) SdlGpuContext::check(call, what, this, &TYPEINFO, __func__, __FILE__, __LINE__)

} // namespace wax
#endif // WAX_SDLGPUCONTEXT_DOT_H
