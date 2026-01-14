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
#include <wg_surfacefileinspector.h>
#include <cstring>

namespace wg
{


const TypeInfo SurfaceFileInspector::TYPEINFO = { "SurfaceFileInspector", &Object::TYPEINFO };


//.____ create() _____________________________________________________________

SurfaceFileInspector_p SurfaceFileInspector::create( const void * pSurfaceFileInMemory )
{
	return SurfaceFileInspector_p( new SurfaceFileInspector(pSurfaceFileInMemory) );
}

//____ constructor ___________________________________________________________

SurfaceFileInspector::SurfaceFileInspector( const void * pSurfaceFileInMemory )
{
	// Read the header

	int headerSize = * (const int16_t*)&((const char*)pSurfaceFileInMemory)[6];
	std::memcpy( &m_header, pSurfaceFileInMemory, headerSize);
}

//____ typeInfo() _________________________________________________________

const TypeInfo& SurfaceFileInspector::typeInfo(void) const
{
	return TYPEINFO;
}

//____ surfaceBlueprint() ____________________________________________________________

Surface::Blueprint SurfaceFileInspector::surfaceBlueprint()
{
	return WGBP(Surface,
				_.size 			= {m_header.width, m_header.height},
				_.scale 		= m_header.scale,
				_.format 		= m_header.format,
				_.buffered 		= m_header.buffered,
				_.canvas 		= m_header.canvas,
				_.dynamic 		= m_header.dynamic,
				_.mipmap 		= m_header.mipmap,
				_.identity		= m_header.identity,
				_.tiling		= m_header.tiling,
				_.sampleMethod 	= m_header.sampleMethod );
}

//____ pixelDataOffset() ______________________________________________________

int SurfaceFileInspector::pixelDataOffset()
{
	return m_header.headerBytes + m_header.paletteBytes;
}

//____ pixelDataBytes() _______________________________________________________

int SurfaceFileInspector::pixelDataBytes()
{
	return m_header.pixelBytes - m_header.pixelDataPadding;
}

//____ paletteDataOffset() ____________________________________________________

int SurfaceFileInspector::paletteDataOffset()
{
	return m_header.headerBytes;
}

//____ paletteDataBytes() _____________________________________________________

int SurfaceFileInspector::paletteDataBytes()
{
	return m_header.paletteBytes - m_header.paletteDataPadding;
}

//____ extraDataOffset() ______________________________________________________

int SurfaceFileInspector::extraDataOffset()
{
	return m_header.headerBytes + m_header.paletteBytes + m_header.pixelBytes;
}

//____ extraDataBytes() _______________________________________________________

int SurfaceFileInspector::extraDataBytes()
{
	return m_header.extraDataBytes;
}

//____ isPixelDataCompressed() ________________________________________________

bool SurfaceFileInspector::isPixelDataCompressed()
{
	return !( m_header.pixelCompression[0] == 'N' && m_header.pixelCompression[1] == 'O' &&
			 m_header.pixelCompression[2] == 'N' && m_header.pixelCompression[3] == 'E');
}

//____ isPaletteDataCompressed() ______________________________________________

bool SurfaceFileInspector::isPaletteDataCompressed()
{
	 return !( m_header.paletteCompression[0] == 'N' && m_header.paletteCompression[1] == 'O' &&
			  m_header.paletteCompression[2] == 'N' && m_header.paletteCompression[3] == 'E');
}

//____ isExtraDataCompressed() ________________________________________________

bool SurfaceFileInspector::isExtraDataCompressed()
{
	return !( m_header.extraDataCompression[0] == 'N' && m_header.extraDataCompression[1] == 'O' &&
			 m_header.extraDataCompression[2] == 'N' && m_header.extraDataCompression[3] == 'E');
}


} // namespace wg



