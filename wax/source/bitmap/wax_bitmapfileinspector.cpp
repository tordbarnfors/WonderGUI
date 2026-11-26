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
#include <wax_bitmapfileinspector.h>
#include <cstring>

namespace wax
{


const TypeInfo BitmapFileInspector::TYPEINFO = { "BitmapFileInspector", &Object::TYPEINFO };


//.____ create() _____________________________________________________________

BitmapFileInspector_p BitmapFileInspector::create( const void * pBitmapFileInMemory )
{
	return BitmapFileInspector_p( new BitmapFileInspector(pBitmapFileInMemory) );
}

//____ constructor ___________________________________________________________

BitmapFileInspector::BitmapFileInspector( const void * pBitmapFileInMemory )
{
	// Read the header

	int headerSize = * (const int16_t*)&((const char*)pBitmapFileInMemory)[6];
	std::memcpy( &m_header, pBitmapFileInMemory, headerSize);
}

//____ typeInfo() _________________________________________________________

const TypeInfo& BitmapFileInspector::typeInfo(void) const
{
	return TYPEINFO;
}

//____ bitmapBlueprint() ____________________________________________________________

Bitmap::Blueprint BitmapFileInspector::bitmapBlueprint()
{
	BitmapFileLayout layout = bitmapFileLayout(m_header);

	return WAXBP(Bitmap,
				_.size 			= {m_header.width, m_header.height},
				_.scale 		= m_header.scale,
				_.format 		= layout.format,
				_.colorSpace	= layout.colorSpace,
				_.bigEndian		= layout.bigEndian,
				_.canvas 		= m_header.canvas,
				_.dynamic 		= m_header.dynamic,
				_.mipmap 		= m_header.mipmap,
				_.identity		= m_header.identity,
				_.tiling		= m_header.tiling,
				_.sampleMethod 	= m_header.sampleMethod );
}

//____ pixelDataOffset() ______________________________________________________

int BitmapFileInspector::pixelDataOffset()
{
	return m_header.headerBytes + m_header.paletteBytes;
}

//____ pixelDataBytes() _______________________________________________________

int BitmapFileInspector::pixelDataBytes()
{
	return m_header.pixelBytes - m_header.pixelDataPadding;
}

//____ paletteDataOffset() ____________________________________________________

int BitmapFileInspector::paletteDataOffset()
{
	return m_header.headerBytes;
}

//____ paletteDataBytes() _____________________________________________________

int BitmapFileInspector::paletteDataBytes()
{
	return m_header.paletteBytes - m_header.paletteDataPadding;
}

//____ extraDataOffset() ______________________________________________________

int BitmapFileInspector::extraDataOffset()
{
	return m_header.headerBytes + m_header.paletteBytes + m_header.pixelBytes;
}

//____ extraDataBytes() _______________________________________________________

int BitmapFileInspector::extraDataBytes()
{
	return m_header.extraDataBytes;
}

//____ isPixelDataCompressed() ________________________________________________

bool BitmapFileInspector::isPixelDataCompressed()
{
	return ( m_header.pixelCompression != Util::makeEndianSpecificToken( 'N','O','N','E' ));
}

//____ isPaletteDataCompressed() ______________________________________________

bool BitmapFileInspector::isPaletteDataCompressed()
{
	return ( m_header.paletteCompression != Util::makeEndianSpecificToken( 'N','O','N','E' ));
}

//____ isExtraDataCompressed() ________________________________________________

bool BitmapFileInspector::isExtraDataCompressed()
{
	return ( m_header.extraDataCompression != Util::makeEndianSpecificToken( 'N','O','N','E' ));
}


} // namespace wax



