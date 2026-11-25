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

#include <wax_sdlgpucontext.h>
#include <wax_sdlgpushaders.h>

#include <wax_base.h>

#include <SDL3/SDL_error.h>

#include <cstdio>
#include <cstring>

namespace wax
{
	SDL_GPUDevice *		SdlGpuContext::s_pDevice = nullptr;
	SDL_GPUShaderFormat	SdlGpuContext::s_shaderFormat = SDL_GPU_SHADERFORMAT_INVALID;
	bool				SdlGpuContext::s_bTightUploadRows = false;
	int					SdlGpuContext::s_nBackends = 0;

	//____ init() ________________________________________________________________
	//
	// Picks the shaders by what the device takes, of the formats we have for the platform.

	bool SdlGpuContext::init( SDL_GPUDevice * pDevice )
	{
		SDL_GPUShaderFormat formats = SDL_GetGPUShaderFormats(pDevice);

		s_shaderFormat = SDL_GPU_SHADERFORMAT_INVALID;

		if( (formats & SDL_GPU_SHADERFORMAT_DXIL) && SdlGpuShaders::nbDxil > 0 )
			s_shaderFormat = SDL_GPU_SHADERFORMAT_DXIL;
		else if( (formats & SDL_GPU_SHADERFORMAT_MSL) && SdlGpuShaders::nbMsl > 0 )
			s_shaderFormat = SDL_GPU_SHADERFORMAT_MSL;
		else if( (formats & SDL_GPU_SHADERFORMAT_SPIRV) && SdlGpuShaders::nbSpirv > 0 )
			s_shaderFormat = SDL_GPU_SHADERFORMAT_SPIRV;

		if( s_shaderFormat == SDL_GPU_SHADERFORMAT_INVALID )
		{
			char buffer[256];
			snprintf(buffer, sizeof(buffer), "The SDL_GPU device (%s) takes no shaders we have for this platform, nothing will render.",
					 SDL_GetGPUDeviceDriver(pDevice));
			Base::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, buffer, nullptr, nullptr, __func__, __FILE__, __LINE__);
			return false;
		}

		const char * pDriver = SDL_GetGPUDeviceDriver(pDevice);

		s_bTightUploadRows = pDriver && strcmp(pDriver, "metal") == 0;

		s_pDevice = pDevice;
		return true;
	}

	//____ exit() ________________________________________________________________

	void SdlGpuContext::exit()
	{
		if( s_pDevice )
			SDL_WaitForGPUIdle(s_pDevice);

		s_pDevice = nullptr;
		s_shaderFormat = SDL_GPU_SHADERFORMAT_INVALID;
	}

	//____ check() _______________________________________________________________

	bool SdlGpuContext::check( bool bOK, const char * pWhat, const Object * pObject, const TypeInfo * pClassType,
							   const char * func, const char * file, int line )
	{
		if( !bOK )
		{
			char buffer[512];
			snprintf(buffer, sizeof(buffer), "%s failed: %s", pWhat, SDL_GetError());
			Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, buffer, pObject, pClassType, func, file, line);
		}

		return bOK;
	}

	//____ submitAndWait() _______________________________________________________

	bool SdlGpuContext::submitAndWait( SDL_GPUCommandBuffer * pCmd )
	{
		SDL_GPUFence * pFence = SDL_SubmitGPUCommandBufferAndAcquireFence(pCmd);

		if( !check(pFence != nullptr, "SDL_SubmitGPUCommandBufferAndAcquireFence", nullptr, nullptr, __func__, __FILE__, __LINE__) )
			return false;

		bool bOK = check(SDL_WaitForGPUFences(s_pDevice, true, &pFence, 1), "SDL_WaitForGPUFences", nullptr, nullptr, __func__, __FILE__, __LINE__);

		SDL_ReleaseGPUFence(s_pDevice, pFence);
		return bOK;
	}

	//____ BufferUploads::add() __________________________________________________

	void SdlGpuContext::BufferUploads::add( SDL_GPUBuffer * pBuffer, uint32_t offset, const void * pData, uint32_t bytes, bool bCycle )
	{
		if( pBuffer && pData && bytes > 0 )
			m_uploads.push_back( { pBuffer, offset, pData, bytes, bCycle } );
	}

	//____ BufferUploads::record() _______________________________________________

	bool SdlGpuContext::BufferUploads::record( SDL_GPUCopyPass * pCopyPass )
	{
		if( m_uploads.empty() )
			return true;

		// Each starts on 16 bytes, which no API needs for buffers, but costs next to nothing.

		uint32_t total = 0;

		for( auto& upload : m_uploads )
			total += (upload.bytes + 15) & ~15u;

		SDL_GPUTransferBufferCreateInfo info = {};
		info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		info.size = total;

		SDL_GPUTransferBuffer * pTransfer = SDL_CreateGPUTransferBuffer(s_pDevice, &info);

		if( !check(pTransfer != nullptr, "SDL_CreateGPUTransferBuffer", nullptr, nullptr, __func__, __FILE__, __LINE__) )
		{
			m_uploads.clear();
			return false;
		}

		auto pMapped = (uint8_t*) SDL_MapGPUTransferBuffer(s_pDevice, pTransfer, false);

		if( !check(pMapped != nullptr, "SDL_MapGPUTransferBuffer", nullptr, nullptr, __func__, __FILE__, __LINE__) )
		{
			SDL_ReleaseGPUTransferBuffer(s_pDevice, pTransfer);
			m_uploads.clear();
			return false;
		}

		uint32_t ofs = 0;

		for( auto& upload : m_uploads )
		{
			std::memcpy(pMapped + ofs, upload.pData, upload.bytes);
			ofs += (upload.bytes + 15) & ~15u;
		}

		SDL_UnmapGPUTransferBuffer(s_pDevice, pTransfer);

		ofs = 0;

		for( auto& upload : m_uploads )
		{
			SDL_GPUTransferBufferLocation src = { pTransfer, ofs };
			SDL_GPUBufferRegion dst = { upload.pBuffer, upload.offset, upload.bytes };

			SDL_UploadToGPUBuffer(pCopyPass, &src, &dst, upload.bCycle);

			ofs += (upload.bytes + 15) & ~15u;
		}

		// Released when the GPU is done with the copies.

		SDL_ReleaseGPUTransferBuffer(s_pDevice, pTransfer);

		m_uploads.clear();
		return true;
	}

} // namespace wax
