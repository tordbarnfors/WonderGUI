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
#ifndef WAX_DX12BUFFERPOOL_DOT_H
#define WAX_DX12BUFFERPOOL_DOT_H
#pragma once

#include <wax_rangeallocator.h>

#include <wrl.h>
#include <d3d12.h>

#include <cstdint>
#include <vector>

namespace wax
{

	//____ DX12BufferPool ________________________________________________________
	/**
	 * @brief Small buffers the CPU writes and the GPU reads, out of blocks they share.
	 *
	 * A buffer of its own is a committed resource, which D3D12 makes at least 64 KB and takes a
	 * while to create. Edgemaps, Warpmaps and palettes are mostly far smaller than that, so they
	 * get a range of a block instead: an upload heap buffer, mapped for as long as it exists.
	 * D3D12 binds such buffers by address, so a range is used just as a buffer of its own would
	 * be. Blocks are 64 KB, the least D3D12 makes a buffer, and made when needed. What is too big
	 * for one gets a resource of its own.
	 *
	 * A block is released when the last buffer of it is given back, unless it is the only empty
	 * one: one empty block is kept, so that a buffer made and let go of over and over doesn't take
	 * making a block each time. An application with no buffers left has one block of 64 KB.
	 *
	 * The memory is write-combined: quick to write, very slow to read. So a buffer is only for
	 * what is written and never read back, and what is taken of the blocks is kept track of in
	 * ordinary memory, see RangeAllocator.
	 *
	 * A buffer given back may still be read by draws that are recorded or on their way through
	 * the GPU, so its range waits until the GPU has passed them before it is handed out again.
	 * That is told by one fence for all submissions of all DX12Backends, which they report with
	 * submitted(), as VulkanContext has it. Nothing has to hold on to a buffer for the draws that
	 * read it, or wait for the GPU when one is replaced.
	 *
	 * Set up and torn down by DX12Backend::setDevice(). Not thread safe, like the rest of Wax.
	 */

	class DX12BufferPool
	{
	public:

		struct Buffer
		{
			uint8_t *					pData = nullptr;		// Where to write. Never read from it.
			D3D12_GPU_VIRTUAL_ADDRESS	gpuAddress = 0;
			int							size = 0;				// As asked for.

			// For the pool.

			int							block = -1;				// The block it is a range of, -1 for a resource of its own.
			int							offset = 0;
			uint32_t					generation = 0;			// The device it is on, see isValid().
			Microsoft::WRL::ComPtr<ID3D12Resource>	resource;	// Its own, when too big for the blocks.
		};

		//.____ Device _______________________________________________________

		static void		setDevice( ID3D12Device * pDevice, ID3D12CommandQueue * pQueue );
		static void		exitDevice();							// Lets go of everything. Buffers handed out are no longer valid.

		//.____ Buffers ______________________________________________________

		// Gets a buffer of the given size, with whatever was in that memory. Returns false if
		// there is no device or it is lost, which isn't reported here, or when out of memory,
		// which is.

		static bool		alloc( int bytes, Buffer& buffer );

		// Gives a buffer back and clears it. Draws recorded so far may still read it.

		static void		free( Buffer& buffer );

		// False for a buffer never allocated, given back, or from a device that has been let go
		// of, whose memory is gone.

		static bool		isValid( const Buffer& buffer ) { return buffer.pData != nullptr && buffer.generation == s_generation; }

		//.____ Submissions __________________________________________________

		// Called by a DX12Backend right after it has had the command queue execute what it
		// recorded.

		static void		submitted();

		// Takes back the buffers the GPU is done with. Called by DX12Backends once a frame, and
		// when a buffer is asked for.

		static void		collect();

	private:

		struct Block
		{
			Block() : ranges(c_blockSize, c_alignment) {}

			Microsoft::WRL::ComPtr<ID3D12Resource>	resource;		// Null for a slot whose block has been released.
			uint8_t *								pData = nullptr;
			D3D12_GPU_VIRTUAL_ADDRESS				gpuAddress = 0;
			RangeAllocator							ranges;
		};

		// A buffer given back, until the GPU has passed the submission of the given value. The
		// value isn't known for one given back while a backend is recording.

		struct Waiting
		{
			UINT64		value;
			bool		bValueKnown;
			int			block;
			int			offset;
			int			size;
			Microsoft::WRL::ComPtr<ID3D12Resource>	resource;
		};

		static bool		_createResource( int bytes, Microsoft::WRL::ComPtr<ID3D12Resource>& resource, uint8_t *& pData );
		static int		_createBlock();
		static void		_release( Waiting& waiting );
		static void		_blockEmptied( int block );

		// Ranges start at and are whole numbers of this many bytes: a multiple of the float4 of
		// edgemaps and palettes (16), of the vertex of a Warpmap (24) and of a cache line (64), so
		// that any of them can start at any range and none shares a cache line with another.

		static const int	c_alignment = 192;

		// The least D3D12 makes a buffer, so nothing is lost to rounding, and little is held by a
		// block with a single buffer left in it.

		static const int	c_blockSize = 64 * 1024;
		static const int	c_blockCapacity = (c_blockSize / c_alignment) * c_alignment;	// Larger buffers get a resource of their own.

		static Microsoft::WRL::ComPtr<ID3D12Device>			s_pDevice;
		static Microsoft::WRL::ComPtr<ID3D12CommandQueue>	s_pQueue;
		static Microsoft::WRL::ComPtr<ID3D12Fence>			s_pFence;			// Counts the submissions of all backends.
		static UINT64										s_lastSubmitted;

		static std::vector<Block>							s_blocks;
		static std::vector<Waiting>							s_waiting;

		static uint32_t										s_generation;		// Stepped when a device is let go of.
	};

} // namespace wax
#endif // WAX_DX12BUFFERPOOL_DOT_H
