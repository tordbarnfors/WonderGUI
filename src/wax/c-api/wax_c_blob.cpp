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
#include <wax_c_blob.h>
#include <wax_blob.h>

using namespace wax;

inline Blob* getPtr(wax_obj obj) {
	return static_cast<Blob*>(reinterpret_cast<Object*>(obj));
}


wax_obj wax_createBlob( int size )
{
  auto pBlob = Blob::create(size);
  pBlob->retain();
  return static_cast<Object*>(pBlob);
}

wax_obj 	wax_createBlobFromData( void * pData, int size, void(*destructor)() )
{
  auto pBlob = Blob::create(pData,size,destructor);
  pBlob->retain();
	return static_cast<Object*>(pBlob);
}

int wax_blobSize( wax_obj blob )
{
	return getPtr(blob)->size();
}

void *	wax_blobData( wax_obj blob )
{
	return getPtr(blob)->data();
}

