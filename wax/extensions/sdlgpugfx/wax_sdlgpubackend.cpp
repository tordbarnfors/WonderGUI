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

#include <wax_sdlgpubackend.h>
#include <wax_sdlgpubitmap.h>
#include <wax_sdlgpubitmapfactory.h>
#include <wax_sdlgpuedgemap.h>
#include <wax_sdlgpuedgemapfactory.h>
#include <wax_sdlgpuwarpmap.h>
#include <wax_sdlgpuwarpmapfactory.h>

#include <wax_base.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstddef>

namespace wax
{
	const TypeInfo SdlGpuBackend::TYPEINFO = { "SdlGpuBackend", &GpuBackend::TYPEINFO };

	using SdlGpuShaders::Resource;

	// Uniform buffer slots, see ../hlsl/constants.hlsl and sdlgpu_dual.hlsl.

	static const uint32_t c_sdlOutputSlot = 3;

	//____ create() ______________________________________________________________

	SdlGpuBackend_p SdlGpuBackend::create()
	{
		if (!SdlGpuContext::isReady())
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite,
				"No SDL_GPU device set. Call SdlGpuBackend::setDevice() before creating a backend.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return SdlGpuBackend_p(new SdlGpuBackend());
	}

	//____ setDevice() ___________________________________________________________

	bool SdlGpuBackend::setDevice( SDL_GPUDevice * pDevice )
	{
		if (pDevice == SdlGpuContext::device() && pDevice)
			return true;

		if (SdlGpuContext::nbBackends() > 0)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::IllegalCall,
				"Can't change SDL_GPU device while SdlGpuBackends exist.",
				nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return false;
		}

		SdlGpuContext::exit();

		if (!pDevice)
			return true;

		return SdlGpuContext::init(pDevice);
	}

	//____ constructor ___________________________________________________________

	SdlGpuBackend::SdlGpuBackend()
	{
		m_pBitmapFactory = SdlGpuBitmapFactory::create();
		m_pEdgemapFactory = SdlGpuEdgemapFactory::create();
		m_pWarpmapFactory = SdlGpuWarpmapFactory::create();

		SdlGpuContext::addBackend();

		m_bDeviceObjectsOK = _createDeviceObjects();

		if (!m_bDeviceObjectsOK)
			Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create the SDL_GPU objects of the backend, nothing will render.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
	}

	//____ destructor ____________________________________________________________
	//
	// SDL_GPU keeps what we release until the GPU is done with it.

	SdlGpuBackend::~SdlGpuBackend()
	{
		_destroyDeviceObjects();
		SdlGpuContext::removeBackend();
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& SdlGpuBackend::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _createDeviceObjects() ________________________________________________

	bool SdlGpuBackend::_createDeviceObjects()
	{
		SDL_GPUDevice * pDevice = SdlGpuContext::device();

		// Shaders are made when a pipeline first needs them, see _shader().

		m_shaders.assign(SdlGpuShaders::nbShaders, nullptr);

		for (int i = 0; i < 4; i++)
		{
			bool bTiling = (i & 2) != 0;
			bool bBilinear = (i & 1) != 0;

			SDL_GPUSamplerAddressMode addressMode = bTiling ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

			SDL_GPUSamplerCreateInfo info = {};
			info.min_filter = bBilinear ? SDL_GPU_FILTER_LINEAR : SDL_GPU_FILTER_NEAREST;
			info.mag_filter = bBilinear ? SDL_GPU_FILTER_LINEAR : SDL_GPU_FILTER_NEAREST;
			info.mipmap_mode = bBilinear ? SDL_GPU_SAMPLERMIPMAPMODE_LINEAR : SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
			info.address_mode_u = addressMode;
			info.address_mode_v = addressMode;
			info.address_mode_w = addressMode;
			info.min_lod = 0.f;
			info.max_lod = 1000.f;

			m_samplers[i] = SDL_CreateGPUSampler(pDevice, &info);

			if (!WAX_SDL_CHECK(m_samplers[i] != nullptr, "SDL_CreateGPUSampler"))
				return false;
		}

		SDL_GPUBufferCreateInfo info = {};
		info.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
		info.size = 256;

		m_pDummyBuffer = SDL_CreateGPUBuffer(pDevice, &info);

		return WAX_SDL_CHECK(m_pDummyBuffer != nullptr, "SDL_CreateGPUBuffer");
	}

	//____ _destroyDeviceObjects() _______________________________________________

	void SdlGpuBackend::_destroyDeviceObjects()
	{
		SDL_GPUDevice * pDevice = SdlGpuContext::device();
		if (!pDevice)
			return;

		if (m_bSessionTransferMapped)
			SDL_UnmapGPUTransferBuffer(pDevice, m_pSessionTransfer);

		m_bSessionTransferMapped = false;

		for (auto& entry : m_pipelines)
		{
			for (auto pDraw : entry.second.pDraws)
				if (pDraw)
					SDL_ReleaseGPUGraphicsPipeline(pDevice, pDraw);
		}

		m_pipelines.clear();

		for (auto pShader : m_shaders)
			if (pShader)
				SDL_ReleaseGPUShader(pDevice, pShader);

		m_shaders.clear();

		for (auto& pSampler : m_samplers)
		{
			if (pSampler)
				SDL_ReleaseGPUSampler(pDevice, pSampler);
			pSampler = nullptr;
		}

		for (SDL_GPUBuffer ** ppBuffer : { &m_pVertexBuffer, &m_pColorBuffer, &m_pExtrasBuffer, &m_pDummyBuffer })
		{
			if (*ppBuffer)
				SDL_ReleaseGPUBuffer(pDevice, *ppBuffer);
			*ppBuffer = nullptr;
		}

		if (m_pSessionTransfer)
			SDL_ReleaseGPUTransferBuffer(pDevice, m_pSessionTransfer);

		m_pSessionTransfer = nullptr;
		m_buffers = FrameBuffers();
	}

	//____ beginRender() _________________________________________________________
	//
	// Each session has a command buffer of its own, see _execute(), so a frame is nothing more
	// than the sessions in it.

	void SdlGpuBackend::beginRender()
	{
		m_bRendering = m_bDeviceObjectsOK && SdlGpuContext::isReady();

		if (!m_bRendering)
			m_buffers = FrameBuffers();
	}

	//____ endRender() ___________________________________________________________

	void SdlGpuBackend::endRender()
	{
		m_bRendering = false;
		m_pOwnerCmd = nullptr;
	}

	//____ setCommandBuffer() ____________________________________________________

	void SdlGpuBackend::setCommandBuffer( SDL_GPUCommandBuffer * pCommandBuffer )
	{
		m_pOwnerCmd = pCommandBuffer;
	}

	//____ _isRecording() ________________________________________________________

	bool SdlGpuBackend::_isRecording() const
	{
		return m_bRendering;
	}

	//____ _isNativeBitmap() ____________________________________________________

	bool SdlGpuBackend::_isNativeBitmap( const Object * pObject ) const
	{
		return pObject && _isOfType(pObject->typeInfo(), SdlGpuBitmap::TYPEINFO);
	}

	//____ _isNativeEdgemap() ____________________________________________________

	bool SdlGpuBackend::_isNativeEdgemap( const Object * pObject ) const
	{
		return pObject && _isOfType(pObject->typeInfo(), SdlGpuEdgemap::TYPEINFO);
	}

	//____ _isNativeWarpmap() ____________________________________________________

	bool SdlGpuBackend::_isNativeWarpmap( const Object * pObject ) const
	{
		return pObject && _isOfType(pObject->typeInfo(), SdlGpuWarpmap::TYPEINFO);
	}

	//____ _beginSessionOnDevice() _______________________________________________

	void SdlGpuBackend::_beginSessionOnDevice( Bitmap * pCanvas )
	{
		m_ops.clear();
		m_patches.clear();
		m_sessionObjects.clear();
		m_constantSnapshots.clear();

		m_bTargetBound = false;
		m_targetFormat = SDL_GPU_TEXTUREFORMAT_INVALID;
		m_targetSize = SizeI();
	}

	//____ _endSessionOnDevice() _________________________________________________

	void SdlGpuBackend::_endSessionOnDevice()
	{
		_execute();

		m_ops.clear();
		m_patches.clear();
		m_sessionObjects.clear();
		m_constantSnapshots.clear();

		m_buffers = FrameBuffers();
		m_bTargetBound = false;
	}

	//____ _reserveFrameBuffers() ________________________________________________
	//
	// The transfer buffer is mapped with cycling, which gives us new memory when the GPU still
	// has a session before us to copy from it.

	bool SdlGpuBackend::_reserveFrameBuffers( int nVertices, int nColors, int nExtras, int nObjects )
	{
		SDL_GPUDevice * pDevice = SdlGpuContext::device();

		m_buffers = FrameBuffers();

		if (m_bSessionTransferMapped)
		{
			SDL_UnmapGPUTransferBuffer(pDevice, m_pSessionTransfer);
			m_bSessionTransferMapped = false;
		}

		auto grow = [this,pDevice](SDL_GPUBuffer*& pBuffer, int& capacity, int needed, uint32_t entrySize, SDL_GPUBufferUsageFlags usage) -> bool
		{
			needed = std::max(needed, 1);

			if (pBuffer && needed <= capacity)
				return true;

			int newCapacity = std::max(capacity * 2, needed);

			SDL_GPUBufferCreateInfo info = {};
			info.usage = usage;
			info.size = uint32_t(newCapacity) * entrySize;

			SDL_GPUBuffer * pNew = SDL_CreateGPUBuffer(pDevice, &info);

			if (!WAX_SDL_CHECK(pNew != nullptr, "SDL_CreateGPUBuffer"))
				return false;

			if (pBuffer)
				SDL_ReleaseGPUBuffer(pDevice, pBuffer);

			pBuffer = pNew;
			capacity = newCapacity;
			return true;
		};

		bool bOK = grow(m_pVertexBuffer, m_vertexCapacity, nVertices, sizeof(Vertex), SDL_GPU_BUFFERUSAGE_VERTEX);
		bOK = bOK && grow(m_pColorBuffer, m_colorCapacity, nColors, sizeof(Color8), SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ);
		bOK = bOK && grow(m_pExtrasBuffer, m_extrasCapacity, nExtras, sizeof(Extras), SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ);

		if (!bOK)
			return false;

		uint32_t transferSize = uint32_t(m_vertexCapacity) * sizeof(Vertex) + uint32_t(m_colorCapacity) * sizeof(Color8) + uint32_t(m_extrasCapacity) * sizeof(Extras);

		if (!m_pSessionTransfer || transferSize > m_sessionTransferSize)
		{
			SDL_GPUTransferBufferCreateInfo info = {};
			info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
			info.size = transferSize;

			SDL_GPUTransferBuffer * pNew = SDL_CreateGPUTransferBuffer(pDevice, &info);

			if (!WAX_SDL_CHECK(pNew != nullptr, "SDL_CreateGPUTransferBuffer"))
				return false;

			if (m_pSessionTransfer)
				SDL_ReleaseGPUTransferBuffer(pDevice, m_pSessionTransfer);

			m_pSessionTransfer = pNew;
			m_sessionTransferSize = transferSize;
		}

		auto pMapped = (uint8_t*) SDL_MapGPUTransferBuffer(pDevice, m_pSessionTransfer, true);

		if (!WAX_SDL_CHECK(pMapped != nullptr, "SDL_MapGPUTransferBuffer"))
			return false;

		m_bSessionTransferMapped = true;

		m_buffers.pVertexBeg = (Vertex*) pMapped;
		m_buffers.pVertexPtr = m_buffers.pVertexBeg;
		m_buffers.pVertexEnd = m_buffers.pVertexBeg + m_vertexCapacity;

		m_buffers.pColorBeg = (Color8*) m_buffers.pVertexEnd;
		m_buffers.pColorPtr = m_buffers.pColorBeg;
		m_buffers.pColorEnd = m_buffers.pColorBeg + m_colorCapacity;

		m_buffers.pExtrasBeg = (Extras*) m_buffers.pColorEnd;
		m_buffers.pExtrasPtr = m_buffers.pExtrasBeg;
		m_buffers.pExtrasEnd = m_buffers.pExtrasBeg + m_extrasCapacity;

		return true;
	}

	//____ _record() _____________________________________________________________

	void SdlGpuBackend::_record( Op op, int a, int b, Object * pObject, const RectI& rect, const Pipeline * pPipeline )
	{
		m_ops.push_back( { op, a, b, pObject, pPipeline, rect } );
	}

	//____ _holdForSession() _____________________________________________________
	//
	// Keeps an object the recording points at until it has been played back.

	void SdlGpuBackend::_holdForSession( Object * pObject )
	{
		if( pObject && (m_sessionObjects.empty() || m_sessionObjects.back().rawPtr() != pObject) )
			m_sessionObjects.push_back(pObject);
	}

	//____ _bindCanvas() _________________________________________________________

	bool SdlGpuBackend::_bindCanvas( Bitmap * pCanvas, SizeI& size, CanvasKind& kind )
	{
		if (!m_bRendering)
			return false;

		auto pBitmap = static_cast<SdlGpuBitmap*>(pCanvas);

		if (pBitmap && (!pBitmap->m_bCanvas || !pBitmap->texture()))
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Bitmap can not be used as canvas, rendering to default canvas instead.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			pBitmap = nullptr;
		}

		m_bTargetBound = false;
		m_bSessionOnDefaultCanvas = (pBitmap == nullptr);

		if (pBitmap)
		{
			size = pBitmap->pixelSize();
			kind = _canvasKind(pBitmap);
			m_targetFormat = pBitmap->textureFormat();
		}
		else
		{
			if (!m_pDefaultTexture)
				return false;

			size = { m_defaultCanvas.size.w / 64, m_defaultCanvas.size.h / 64 };
			kind = CanvasKind::WithAlpha;
			m_targetFormat = m_defaultFormat;
		}

		_holdForSession(pBitmap);
		_record(Op::BindCanvas, 0, 0, pBitmap);

		m_bTargetBound = true;
		m_targetSize = size;
		return true;
	}

	//____ _bindPipeline() _______________________________________________________

	bool SdlGpuBackend::_bindPipeline( PipelineKind kind, BlendMode blendMode )
	{
		if (!m_bTargetBound)
			return false;

		Pipeline * pPipeline = _pipeline(kind, blendMode);

		if (!pPipeline)
			return false;

		_record(Op::SetPipeline, 0, 0, nullptr, RectI(), pPipeline);
		return true;
	}

	//____ _pipeline() ___________________________________________________________
	//
	// Pipelines are created the first time a combination of canvas format, kind and blend mode
	// shows up, also when that fails, so a failure is only tried once.

	SdlGpuBackend::Pipeline * SdlGpuBackend::_pipeline( PipelineKind kind, BlendMode blendMode )
	{
		uint64_t key = _pipelineKey(uint64_t(m_targetFormat), m_activeCanvasKind, kind, blendMode);

		auto it = m_pipelines.find(key);
		if (it == m_pipelines.end())
			it = m_pipelines.emplace(key, _createPipeline(kind, _blendState(kind, blendMode, m_activeCanvasKind), m_targetFormat)).first;

		return it->second.pDraws[0] ? &it->second : nullptr;
	}

	//____ _createPipeline() _____________________________________________________
	//
	// Blending is that of GpuBackend::_blendState(), in SDL_GPU terms, but for the blending with a
	// second color, which SDL_GPU doesn't have. It always has the canvas keep 1 - second color,
	// which is the same for all channels:
	//
	// - With the source as it is (Replace, Morph): two draws, the first leaves canvas * (1 - second
	//   color), with the second color as its source, the second adds the source.
	// - With the source times the canvas (Multiply): one draw of canvas * (source + 1 - second
	//   color), which the Dual shader puts together.
	//
	// Alpha that is kept is kept by both. An alpha only canvas is R8, as in VulkanBackend.

	SdlGpuBackend::Pipeline SdlGpuBackend::_createPipeline( PipelineKind kind, const BlendState& blend, SDL_GPUTextureFormat format )
	{
		static const SDL_GPUBlendFactor factors[] = {								// In the order of BlendFactor.
			SDL_GPU_BLENDFACTOR_ZERO, SDL_GPU_BLENDFACTOR_ONE,
			SDL_GPU_BLENDFACTOR_SRC_COLOR, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR, SDL_GPU_BLENDFACTOR_SRC_ALPHA, SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
			SDL_GPU_BLENDFACTOR_DST_COLOR, SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR, SDL_GPU_BLENDFACTOR_DST_ALPHA, SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
			SDL_GPU_BLENDFACTOR_INVALID, SDL_GPU_BLENDFACTOR_INVALID, SDL_GPU_BLENDFACTOR_INVALID, SDL_GPU_BLENDFACTOR_INVALID };	// No second color.

		static const SDL_GPUBlendOp ops[] = {										// In the order of BlendOp.
			SDL_GPU_BLENDOP_ADD, SDL_GPU_BLENDOP_REVERSE_SUBTRACT, SDL_GPU_BLENDOP_MIN, SDL_GPU_BLENDOP_MAX };

		static_assert(sizeof(factors) / sizeof(factors[0]) == size_t(BlendFactor::Size) && sizeof(ops) / sizeof(ops[0]) == size_t(BlendOp::Size),
					  "An SDL_GPU value for each blend factor and operation");

		SDL_GPUColorTargetBlendState state = {};
		state.enable_blend = blend.bEnable;
		state.color_blend_op = ops[int(blend.colorOp)];
		state.src_color_blendfactor = factors[int(blend.srcColor)];
		state.dst_color_blendfactor = factors[int(blend.dstColor)];
		state.alpha_blend_op = ops[int(blend.alphaOp)];
		state.src_alpha_blendfactor = factors[int(blend.srcAlpha)];
		state.dst_alpha_blendfactor = factors[int(blend.dstAlpha)];
		state.enable_color_write_mask = true;
		state.color_write_mask = SDL_GPU_COLORCOMPONENT_R | SDL_GPU_COLORCOMPONENT_G | SDL_GPU_COLORCOMPONENT_B;

		if (blend.bWriteAlpha)
			state.color_write_mask |= SDL_GPU_COLORCOMPONENT_A;

		Pipeline pipeline;
		pipeline.pVertexShader = nullptr;

		if (!blend.bDualSource)
		{
			pipeline.pDraws[0] = _createDraw(kind, _pixelShaderName(kind, false), state, format);
		}
		else
		{
			bool bKeepAlpha = (blend.srcAlpha == BlendFactor::Zero && blend.dstAlpha == BlendFactor::One);
			bool bMultiply = (blend.srcColor == BlendFactor::DstColor);

			if (!bMultiply && blend.srcColor != BlendFactor::One)
			{
				Base::reportError(ErrorLevel::Error, ErrorCode::Internal, "Blending with a second color that SdlGpuBackend has no draws for.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				return pipeline;
			}

			SDL_GPUColorTargetBlendState first = state;
			first.color_blend_op = SDL_GPU_BLENDOP_ADD;
			first.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
			first.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
			first.dst_color_blendfactor = bMultiply ? SDL_GPU_BLENDFACTOR_SRC_COLOR : SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
			first.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
			first.dst_alpha_blendfactor = bKeepAlpha ? SDL_GPU_BLENDFACTOR_ONE : bMultiply ? SDL_GPU_BLENDFACTOR_SRC_ALPHA : SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

			pipeline.pDraws[0] = _createDraw(kind, _pixelShaderName(kind, true), first, format);
			pipeline.sdlOutput = bMultiply ? 2 : 1;

			if (!bMultiply && pipeline.pDraws[0])
			{
				SDL_GPUColorTargetBlendState second = first;
				second.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
				second.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
				second.src_alpha_blendfactor = bKeepAlpha ? SDL_GPU_BLENDFACTOR_ZERO : SDL_GPU_BLENDFACTOR_ONE;
				second.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;

				pipeline.pDraws[1] = _createDraw(kind, _pixelShaderName(kind, false), second, format);

				if (!pipeline.pDraws[1])
				{
					SDL_ReleaseGPUGraphicsPipeline(SdlGpuContext::device(), pipeline.pDraws[0]);
					pipeline.pDraws[0] = nullptr;
				}
			}
		}

		// The resources are those of the first draw, which the second shares, as a shader without
		// WAX_DUAL_SOURCE reads what the one with does.

		_shader(_vertexShaderName(kind), &pipeline.pVertexShader);
		_shader(_pixelShaderName(kind, blend.bDualSource), &pipeline.pPixelShader);

		return pipeline;
	}

	//____ _createDraw() _________________________________________________________

	SDL_GPUGraphicsPipeline * SdlGpuBackend::_createDraw( PipelineKind kind, const char * pPixelShader, SDL_GPUColorTargetBlendState blend, SDL_GPUTextureFormat format )
	{
		SDL_GPUShader * pVertex = _shader(_vertexShaderName(kind), nullptr);
		SDL_GPUShader * pPixel = _shader(pPixelShader, nullptr);

		if (!pVertex || !pPixel)
			return nullptr;

		// One vertex layout for all but warps, whose vertex shader reads the vertices of the
		// Warpmap itself. Only the attributes the vertex shader reads, see _nbVertexAttributes().

		SDL_GPUVertexBufferDescription vertexBuffer = {};
		vertexBuffer.slot = 0;
		vertexBuffer.pitch = sizeof(Vertex);
		vertexBuffer.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;

		SDL_GPUVertexAttribute attributes[3] = {
			{ 0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, uint32_t(offsetof(Vertex, x)) },
			{ 1, 0, SDL_GPU_VERTEXELEMENTFORMAT_UINT, uint32_t(offsetof(Vertex, colorOfs)) },
			{ 2, 0, SDL_GPU_VERTEXELEMENTFORMAT_UINT, uint32_t(offsetof(Vertex, extrasOfs)) }
		};

		uint32_t nAttributes = uint32_t(_nbVertexAttributes(kind));

		SDL_GPUColorTargetDescription target = {};
		target.format = format;
		target.blend_state = blend;

		SDL_GPUGraphicsPipelineCreateInfo info = {};
		info.vertex_shader = pVertex;
		info.fragment_shader = pPixel;
		info.vertex_input_state.vertex_buffer_descriptions = &vertexBuffer;
		info.vertex_input_state.num_vertex_buffers = nAttributes > 0 ? 1 : 0;
		info.vertex_input_state.vertex_attributes = attributes;
		info.vertex_input_state.num_vertex_attributes = nAttributes;
		info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
		info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
		info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
		info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_CLOCKWISE;
		info.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;
		info.target_info.color_target_descriptions = &target;
		info.target_info.num_color_targets = 1;

		SDL_GPUGraphicsPipeline * pPipeline = SDL_CreateGPUGraphicsPipeline(SdlGpuContext::device(), &info);

		WAX_SDL_CHECK(pPipeline != nullptr, "SDL_CreateGPUGraphicsPipeline");
		return pPipeline;
	}

	//____ _shader() _____________________________________________________________
	//
	// The shader by the name GpuBackend has for it, made the first time a pipeline needs it, in the
	// format of the device. Null if there is no such shader, which is reported, or it couldn't be
	// made. What it binds goes to pInfo, if given, also when it couldn't be made.

	SDL_GPUShader * SdlGpuBackend::_shader( const char * pName, const SdlGpuShaders::Shader ** pInfo )
	{
		for (int i = 0; pName && i < SdlGpuShaders::nbShaders; i++)
		{
			const SdlGpuShaders::Shader& shader = SdlGpuShaders::shaders[i];

			if (std::strcmp(shader.pName, pName) != 0)
				continue;

			if (pInfo)
				*pInfo = &shader;

			if (!m_shaders[i])
			{
				SDL_GPUShaderFormat format = SdlGpuContext::shaderFormat();

				const SdlGpuShaders::Code * pCodes = format == SDL_GPU_SHADERFORMAT_DXIL ? SdlGpuShaders::dxil
												   : format == SDL_GPU_SHADERFORMAT_MSL ? SdlGpuShaders::msl : SdlGpuShaders::spirv;
				int nCodes = format == SDL_GPU_SHADERFORMAT_DXIL ? SdlGpuShaders::nbDxil
						   : format == SDL_GPU_SHADERFORMAT_MSL ? SdlGpuShaders::nbMsl : SdlGpuShaders::nbSpirv;

				const SdlGpuShaders::Code * pCode = nullptr;

				for (int c = 0; c < nCodes; c++)
					if (std::strcmp(pCodes[c].pName, pName) == 0)
						pCode = &pCodes[c];

				if (!pCode)
					break;

				SDL_GPUShaderCreateInfo info = {};
				info.code_size = pCode->bytes;
				info.code = pCode->pData;
				info.entrypoint = format == SDL_GPU_SHADERFORMAT_MSL ? "main0" : "main";		// SPIRV-Cross renames main.
				info.format = format;
				info.stage = shader.stage == SdlGpuShaders::Stage::Vertex ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;
				info.num_samplers = shader.nSamplers;
				info.num_storage_textures = 0;
				info.num_storage_buffers = shader.nStorageBuffers;
				info.num_uniform_buffers = shader.nUniformBuffers;

				m_shaders[i] = SDL_CreateGPUShader(SdlGpuContext::device(), &info);

				WAX_SDL_CHECK(m_shaders[i] != nullptr, "SDL_CreateGPUShader");
			}

			return m_shaders[i];
		}

		char buffer[128];
		snprintf(buffer, sizeof(buffer), "Shader %s is missing, run compile_shaders.py.", pName ? pName : "without a name");
		Base::reportError(ErrorLevel::Error, ErrorCode::Internal, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);

		return nullptr;
	}

	//____ _setConstants() _______________________________________________________
	//
	// A copy goes with the recording, which pushes it to the uniform buffer of the block.

	void SdlGpuBackend::_setConstants( ConstantBlock block )
	{
		static_assert(sizeof(CanvasConstants) == 32 && sizeof(PixelConstants) == 16 && sizeof(WarpConstants) == 32, "Constant blocks must match constants.hlsl");

		if (!m_bRendering)
			return;

		ConstantSnapshot snapshot;
		std::memset(&snapshot, 0, sizeof(snapshot));

		switch (block)
		{
			case ConstantBlock::Canvas:
				snapshot.canvas = m_canvasConstants;
				break;

			case ConstantBlock::Pixel:
				snapshot.pixel = m_pixelConstants;
				break;

			case ConstantBlock::Warp:
				snapshot.warp = m_warpConstants;
				break;
		}

		m_constantSnapshots.push_back(snapshot);
		_record(Op::SetConstants, int(block), int(m_constantSnapshots.size()) - 1);
	}

	//____ _draw() _______________________________________________________________

	void SdlGpuBackend::_draw( int firstVertex, int nVertices )
	{
		if (m_bTargetBound && nVertices > 0)
			_record(Op::Draw, firstVertex, nVertices);
	}

	//____ _setScissor() _________________________________________________________

	void SdlGpuBackend::_setScissor( const RectI& rect )
	{
		if (m_bTargetBound)
			_record(Op::SetScissor, 0, 0, nullptr, rect);
	}

	//____ _resetScissor() _______________________________________________________

	void SdlGpuBackend::_resetScissor()
	{
		_setScissor( RectI(0, 0, m_targetSize.w, m_targetSize.h) );
	}

	//____ _prepareBlitSource() __________________________________________________
	//
	// What the CPU has written to it is uploaded when the session is played back, see _execute().

	void SdlGpuBackend::_prepareBlitSource( Bitmap * pBitmap )
	{
	}

	//____ _bindBlitTexture() ____________________________________________________

	bool SdlGpuBackend::_bindBlitTexture( Bitmap * pBitmap )
	{
		auto pSource = static_cast<SdlGpuBitmap*>(pBitmap);

		if (!m_bTargetBound || !pSource->texture())
			return false;

		if (pSource->isIndexed() && !pSource->_paletteBuffer())
			return false;

		_holdForSession(pSource);
		_record(Op::BindBlitTexture, 0, 0, pSource);
		return true;
	}

	//____ _bindBlitSampler() ____________________________________________________
	//
	// Palette based sources are read with Load(), but a sampler goes with the texture, which had
	// better not filter what is indexes. Clipping is up to the shaders, see the samplers.

	void SdlGpuBackend::_bindBlitSampler( Bitmap * pBitmap, bool bClip )
	{
		if (!m_bTargetBound)
			return;

		bool bIndexed = static_cast<SdlGpuBitmap*>(pBitmap)->isIndexed();
		bool bBilinear = !bIndexed && pBitmap->sampleMethod() == SampleMethod::Bilinear;

		_record(Op::BindSampler, (pBitmap->isTiling() ? 2 : 0) + (bBilinear ? 1 : 0));
	}

	//____ _bindEdgemap() ________________________________________________________

	bool SdlGpuBackend::_bindEdgemap( GpuEdgemap * pEdgemap )
	{
		auto pSdlEdgemap = static_cast<SdlGpuEdgemap*>(pEdgemap);

		if (!m_bTargetBound || !pSdlEdgemap->_buffer())
			return false;

		_holdForSession(pSdlEdgemap);
		_record(Op::BindEdgemap, 0, 0, pSdlEdgemap);
		return true;
	}

	//____ _drawWarpmap() ________________________________________________________

	void SdlGpuBackend::_drawWarpmap( Warpmap * pWarpmap, const RectI * pPatches, int nPatches )
	{
		auto pSdlWarpmap = static_cast<SdlGpuWarpmap*>(pWarpmap);

		if (!m_bTargetBound || !pSdlWarpmap->m_pBuffer || pWarpmap->triangleCount() == 0 || nPatches <= 0)
			return;

		_holdForSession(pWarpmap);

		int first = int(m_patches.size());
		m_patches.insert(m_patches.end(), pPatches, pPatches + nPatches);

		_record(Op::DrawWarpmap, first, nPatches, pWarpmap);
		_resetScissor();
	}

	//____ _execute() ____________________________________________________________
	//
	// Plays back the recording of a session in a command buffer of its own, which is submitted, so
	// that it is in front of anything that comes after: other sessions, and reading back. First a
	// copy pass with what the session wrote and what it draws from has changed, then the mip levels
	// that have to be made anew, then a render pass on the canvas.
	//
	// A session on the default canvas whose owner has given a command buffer, see
	// setCommandBuffer(), has its render pass there instead, for the owner to submit. What comes
	// before the render pass is still submitted here, so all that bitmaps, edgemaps and Warpmaps
	// have is on its way to the GPU when a session has ended, and only draws into a texture that
	// nothing of ours reads wait for the owner.

	void SdlGpuBackend::_execute()
	{
		SDL_GPUDevice * pDevice = SdlGpuContext::device();

		if (m_bSessionTransferMapped)
		{
			SDL_UnmapGPUTransferBuffer(pDevice, m_pSessionTransfer);
			m_bSessionTransferMapped = false;
		}

		if (m_ops.empty() || !pDevice)
			return;

		SDL_GPUCommandBuffer * pOwnCmd = SDL_AcquireGPUCommandBuffer(pDevice);

		if (!WAX_SDL_CHECK(pOwnCmd != nullptr, "SDL_AcquireGPUCommandBuffer"))
			return;

		SDL_GPUCopyPass * pCopyPass = SDL_BeginGPUCopyPass(pOwnCmd);
		_recordUploads(pCopyPass);
		SDL_EndGPUCopyPass(pCopyPass);

		_generateMipmaps(pOwnCmd);

		bool bOwnerDraws = m_pOwnerCmd && m_bSessionOnDefaultCanvas;

		if (bOwnerDraws)
		{
			WAX_SDL_CHECK(SDL_SubmitGPUCommandBuffer(pOwnCmd), "SDL_SubmitGPUCommandBuffer");
			m_pCmd = m_pOwnerCmd;
		}
		else
			m_pCmd = pOwnCmd;

		// All uniform buffers get something before the first draw, which may not set them all.

		ConstantSnapshot zero;
		std::memset(&zero, 0, sizeof(zero));

		for (uint32_t slot = 0; slot <= c_sdlOutputSlot; slot++)
		{
			SDL_PushGPUVertexUniformData(m_pCmd, slot, &zero, sizeof(zero));
			SDL_PushGPUFragmentUniformData(m_pCmd, slot, &zero, sizeof(zero));
		}

		m_pPass = nullptr;
		m_pExecTarget = nullptr;
		m_pExecPipeline = nullptr;
		m_pBoundDraw = nullptr;
		m_execSdlOutput = 0;
		m_pExecBlitTexture = nullptr;
		m_pExecSampler = m_samplers[0];
		m_bBindingsChanged = true;

		for (auto& pBuffer : m_execBuffers)
			pBuffer = m_pDummyBuffer;

		m_execBuffers[int(Resource::Colors)] = m_pColorBuffer;
		m_execBuffers[int(Resource::Extras)] = m_pExtrasBuffer;

		static const uint32_t constantSizes[3] = { sizeof(CanvasConstants), sizeof(PixelConstants), sizeof(WarpConstants) };

		for (const Op_t& op : m_ops)
		{
			switch (op.op)
			{
				case Op::BindCanvas:
					_executeBindCanvas(static_cast<SdlGpuBitmap*>(op.pObject));
					break;

				case Op::SetPipeline:
					if (op.pPipeline != m_pExecPipeline)
					{
						m_pExecPipeline = op.pPipeline;
						m_bBindingsChanged = true;
					}
					break;

				case Op::SetConstants:
				{
					const ConstantSnapshot& snapshot = m_constantSnapshots[op.b];

					SDL_PushGPUVertexUniformData(m_pCmd, uint32_t(op.a), &snapshot, constantSizes[op.a]);
					SDL_PushGPUFragmentUniformData(m_pCmd, uint32_t(op.a), &snapshot, constantSizes[op.a]);
					break;
				}

				case Op::Draw:
					for (int i = 0; i < 2 && _prepareDraw(i); i++)
						SDL_DrawGPUPrimitives(m_pPass, uint32_t(op.b), 1, uint32_t(op.a), 0);
					break;

				case Op::SetScissor:
					_executeScissor(op.rect);
					break;

				case Op::BindBlitTexture:
				{
					auto pSource = static_cast<SdlGpuBitmap*>(op.pObject);

					m_pExecBlitTexture = pSource->texture();

					// A palette based source has its palette where edgemaps go, which is free while we blit.

					if (pSource->isIndexed())
						m_execBuffers[int(Resource::EdgemapOrPalette)] = pSource->_paletteBuffer();

					m_bBindingsChanged = true;
					break;
				}

				case Op::BindSampler:
					m_pExecSampler = m_samplers[op.a];
					m_bBindingsChanged = true;
					break;

				case Op::BindEdgemap:
					m_execBuffers[int(Resource::EdgemapOrPalette)] = static_cast<SdlGpuEdgemap*>(op.pObject)->_buffer();
					m_bBindingsChanged = true;
					break;

				case Op::DrawWarpmap:
				{
					auto pWarpmap = static_cast<SdlGpuWarpmap*>(op.pObject);

					if (m_execBuffers[int(Resource::WarpVertices)] != pWarpmap->m_pBuffer)
					{
						m_execBuffers[int(Resource::WarpVertices)] = pWarpmap->m_pBuffer;
						m_bBindingsChanged = true;
					}

					for (int i = 0; i < 2 && _prepareDraw(i); i++)
					{
						SDL_GPUBufferBinding indices = { pWarpmap->m_pBuffer, pWarpmap->m_indexOffset };
						SDL_BindGPUIndexBuffer(m_pPass, &indices, SDL_GPU_INDEXELEMENTSIZE_16BIT);

						for (int patch = 0; patch < op.b; patch++)
						{
							_executeScissor(m_patches[op.a + patch]);
							SDL_DrawGPUIndexedPrimitives(m_pPass, uint32_t(pWarpmap->triangleCount() * 3), 1, 0, 0, 0);
						}
					}
					break;
				}
			}
		}

		_endPass();

		if (!bOwnerDraws)
			WAX_SDL_CHECK(SDL_SubmitGPUCommandBuffer(m_pCmd), "SDL_SubmitGPUCommandBuffer");

		m_pCmd = nullptr;
		m_pExecTarget = nullptr;
		m_pExecPipeline = nullptr;
	}

	//____ _generateMipmaps() ____________________________________________________
	//
	// Mip levels that no longer match level 0 are made anew before anything reads them, for all
	// the session blits from. A session reads what it draws from when it ends, so they are all
	// made before its render pass, which they can't be made in.

	void SdlGpuBackend::_generateMipmaps( SDL_GPUCommandBuffer * pCmd )
	{
		for (const Op_t& op : m_ops)
		{
			if (op.op != Op::BindBlitTexture)
				continue;

			auto pSource = static_cast<SdlGpuBitmap*>(op.pObject);

			if (!pSource->isMipmapStale())
				continue;

			if (pSource->mipLevels() > 1)
				SDL_GenerateMipmapsForGPUTexture(pCmd, pSource->texture());

			pSource->clearMipmapStale();
		}
	}

	//____ _recordUploads() ______________________________________________________
	//
	// What the session wrote, cycling the buffers so that sessions still in flight keep theirs, and
	// what the CPU has changed of what it draws from and into.

	void SdlGpuBackend::_recordUploads( SDL_GPUCopyPass * pCopyPass )
	{
		struct { SDL_GPUBuffer * pBuffer; const void * pBeg; const void * pPtr; uint32_t entrySize; } sessionBuffers[3] = {
			{ m_pVertexBuffer, m_buffers.pVertexBeg, m_buffers.pVertexPtr, sizeof(Vertex) },
			{ m_pColorBuffer, m_buffers.pColorBeg, m_buffers.pColorPtr, sizeof(Color8) },
			{ m_pExtrasBuffer, m_buffers.pExtrasBeg, m_buffers.pExtrasPtr, sizeof(Extras) } };

		const uint8_t * pTransferBeg = (const uint8_t*) m_buffers.pVertexBeg;

		for (auto& buffer : sessionBuffers)
		{
			uint32_t bytes = uint32_t((const uint8_t*) buffer.pPtr - (const uint8_t*) buffer.pBeg);

			if (bytes == 0 || !buffer.pBuffer)
				continue;

			SDL_GPUTransferBufferLocation src = { m_pSessionTransfer, uint32_t((const uint8_t*) buffer.pBeg - pTransferBeg) };
			SDL_GPUBufferRegion dst = { buffer.pBuffer, 0, bytes };

			SDL_UploadToGPUBuffer(pCopyPass, &src, &dst, true);
		}

		SdlGpuContext::BufferUploads uploads;

		for (auto& pObject : m_sessionObjects)
		{
			const TypeInfo& type = pObject->typeInfo();

			if (_isOfType(type, SdlGpuBitmap::TYPEINFO))
			{
				auto pBitmap = static_cast<SdlGpuBitmap*>(pObject.rawPtr());

				if (pBitmap->_hasUploads())
					pBitmap->_recordUploads(pCopyPass, uploads);
			}
			else if (_isOfType(type, SdlGpuEdgemap::TYPEINFO))
				static_cast<SdlGpuEdgemap*>(pObject.rawPtr())->_recordUpload(uploads);
			else if (_isOfType(type, SdlGpuWarpmap::TYPEINFO))
				static_cast<SdlGpuWarpmap*>(pObject.rawPtr())->_recordUpload(uploads);
		}

		uploads.record(pCopyPass);
	}

	//____ _executeBindCanvas() __________________________________________________

	void SdlGpuBackend::_executeBindCanvas( SdlGpuBitmap * pCanvas )
	{
		_endPass();

		if (pCanvas)
		{
			m_pExecTarget = pCanvas->texture();
			m_execTargetSize = pCanvas->pixelSize();

			// What we draw isn't in its alpha map or mip levels.

			pCanvas->notifyRendered();
		}
		else
		{
			m_pExecTarget = m_pDefaultTexture;
			m_execTargetSize = { m_defaultCanvas.size.w / 64, m_defaultCanvas.size.h / 64 };
		}

		m_execScissor = RectI(0, 0, m_execTargetSize.w, m_execTargetSize.h);
		m_pExecPipeline = nullptr;			// Pipelines are made for the format of the canvas.
	}

	//____ _beginPass() __________________________________________________________
	//
	// Only the update rects are drawn, the rest stays, so the canvas is loaded.

	bool SdlGpuBackend::_beginPass()
	{
		if (m_pPass)
			return true;

		if (!m_pExecTarget)
			return false;

		SDL_GPUColorTargetInfo target = {};
		target.texture = m_pExecTarget;
		target.load_op = SDL_GPU_LOADOP_LOAD;
		target.store_op = SDL_GPU_STOREOP_STORE;

		m_pPass = SDL_BeginGPURenderPass(m_pCmd, &target, 1, nullptr);

		if (!WAX_SDL_CHECK(m_pPass != nullptr, "SDL_BeginGPURenderPass"))
		{
			m_pExecTarget = nullptr;			// Nothing more is drawn into it.
			return false;
		}

		SDL_GPUViewport viewport = { 0.f, 0.f, float(m_execTargetSize.w), float(m_execTargetSize.h), 0.f, 1.f };
		SDL_SetGPUViewport(m_pPass, &viewport);

		_executeScissor(m_execScissor);

		// A render pass starts with nothing bound.

		if (m_pVertexBuffer)
		{
			SDL_GPUBufferBinding binding = { m_pVertexBuffer, 0 };
			SDL_BindGPUVertexBuffers(m_pPass, 0, &binding, 1);
		}

		m_pBoundDraw = nullptr;
		m_bBindingsChanged = true;
		return true;
	}

	//____ _endPass() ____________________________________________________________

	void SdlGpuBackend::_endPass()
	{
		if (!m_pPass)
			return;

		SDL_EndGPURenderPass(m_pPass);
		m_pPass = nullptr;
		m_pBoundDraw = nullptr;
	}

	//____ _executeScissor() _____________________________________________________
	//
	// Kept for the render pass, which begins with the first draw. Scissors must be within the canvas.

	void SdlGpuBackend::_executeScissor( const RectI& rect )
	{
		m_execScissor = rect;

		if (!m_pPass)
			return;

		int x1 = std::max(rect.x, 0);
		int y1 = std::max(rect.y, 0);
		int x2 = std::min(rect.x + rect.w, m_execTargetSize.w);
		int y2 = std::min(rect.y + rect.h, m_execTargetSize.h);

		SDL_Rect scissor = { x1, y1, std::max(x2 - x1, 0), std::max(y2 - y1, 0) };
		SDL_SetGPUScissor(m_pPass, &scissor);
	}

	//____ _prepareDraw() ________________________________________________________
	//
	// Binds the pipeline of a draw of the pipeline being played back, and what it reads. Draw 1 is
	// the second of two, see _createPipeline().

	bool SdlGpuBackend::_prepareDraw( int draw )
	{
		if (!m_pExecPipeline || !m_pExecPipeline->pDraws[draw] || !_beginPass())
			return false;

		SDL_GPUGraphicsPipeline * pDraw = m_pExecPipeline->pDraws[draw];

		if (pDraw != m_pBoundDraw)
		{
			SDL_BindGPUGraphicsPipeline(m_pPass, pDraw);
			m_pBoundDraw = pDraw;
		}

		if (draw == 0 && m_pExecPipeline->sdlOutput != 0 && m_pExecPipeline->sdlOutput != m_execSdlOutput)
		{
			uint32_t sdlOutput[4] = { m_pExecPipeline->sdlOutput, 0, 0, 0 };
			SDL_PushGPUFragmentUniformData(m_pCmd, c_sdlOutputSlot, sdlOutput, sizeof(sdlOutput));
			m_execSdlOutput = m_pExecPipeline->sdlOutput;
		}

		if (m_bBindingsChanged)
			_bindResources();

		return true;
	}

	//____ _bindResources() ______________________________________________________
	//
	// Each shader has its storage buffers in slots of its own, see SdlGpuShaders, so they are bound
	// again for every pipeline. Bindings last across pipelines within a render pass.

	void SdlGpuBackend::_bindResources()
	{
		SDL_GPUBuffer * buffers[SdlGpuShaders::c_maxStorageBuffers];

		const SdlGpuShaders::Shader * pVertex = m_pExecPipeline->pVertexShader;
		const SdlGpuShaders::Shader * pPixel = m_pExecPipeline->pPixelShader;

		if (pVertex && pVertex->nStorageBuffers > 0)
		{
			for (int i = 0; i < pVertex->nStorageBuffers; i++)
				buffers[i] = m_execBuffers[int(pVertex->storageBuffers[i])];

			SDL_BindGPUVertexStorageBuffers(m_pPass, 0, buffers, pVertex->nStorageBuffers);
		}

		if (pPixel && pPixel->nStorageBuffers > 0)
		{
			for (int i = 0; i < pPixel->nStorageBuffers; i++)
				buffers[i] = m_execBuffers[int(pPixel->storageBuffers[i])];

			SDL_BindGPUFragmentStorageBuffers(m_pPass, 0, buffers, pPixel->nStorageBuffers);
		}

		if (pPixel && pPixel->nSamplers > 0 && m_pExecBlitTexture)
		{
			SDL_GPUTextureSamplerBinding binding = { m_pExecBlitTexture, m_pExecSampler };
			SDL_BindGPUFragmentSamplers(m_pPass, 0, &binding, 1);
		}

		m_bBindingsChanged = false;
	}

	//____ setDefaultCanvas() ____________________________________________________

	bool SdlGpuBackend::setDefaultCanvas( SDL_GPUTexture * pTexture, SDL_GPUTextureFormat format, SizeSPX size, int scale )
	{
		m_pDefaultTexture = pTexture;
		m_defaultFormat = format;

		m_defaultCanvas.ref = CanvasRef::Default;
		m_defaultCanvas.size = size;
		m_defaultCanvas.scale = scale;

		// Wax has no name for R8G8B8A8, so it is reported as ARGB_8, as by VulkanBackend.

		switch (format)
		{
			case SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM:
			case SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM:
				m_defaultCanvas.format = PixelFormat::ARGB_8;
				m_defaultCanvas.colorSpace = ColorSpace::Linear;
				break;

			case SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB:
			case SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB:
				m_defaultCanvas.format = PixelFormat::ARGB_8;
				m_defaultCanvas.colorSpace = ColorSpace::sRGB;
				break;

			default:
				m_defaultCanvas.format = PixelFormat::Undefined;
				m_pDefaultTexture = nullptr;

				Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam, "Default canvas has a format that isn't supported.",
					this, &TYPEINFO, __func__, __FILE__, __LINE__);
				return false;
		}

		return true;
	}

	//____ canvasInfo() __________________________________________________________

	const CanvasInfo* SdlGpuBackend::canvasInfo(CanvasRef ref) const
	{
		if (ref == CanvasRef::Default)
			return &m_defaultCanvas;

		Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam, "Only Default canvas is supported.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return &m_dummyCanvas;
	}

	//____ bitmapFactory() ______________________________________________________

	BitmapFactory_p SdlGpuBackend::bitmapFactory()
	{
		return m_pBitmapFactory;
	}

	//____ edgemapFactory() ______________________________________________________

	EdgemapFactory_p SdlGpuBackend::edgemapFactory()
	{
		return m_pEdgemapFactory;
	}

	//____ warpmapFactory() ______________________________________________________

	WarpmapFactory_p SdlGpuBackend::warpmapFactory()
	{
		return m_pWarpmapFactory;
	}

	//____ canBeBlitSource() _____________________________________________________

	bool SdlGpuBackend::canBeBlitSource(const TypeInfo& type) const
	{
		return _isOfType(type, SdlGpuBitmap::TYPEINFO);
	}

	//____ canBeCanvas() _________________________________________________________

	bool SdlGpuBackend::canBeCanvas(const TypeInfo& type) const
	{
		return _isOfType(type, SdlGpuBitmap::TYPEINFO);
	}

	//____ waitForCompletion() ___________________________________________________
	//
	// Every session has been submitted when it ended, but for draws into a default canvas that
	// went into the command buffer of its owner, see setCommandBuffer(), which we can't wait for.

	void SdlGpuBackend::waitForCompletion()
	{
		if (SdlGpuContext::device())
			SDL_WaitForGPUIdle(SdlGpuContext::device());
	}

} // namespace wax
