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
#ifndef	WAX_NODECOMPRESSION_DOT_H
#define	WAX_NODECOMPRESSION_DOT_H
#pragma once

#include <wax_compression.h>
#include <wax_types.h>

namespace wax
{
	class NodeCompressor;
	typedef StrongPtr<NodeCompressor>	NodeCompressor_p;
	typedef WeakPtr<NodeCompressor>		NodeCompressor_wp;

	class NodeDecompressor;
	typedef StrongPtr<NodeDecompressor>	NodeDecompressor_p;
	typedef WeakPtr<NodeDecompressor>	NodeDecompressor_wp;

	//____ NodeCompressor ________________________________________________________
	/**
	 * @brief Compresses the nodes of a Warpmap, or any grid of pairs of 32-bit values that change smoothly.
	 *
	 * Each value is predicted from its neighbours already compressed, left + above - above left
	 * (only left on the first row, only above in the first column), and only the difference is
	 * stored. Differences are zigzag coded (0, -1, 1, -2... as 0, 1, 2, 3...) and stored in 1 to 5
	 * bytes, with the length in the leading bits of the first byte as in UTF-8, but with all 8 bits
	 * used in the bytes that follow:
	 *
	 *		0xxxxxxx								7 bits
	 *		10xxxxxx xxxxxxxx						14 bits
	 *		110xxxxx xxxxxxxx xxxxxxxx				21 bits
	 *		1110xxxx xxxxxxxx xxxxxxxx xxxxxxxx		28 bits
	 *		11110000 xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx	32 bits
	 *
	 * Arithmetic wraps around at 32 bits, so any values are restored exactly. Data that doesn't
	 * compress, or isn't whole rows of pairs, is stored as it is.
	 *
	 * Set the row length (in pairs) before compressing. It's stored with the data.
	 *
	 * The compressed data can be decompressed in place, from the end of a buffer of
	 * maxCompressedSize() bytes to its start, as GfxStream does.
	 */

	class NodeCompressor : public Compressor
	{
	public:

		//.____ Creation __________________________________________________________

		static NodeCompressor_p		create();

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		uint32_t idToken() const override;

		const static uint32_t	ID_TOKEN;

		//.____ Misc ____________________________________________________

		bool	setRowLength( int pairs );
		int		rowLength() const { return m_rowLength; }

		int		maxCompressedSize( int uncompressedSize ) override;

		int		compress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;

	protected:
		NodeCompressor() {};
		virtual ~NodeCompressor() {};

		int		m_rowLength = 1;
	};

	//____ NodeDecompressor ______________________________________________________

	class NodeDecompressor : public Decompressor
	{
	public:

		//.____ Creation __________________________________________________________

		static NodeDecompressor_p		create();

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		uint32_t idToken() const override;

		const static uint32_t	ID_TOKEN;

		//.____ Misc ____________________________________________________

		int		decompress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;

	protected:
		NodeDecompressor() {};
		virtual ~NodeDecompressor() {};
	};

}

#endif //WAX_NODECOMPRESSION_DOT_H
