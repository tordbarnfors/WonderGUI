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
#ifndef WG_SURFACEWRITER_DOT_H
#define WG_SURFACEWRITER_DOT_H
#pragma once

#include <ostream>

#include <wg_surfacefileheader.h>
#include <wg_surfacefactory.h>
#include <wg_compressor.h>

namespace wg
{


	class SurfaceWriter;
	typedef	StrongPtr<SurfaceWriter>SurfaceWriter_p;
	typedef	WeakPtr<SurfaceWriter>	SurfaceWriter_wp;




class SurfaceWriter : public Object
{
public:

	struct SaveInfo
	{
		bool identity		= true;
		bool sampleMethod 	= false;
		bool scale			= true;
		bool buffered		= false;
		bool canvas			= false;
		bool dynamic		= false;
		bool tiling			= true;
		bool mipmap			= false;
	};
	
	struct Blueprint
	{
		Finalizer_p			finalizer = nullptr;
		Compressor_p		pixelCompressor;
		Compressor_p		paletteCompressor;
		Compressor_p		extrasCompressor;
		SaveInfo			saveInfo;
	};

	//.____ Creation __________________________________________

	static SurfaceWriter_p	create() { return create( Blueprint() ); }
	static SurfaceWriter_p	create( const Blueprint& blueprint );

	//.____ Identification _________________________________________________

	const TypeInfo&	typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;
	
	void			writeSurfaceToStream( std::ostream& stream, Surface * pSurface, int extraDataSize = 0, char * pExtraData = nullptr );
	Blob_p			writeSurfaceToBlob( Surface * pSurface, int extraDataSize = 0, char * pExtraData = nullptr );

protected:

	template<class BP>
	SurfaceWriter(const BP& bp)
	{
		m_saveInfo = bp.saveInfo;
		m_pPixelCompressor = bp.pixelCompressor;
		m_pPaletteCompressor = bp.paletteCompressor;
		m_pExtrasCompressor = bp.extrasCompressor;

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	virtual ~SurfaceWriter() {}
	
	int				_prepareHeader( SurfaceFileHeader * pHeader, Surface * pSurface, bool bExtrasData );
	bool			_setPixelDataInfo(	SurfaceFileHeader* pHeader, int bytesOfCompressedPixels, int decompressMargin = 0 );
	bool			_setPaletteDataInfo(SurfaceFileHeader* pHeader, int bytesOfCompressedPalette, int decompressMargin = 0 );
	bool			_setExtraDataInfo(SurfaceFileHeader* pHeader, int bytesOfCompressedExtraData, int decompressMargin = 0);

	int				_safePixelBufferSize(Surface* pSurface);
	int				_safePaletteBufferSize(Surface* pSurface);
	int				_safeExtrasBufferSize(void * pBegin, void * pEnd);

	int				_encodePixelData(void* pDest, Surface* pSurface);
	int				_encodePaletteData(void* pDest, Surface* pSurface);
	int				_encodeExtrasData(void* pDest, void * pBegin, void * pEnd);


	SaveInfo			m_saveInfo;

	Compressor_p		m_pPixelCompressor;
	Compressor_p		m_pPaletteCompressor;
	Compressor_p		m_pExtrasCompressor;

};


//==============================================================================

} // namespace wg
#endif // WG_SURFACEREADER_DOT_H

