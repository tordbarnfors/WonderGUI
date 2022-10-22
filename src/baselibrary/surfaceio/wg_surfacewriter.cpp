/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#include <wg_surfacewriter.h>
#include <wg_base.h>

namespace wg
{
	const TypeInfo SurfaceWriter::TYPEINFO = { "SurfaceWriter", &Object::TYPEINFO };


	//.____ create() _____________________________________________________________

	SurfaceWriter_p SurfaceWriter::create( const Blueprint& blueprint )
	{
		return SurfaceWriter_p( new SurfaceWriter(blueprint) );
		
	}
	
	//____ constructor ___________________________________________________________

	SurfaceWriter::SurfaceWriter( const Blueprint& bp )
	{
		m_saveInfo = bp.saveInfo;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& SurfaceWriter::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ writeSurfaceToStream() _____________________________________________

	void SurfaceWriter::writeSurfaceToStream( std::ostream& stream, Surface * pSurface, int extraDataSize, char * pExtraData )
	{
		//TODO: Should clear X-channel on BGRX surfaces? Make it optional?
		
		// Fill in header
		
		SurfaceFileHeader	header;
		int headerBytes = _generateHeader(&header, pSurface, extraDataSize);
		
		// Write header

		stream.write((char*) &header, headerBytes);

		// Write clut
		
		if( pSurface->clut() )
			stream.write( (char*) pSurface->clut(), 256*sizeof(Color8) );
		
		// Write pixels
		
		int lineBytes = header.width * pSurface->pixelBytes();
		auto pixbuf = pSurface->allocPixelBuffer();
		pSurface->pushPixels(pixbuf);
		
		if( pixbuf.pitch > lineBytes )
		{
			// Pitch is involved, we need to write line by line
			
			char * pPixels = (char *) pixbuf.pPixels;
			
			for( int y = 0 ; y < header.height ; y++ )
			{
				stream.write( pPixels, lineBytes );
				pPixels += pixbuf.pitch;
			}
		}
		else
		{
			stream.write( (char*) pixbuf.pPixels, lineBytes * header.height );
		}

		pSurface->freePixelBuffer(pixbuf);

		// write extra data
		
		if( extraDataSize > 0 )
			stream.write( pExtraData, extraDataSize );
	}

	//____ _writeSurfaceToBlob() _________________________________________________

	Blob_p SurfaceWriter::writeSurfaceToBlob( Surface * pSurface, int extraDataSize, char * pExtraData )
	{
		// Fill in header
		
		SurfaceFileHeader	header;
		int headerBytes = _generateHeader(&header, pSurface, extraDataSize);
	
		// Calculate size needed for blob
		
		int size = headerBytes + header.paletteBytes + header.pixelBytes + header.extraDataBytes;
		
		Blob_p pBlob = Blob::create(size);
		
		char * pWrite = (char*) pBlob->data();
		
		// Write header

		std::memcpy(pWrite, &header, headerBytes);
		pWrite += headerBytes;
		
		// Write clut
		
		if( pSurface->clut() )
		{
			std::memcpy(pWrite, pSurface->clut(), 256*sizeof(Color8) );
			pWrite += 256*sizeof(Color8);
		}
		
		// Write pixels
		
		int lineBytes = header.width * pSurface->pixelBytes();
		auto pixbuf = pSurface->allocPixelBuffer();
		pSurface->pushPixels(pixbuf);
		
		if( pixbuf.pitch > lineBytes )
		{
			// Pitch is involved, we need to write line by line
			
			char * pPixels = (char *) pixbuf.pPixels;
			
			for( int y = 0 ; y < header.height ; y++ )
			{
				std::memcpy(pWrite, pPixels, lineBytes );
				pWrite += lineBytes;
				pPixels += pixbuf.pitch;
			}
		}
		else
		{
			std::memcpy(pWrite, pixbuf.pPixels, lineBytes * header.height );
			pWrite += lineBytes * header.height;
		}

		pSurface->freePixelBuffer(pixbuf);

		// write extra data
		
		if( extraDataSize > 0 )
		{
			std::memcpy( pWrite, pExtraData, extraDataSize );
			pWrite += extraDataSize;
		}

		return pBlob;
	}


	//____ _generateHeader() _____________________________________________________

	int SurfaceWriter::_generateHeader( SurfaceFileHeader * pHeader, Surface * pSurface, int extraDataSize )
	{
		auto bp = pSurface->blueprint();
		
		auto sz = pSurface->pixelSize();
		
		pHeader->width = bp.size.w;
		pHeader->height = bp.size.h;
		pHeader->format = bp.format;

		if( m_saveInfo.sampleMethod )
			pHeader->sampleMethod = bp.sampleMethod;
		
		if( m_saveInfo.buffered )
			pHeader->buffered = bp.buffered;
		
		if( m_saveInfo.canvas )
			pHeader->canvas = bp.canvas;
		
		if( m_saveInfo.dynamic )
			pHeader->dynamic = bp.dynamic;
		
		if( m_saveInfo.tiling )
			pHeader->tiling = bp.tiling;
		
		if( m_saveInfo.mipmap )
			pHeader->mipmap = bp.mipmap;

		if( m_saveInfo.scale )
			pHeader->scale = bp.scale;
		
		if( m_saveInfo.identity )
			pHeader->identity = bp.identity;
		
		if( bp.clut )
		{
			pHeader->paletteBytes = 256*sizeof(Color8);
			pHeader->paletteEntries = 256;
		}
		

		int headerBytes;
		if( extraDataSize != 0 )
			headerBytes = 88;
		else if( bp.clut )
			headerBytes = 80;
		else if( (m_saveInfo.scale && bp.scale != 64) || (m_saveInfo.identity && bp.identity != 0) )
			headerBytes = 40;
		else
			headerBytes = 32;
		
		pHeader->headerBytes = headerBytes;
		
		int lineBytes = bp.size.w * pSurface->pixelBytes();

		pHeader->pixelBytes = lineBytes * bp.size.h;
		pHeader->extraDataBytes = extraDataSize;

		return headerBytes;
	}



} // namespace wg









