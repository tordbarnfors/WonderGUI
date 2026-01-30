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
#ifndef WG_SURFACEREADER_DOT_H
#define WG_SURFACEREADER_DOT_H
#pragma once

#include <istream>
#include <vector>

#include <wg_surfacefileheader.h>
#include <wg_surfacefactory.h>
#include <wg_compressor.h>

namespace wg
{


	class SurfaceReader;
	typedef	StrongPtr<SurfaceReader>SurfaceReader_p;
	typedef	WeakPtr<SurfaceReader>	SurfaceReader_wp;


class SurfaceReader : public Object
{
public:

	//.____ Blueprint _________________________________________________________

	struct Blueprint
	{
		std::vector<Compressor_p>	compressors;
		bool						autoCompressors = true;		// Create and use known compressors on demand.
		SurfaceFactory_p			factory;
		Finalizer_p					finalizer = nullptr;
	};

	//.____ Creation __________________________________________________________

	static SurfaceReader_p	create() { return create( Blueprint() ); }
	static SurfaceReader_p	create( const Blueprint& blueprint );

	//.____ Identification ____________________________________________________

	const TypeInfo&		typeInfo(void) const override;
	const static TypeInfo	TYPEINFO;

	//.____ Misc ______________________________________________________________
	
	Surface_p		readSurfaceFromStream( std::istream& stream );
	Surface_p		readSurfaceFromStream(std::istream& stream, const Surface::Blueprint& extraFlags );

	Surface_p		readSurfaceFromBlob(const Blob* pBlob);
	Surface_p		readSurfaceFromBlob( const Blob * pBlob, const Surface::Blueprint& extraFlags );

	Surface_p		readSurfaceFromMemory(const char* pData);
	Surface_p		readSurfaceFromMemory( const char * pData, const Surface::Blueprint& extraFlags );
	
protected:

	template<class BP> SurfaceReader(const BP& bp)
	{
		m_pFactory = bp.factory;
		m_bAutoCompressors = bp.autoCompressors;

		for( auto& p : bp.compressors )
			m_compressors.push_back(p);

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	virtual ~SurfaceReader() {}
	
	Surface::Blueprint	_blueprintFromHeader( const SurfaceFileHeader * pHeader );
	
	int				_addFlagsFromOtherBlueprint(Surface::Blueprint& dest, const Surface::Blueprint& extraFlags);

	Surface_p		_readSurfFromStream(std::istream& stream, const Surface::Blueprint& extraFlags);
	Surface_p		_readQOIFromStream(std::istream& stream, const Surface::Blueprint& extraFlags);

	void			_copyUncompressedFromMemory(void * pDest, const void * pSource, int rowBytes, int pitch, int rows);

	Compressor *	_findCompressor( uint32_t idToken );


	SurfaceFactory_p	m_pFactory;

	std::vector<Compressor_p>	m_compressors;

	bool				m_bAutoCompressors = true;
};


//==============================================================================

} // namespace wg
#endif // WG_SURFACEREADER_DOT_H



