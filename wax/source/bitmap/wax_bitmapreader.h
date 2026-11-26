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
#ifndef WAX_BITMAPREADER_DOT_H
#define WAX_BITMAPREADER_DOT_H
#pragma once

#include <istream>
#include <vector>

#include <wax_bitmapfileheader.h>
#include <wax_bitmapfactory.h>
#include <wax_compression.h>

namespace wax
{


	class BitmapReader;
	typedef	StrongPtr<BitmapReader>BitmapReader_p;
	typedef	WeakPtr<BitmapReader>	BitmapReader_wp;


class BitmapReader : public Object
{
public:

	//.____ Blueprint _________________________________________________________

	struct Blueprint
	{
		BitmapFactory_p			factory;
		Finalizer_p					finalizer = nullptr;
	};

	//.____ Creation __________________________________________________________

	static BitmapReader_p	create() { return create( Blueprint() ); }
	static BitmapReader_p	create( const Blueprint& blueprint );

	//.____ Identification ____________________________________________________

	const TypeInfo&		typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;

	//.____ Misc ______________________________________________________________
	
	Bitmap_p		readBitmapFromStream( std::istream& stream );
	Bitmap_p		readBitmapFromStream(std::istream& stream, const Bitmap::Blueprint& extraFlags );

	Bitmap_p		readBitmapFromBlob(const Blob* pBlob);
	Bitmap_p		readBitmapFromBlob( const Blob * pBlob, const Bitmap::Blueprint& extraFlags );

	Bitmap_p		readBitmapFromMemory(const char* pData);
	Bitmap_p		readBitmapFromMemory( const char * pData, const Bitmap::Blueprint& extraFlags );
	
protected:

	template<class BP> BitmapReader(const BP& bp)
	{
		m_pFactory = bp.factory;

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	virtual ~BitmapReader() {}
	
	Bitmap::Blueprint	_blueprintFromHeader( const BitmapFileHeader * pHeader, const BitmapFileLayout& layout );
	Bitmap_p		_createBitmap( const BitmapFileHeader& header, const char * pData, const Bitmap::Blueprint& extraFlags );
	
	int				_addFlagsFromOtherBlueprint(Bitmap::Blueprint& dest, const Bitmap::Blueprint& extraFlags);

	Bitmap_p		_readBitmapFromStream(std::istream& stream, const Bitmap::Blueprint& extraFlags);
	Bitmap_p		_readQOIFromStream(std::istream& stream, const Bitmap::Blueprint& extraFlags);
	Bitmap_p		_readQOIFromMemory(const char * pData, const char * pDataEnd, const Bitmap::Blueprint& extraFlags);

	struct QOIHeader
	{
		int		width;
		int		height;
		int		channels;		// 3 = RGB, 4 = RGBA
		int		colorspace;		// 0 = sRGB with linear alpha, 1 = all channels linear
	};

	bool			_parseQOIHeader(const uint8_t * pHeader, QOIHeader& header);
	Bitmap_p		_createBitmapFromQOI(const QOIHeader& header, const uint8_t * pChunks, const uint8_t * pChunksEnd, const Bitmap::Blueprint& extraFlags);

	void			_copyUncompressedFromMemory(void * pDest, const void * pSource, int rowBytes, int pitch, int rows);


	BitmapFactory_p	m_pFactory;
};


//==============================================================================

} // namespace wax
#endif // WAX_BITMAPREADER_DOT_H



