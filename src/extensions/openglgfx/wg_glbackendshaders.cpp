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
#ifndef WG_GLSHADERS_DOT_H
#define WG_GLSHADERS_DOT_H

#include <wg_glbackend.h>  


// GLSL function evaluating a tint block (see TintTools::writeGpuTintBlock()) at a pixel center.

#define WG_GL_TINT_FUNC \
"vec4 evalTint(samplerBuffer buf, int ofs, vec2 pos)								" \
"{																					" \
"	int nLayers = int(texelFetch(buf, ofs).x);										" \
"	int p = ofs + 1;																" \
"	vec4 result = vec4(0.0);														" \
"	for (int l = 0; l < nLayers; l++)												" \
"	{																				" \
"		vec4 info = texelFetch(buf, p);												" \
"		vec4 geo = texelFetch(buf, p + 1);											" \
"		vec4 c = texelFetch(buf, p + 2);											" \
"		int N = int(info.z);														" \
"		if (N > 0)																	" \
"		{																			" \
"			float t = info.x < 0.5 ? geo.x * pos.x + geo.y * pos.y + geo.z : length((pos - geo.xy) * geo.zw);	" \
"			float fN = float(N);													" \
"			float fi = floor(clamp(t, -1e7, 1e7) * fN);								" \
"			int spread = int(info.y);												" \
"			if (spread == 0)														" \
"			{																		" \
"				if (fi >= 0.0)														" \
"					c = texelFetch(buf, p + 3 + int(min(fi, fN)));					" \
"			}																		" \
"			else if (spread == 1)													" \
"				c = texelFetch(buf, p + 3 + int(fi - fN * floor(fi / fN)));		" \
"			else																	" \
"			{																		" \
"				float r = fi - 2.0 * fN * floor(fi / (2.0 * fN));					" \
"				c = texelFetch(buf, p + 3 + int(r >= fN ? 2.0 * fN - 1.0 - r : r));	" \
"			}																		" \
"		}																			" \
"		result += c * info.w;														" \
"		p += 3 + (N > 0 ? N + 1 : 0);												" \
"	}																				" \
"	return result;																	" \
"}																					"

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
"flat out int tintOfs;  out vec2 tintPos;  "
"void main()                                               "
"{                                                         "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;             "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                   "
"   gl_Position.w = 1.0;                                   "
"   fragColor = texelFetch(colorBufferId, colorOfs);	   "
"   tintOfs = int(tintmapOfs.x);  tintPos = vec2(pos);  "
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
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   outColor = evalTint(tintmapBufferId, tintOfs, tintPos)"
"	           * fragColor;           "
"}                                      ";


const char GlBackend::fillFragmentShaderTintmap_A8[] =

"#version 330 core\n"
"uniform samplerBuffer tintmapBufferId;	"
"in vec4 fragColor;                     "
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"out vec4 outColor;                     "
"void main()                            "
"{                                      "
"   outColor.r = evalTint(tintmapBufferId, tintOfs, tintPos).a"
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
"flat out int tintOfs;  out vec2 tintPos;  "
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
"   tintOfs = int(tintmapOfs.x);  tintPos = vec2(pos);  "
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
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
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

"   vec4 fragColor = evalTint(tintmapBufferId, tintOfs, tintPos); "

"   color *= fragColor;"
"}												";




// Blur from a palette based source. Each tap is looked up in the palette
// before it is weighted, as paletteBlitNearestFragmentShader does it. The index
// texture is read with nearest sampling, so taps never blend indexes.

const char GlBackend::paletteBlurFragmentShader[] =

"#version 330 core\n"

"struct BlurInfo"
"{"
"	vec4   colorMtx[9];"
"	vec2   offset[9];"
"};"

"uniform BlurInfo blurInfo;                     "
"uniform sampler2D texId;						"
"uniform sampler2D paletteId;					"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"out vec4 color;								"

"void main()									"
"{												"
"	color = texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[0]).r,0.5f)) * blurInfo.colorMtx[0];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[1]).r,0.5f)) * blurInfo.colorMtx[1];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[2]).r,0.5f)) * blurInfo.colorMtx[2];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[3]).r,0.5f)) * blurInfo.colorMtx[3];"
"   color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[4]).r,0.5f)) * blurInfo.colorMtx[4];  "
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[5]).r,0.5f)) * blurInfo.colorMtx[5];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[6]).r,0.5f)) * blurInfo.colorMtx[6];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[7]).r,0.5f)) * blurInfo.colorMtx[7];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[8]).r,0.5f)) * blurInfo.colorMtx[8];"

"   color *= fragColor;"
"}												";


const char GlBackend::paletteBlurFragmentShaderTintmap[] =

"#version 330 core\n"

"struct BlurInfo"
"{"
"	vec4   colorMtx[9];"
"	vec2   offset[9];"
"};"

"uniform BlurInfo blurInfo;                     "
"uniform sampler2D texId;						"
"uniform sampler2D paletteId;					"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"out vec4 color;								"

"void main()									"
"{												"
"	color = texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[0]).r,0.5f)) * blurInfo.colorMtx[0];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[1]).r,0.5f)) * blurInfo.colorMtx[1];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[2]).r,0.5f)) * blurInfo.colorMtx[2];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[3]).r,0.5f)) * blurInfo.colorMtx[3];"
"   color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[4]).r,0.5f)) * blurInfo.colorMtx[4];  "
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[5]).r,0.5f)) * blurInfo.colorMtx[5];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[6]).r,0.5f)) * blurInfo.colorMtx[6];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[7]).r,0.5f)) * blurInfo.colorMtx[7];"
"	color += texture(paletteId, vec2(texture(texId, texUV + blurInfo.offset[8]).r,0.5f)) * blurInfo.colorMtx[8];"

"   vec4 fragColor = evalTint(tintmapBufferId, tintOfs, tintPos); "

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
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"out vec4 color;								"
"void main()									"
"{												"
"   vec4 fragColor = evalTint(tintmapBufferId, tintOfs, tintPos); "
"   color = texture(texId, texUV) * fragColor;  "
"}												";

const char GlBackend::blitFragmentShaderTintmap_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"out vec4 color;								"
"void main()									"
"{												"
"   float fragA = evalTint(tintmapBufferId, tintOfs, tintPos).a; "
"   color.r = texture(texId, texUV).a * fragA;  "
"}												";

const char GlBackend::alphaBlitFragmentShaderTintmap[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"out vec4 color;								"
"void main()									"
"{												"
"   color = evalTint(tintmapBufferId, tintOfs, tintPos); "
"   color.a *= texture(texId, texUV).r;         "
"}												";

const char GlBackend::alphaBlitFragmentShaderTintmap_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"out vec4 color;								"
"void main()									"
"{												"
"   float fragA = evalTint(tintmapBufferId, tintOfs, tintPos).a; "

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
"flat out int tintOfs;  out vec2 tintPos;  "
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
"   tintOfs = int(tintmapOfs.x);  tintPos = vec2(pos);  "
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
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"out vec4 color;								"
"void main()									"
"{												"
"   vec4 fragColor = evalTint(tintmapBufferId, tintOfs, tintPos); "

"   color = texture(paletteId, vec2(texture(texId, texUV).r,0.5f)) * fragColor;	"
"}												";

const char GlBackend::paletteBlitNearestFragmentShaderTintmap_A8[] =

"#version 330 core\n"

"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"uniform sampler2D paletteId;						"
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"out vec4 color;								"
"void main()									"
"{												"
"   float fragA = evalTint(tintmapBufferId, tintOfs, tintPos).a; "
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
"flat out int tintOfs;  out vec2 tintPos;  "
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
"   tintOfs = int(tintmapOfs.x);  tintPos = vec2(pos);  "
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
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
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

"   vec4 fragColor = evalTint(tintmapBufferId, tintOfs, tintPos); "

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
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
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

"   float fragA = evalTint(tintmapBufferId, tintOfs, tintPos).a; "

"   color.r = (out0 * (1-fract(uvFrac.y)) + out1 * fract(uvFrac.y)) * fragA;	"
"}												";


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
"flat out int tintOfs;  out vec2 tintPos;  "
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
"   tintOfs = int(tintmapOfs.x);  tintPos = vec2(pos);  "
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
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"flat in vec4 rect;						"
"out vec4 outColor;                     "
"void main()                            "
"{										"
"   vec4 inColor = evalTint(tintmapBufferId, tintOfs, tintPos)"
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
"flat in int tintOfs;  in vec2 tintPos;  " WG_GL_TINT_FUNC
"flat in vec4 rect;						"
"out vec4 outColor;                     "
"void main()                            "
"{										"
"   float inAlpha = evalTint(tintmapBufferId, tintOfs, tintPos).a"
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
"out vec2 texUV;										"
"flat out int edgemapPitch;								"
"flat out int flatColorsOfs;							"
"flat out int tintTableOfs;								"
"flat out int tintOfs;									"
"out vec2 tintPos;										"

"void main()											"
"{                                                      "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;              "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "

"   vec4 extras = texelFetch(extrasBufferId, extrasOfs);		"
"   edgemapPitch = int(extras.x);						"
"   flatColorsOfs = int(extras.y);						"
"   tintTableOfs = int(extras.z);						"
"   texUV = uv;											"
"   tintOfs = int(tintmapOfs.x);"
"   tintPos = vec2(pos);"
"}                                                      ";

// Segment colors are either flat or a tint in edgemap space. The edgemap buffer holds a
// table with the offset of each segment's tint block, -1 for flat colored segments.

#define WG_GL_SEGCOLOR_FUNC \
"vec4 segColor(int seg, int flatColorsOfs, int tintTableOfs, vec2 epos)			" \
"{																				" \
"	int tofs = int(texelFetch(edgemapId, tintTableOfs + seg).x);					" \
"	return tofs < 0 ? texelFetch(edgemapId, flatColorsOfs + seg) : evalTint(edgemapId, tofs, epos);	" \
"}																				"

const char GlBackend::segmentsFragmentShader[] =

"#version 330 core\n"
"uniform samplerBuffer tintmapBufferId;			"
"uniform samplerBuffer edgemapId;				"
"in vec2 texUV;									"
"flat in int edgemapPitch;						"
"flat in int flatColorsOfs;						"
"flat in int tintTableOfs;						"
"flat in int tintOfs;							"
"in vec2 tintPos;								"
WG_GL_TINT_FUNC
WG_GL_SEGCOLOR_FUNC

"out vec4 color;								"
"void main()									"
"{												"
"	float totalAlpha = 0.f;"
"	vec3	rgbAcc = vec3(0,0,0);"

"	float factor = 1.f;"

"	vec2 epos = vec2(floor(texUV.x) + 0.5, texUV.y + 0.5);"		// Pixel center in edgemap space.

"	for( int i = 0 ; i < $EDGES ; i++ )"
"	{"

"		vec4 col = segColor(i, flatColorsOfs, tintTableOfs, epos);"

"		vec4 edge = texelFetch(edgemapId, int(texUV.x)*edgemapPitch+i );"

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

"	vec4 col = segColor($EDGES, flatColorsOfs, tintTableOfs, epos);"

"	float useFactor = factor*col.a;"
"	totalAlpha += useFactor;"
"	rgbAcc += col.rgb * useFactor;"

"   col.a = totalAlpha; "
"   col.rgb = totalAlpha > 0.0 ? rgbAcc/totalAlpha : vec3(0.0);"

"   color = tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, tintPos) * col : col;"
"}";


const char GlBackend::segmentsFragmentShader_A8[] =

"#version 330 core\n"
"uniform samplerBuffer tintmapBufferId;	"
"uniform samplerBuffer edgemapId;				"
"in vec2 texUV;									"
"flat in int edgemapPitch;						"
"flat in int flatColorsOfs;						"
"flat in int tintTableOfs;						"
"flat in int tintOfs;							"
"in vec2 tintPos;								"
WG_GL_TINT_FUNC
WG_GL_SEGCOLOR_FUNC

"out vec4 color;								"
"void main()									"
"{												"
"	float totalAlpha = 0.f;"

"	float factor = 1.f;"

"	vec2 epos = vec2(floor(texUV.x) + 0.5, texUV.y + 0.5);"

"	for( int i = 0 ; i < $EDGES ; i++ )"
"	{"

"		vec4 col = segColor(i, flatColorsOfs, tintTableOfs, epos);"

"		vec4 edge = texelFetch(edgemapId, int(texUV.x)*edgemapPitch+i );"

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

"	vec4 col = segColor($EDGES, flatColorsOfs, tintTableOfs, epos);"
"	float useFactor = factor*col.a;"
"	totalAlpha += useFactor;"

"   color.r = (tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, tintPos).a : 1.0) * totalAlpha;"
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
