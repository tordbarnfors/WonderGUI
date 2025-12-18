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
#include <wg_c_streamwriter.h>
#include <wg_streamwriter.h>

using namespace wg;

inline StreamWriter* getPtr(wg_obj obj) {
	return static_cast<StreamWriter*>(reinterpret_cast<Object*>(obj));
}

wg_obj wg_createStreamWriter(wg_writeStream_func dispatcher)
{
	auto p = StreamWriter::create(dispatcher);
	p->retain();
	return p.rawPtr();
}

wg_component wg_getStreamWriterInput(wg_obj streamWriter)
{
	auto p = getPtr(streamWriter);
	return { static_cast<Object*>(p), &p->input };
}
