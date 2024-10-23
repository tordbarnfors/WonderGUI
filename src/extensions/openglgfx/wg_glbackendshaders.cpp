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


const char GlBackend::fillTintmapVertexShader[] =

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
"layout(location = 4) in vec2 tintmapOfs;                  "
"out vec4 fragColor;                                       "
"out vec2 tintmapUU;                                      "
"void main()                                               "
"{                                                         "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;             "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   fragColor = texelFetch(colorBufferId, colorOfs);	   "
"   tintmapUU  =  tintmapOfs;						  "
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

const char GlBackend::fillFragmentShaderTintmap[] =

"#version 330 core\n"
"uniform samplerBuffer tintmapBufferId;	"
"in vec4 fragColor;                     "
"in vec2 tintmapUU;                     "
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   outColor = texelFetch(tintmapBufferId, int(tintmapUU.x) )"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) )"
"	           * fragColor;           "
"}                                      ";


const char GlBackend::fillFragmentShaderTintmap_A8[] =

"#version 330 core\n"
"uniform samplerBuffer tintmapBufferId;	"
"in vec4 fragColor;                     "
"in vec2 tintmapUU;                     "
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   outColor.r = texelFetch(tintmapBufferId, int(tintmapUU.x) ).a"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) ).a"
"	           * fragColor.a;           "
"}                                      ";



const char GlBackend::blitVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer extrasBufferId;						   "
"uniform samplerBuffer colorBufferId;					   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 2) in int colorOfs;                     "
"layout(location = 3) in int extrasOfs;				       "
"out vec2 texUV;                                           "
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;            "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasBufferId, extrasOfs);	   "
"   vec4 transform = texelFetch(extrasBufferId, extrasOfs+1);	   "
"   vec2 src = srcDst.xy;                                  "
"   vec2 dst = srcDst.zw;                                  "
"   texUV.x = (src.x + 0.0001f + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z) / texSize.x; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   texUV.y = (src.y + 0.0001f + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w) / texSize.y; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   fragColor = texelFetch(colorBufferId, colorOfs);				"
"}                                                         ";


const char GlBackend::blitTintmapVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer extrasBufferId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 3) in int extrasOfs;				       "
"layout(location = 4) in vec2 tintmapOfs;                  "
"out vec2 texUV;                                           "
"out vec2 tintmapUU;                                      "
"void main()                                               "
"{                                                         "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;            "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasBufferId, extrasOfs);	   "
"   vec4 transform = texelFetch(extrasBufferId, extrasOfs+1);	   "
"   vec2 src = srcDst.xy;                                  "
"   vec2 dst = srcDst.zw;                                  "
"   texUV.x = (src.x + 0.0001f + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z) / texSize.x; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   texUV.y = (src.y + 0.0001f + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w) / texSize.y; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   tintmapUU  =  tintmapOfs;						  "
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

"   color *= fragColor;"
"}												";


const char GlBackend::blurFragmentShaderTintmap[] =

"#version 330 core\n"

"struct BlurInfo"
"{"
"	vec4   colorMtx[9];"
"	vec2   offset[9];"
"};"

"uniform BlurInfo blurInfo;                     "
"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"in vec2 tintmapUU;								"
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

"   vec4 fragColor = texelFetch(tintmapBufferId, int(tintmapUU.x) )"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) ); "

"   color *= fragColor;"
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


const char GlBackend::blitFragmentShaderTintmap[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"in vec2 tintmapUU;								"
"out vec4 color;								"
"void main()									"
"{												"
"   vec4 fragColor = texelFetch(tintmapBufferId, int(tintmapUU.x) )"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) ); "
"   color = texture(texId, texUV) * fragColor;  "
"}												";

const char GlBackend::blitFragmentShaderTintmap_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"in vec2 tintmapUU;								"
"out vec4 color;								"
"void main()									"
"{												"
"   float fragA = texelFetch(tintmapBufferId, int(tintmapUU.x) ).a"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) ).a; "
"   color.r = texture(texId, texUV).a * fragA;  "
"}												";

const char GlBackend::alphaBlitFragmentShaderTintmap[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"in vec2 tintmapUU;								"
"out vec4 color;								"
"void main()									"
"{												"
"   color = texelFetch(tintmapBufferId, int(tintmapUU.x) )"
"           * texelFetch(tintmapBufferId, int(tintmapUU.y) ); "
"   color.a *= texture(texId, texUV).r;         "
"}												";

const char GlBackend::alphaBlitFragmentShaderTintmap_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"in vec2 tintmapUU;								"
"out vec4 color;								"
"void main()									"
"{												"
"   float fragA = texelFetch(tintmapBufferId, int(tintmapUU.x) ).a"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) ).a; "

"   color.r = fragA * texture(texId, texUV).r;  "
"}												";


const char GlBackend::paletteBlitNearestVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer extrasBufferId;						   "
"uniform samplerBuffer colorBufferId;					   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 2) in int colorOfs;                    "
"layout(location = 3) in int extrasOfs;                    "
"out vec2 texUV;                                           "
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;            "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasBufferId, extrasOfs);	   "
"   vec4 transform = texelFetch(extrasBufferId, extrasOfs+1);	   "
"   vec2 src = srcDst.xy;                                  "
"   vec2 dst = srcDst.zw;                                  "
"   texUV.x = (src.x + 0.0001f + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z) / texSize.x; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   texUV.y = (src.y + 0.0001f + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w) / texSize.y; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   fragColor = texelFetch(colorBufferId, colorOfs);				"
"}                                                         ";


const char GlBackend::paletteBlitNearestTintmapVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer extrasBufferId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 3) in int extrasOfs;                    "
"layout(location = 4) in vec2 tintmapOfs;                  "
"out vec2 texUV;                                           "
"out vec2 tintmapUU;                                      "
"void main()                                               "
"{                                                         "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;            "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasBufferId, extrasOfs);	   "
"   vec4 transform = texelFetch(extrasBufferId, extrasOfs+1);	   "
"   vec2 src = srcDst.xy;                                  "
"   vec2 dst = srcDst.zw;                                  "
"   texUV.x = (src.x + 0.0001f + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z) / texSize.x; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   texUV.y = (src.y + 0.0001f + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w) / texSize.y; "      //TODO: Replace this ugly +0.02f fix with whatever is correct.
"   tintmapUU  =  tintmapOfs;						  "
"}														";


const char GlBackend::paletteBlitNearestFragmentShader[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform sampler2D paletteId;					"
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


const char GlBackend::paletteBlitNearestFragmentShaderTintmap[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"uniform sampler2D paletteId;					"
"in vec2 texUV;									"
"in vec2 tintmapUU;								"
"out vec4 color;								"
"void main()									"
"{												"
"   vec4 fragColor = texelFetch(tintmapBufferId, int(tintmapUU.x) )"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) ); "

"   color = texture(paletteId, vec2(texture(texId, texUV).r,0.5f)) * fragColor;	"
"}												";

const char GlBackend::paletteBlitNearestFragmentShaderTintmap_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"uniform sampler2D paletteId;						"
"in vec2 texUV;									"
"in vec2 tintmapUU;								"
"out vec4 color;								"
"void main()									"
"{												"
"   float fragA = texelFetch(tintmapBufferId, int(tintmapUU.x) ).a"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) ).a; "
"   color.r = texture(paletteId, vec2(texture(texId, texUV).r,0.5f)).a * fragA;	"
"}												";



const char GlBackend::paletteBlitInterpolateVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer extrasBufferId;						   "
"uniform samplerBuffer colorBufferId;					   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 2) in int colorOfs;                    "
"layout(location = 3) in int extrasOfs;                    "
"out vec2 texUV00;                                         "
"out vec2 texUV11;                                         "
"out vec2 uvFrac;                                         "
"out vec4 fragColor;                                       "
"void main()                                               "
"{                                                         "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;            "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasBufferId, extrasOfs);		   "
"   vec4 transform = texelFetch(extrasBufferId, extrasOfs+1);	   "
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
"   fragColor = texelFetch(colorBufferId, colorOfs);				"
"}                                                         ";


const char GlBackend::paletteBlitInterpolateTintmapVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer extrasBufferId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 3) in int extrasOfs;                    "
"layout(location = 4) in vec2 tintmapOfs;                  "
"out vec2 texUV00;                                         "
"out vec2 texUV11;                                         "
"out vec2 uvFrac;                                         "
"out vec2 tintmapUU;                                      "
"void main()                                               "
"{                                                         "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;            "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   vec4 srcDst = texelFetch(extrasBufferId, extrasOfs);		   "
"   vec4 transform = texelFetch(extrasBufferId, extrasOfs+1);	   "
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
"   tintmapUU  =  tintmapOfs;						  "
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


const char GlBackend::paletteBlitInterpolateFragmentShaderTintmap[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"uniform sampler2D paletteId;						"
"in vec2 texUV00;								"
"in vec2 texUV11;								"
"in vec2 uvFrac;								"
"in vec2 tintmapUU;								"
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

"   vec4 fragColor = texelFetch(tintmapBufferId, int(tintmapUU.x) )"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) ); "

"   color = (out0 * (1-fract(uvFrac.y)) + out1 * fract(uvFrac.y)) * fragColor;	"
"}												";

const char GlBackend::paletteBlitInterpolateFragmentShaderTintmap_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"uniform sampler2D paletteId;						"
"in vec2 texUV00;								"
"in vec2 texUV11;								"
"in vec2 uvFrac;								"
"in vec2 tintmapUU;								"
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

"   float fragA = texelFetch(tintmapBufferId, int(tintmapUU.x) ).a"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) ).a; "

"   color.r = (out0 * (1-fract(uvFrac.y)) + out1 * fract(uvFrac.y)) * fragA;	"
"}												";


const char GlBackend::plotVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"


"uniform samplerBuffer colorBufferId;					"
"layout(location = 0) in ivec2 pos;                     "
"layout(location = 2) in int colorOfs;                  "
"out vec4 fragColor;									"
"void main()                                            "
"{                                                      "
"   gl_Position.x = (pos.x+0.5)*2.0/canvasWidth - 1.0;  "
"   gl_Position.y = ((canvasYOfs + canvasYMul*pos.y)+0.5)*2.0/canvasHeight - 1,0;	"
"   gl_Position.z = 0.0;                                "
"   gl_Position.w = 1.0;                                "
"   fragColor = texelFetch(colorBufferId, colorOfs);    "
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

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer colorBufferId;					"
"uniform samplerBuffer extrasBufferId;					"

"layout(location = 0) in ivec2 pos;                     "
"layout(location = 2) in int colorOfs;                  "
"layout(location = 3) in int extrasOfs;                 "

"out vec4 fragColor;                                    "
"flat out float s;										"
"flat out float w;										"
"flat out float slope;									"
"flat out float ifSteep;								"
"flat out float ifMild;									"
"void main()                                            "
"{                                                      "
"   gl_Position.x = pos.x*2.0/canvasWidth - 1.0;        "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2.0/canvasHeight - 1.0;"
"   gl_Position.z = 0.0;                                "
"   gl_Position.w = 1.0;                                "
"   fragColor = texelFetch(colorBufferId, colorOfs);	"

"   vec4 x = texelFetch(extrasBufferId, extrasOfs);		"
"   s = x.x;											"
"   w = x.y;											"
"   slope = canvasYMul*x.z;								"
"   ifSteep = x.w;										"
"   ifMild = 1.0 - ifSteep;								"
"}                                                      ";


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

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer colorBufferId;					    "
"uniform samplerBuffer extrasBufferId;						"
"layout(location = 0) in ivec2 pos;                         "
"layout(location = 2) in int colorOfs;                      "
"layout(location = 3) in int extrasOfs;                     "
"out vec4 fragColor;                                        "
"flat out vec4 rect;										"
"void main()                                                "
"{                                                          "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;              "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "
"   fragColor = texelFetch(colorBufferId, colorOfs);		"
"   rect = texelFetch(extrasBufferId, extrasOfs);			"
"   rect.y = canvasYOfs + canvasYMul*rect.y;				"
"   rect.zw += vec2(0.5f,0.5f);								"		// Adding offset here so we don't have to do it in pixel shader.
"}                                                          ";


const char GlBackend::aaFillTintmapVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer colorBufferId;					    "
"uniform samplerBuffer extrasBufferId;						"
"layout(location = 0) in ivec2 pos;                         "
"layout(location = 2) in int colorOfs;                      "
"layout(location = 3) in int extrasOfs;                     "
"layout(location = 4) in vec2 tintmapOfs;                   "
"out vec4 fragColor;                                        "
"flat out vec4 rect;										"
"out vec2 tintmapUU;                                        "
"void main()                                                "
"{                                                          "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;              "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "
"   fragColor = texelFetch(colorBufferId, colorOfs);		"
"   rect = texelFetch(extrasBufferId, extrasOfs);			"
"   rect.y = canvasYOfs + canvasYMul*rect.y;				"
"   rect.zw += vec2(0.5f,0.5f);								"		// Adding offset here so we don't have to do it in pixel shader.
"   tintmapUU  =  tintmapOfs;						        "
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


const char GlBackend::aaFillFragmentShaderTintmap[] =

"#version 330 core\n"
"uniform samplerBuffer tintmapBufferId;	"
"in vec4 fragColor;						"
"in vec2 tintmapUU;                     "
"flat in vec4 rect;						"
"out vec4 outColor;                     "
"void main()                            "
"{										"
"   vec4 inColor = texelFetch(tintmapBufferId, int(tintmapUU.x) )"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) )"
"	           * fragColor;           "
"   outColor.rgb = inColor.rgb;             "
"	vec2 middleofs = abs(gl_FragCoord.xy - rect.xy);   "
"	vec2 alphas = clamp(rect.zw  - middleofs, 0.f, 1.f);  "
"	outColor.a = inColor.a * alphas.x * alphas.y;  "
"}                                      ";

const char GlBackend::aaFillFragmentShaderTintmap_A8[] =

"#version 330 core\n"
"uniform samplerBuffer tintmapBufferId;	"
"in vec4 fragColor;						"
"in vec2 tintmapUU;                     "
"flat in vec4 rect;						"
"out vec4 outColor;                     "
"void main()                            "
"{										"
"   float inAlpha = texelFetch(tintmapBufferId, int(tintmapUU.x) ).a"
"               * texelFetch(tintmapBufferId, int(tintmapUU.y) ).a"
"	           * fragColor.a;           "
"	vec2 middleofs = abs(gl_FragCoord.xy - rect.xy);   "
"	vec2 alphas = clamp(rect.zw  - middleofs, 0.f, 1.f);  "
"	outColor.r = inAlpha * alphas.x * alphas.y;  "
"}                                      ";


const char GlBackend::segmentsVertexShader[] =

"#version 330 core\n"

"layout(std140) uniform Canvas"
"{"
"	float  canvasWidth;"
"	float  canvasHeight;"
"	float  canvasYOfs;"
"	float  canvasYMul;"
"};"

"uniform samplerBuffer extrasBufferId;					"
"layout(location = 0) in ivec2 pos;                     "
"layout(location = 1) in vec2 uv;					    "
"layout(location = 3) in int extrasOfs;                 "
"layout(location = 4) in vec2 tintmapOfs;               "
"layout(location = 5) in vec2 colorstripOfs;            "
"out vec2 texUV;										"
"flat out int segments;									"
"flat out int colorstripPitchX;							"
"flat out int colorstripPitchY;							"
"out vec2 tintmapUU;"
"out vec2 colorstripUU;"

"void main()											"
"{                                                      "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;              "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "

"   vec4 extras = texelFetch(extrasBufferId, extrasOfs);		"
"   segments = int(extras.x);							"
"   colorstripPitchX = int(extras.z);						"
"   colorstripPitchY = int(extras.w);						"
"   texUV = uv;											"
"   tintmapUU = tintmapOfs;"
"   colorstripUU = colorstripOfs;"
"}                                                      ";


const char GlBackend::segmentsFragmentShader[] =

"#version 330 core\n"
"uniform samplerBuffer tintmapBufferId;			"
"uniform samplerBuffer edgemapId;				"
"in vec2 texUV;									"
"flat in int segments;							"
"flat in int colorstripPitchX;					"
"flat in int colorstripPitchY;					"
"in vec2 tintmapUU;"
"in vec2 colorstripUU;"

"out vec4 color;								"
"void main()									"
"{												"
"	float totalAlpha = 0.f;"
"	vec3	rgbAcc = vec3(0,0,0);"

"	float factor = 1.f;"

"   int colorstripX = int(colorstripUU.x);"
"   int colorstripY = int(colorstripUU.y);"

"	for( int i = 0 ; i < $EDGES ; i++ )"
"	{"

"		vec4 col = texelFetch(edgemapId, colorstripX )"
"			        * texelFetch(edgemapId, colorstripY );"

"		colorstripX += colorstripPitchX; "
"		colorstripY += colorstripPitchY; "

"		vec4 edge = texelFetch(edgemapId, int(texUV.x)*(segments-1)+i );"

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

"		vec4 col = texelFetch(edgemapId, colorstripX )"
"			        * texelFetch(edgemapId, colorstripY );"

"	float useFactor = factor*col.a;"
"	totalAlpha += useFactor;"
"	rgbAcc += col.rgb * useFactor;"

"   col.a = totalAlpha; "
"   col.rgb = (rgbAcc/totalAlpha);"

"    color = texelFetch(tintmapBufferId, int(tintmapUU.x) )"
"              * texelFetch(tintmapBufferId, int(tintmapUU.y) )"
"	           * col;           "

"}";


const char GlBackend::segmentsFragmentShader_A8[] =

"#version 330 core\n"
"uniform samplerBuffer tintmapBufferId;	"
"uniform samplerBuffer edgemapId;				"
"in vec2 texUV;									"
"flat in int segments;							"
"flat in int colorstripPitchX;					"
"flat in int colorstripPitchY;					"
"in vec2 tintmapUU;"
"in vec2 colorstripUU;"

"out vec4 color;								"
"void main()									"
"{												"
"	float totalAlpha = 0.f;"

"	float factor = 1.f;"

"   int colorstripX = int(colorstripUU.x);"
"   int colorstripY = int(colorstripUU.y);"

"	for( int i = 0 ; i < $EDGES ; i++ )"
"	{"

"		vec4 col = texelFetch(edgemapId, colorstripX )"
"			        * texelFetch(edgemapId, colorstripY );"

"		colorstripX += colorstripPitchX; "
"		colorstripY += colorstripPitchY; "

"		vec4 edge = texelFetch(edgemapId, int(texUV.x)*(segments-1)+i );"

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

"		vec4 col = texelFetch(edgemapId, colorstripX )"
"			        * texelFetch(edgemapId, colorstripY );"
"	float useFactor = factor*col.a;"
"	totalAlpha += useFactor;"

"    color.r = texelFetch(tintmapBufferId, int(tintmapUU.x) ).a"
"              * texelFetch(tintmapBufferId, int(tintmapUU.y) ).a"
"	           * totalAlpha;           "


"}";




/*
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
*/

}

#endif //WG_GLSHADERS_DOT_H
