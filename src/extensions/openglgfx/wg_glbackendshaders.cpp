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
#ifndef WG_GLSHADERS_DOT_H
#define WG_GLSHADERS_DOT_H

#include <wg_glbackend.h>  

namespace wg {


const char GlBackend::fillVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer colorBufferId;					   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 2) in int colorOfs;                     "
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;            "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   fragColor = texelFetch(colorBufferId, colorOfs);	   "
"}                                                         ";

const char GlBackend::fillGradientVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	vec2   xcanvasSize;"
"	int    xcanvasYOfs;"
"	int    xcanvasYMul;"
"   vec4   xflatTint;"
"   ivec2  xtintRectPos;"
"   ivec2  xtintRectSize;"
"   vec4   xtopLeftTint;"
"   vec4   xtopRightTint;"
"   vec4   xbottomRightTint;"
"   vec4   xbottomLeftTint;"
"};"


"uniform samplerBuffer extrasId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;					"
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "

"   vec4 tintRect = texelFetch(extrasId, tintInfoOfs+1);	"
"   vec4 topLeftTint = texelFetch(extrasId, tintInfoOfs+2);	"
"   vec4 topRightTint = texelFetch(extrasId, tintInfoOfs+3);	"
"   vec4 bottomRightTint = texelFetch(extrasId, tintInfoOfs+4);	"
"   vec4 bottomLeftTint = texelFetch(extrasId, tintInfoOfs+5);	"

"   vec2 tintRectPos = tintRect.xy;	"
"   vec2 tintRectSize = tintRect.zw;	"


"	vec2 tintOfs = (pos - tintRectPos) / vec2(tintRectSize); "
"   vec4 lineStartTint = topLeftTint + (bottomLeftTint - topLeftTint) * tintOfs.y;"
"   vec4 lineEndTint = topRightTint + (bottomRightTint - topRightTint) * tintOfs.y;"
"   vec4 gradientTint = lineStartTint + (lineEndTint - lineStartTint) * tintOfs.x;"

"   fragColor = texelFetch(extrasId, tintInfoOfs) * gradientTint * texelFetch(extrasId, extrasOfs);		   "
"}                                                         ";



const char GlBackend::fillFragmentShader[] =

"#version 330 core\n"
"out vec4 outColor;                     "
"in vec4 fragColor;                         "
"void main()                            "
"{                                      "
"   outColor = fragColor;                   "
"}                                      ";

const char GlBackend::fillFragmentShader_A8[] =

"#version 330 core\n"
"out vec4 outColor;                     "
"in vec4 fragColor;                         "
"void main()                            "
"{                                      "
"   outColor.r = fragColor.a;                   "
"}                                      ";



const char GlBackend::blitVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	vec2   xcanvasSize;"
"	int    xcanvasYOfs;"
"	int    xcanvasYMul;"
"   vec4   xflatTint;"
"   ivec2  xtintRectPos;"
"   ivec2  xtintRectSize;"
"   vec4   xtopLeftTint;"
"   vec4   xtopRightTint;"
"   vec4   xbottomRightTint;"
"   vec4   xbottomLeftTint;"
"};"


"uniform samplerBuffer extrasId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec2 texUV;                                           "
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasId, extrasOfs);	   "
"   vec4 transform = texelFetch(extrasId, extrasOfs+1);	   "
"   vec2 src = srcDst.xy;                                  "
"   vec2 dst = srcDst.zw;                                  "
"   texUV.x = (src.x + 0.0001f + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z) / texSize.x; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   texUV.y = (src.y + 0.0001f + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w) / texSize.y; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   fragColor = texelFetch(extrasId, tintInfoOfs);		   "
"}                                                         ";

const char GlBackend::blitGradientVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	vec2   xcanvasSize;"
"	int    xcanvasYOfs;"
"	int    xcanvasYMul;"
"   vec4   xflatTint;"
"   ivec2  xtintRectPos;"
"   ivec2  xtintRectSize;"
"   vec4   xtopLeftTint;"
"   vec4   xtopRightTint;"
"   vec4   xbottomRightTint;"
"   vec4   xbottomLeftTint;"
"};"


"uniform samplerBuffer extrasId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;				"
"out vec2 texUV;                                           "
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasId, extrasOfs);	   "
"   vec4 transform = texelFetch(extrasId, extrasOfs+1);	   "
"   vec2 src = srcDst.xy;                                  "
"   vec2 dst = srcDst.zw;                                  "
"   texUV.x = (src.x + 0.0001f + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z) / texSize.x; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   texUV.y = (src.y + 0.0001f + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w) / texSize.y; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.

"   vec4 tintRect = texelFetch(extrasId, tintInfoOfs+1);	"
"   vec4 topLeftTint = texelFetch(extrasId, tintInfoOfs+2);	"
"   vec4 topRightTint = texelFetch(extrasId, tintInfoOfs+3);	"
"   vec4 bottomRightTint = texelFetch(extrasId, tintInfoOfs+4);	"
"   vec4 bottomLeftTint = texelFetch(extrasId, tintInfoOfs+5);	"

"   vec2 tintRectPos = tintRect.xy;	"
"   vec2 tintRectSize = tintRect.zw;	"


"	vec2 tintOfs = (pos - tintRectPos) / vec2(tintRectSize); "
"   vec4 lineStartTint = topLeftTint + (bottomLeftTint - topLeftTint) * tintOfs.y;"
"   vec4 lineEndTint = topRightTint + (bottomRightTint - topRightTint) * tintOfs.y;"
"   vec4 gradientTint = lineStartTint + (lineEndTint - lineStartTint) * tintOfs.x;"

"   fragColor = texelFetch(extrasId, tintInfoOfs) * gradientTint;		   "
"}                                                         ";


const char GlBackend::blurFragmentShader[] =

"#version 330 core\n"

"struct BlurInfo"
"{"
"	vec4   colorMtx[9];"
"	vec2   offset[9];"
"};"

"uniform BlurInfo blurInfo;                     "
"uniform sampler2D texId;						"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"out vec4 color;								"

"void main()									"
"{												"
"	color = texture(texId, texUV + blurInfo.offset[0]) * blurInfo.colorMtx[0];"
"	color += texture(texId, texUV + blurInfo.offset[1]) * blurInfo.colorMtx[1];"
"	color += texture(texId, texUV + blurInfo.offset[2]) * blurInfo.colorMtx[2];"
"	color += texture(texId, texUV + blurInfo.offset[3]) * blurInfo.colorMtx[3];"
"   color += texture(texId, texUV + blurInfo.offset[4]) * blurInfo.colorMtx[4];  "
"	color += texture(texId, texUV + blurInfo.offset[5]) * blurInfo.colorMtx[5];"
"	color += texture(texId, texUV + blurInfo.offset[6]) * blurInfo.colorMtx[6];"
"	color += texture(texId, texUV + blurInfo.offset[7]) * blurInfo.colorMtx[7];"
"	color += texture(texId, texUV + blurInfo.offset[8]) * blurInfo.colorMtx[8];"
"}												";


const char GlBackend::blitFragmentShader[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"out vec4 color;								"
"void main()									"
"{												"
"   color = texture(texId, texUV) * fragColor;  "
"}												";

const char GlBackend::blitFragmentShader_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"out vec4 color;								"
"void main()									"
"{												"
"   color.r = texture(texId, texUV).a * fragColor.a;  "
"}												";


const char GlBackend::alphaBlitFragmentShader[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"out vec4 color;								"
"void main()									"
"{												"
"   color = fragColor;							"
"   color.a *= texture(texId, texUV).r;         "
"}												";

const char GlBackend::alphaBlitFragmentShader_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"out vec4 color;								"
"void main()									"
"{												"
"   color.r = fragColor.a * texture(texId, texUV).r;         "
"}												";



const char GlBackend::paletteBlitNearestVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	vec2   xcanvasSize;"
"	int    xcanvasYOfs;"
"	int    xcanvasYMul;"
"   vec4   xflatTint;"
"   ivec2  xtintRectPos;"
"   ivec2  xtintRectSize;"
"   vec4   xtopLeftTint;"
"   vec4   xtopRightTint;"
"   vec4   xbottomRightTint;"
"   vec4   xbottomLeftTint;"
"};"

"uniform samplerBuffer extrasId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec2 texUV;                                           "
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasId, extrasOfs);		   "
"   vec4 transform = texelFetch(extrasId, extrasOfs+1);	   "
"   vec2 src = srcDst.xy;                                  "
"   vec2 dst = srcDst.zw;                                  "
"   texUV.x = (src.x + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z) / texSize.x; "
"   texUV.y = (src.y + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w) / texSize.y; "
"   fragColor = texelFetch(extrasId, tintInfoOfs);		   "
"}                                                         ";

const char GlBackend::paletteBlitNearestGradientVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	vec2   xcanvasSize;"
"	int    xcanvasYOfs;"
"	int    xcanvasYMul;"
"   vec4   xflatTint;"
"   ivec2  xtintRectPos;"
"   ivec2  xtintRectSize;"
"   vec4   xtopLeftTint;"
"   vec4   xtopRightTint;"
"   vec4   xbottomRightTint;"
"   vec4   xbottomLeftTint;"
"};"

"uniform samplerBuffer extrasId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec2 texUV;                                           "
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasId, extrasOfs);		   "
"   vec4 transform = texelFetch(extrasId, extrasOfs+1);	   "
"   vec2 src = srcDst.xy;                                  "
"   vec2 dst = srcDst.zw;                                  "
"   texUV.x = (src.x + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z) / texSize.x; "
"   texUV.y = (src.y + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w) / texSize.y; "

"   vec4 tintRect = texelFetch(extrasId, tintInfoOfs+1);	"
"   vec4 topLeftTint = texelFetch(extrasId, tintInfoOfs+2);	"
"   vec4 topRightTint = texelFetch(extrasId, tintInfoOfs+3);	"
"   vec4 bottomRightTint = texelFetch(extrasId, tintInfoOfs+4);	"
"   vec4 bottomLeftTint = texelFetch(extrasId, tintInfoOfs+5);	"

"   vec2 tintRectPos = tintRect.xy;	"
"   vec2 tintRectSize = tintRect.zw;	"

"	vec2 tintOfs = (pos - tintRectPos) / vec2(tintRectSize); "
"   vec4 lineStartTint = topLeftTint + (bottomLeftTint - topLeftTint) * tintOfs.y;"
"   vec4 lineEndTint = topRightTint + (bottomRightTint - topRightTint) * tintOfs.y;"
"   vec4 gradientTint = lineStartTint + (lineEndTint - lineStartTint) * tintOfs.x;"

"   fragColor = texelFetch(extrasId, tintInfoOfs) * gradientTint;		   "
"}                                                         ";


const char GlBackend::paletteBlitNearestFragmentShader[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform sampler2D paletteId;						"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"out vec4 color;								"
"void main()									"
"{												"
"   color = texture(paletteId, vec2(texture(texId, texUV).r,0.5f)) * fragColor;	"
"}												";

const char GlBackend::paletteBlitNearestFragmentShader_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform sampler2D paletteId;						"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"out vec4 color;								"
"void main()									"
"{												"
"   color.r = texture(paletteId, vec2(texture(texId, texUV).r,0.5f)).a * fragColor.a;	"
"}												";


const char GlBackend::paletteBlitInterpolateVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	vec2   xcanvasSize;"
"	int    xcanvasYOfs;"
"	int    xcanvasYMul;"
"   vec4   xflatTint;"
"   ivec2  xtintRectPos;"
"   ivec2  xtintRectSize;"
"   vec4   xtopLeftTint;"
"   vec4   xtopRightTint;"
"   vec4   xbottomRightTint;"
"   vec4   xbottomLeftTint;"
"};"

"uniform samplerBuffer extrasId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec2 texUV00;                                         "
"out vec2 texUV11;                                         "
"out vec2 uvFrac;                                         "
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasId, extrasOfs);		   "
"   vec4 transform = texelFetch(extrasId, extrasOfs+1);	   "
"   vec2 src = srcDst.xy;                                  "
"   vec2 dst = srcDst.zw;                                  "
//"   float texU = src.x + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z; "
//"   float texV = src.y + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w; "

//"   float texU = src.x + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z; "
//"   float texV = src.y + (pos.y - dst.y) * transform.w + (pos.x - dst.x) * transform.y; "

"   vec2 texUV = src + (pos-dst) * transform.xw + (pos.yx - dst.yx) * transform.zy;"
"   texUV -= 0.5f;"

"   uvFrac = texUV;"
"   texUV00 = texUV/texSize;				"
"   texUV11 = (texUV+1)/texSize;			"
"   fragColor = texelFetch(extrasId, tintInfoOfs);		   "
"}                                                         ";

const char GlBackend::paletteBlitInterpolateGradientVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	vec2   xcanvasSize;"
"	int    xcanvasYOfs;"
"	int    xcanvasYMul;"
"   vec4   xflatTint;"
"   ivec2  xtintRectPos;"
"   ivec2  xtintRectSize;"
"   vec4   xtopLeftTint;"
"   vec4   xtopRightTint;"
"   vec4   xbottomRightTint;"
"   vec4   xbottomLeftTint;"
"};"

"uniform samplerBuffer extrasId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec2 texUV00;                                         "
"out vec2 texUV11;                                         "
"out vec2 uvFrac;                                         "
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasId, extrasOfs);		   "
"   vec4 transform = texelFetch(extrasId, extrasOfs+1);	   "
"   vec2 src = srcDst.xy;                                  "
"   vec2 dst = srcDst.zw;                                  "
//"   float texU = src.x + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z; "
//"   float texV = src.y + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w; "

//"   float texU = src.x + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z; "
//"   float texV = src.y + (pos.y - dst.y) * transform.w + (pos.x - dst.x) * transform.y; "

"   vec2 texUV = src + (pos-dst) * transform.xw + (pos.yx - dst.yx) * transform.zy;"
"   texUV -= 0.5f;"

"   uvFrac = texUV;"
"   texUV00 = texUV/texSize;				"
"   texUV11 = (texUV+1)/texSize;			"

"   vec4 tintRect = texelFetch(extrasId, tintInfoOfs+1);	"
"   vec4 topLeftTint = texelFetch(extrasId, tintInfoOfs+2);	"
"   vec4 topRightTint = texelFetch(extrasId, tintInfoOfs+3);	"
"   vec4 bottomRightTint = texelFetch(extrasId, tintInfoOfs+4);	"
"   vec4 bottomLeftTint = texelFetch(extrasId, tintInfoOfs+5);	"

"   vec2 tintRectPos = tintRect.xy;	"
"   vec2 tintRectSize = tintRect.zw;	"


"	vec2 tintOfs = (pos - tintRectPos) / vec2(tintRectSize); "
"   vec4 lineStartTint = topLeftTint + (bottomLeftTint - topLeftTint) * tintOfs.y;"
"   vec4 lineEndTint = topRightTint + (bottomRightTint - topRightTint) * tintOfs.y;"
"   vec4 gradientTint = lineStartTint + (lineEndTint - lineStartTint) * tintOfs.x;"

"   fragColor = texelFetch(extrasId, tintInfoOfs) * gradientTint;		   "
"}                                                         ";


const char GlBackend::paletteBlitInterpolateFragmentShader[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform sampler2D paletteId;						"
"in vec2 texUV00;								"
"in vec2 texUV11;								"
"in vec2 uvFrac;								"
"in vec4 fragColor;								"
"out vec4 color;								"
"void main()									"
"{												"
"   float index00 = texture(texId, texUV00).r;		"
"   float index01 = texture(texId, vec2(texUV11.x,texUV00.y) ).r;		"
"   float index10 = texture(texId, vec2(texUV00.x,texUV11.y) ).r;		"
"   float index11 = texture(texId, texUV11).r;		"
"   vec4 color00 = texture(paletteId, vec2(index00,0.5f));	"
"   vec4 color01 = texture(paletteId, vec2(index01,0.5f));	"
"   vec4 color10 = texture(paletteId, vec2(index10,0.5f));	"
"   vec4 color11 = texture(paletteId, vec2(index11,0.5f));	"

"   vec4 out0 = color00 * (1-fract(uvFrac.x)) + color01 * fract(uvFrac.x);	"
"   vec4 out1 = color10 * (1-fract(uvFrac.x)) + color11 * fract(uvFrac.x);	"
"   color = (out0 * (1-fract(uvFrac.y)) + out1 * fract(uvFrac.y)) * fragColor;	"
"}												";

const char GlBackend::paletteBlitInterpolateFragmentShader_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform sampler2D paletteId;						"
"in vec2 texUV00;								"
"in vec2 texUV11;								"
"in vec2 uvFrac;								"
"in vec4 fragColor;								"
"out vec4 color;								"
"void main()									"
"{												"
"   float index00 = texture(texId, texUV00).r;		"
"   float index01 = texture(texId, vec2(texUV11.x,texUV00.y) ).r;		"
"   float index10 = texture(texId, vec2(texUV00.x,texUV11.y) ).r;		"
"   float index11 = texture(texId, texUV11).r;		"
"   float color00 = texture(paletteId, vec2(index00,0.5f)).a;	"
"   float color01 = texture(paletteId, vec2(index01,0.5f)).a;	"
"   float color10 = texture(paletteId, vec2(index10,0.5f)).a;	"
"   float color11 = texture(paletteId, vec2(index11,0.5f)).a;	"

"   float out0 = color00 * (1-fract(uvFrac.x)) + color01 * fract(uvFrac.x);	"
"   float out1 = color10 * (1-fract(uvFrac.x)) + color11 * fract(uvFrac.x);	"
"   color.r = (out0 * (1-fract(uvFrac.y)) + out1 * fract(uvFrac.y)) * fragColor.a;	"
"}												";



const char GlBackend::plotVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	vec2   xcanvasSize;"
"	int    xcanvasYOfs;"
"	int    xcanvasYMul;"
"   vec4   xflatTint;"
"   ivec2  xtintRectPos;"
"   ivec2  xtintRectSize;"
"   vec4   xtopLeftTint;"
"   vec4   xtopRightTint;"
"   vec4   xbottomRightTint;"
"   vec4   xbottomLeftTint;"
"};"


"uniform samplerBuffer extrasId;						"
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec4 fragColor;									"
"void main()                                            "
"{                                                      "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.x = (pos.x+0.5)*2.0/canvasInfo.x - 1.0; "
"   gl_Position.y = ((canvasInfo.z + canvasInfo.w*pos.y)+0.5)*2.0/canvasInfo.y - 1,0;	"
"   gl_Position.z = 0.0;                                "
"   gl_Position.w = 1.0;                                "
"   fragColor = texelFetch(extrasId, tintInfoOfs) * texelFetch(extrasId, extrasOfs);  "
"}                                                      ";


const char GlBackend::plotFragmentShader[] =

"#version 330 core\n"
"in vec4 fragColor;                     "
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   outColor = fragColor;				"
"}                                      ";

const char GlBackend::plotFragmentShader_A8[] =

"#version 330 core\n"
"in vec4 fragColor;                     "
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   outColor.r = fragColor.a;				"
"}                                      ";



const char GlBackend::lineFromToVertexShader[] =

"#version 330 core\n"

"uniform samplerBuffer extrasId;								"
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec4 fragColor;                                        "
"flat out float s;												"
"flat out float w;												"
"flat out float slope;											"
"flat out float ifSteep;											"
"flat out float ifMild;											"
"void main()                                                "
"{                                                          "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;             "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;             "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "
"   fragColor = texelFetch(extrasId, tintInfoOfs) * texelFetch(extrasId, extrasOfs);			"
"   int ofs = extrasOfs+1;									"
"   vec4 x = texelFetch(extrasId, ofs);						"
"   s = x.x;												"
"   w = x.y;												"
"   slope = canvasInfo.w*x.z;										"
"   ifSteep = x.w;											"
"   ifMild = 1.0 - ifSteep;									"
"}                                                          ";


const char GlBackend::lineFromToFragmentShader[] =

"#version 330 core\n"
"in vec4 fragColor;                     "
"flat in float s;							"
"flat in float w;							"
"flat in float slope;						"
"flat in float ifSteep;						"
"flat in float ifMild;						"
"out vec4 outColor;                     "
"void main()                            "
"{										"
"   outColor.rgb = fragColor.rgb;		"
"   outColor.a = fragColor.a * clamp(w - abs(gl_FragCoord.x*ifSteep + gl_FragCoord.y*ifMild - s - (gl_FragCoord.x*ifMild + gl_FragCoord.y*ifSteep) * slope), 0.0, 1.0); "
"}                                      ";

const char GlBackend::lineFromToFragmentShader_A8[] =

"#version 330 core\n"
"in vec4 fragColor;                     "
"flat in float s;							"
"flat in float w;							"
"flat in float slope;						"
"flat in float ifSteep;						"
"flat in float ifMild;						"
"out vec4 outColor;                     "
"void main()                            "
"{										"
"   outColor.r = fragColor.a * clamp(w - abs(gl_FragCoord.x*ifSteep + gl_FragCoord.y*ifMild - s - (gl_FragCoord.x*ifMild + gl_FragCoord.y*ifSteep) * slope), 0.0, 1.0); "
"}                                      ";


const char GlBackend::aaFillVertexShader[] =

"#version 330 core\n"

"uniform samplerBuffer extrasId;								"
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec4 fragColor;                                        "
"flat out vec4 rect;										"
"void main()                                                "
"{                                                          "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "
"   fragColor = texelFetch(extrasId, tintInfoOfs) * texelFetch(extrasId, extrasOfs);			"
"   rect = texelFetch(extrasId, extrasOfs+1);				"
"   rect.y = canvasInfo.z + canvasInfo.w*rect.y;							"
"   rect.zw += vec2(0.5f,0.5f);								"		// Adding offset here so we don't have to do it in pixel shader.
"}                                                          ";

const char GlBackend::aaFillGradientVertexShader[] =

"#version 330 core\n"

"uniform samplerBuffer extrasId;								"
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec4 fragColor;                                        "
"flat out vec4 rect;										"
"void main()                                                "
"{                                                          "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "

"   vec4 tintRect = texelFetch(extrasId, tintInfoOfs+1);	"
"   vec4 topLeftTint = texelFetch(extrasId, tintInfoOfs+2);	"
"   vec4 topRightTint = texelFetch(extrasId, tintInfoOfs+3);	"
"   vec4 bottomRightTint = texelFetch(extrasId, tintInfoOfs+4);	"
"   vec4 bottomLeftTint = texelFetch(extrasId, tintInfoOfs+5);	"

"   vec2 tintRectPos = tintRect.xy;	"
"   vec2 tintRectSize = tintRect.zw;	"

"	vec2 tintOfs = (pos - tintRectPos) / vec2(tintRectSize); "
"   vec4 lineStartTint = topLeftTint + (bottomLeftTint - topLeftTint) * tintOfs.y;"
"   vec4 lineEndTint = topRightTint + (bottomRightTint - topRightTint) * tintOfs.y;"
"   vec4 gradientTint = lineStartTint + (lineEndTint - lineStartTint) * tintOfs.x;"

"   fragColor = texelFetch(extrasId, tintInfoOfs) * gradientTint * texelFetch(extrasId, extrasOfs);		   "

"   rect = texelFetch(extrasId, extrasOfs+1);				"
"   rect.y = canvasInfo.z + canvasInfo.w*rect.y;							"
"   rect.zw += vec2(0.5f,0.5f);								"		// Adding offset here so we don't have to do it in pixel shader.
"}                                                          ";



const char GlBackend::aaFillFragmentShader[] =

"#version 330 core\n"
"in vec4 fragColor;						"
"flat in vec4 rect;						"
"out vec4 outColor;                     "
"void main()                            "
"{										"
"   outColor.rgb = fragColor.rgb;             "
"	vec2 middleofs = abs(gl_FragCoord.xy - rect.xy);   "
"	vec2 alphas = clamp(rect.zw  - middleofs, 0.f, 1.f);  "
"	outColor.a = fragColor.a * alphas.x * alphas.y;  "
"}                                      ";

const char GlBackend::aaFillFragmentShader_A8[] =

"#version 330 core\n"
"in vec4 fragColor;						"
"flat in vec4 rect;						"
"out vec4 outColor;                     "
"void main()                            "
"{										"
"	vec2 middleofs = abs(gl_FragCoord.xy - rect.xy);   "
"	vec2 alphas = clamp(rect.zw  - middleofs, 0.f, 1.f);  "
"	outColor.r = fragColor.a * alphas.x * alphas.y;  "
"}                                      ";


const char GlBackend::segmentsVertexShader[] =

"#version 330 core\n"

"uniform samplerBuffer extrasId;						"
"uniform samplerBuffer colorsId;					"
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 uv;					   "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec2 texUV;										"
"out vec4 fragColor;                                    "
"flat out int segments;									"
"flat out int stripesOfs;								"
"out vec2 paletteOfs;"

"void main()											"
"{                                                      "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                "
"   gl_Position.w = 1.0;                                "
"   vec4 extras = texelFetch(extrasId, extrasOfs);		"
"   vec4 extras2 = texelFetch(extrasId, extrasOfs+1);	"		// x,y = first color ofs. z,w = texture width/height
"   segments = int(extras.x);							"
"   stripesOfs = int(extras.y);							"
"   texUV = uv;											"

"   float xTintOfs = uv.x/extras.z*1/extras2.z;"
"   float yTintOfs = uv.y/extras.w*1/extras2.w;"

"   paletteOfs = vec2(extras2.x+xTintOfs,extras2.y+yTintOfs);"
"   fragColor = texelFetch(extrasId, tintInfoOfs);								"
"}                                                      ";

const char GlBackend::segmentsVertexShaderGradient[] =

"#version 330 core\n"

"uniform samplerBuffer extrasId;						"
"uniform samplerBuffer colorsId;					"
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 uv;					   "
"layout(location = 2) in int extrasOfs;                    "
"layout(location = 3) in int canvasInfoOfs;                "
"layout(location = 4) in int tintInfoOfs;                "
"out vec2 texUV;										"
"out vec4 fragColor;                                    "
"flat out int segments;									"
"flat out int stripesOfs;								"
"out vec2 paletteOfs;"

"void main()											"
"{                                                      "
"   vec4 canvasInfo = texelFetch(extrasId, canvasInfoOfs);	"
"   gl_Position.x = pos.x*2/canvasInfo.x - 1.0;            "
"   gl_Position.y = (canvasInfo.z + canvasInfo.w*pos.y)*2/canvasInfo.y - 1.0;    "
"   gl_Position.z = 0.0;                                "
"   gl_Position.w = 1.0;                                "
"   vec4 extras = texelFetch(extrasId, extrasOfs);		"
"   vec4 extras2 = texelFetch(extrasId, extrasOfs+1);	"		// x,y = first color ofs. z,w = texture width/height
"   segments = int(extras.x);							"
"   stripesOfs = int(extras.y);							"
"   texUV = uv;											"

"   float xTintOfs = uv.x/extras.z*1/extras2.z;"
"   float yTintOfs = uv.y/extras.w*1/extras2.w;"

"   paletteOfs = vec2(extras2.x+xTintOfs,extras2.y+yTintOfs);"

"   vec4 tintRect = texelFetch(extrasId, tintInfoOfs+1);	"
"   vec4 topLeftTint = texelFetch(extrasId, tintInfoOfs+2);	"
"   vec4 topRightTint = texelFetch(extrasId, tintInfoOfs+3);	"
"   vec4 bottomRightTint = texelFetch(extrasId, tintInfoOfs+4);	"
"   vec4 bottomLeftTint = texelFetch(extrasId, tintInfoOfs+5);	"

"   vec2 tintRectPos = tintRect.xy;	"
"   vec2 tintRectSize = tintRect.zw;	"

"	vec2 tintOfs = (pos - tintRectPos) / vec2(tintRectSize); "
"   vec4 lineStartTint = topLeftTint + (bottomLeftTint - topLeftTint) * tintOfs.y;"
"   vec4 lineEndTint = topRightTint + (bottomRightTint - topRightTint) * tintOfs.y;"
"   vec4 gradientTint = lineStartTint + (lineEndTint - lineStartTint) * tintOfs.x;"

"   fragColor = texelFetch(extrasId, tintInfoOfs) * gradientTint;		   "
"}                                                      ";

const char GlBackend::segmentsFragmentShader[] =

"#version 330 core\n"
"uniform samplerBuffer stripesId;				"
"uniform sampler2D	paletteId;					"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"flat in int segments;							"
"flat in int stripesOfs;						"
"in vec2 paletteOfs;"

"out vec4 color;								"
"void main()									"
"{												"
"	float totalAlpha = 0.f;"
"	vec3	rgbAcc = vec3(0,0,0);"

"	float factor = 1.f;"
"   vec2 palOfs = paletteOfs;"
"	for( int i = 0 ; i < $EDGES ; i++ )"
"	{"
"  		vec4 col = texture(paletteId, palOfs);"
"  		palOfs.x += 1/$MAXSEG.f;"

"		vec4 edge = texelFetch(stripesId, stripesOfs + int(texUV.x)*(segments-1)+i );"

"		float x = (texUV.y - edge.r) * edge.g;"
"		float adder = edge.g / 2.f;"
"		if (x < 0.f)"
"			adder = edge.b;"
"		else if (x + edge.g > 1.f)"
"			adder = edge.a;"
"		float factor2 = clamp(x + adder, 0.f, 1.f);"

"		float useFactor = (factor - factor2)*col.a;"
"		totalAlpha += useFactor;"
"		rgbAcc += col.rgb * useFactor;"

"		factor = factor2;"
"	}"

"  	vec4 col = texture(paletteId, palOfs);"
"	float useFactor = factor*col.a;"
"	totalAlpha += useFactor;"
"	rgbAcc += col.rgb * useFactor;"

"   color.a = totalAlpha * fragColor.a; "
"   color.rgb = (rgbAcc/totalAlpha) * fragColor.rgb;"
"}";

const char GlBackend::segmentsFragmentShader_A8[] =

"#version 330 core\n"
"uniform samplerBuffer stripesId;				"
"uniform sampler2D	paletteId;					"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"flat in int segments;							"
"flat in int stripesOfs;						"
"in vec2 paletteOfs;"

"out vec4 color;								"
"void main()									"
"{												"
"	float totalAlpha = 0.f;"

"	float factor = 1.f;"
"   vec2 palOfs = paletteOfs;"
"	for( int i = 0 ; i < $EDGES ; i++ )"
"	{"
"  		vec4 col = texture(paletteId, palOfs);"
"  		palOfs.x += 1/$MAXSEG.f;"

"		vec4 edge = texelFetch(stripesId, stripesOfs + int(texUV.x)*(segments-1)+i );"

"		float x = (texUV.y - edge.r) * edge.g;"
"		float adder = edge.g / 2.f;"
"		if (x < 0.f)"
"			adder = edge.b;"
"		else if (x + edge.g > 1.f)"
"			adder = edge.a;"
"		float factor2 = clamp(x + adder, 0.f, 1.f);"

"		float useFactor = (factor - factor2)*col.a;"
"		totalAlpha += useFactor;"

"		factor = factor2;"
"	}"

"  	vec4 col = texture(paletteId, palOfs);"
"	float useFactor = factor*col.a;"
"	totalAlpha += useFactor;"

"   color.r = totalAlpha * fragColor.a; "
"}";


/*
static const char segmentsFragmentShader2[] =

"#version 330 core\n"
"uniform samplerBuffer stripesId;				"
"uniform sampler2D	paletteId;					"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"flat in int segments;							"
"flat in int stripesOfs;						"
"in vec2 paletteOfs;"

"out vec4 color;								"
"void main()									"
"{												"
"	vec4 edge1 = texelFetch(stripesId, stripesOfs + int(texUV.x)*(segments-1) ); "
"	vec4 edge2 = texelFetch(stripesId, stripesOfs + int(texUV.x)*(segments-1)+1 ); "

"   vec4 col1 = texture(paletteId, paletteOfs);  "
"   vec4 col2 = texture(paletteId, vec2(paletteOfs.x+1/16.f,paletteOfs.y));  "
"   vec4 col3 = texture(paletteId, vec2(paletteOfs.x+2/16.f,paletteOfs.y));  "


"	float factor1 = 1.f; "

"	float x1 = (texUV.y - edge1.r) * edge1.g;"
"	float adder1 = edge1.g / 2.f;"
"	if (x1 < 0.f)"
"		adder1 = edge1.b;"
"	else if (x1 + edge1.g > 1.f)"
"		adder1 = edge1.a;"
"	float factor2 = clamp(x1 + adder1, 0.f, 1.f);"

"	float x2 = (texUV.y - edge2.r) * edge2.g;"
"	float adder2 = edge2.g / 2.f;"
"	if (x2 < 0.f)"
"		adder2 = edge2.b;"
"	else if (x2 + edge2.g > 1.f)"
"		adder2 = edge2.a;"
"	float factor3 = clamp(x2 + adder2, 0.f, 1.f);"

"   factor1 = (factor1 -factor2)*col1.a;"
"   factor2 = (factor2 -factor3)*col2.a;"
"   factor3 = (factor3)*col3.a;"

"   float totalAlpha = factor1 + factor2 + factor3;"

"   vec4 result;"
"   result.a = totalAlpha; "
"   result.rgb = (col1.rgb * factor1 + col2.rgb * factor2 + col3.rgb * factor3)/totalAlpha;"
"   color = result * fragColor;"
"}												";
*/

}

#endif //WG_GLSHADERS_DOT_H
