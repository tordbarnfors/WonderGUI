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
#ifndef WAX_C_STREAMENCODER_DOT_H
#define WAX_C_STREAMENCODER_DOT_H
#pragma once

#include <wax_c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wax_streamEncoderBP __________________________________________________________

	typedef struct wax_streamEncoderBP_struct			// NOT BINARY EQUIVALENT!
	{
		int				bufferBytes;
		uint16_t		objectIdStart;
		wax_pixelFormat	pixelFormat;
		wax_sampleMethod	sampleMethod;
		wax_component	sink;
	} wax_streamEncoderBP;

	//

//	wax_obj			wax_createStreamEncoder(wax_component sinkForStream, int bufferBytes );
	WAX_EXPORT wax_obj			wax_createStreamEncoder( const wax_streamEncoderBP* blueprint);

	WAX_EXPORT void				wax_setEncoderStream(wax_obj encoder, wax_component sinkForStream);

	WAX_EXPORT void				wax_setEncoderDefaultPixelFormat(wax_obj encoder, wax_pixelFormat pixelFormat);
	WAX_EXPORT wax_pixelFormat 	wax_getEncoderDefaultPixelFormat(wax_obj encoder);

	WAX_EXPORT void				wax_setEncoderDefaultSampleMethod(wax_obj encoder, wax_sampleMethod sampleMethod);
	WAX_EXPORT wax_sampleMethod	wax_getEncoderDefaultSampleMethod(wax_obj encoder);

	WAX_EXPORT void				wax_flushEncoder(wax_obj encoder);

#ifdef __cplusplus
}
#endif

#endif
