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

#ifndef WG_C_GEARTYPES_DOT_H
#define WG_C_GEARTYPES_DOT_H
#pragma once

#include <stdint.h>

#include <wg_export.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* wg_obj;


typedef	void(*wg_finalizer_func)(wg_obj);

//____ wg_component __________________________________________________________

typedef struct wg_component_struct
{
	wg_obj object;
	void * pInterface;
} wg_component;


//____ wg_typeInfo ___________________________________________________________

typedef struct wg_typeInfo_struct
{
	const char* className;
	const struct wg_typeInfo_struct* pSuperClass;
} wg_typeInfo;

//____ wg_errorLevel ________________________________________________

typedef enum
{
	WG_WARNING = 0,
	WG_SILENT_ERROR,
	WG_ERROR,
	WG_CRITICAL
} wg_errorLevel;

//____ wg_errorCode ____________________________________________________

typedef enum
{
	WG_ERROR_OUT_OF_RANGE = 0,
	WG_ERROR_INVALID_ITERATOR,
	WG_ERROR_INVALID_PARAM,
	WG_ERROR_FAILED_PREREQUISITE,
	WG_ERROR_OPENGL,						// OPENGL HAS ISSUED A GLERROR
	WG_ERROR_INTERNAL,
	WG_ERROR_SYSTEM_INTEGRITY,
	WG_ERROR_ILLEGAL_CALL,
	WG_ERROR_RESOURCE_EXHAUSTED,			// A LIMITED INTERNAL RESOURCE HAS BEEN EXHAUSTED
	WG_ERROR_PERFORMANCE,					// ONLY USED IN COMBO WITH WG_WARNING.
	WG_ERROR_OTHER
} wg_errorCode;

//____ wg_errorInfo _____________________________________________________

typedef struct wg_errorInfo_struct
{
	wg_errorLevel	severity;
	wg_errorCode		code;
	const char * 		message;
	const void *		object;
	const char *		classname;
	const char *		function;
	const char *		file;
	int					line;
} wg_errorInfo;


#ifdef __cplusplus
}
#endif

#endif
