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

#include <wg_c_blob.h>
#include <wg_blob.h>

using namespace wg;

inline Blob* getPtr(wg_obj obj) {
	return static_cast<Blob*>(reinterpret_cast<Object*>(obj));
}


wg_obj wg_createBlob( int size )
{
  auto pBlob = Blob::create(size);
  pBlob->retain();
  return static_cast<Object*>(pBlob);
}

wg_obj 	wg_createBlobFromData( void * pData, int size, void(*destructor)() )
{
  auto pBlob = Blob::create(pData,size,destructor);
  pBlob->retain();
	return static_cast<Object*>(pBlob);
}

int wg_blobSize( wg_obj blob )
{
	return getPtr(blob)->size();
}

void *	wg_blobData( wg_obj blob )
{
	return getPtr(blob)->data();
}

