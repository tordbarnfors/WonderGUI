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

#ifndef WG_C_STREAMENCODER_DOT_H
#define WG_C_STREAMENCODER_DOT_H
#pragma once

#include <wg_c_gfxtypes.h>

#ifdef __cplusplus
extern "C" {
#endif

	//____ wg_streamEncoderBP __________________________________________________________

	typedef struct wg_streamEncoderBP_struct			// NOT BINARY EQUIVALENT!
	{
		int				bufferBytes;
		uint16_t		objectIdStart;
		wg_pixelFormat	pixelFormat;
		wg_sampleMethod	sampleMethod;
		wg_component	sink;
	} wg_streamEncoderBP;

	//

//	wg_obj			wg_createStreamEncoder(wg_component sinkForStream, int bufferBytes );
	wg_obj			wg_createStreamEncoder( const wg_streamEncoderBP* blueprint);

	void			wg_setEncoderStream(wg_obj encoder, wg_component sinkForStream);

	void			wg_setEncoderDefaultPixelFormat(wg_obj encoder, wg_pixelFormat pixelFormat);
	wg_pixelFormat 	wg_getEncoderDefaultPixelFormat(wg_obj encoder);

	void			wg_setEncoderDefaultSampleMethod(wg_obj encoder, wg_sampleMethod sampleMethod);
	wg_sampleMethod wg_getEncoderDefaultSampleMethod(wg_obj encoder);

	void			wg_flushEncoder();

#ifdef __cplusplus
}
#endif

#endif
