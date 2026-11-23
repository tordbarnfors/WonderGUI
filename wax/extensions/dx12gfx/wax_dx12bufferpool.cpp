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

#define NOMINMAX

#include <wax_dx12bufferpool.h>
#include <wax_dx12backend.h>
#include <wax_base.h>

#include <algorithm>

namespace wax
{

	Microsoft::WRL::ComPtr<ID3D12Device>		DX12BufferPool::s_pDevice;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>	DX12BufferPool::s_pQueue;
	Microsoft::WRL::ComPtr<ID3D12Fence>			DX12BufferPool::s_pFence;
	UINT64										DX12BufferPool::s_lastSubmitted = 0;
	std::vector<DX12BufferPool::Block>			DX12BufferPool::s_blocks;
	std::vector<DX12BufferPool::Waiting>		DX12BufferPool::s_waiting;
	uint32_t									DX12BufferPool::s_generation = 1;

	//____ setDevice() ___________________________________________________________

	void DX12BufferPool::setDevice( ID3D12Device * pDevice, ID3D12CommandQueue * pQueue )
	{
		if( s_pDevice )
			exitDevice();

		s_pDevice = pDevice;
		s_pQueue = pQueue;
		s_lastSubmitted = 0;

		// Without a fence nothing is known about what the GPU has done, and buffers given back
		// wait until the device is let go of.

		if( pDevice && FAILED(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(s_pFence.ReleaseAndGetAddressOf()))) )
		{
			s_pFence = nullptr;

			if( !DX12Backend::checkDeviceLost() )
				Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create fence for the buffer pool.",
					nullptr, &DX12Backend::TYPEINFO, __func__, __FILE__, __LINE__);
		}

		if( s_pFence )
			s_pFence->SetName(L"Wax DX12BufferPool: fence");
	}

	//____ exitDevice() __________________________________________________________
	//
	// Whoever has a buffer finds out by isValid(), or has let go of it already. Releasing a
	// block unmaps it.

	void DX12BufferPool::exitDevice()
	{
		s_waiting.clear();
		s_blocks.clear();

		s_pFence = nullptr;
		s_pQueue = nullptr;
		s_pDevice = nullptr;

		s_lastSubmitted = 0;
		s_generation++;
	}

	//____ alloc() _______________________________________________________________

	bool DX12BufferPool::alloc( int bytes, Buffer& buffer )
	{
		buffer = Buffer();

		if( bytes <= 0 || !s_pDevice || DX12Backend::isDeviceLost() )
			return false;

		// What the GPU is done with may be all the room we need.

		collect();

		if( bytes > c_blockCapacity )
		{
			if( !_createResource(bytes, buffer.resource, buffer.pData) )
				return false;

			buffer.gpuAddress = buffer.resource->GetGPUVirtualAddress();
		}
		else
		{
			// Older blocks first, so that newer ones get the chance to empty and be released. A
			// new one is only made when no block has room, so not while one is empty.

			int block = -1;
			int offset = -1;

			for( int i = 0 ; i < int(s_blocks.size()) && offset < 0 ; i++ )
			{
				if( s_blocks[i].resource )
				{
					offset = s_blocks[i].ranges.alloc(bytes);
					block = i;
				}
			}

			if( offset < 0 )
			{
				block = _createBlock();
				if( block < 0 )
					return false;

				offset = s_blocks[block].ranges.alloc(bytes);
			}

			buffer.block = block;
			buffer.offset = offset;
			buffer.pData = s_blocks[block].pData + offset;
			buffer.gpuAddress = s_blocks[block].gpuAddress + offset;
		}

		buffer.size = bytes;
		buffer.generation = s_generation;
		return true;
	}

	//____ free() ________________________________________________________________
	//
	// Draws recorded but not yet submitted may read the buffer too, and are submitted with a
	// value we don't know yet. With nothing being recorded, everything that may read it has
	// been submitted, and if the GPU is done with that already, there is nothing to wait for.

	void DX12BufferPool::free( Buffer& buffer )
	{
		if( buffer.generation == s_generation && (buffer.block >= 0 || buffer.resource) )
		{
			Waiting waiting;

			waiting.bValueKnown = !DX12Backend::_isAnyRecording();
			waiting.value = s_lastSubmitted;
			waiting.block = buffer.block;
			waiting.offset = buffer.offset;
			waiting.size = buffer.size;
			waiting.resource = buffer.resource;

			if( waiting.bValueKnown && s_pFence && s_pFence->GetCompletedValue() >= waiting.value )
				_release(waiting);
			else
				s_waiting.push_back(waiting);
		}

		buffer = Buffer();
	}

	//____ submitted() ___________________________________________________________

	void DX12BufferPool::submitted()
	{
		if( !s_pFence || !s_pQueue )
			return;

		s_lastSubmitted++;
		s_pQueue->Signal(s_pFence.Get(), s_lastSubmitted);
	}

	//____ collect() _____________________________________________________________
	//
	// A buffer given back while a backend was recording gets its value once nothing is being
	// recorded: what was recorded then has been submitted by now, or never will be. A device
	// that is removed completes its fences with the highest value there is, so nothing is left
	// waiting for it.

	void DX12BufferPool::collect()
	{
		if( s_waiting.empty() || !s_pFence )
			return;

		bool bRecording = DX12Backend::_isAnyRecording();
		UINT64 done = s_pFence->GetCompletedValue();

		auto it = std::remove_if( s_waiting.begin(), s_waiting.end(), [bRecording, done]( Waiting& waiting )
		{
			if( !waiting.bValueKnown )
			{
				if( bRecording )
					return false;

				waiting.value = s_lastSubmitted;
				waiting.bValueKnown = true;
			}

			if( waiting.value > done )
				return false;

			_release(waiting);
			return true;
		});

		s_waiting.erase( it, s_waiting.end() );
	}

	//____ _release() ____________________________________________________________
	//
	// The memory is free again: a range of a block, or a resource of its own, which goes here.

	void DX12BufferPool::_release( Waiting& waiting )
	{
		if( waiting.block >= 0 && waiting.block < int(s_blocks.size()) )
		{
			s_blocks[waiting.block].ranges.free(waiting.offset, waiting.size);

			if( s_blocks[waiting.block].ranges.isEmpty() )
				_blockEmptied(waiting.block);
		}

		waiting.resource = nullptr;
	}

	//____ _blockEmptied() _______________________________________________________
	//
	// One empty block is kept for what comes next, so that a buffer made and let go of over and
	// over doesn't take making a block each time. With one empty already, the newer of the two
	// is released, so there is never more than one. A range that waits for the GPU still counts
	// as taken.

	void DX12BufferPool::_blockEmptied( int block )
	{
		int other = -1;

		for( int i = 0 ; i < int(s_blocks.size()) && other < 0 ; i++ )
		{
			if( i != block && s_blocks[i].resource && s_blocks[i].ranges.isEmpty() )
				other = i;
		}

		if( other < 0 )
			return;

		Block& released = s_blocks[std::max(block, other)];

		released.resource = nullptr;				// Unmaps it too.
		released.pData = nullptr;
		released.gpuAddress = 0;

		while( !s_blocks.empty() && !s_blocks.back().resource )
			s_blocks.pop_back();
	}

	//____ _createBlock() ________________________________________________________
	//
	// Returns the index of a new block, -1 if it couldn't be created.

	int DX12BufferPool::_createBlock()
	{
		Block block;

		if( !_createResource(c_blockSize, block.resource, block.pData) )
			return -1;

		block.resource->SetName(L"Wax DX12BufferPool: block");
		block.gpuAddress = block.resource->GetGPUVirtualAddress();

		// In the place of one that has been released, if there is one.

		for( int i = 0 ; i < int(s_blocks.size()) ; i++ )
		{
			if( !s_blocks[i].resource )
			{
				s_blocks[i] = block;
				return i;
			}
		}

		s_blocks.push_back(block);
		return int(s_blocks.size()) - 1;
	}

	//____ _createResource() _____________________________________________________
	//
	// An upload heap buffer, mapped for writing.

	bool DX12BufferPool::_createResource( int bytes, Microsoft::WRL::ComPtr<ID3D12Resource>& resource, uint8_t *& pData )
	{
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC bufDesc = {};
		bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufDesc.Width = UINT64(bytes);
		bufDesc.Height = 1;
		bufDesc.DepthOrArraySize = 1;
		bufDesc.MipLevels = 1;
		bufDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufDesc.SampleDesc = { 1, 0 };
		bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		D3D12_RANGE readRange = { 0, 0 };			// We only write.
		void * pMapped = nullptr;

		if( FAILED(s_pDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufDesc,
													  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(resource.ReleaseAndGetAddressOf()))) ||
			FAILED(resource->Map(0, &readRange, &pMapped)) )
		{
			resource = nullptr;
			pData = nullptr;

			// A lost device is reported once by DX12Backend, not by everything that needs a buffer.

			if( !DX12Backend::checkDeviceLost() )
				Base::throwError(ErrorLevel::Error, ErrorCode::RenderFailure, "Failed to create a buffer for edgemaps, warpmaps and palettes.",
					nullptr, &DX12Backend::TYPEINFO, __func__, __FILE__, __LINE__);

			return false;
		}

		resource->SetName(L"Wax DX12BufferPool: buffer of its own");

		pData = static_cast<uint8_t*>(pMapped);
		return true;
	}

} // namespace wax
