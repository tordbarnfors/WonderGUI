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
#ifndef	WG_MESHCOMPRESSION_DOT_H
#define	WG_MESHCOMPRESSION_DOT_H
#pragma once

#include <wg_compression.h>
#include <wg_gfxtypes.h>

namespace wg
{
	class MeshCompressor;
	typedef StrongPtr<MeshCompressor>	MeshCompressor_p;
	typedef WeakPtr<MeshCompressor>		MeshCompressor_wp;

	class MeshDecompressor;
	typedef StrongPtr<MeshDecompressor>	MeshDecompressor_p;
	typedef WeakPtr<MeshDecompressor>	MeshDecompressor_wp;

	//____ MeshCompressor ________________________________________________________
	/**
	 * @brief Compresses the vertices and triangles of a Warpmap, or any sequences of 32-bit values that change smoothly.
	 *
	 * The data is a number of sequences of the same length, one after the other, such as all x of
	 * the vertices of a mesh, then all y and so on, or all first corners of its triangles, then
	 * all second and third. Each sequence is predicted from its own values lag steps back, either
	 * as they are (order 1) or continuing the change from the ones before (order 2), with lag and
	 * order chosen for each sequence by trying lags 1 to 32 on 8 short windows spread over it, and
	 * only the difference is stored. A mesh that follows a shape column by column has values that change the same way
	 * from one column to the next, or one vertex to the next, so most differences are small, and
	 * those of the corners of triangles mostly 0.
	 *
	 * Differences are zigzag coded (0, -1, 1, -2... as 0, 1, 2, 3...) and a run of zeros is a 0 code
	 * followed by its length - 1, other differences their zigzag code. Codes are stored in 1 to 5
	 * bytes, with the length in the leading bits of the first byte as in UTF-8, but with all 8 bits
	 * used in the bytes that follow:
	 *
	 *		0xxxxxxx								7 bits
	 *		10xxxxxx xxxxxxxx						14 bits
	 *		110xxxxx xxxxxxxx xxxxxxxx				21 bits
	 *		1110xxxx xxxxxxxx xxxxxxxx xxxxxxxx		28 bits
	 *		11110000 xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx	32 bits
	 *
	 * The data starts with its mode, then the number of sequences and their length as codes, and
	 * each sequence with a code of (lag - 1) * 2 + order - 1. Arithmetic wraps around at 32 bits,
	 * so any values are restored exactly. Data that doesn't compress, or isn't whole sequences of
	 * 32-bit values, is stored as it is.
	 *
	 * Set the number of sequences before compressing. It's stored with the data.
	 *
	 * The compressed data can be decompressed in place, from the end of a buffer of
	 * maxCompressedSize() bytes to its start, as GfxStream does.
	 */

	class MeshCompressor : public Compressor
	{
	public:

		//.____ Creation __________________________________________________________

		static MeshCompressor_p		create();

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		uint32_t idToken() const override;
		const static uint32_t	ID_TOKEN;

		//.____ Misc ____________________________________________________

		bool	setSequences( int sequences );
		int		sequences() const { return m_sequences; }

		int		maxCompressedSize( int uncompressedSize ) override;
		int		compress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;

	protected:
		MeshCompressor() {};
		virtual ~MeshCompressor() {};

		int		m_sequences = 1;
	};

	//____ MeshDecompressor ______________________________________________________

	class MeshDecompressor : public Decompressor
	{
	public:

		//.____ Creation __________________________________________________________

		static MeshDecompressor_p		create();

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		uint32_t idToken() const override;
		const static uint32_t	ID_TOKEN;

		//.____ Misc ____________________________________________________

		int		decompress( void * pDest, const void * pSrcBegin, const void * pSrcEnd ) override;

	protected:
		MeshDecompressor() {};
		virtual ~MeshDecompressor() {};
	};
}

#endif //WG_MESHCOMPRESSION_DOT_H
