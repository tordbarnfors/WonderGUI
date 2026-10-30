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
#ifndef WAX_GLSHADERS_DOT_H
#define WAX_GLSHADERS_DOT_H

#include <wax_glbackend.h>  


// Uniform block shared by all shaders, see GlBackend::CanvasUBO. The vertex and fragment shaders
// of a program declare it the same way.

#define WAX_GL_CANVAS_BLOCK \
"layout(std140) uniform Canvas														" \
"{																					" \
"	float  canvasWidth;																" \
"	float  canvasHeight;															" \
"	float  canvasYOfs;																" \
"	float  canvasYMul;																" \
"	vec4   blendInfo;																" \
"};																					"

// Output of fragment shaders to canvases with color. Canvases with alpha hold premultiplied
// colors. A shader works out the premultiplied color of what it draws times the part of the
// pixel it covers (pc) and that part (cov). writeColor() passes them on as the blend mode needs
// them (see GlBackend::_setBlendMode()), as told by blendInfo:
//
//   x = 0: pc, with its alpha as the second color (Blend, Subtract and Multiply).
//   x = 1: pc times y (the morph factor), with the covered part times y as the second color, which
//          tells the blender how much of the canvas to keep (Replace and Morph). Canvases without
//          alpha (z set) keep the color of the result, which is the result divided by its alpha.
//          The canvas is opaque, so that alpha only depends on the source, and the result can be
//          divided by it here. A transparent Replace writes the color of the source, which
//          premultiplied is black.
//   x = 2: the source faded towards white by the alpha of pc (Min).
//   x = 3: pc with its strongest channel as alpha, by which Add, Invert and Max raise the alpha
//          of the canvas.

#define WAX_GL_OUTPUT \
WAX_GL_CANVAS_BLOCK \
"layout(location = 0, index = 0) out vec4 color;									" \
"layout(location = 0, index = 1) out vec4 colorFactor;								" \
"vec4 premul(vec4 c) { return vec4(c.rgb * c.a, c.a); }								" \
"void writeColor(vec4 pc, float cov)												" \
"{																					" \
"	if (blendInfo.x > 2.5)															" \
"	{																				" \
"		color = vec4(pc.rgb, max(pc.r, max(pc.g, pc.b)));							" \
"		colorFactor = vec4(0.0);													" \
"	}																				" \
"	else if (blendInfo.x > 1.5)														" \
"	{																				" \
"		color = vec4(vec3(1.0 - pc.a) + pc.rgb, 1.0);								" \
"		colorFactor = vec4(0.0);													" \
"	}																				" \
"	else if (blendInfo.x > 0.5)														" \
"	{																				" \
"		float m = blendInfo.y;														" \
"		float k = 1.0;																" \
"		if (blendInfo.z > 0.5)														" \
"		{																			" \
"			float resultAlpha = 1.0 - m + (cov > 0.0 ? pc.a / cov : 0.0) * m;		" \
"			k = resultAlpha > 0.0 ? 1.0 / resultAlpha : 0.0;						" \
"		}																			" \
"		color = pc * (m * k);														" \
"		colorFactor = vec4(cov * (1.0 - (1.0 - m) * k));							" \
"	}																				" \
"	else																			" \
"	{																				" \
"		color = pc;																	" \
"		colorFactor = vec4(pc.a);													" \
"	}																				" \
"}																					"

// Output of fragment shaders to Alpha_8 canvases, which hold alpha in the red channel. Paint
// modes work on alpha as on other canvases, effect modes take the alpha of the source as the
// value, like an opaque source of that color. pa is the alpha of what is drawn times the part
// of the pixel it covers, cov that part. blendInfo is as for WAX_GL_OUTPUT, but x = 1 is also
// used by Multiply.

#define WAX_GL_OUTPUT_A8 \
WAX_GL_CANVAS_BLOCK \
"layout(location = 0, index = 0) out vec4 color;									" \
"layout(location = 0, index = 1) out vec4 colorFactor;								" \
"void writeAlpha(float pa, float cov)												" \
"{																					" \
"	if (blendInfo.x > 1.5)															" \
"	{																				" \
"		color = vec4(1.0 - cov + pa);												" \
"		colorFactor = vec4(0.0);													" \
"	}																				" \
"	else if (blendInfo.x > 0.5)														" \
"	{																				" \
"		color = vec4(pa * blendInfo.y);												" \
"		colorFactor = vec4(cov * blendInfo.y);										" \
"	}																				" \
"	else																			" \
"	{																				" \
"		color = vec4(pa);															" \
"		colorFactor = vec4(pa);														" \
"	}																				" \
"}																					"

// GLSL function evaluating a tint block (see TintTools::writeGpuTintBlock()) at a pixel center.

#define WAX_GL_TINT_FUNC \
"vec4 evalTint(samplerBuffer buf, int ofs, vec2 pos)								" \
"{																					" \
"	int nLayers = int(texelFetch(buf, ofs).x);										" \
"	int p = ofs + 1;																" \
"	vec4 result = vec4(0.0);														" \
"	for (int l = 0; l < nLayers; l++)												" \
"	{																				" \
"		vec4 info = texelFetch(buf, p);												" \
"		vec4 geo = texelFetch(buf, p + 1);											" \
"		float srgb = texelFetch(buf, p + 2).x;										" \
"		int nStops = int(info.z);													" \
"		int posOfs = p + 3;															" \
"		int colOfs = posOfs + (nStops + 3) / 4;										" \
"		if (nStops > 0)																" \
"		{																			" \
"			float t = info.x < 0.5 ? geo.x * pos.x + geo.y * pos.y + geo.z : length((pos - geo.xy) * geo.zw);	" \
"			t = clamp(t, -1e7, 1e7);												" \
"			int spread = int(info.y);												" \
"			if (spread == 0)														" \
"				t = clamp(t, 0.0, 1.0);												" \
"			else if (spread == 1)													" \
"				t = t - floor(t);													" \
"			else																	" \
"			{																		" \
"				float r = t - 2.0 * floor(t * 0.5);									" \
"				t = r > 1.0 ? 2.0 - r : r;											" \
"			}																		" \
"			int k = -1;																" \
"			for (int i = 0; i < nStops; i++)										" \
"			{																		" \
"				if (texelFetch(buf, posOfs + i / 4)[i % 4] <= t)					" \
"					k = i;															" \
"			}																		" \
"			vec4 c;																	" \
"			if (k < 0)																" \
"				c = texelFetch(buf, colOfs);										" \
"			else if (k == nStops - 1)												" \
"				c = texelFetch(buf, colOfs + k);									" \
"			else																	" \
"			{																		" \
"				float p0 = texelFetch(buf, posOfs + k / 4)[k % 4];					" \
"				float p1 = texelFetch(buf, posOfs + (k + 1) / 4)[(k + 1) % 4];		" \
"				c = mix(texelFetch(buf, colOfs + k), texelFetch(buf, colOfs + k + 1), (t - p0) / (p1 - p0));	" \
"			}																		" \
"			if (srgb > 0.5)															" \
"			{																		" \
"				vec3 lo = c.rgb / 12.92;											" \
"				vec3 hi = pow((c.rgb + 0.055) / 1.055, vec3(2.4));					" \
"				c.rgb = mix(hi, lo, vec3(lessThanEqual(c.rgb, vec3(0.04045))));		" \
"			}																		" \
"			result += c * info.w;													" \
"		}																			" \
"		p = colOfs + nStops;														" \
"	}																				" \
"	return result;																	" \
"}																					"

// GLSL function reading a blit source as a premultiplied color, which is how textures with alpha
// hold their colors. A source without alpha (RGB8 texture) reads as opaque also outside the
// source, where the transparent border of a ClipBlit should make it transparent. So when told
// to (rgbxClip bit 0), the alpha is worked out from the position: 1 inside and 0 outside with
// nearest sampling, and the part of the bilinear footprint that lies inside with bilinear
// sampling (bit 1), which is what the sampler does with the colors. The colors are then
// premultiplied by that alpha, as the border outside the source is black. Needs uniform
// texId declared first.

#define WAX_GL_SOURCE_SAMPLE \
"uniform int rgbxClip;																" \
"vec4 sampleSource(vec2 uv)															" \
"{																					" \
"	vec4 c = texture(texId, uv);													" \
"	if ((rgbxClip & 1) != 0)														" \
"	{																				" \
"		if ((rgbxClip & 2) == 0)													" \
"			c.a = (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0)))) ? 0.0 : 1.0;	" \
"		else																		" \
"		{																			" \
"			vec2 size = vec2(textureSize(texId, 0));								" \
"			vec2 p = uv * size;														" \
"			vec2 coverage = clamp(min(p + 0.5, size - p + 0.5), 0.0, 1.0);			" \
"			c.a = coverage.x * coverage.y;											" \
"		}																			" \
"	}																				" \
"	return c;																		" \
"}																					"

// GLSL function looking up the palette entry for an index read from an R8 (Index_8) or R16
// (Index_16) index texture. Palette entries are premultiplied, see GlSurface::_uploadPalette(). The palette texture is up to 256 texels wide with as many rows as
// needed for paletteCapacity() entries (see GlSurface::_uploadPalette()), so the normalized
// index can't be used as a texture coordinate. Instead the index is turned into the coordinate
// of its texel's center, with paletteInfo set by GlBackend::_setPaletteInfo():
//
//   x: largest index of the index texture (255 or 65535), y: width, z: height, w: entries.
//
// Indexes beyond the palette get its last entry, like DX12 and Metal do. Needs uniform
// paletteId declared first.
//
// This samples with texture() rather than texelFetch() and textureSize(), which on a 2013
// Mac Pro (AMD, macOS OpenGL 4.1) gave random groups of black pixels.

#define WAX_GL_PALETTE_FUNC \
"uniform vec4 paletteInfo;															" \
"vec4 paletteLookup(float normIndex)												" \
"{																					" \
"	float index = min(floor(normIndex * paletteInfo.x + 0.5), paletteInfo.w - 1.0);	" \
"	float row = floor(index / paletteInfo.y);										" \
"	float column = index - row * paletteInfo.y;										" \
"	return texture(paletteId, vec2((column + 0.5) / paletteInfo.y, (row + 0.5) / paletteInfo.z));	" \
"}																					"

namespace wax {


const char GlBackend::fillVertexShader[] =

"#version 330 core\n"

WAX_GL_CANVAS_BLOCK

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

WAX_GL_CANVAS_BLOCK

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
WAX_GL_OUTPUT
"in vec4 fragColor;									"
"void main()										"
"{													"
"   writeColor(premul(fragColor), 1.0);				"
"}													";


const char GlBackend::fillFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"in vec4 fragColor;									"
"void main()										"
"{													"
"   writeAlpha(fragColor.a, 1.0);					"
"}													";

const char GlBackend::fillFragmentShaderTintmap[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform samplerBuffer tintmapBufferId;				"
"in vec4 fragColor;									"
"flat in int tintOfs;  in vec2 tintPos;  " WAX_GL_TINT_FUNC
"void main()										"
"{													"
"   writeColor(premul(evalTint(tintmapBufferId, tintOfs, tintPos) * fragColor), 1.0);	"
"}													";


const char GlBackend::fillFragmentShaderTintmap_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform samplerBuffer tintmapBufferId;				"
"in vec4 fragColor;									"
"flat in int tintOfs;  in vec2 tintPos;  " WAX_GL_TINT_FUNC
"void main()										"
"{													"
"   writeAlpha(evalTint(tintmapBufferId, tintOfs, tintPos).a * fragColor.a, 1.0);	"
"}													";



const char GlBackend::blitVertexShader[] =

"#version 330 core\n"

WAX_GL_CANVAS_BLOCK

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

WAX_GL_CANVAS_BLOCK

"uniform samplerBuffer colorBufferId;						   "
"layout(location = 2) in int colorOfs;                     "
"uniform samplerBuffer extrasBufferId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 3) in int extrasOfs;				       "
"layout(location = 4) in vec2 tintmapOfs;                  "
"out vec2 texUV;                                           "
"flat out int tintOfs;  out vec2 tintPos;  "
"flat out vec4 flatTint;                                   "
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
"   flatTint = texelFetch(colorBufferId, colorOfs);        "
"}                                                         ";





// Blur shaders, one per [source kind][tint][canvas]. Source kinds are ordinary sources,
// Alpha_8 sources (their R8 texture is read as white with alpha, like the alpha blits do) and
// palette based sources (each tap is looked up in the palette before it is weighted, the index
// texture is read with nearest sampling so taps never blend indexes). On an Alpha_8 canvas
// the resulting alpha is written to the red channel, like the other _A8 shaders do.

#define WAX_GL_BLUR_HEAD \
"#version 330 core\n" \
"struct BlurInfo { vec4 colorMtx[9]; vec2 offset[9]; };							" \
"uniform BlurInfo blurInfo;															" \
"uniform sampler2D texId;															" \
"in vec2 texUV;																		"

#define WAX_GL_BLUR_TAP \
"vec4 tap(vec2 uv) { return texture(texId, uv); }									"

#define WAX_GL_BLUR_TAP_ALPHA \
"vec4 tap(vec2 uv) { return vec4(texture(texId, uv).r); }							"

#define WAX_GL_BLUR_TAP_PALETTE \
"uniform sampler2D paletteId;														" WAX_GL_PALETTE_FUNC \
"vec4 tap(vec2 uv) { return paletteLookup(texture(texId, uv).r); }					"

#define WAX_GL_BLUR_FLAT \
"in vec4 fragColor;																	" \
"vec4 tintColor() { return fragColor; }												"

#define WAX_GL_BLUR_TINT \
"uniform samplerBuffer tintmapBufferId;												" \
"flat in int tintOfs;  in vec2 tintPos;  flat in vec4 flatTint;						" WAX_GL_TINT_FUNC \
"vec4 tintColor() { return evalTint(tintmapBufferId, tintOfs, tintPos) * flatTint; }"

// Taps are premultiplied, like the textures. The colors of the taps are blurred and alpha is
// taken from the center tap, which gives a straight color.

#define WAX_GL_BLUR_SUM \
"vec4 blurred()																		" \
"{																					" \
"	vec4 c = vec4(0.0);																" \
"	for (int i = 0; i < 9; i++)														" \
"	{																				" \
"		vec4 t = tap(texUV + blurInfo.offset[i]);									" \
"		c += vec4(t.a > 0.0 ? t.rgb / t.a : vec3(0.0), t.a) * blurInfo.colorMtx[i];	" \
"	}																				" \
"	return vec4(clamp(c.rgb, 0.0, 1.0), c.a);										" \
"}																					"

#define WAX_GL_BLUR_OUT \
WAX_GL_OUTPUT \
"void main() { writeColor(premul(blurred() * tintColor()), 1.0); }					"

#define WAX_GL_BLUR_OUT_A8 \
WAX_GL_OUTPUT_A8 \
"void main() { writeAlpha(blurred().a * tintColor().a, 1.0); }						"

#define WAX_GL_BLUR_SHADERS(TAP) \
	{ { WAX_GL_BLUR_HEAD TAP WAX_GL_BLUR_FLAT WAX_GL_BLUR_SUM WAX_GL_BLUR_OUT, WAX_GL_BLUR_HEAD TAP WAX_GL_BLUR_FLAT WAX_GL_BLUR_SUM WAX_GL_BLUR_OUT_A8 }, \
	  { WAX_GL_BLUR_HEAD TAP WAX_GL_BLUR_TINT WAX_GL_BLUR_SUM WAX_GL_BLUR_OUT, WAX_GL_BLUR_HEAD TAP WAX_GL_BLUR_TINT WAX_GL_BLUR_SUM WAX_GL_BLUR_OUT_A8 } }

const char * const GlBackend::blurFragmentShaders[3][2][2] =
{
	WAX_GL_BLUR_SHADERS(WAX_GL_BLUR_TAP),
	WAX_GL_BLUR_SHADERS(WAX_GL_BLUR_TAP_ALPHA),
	WAX_GL_BLUR_SHADERS(WAX_GL_BLUR_TAP_PALETTE)
};




const char GlBackend::blitFragmentShader[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform sampler2D texId;						" WAX_GL_SOURCE_SAMPLE
"in vec2 texUV;									"
"in vec4 fragColor;								"
"void main()									"
"{												"
"   writeColor(sampleSource(texUV) * premul(fragColor), 1.0);	"
"}												";

const char GlBackend::blitFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform sampler2D texId;						" WAX_GL_SOURCE_SAMPLE
"in vec2 texUV;									"
"in vec4 fragColor;								"
"void main()									"
"{												"
"   writeAlpha(sampleSource(texUV).a * fragColor.a, 1.0);		"
"}												";


const char GlBackend::alphaBlitFragmentShader[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform sampler2D texId;						"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"void main()									"
"{												"
"   writeColor(premul(fragColor) * texture(texId, texUV).r, 1.0);	"
"}												";

const char GlBackend::alphaBlitFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform sampler2D texId;						"
"in vec2 texUV;									"
"in vec4 fragColor;								"
"void main()									"
"{												"
"   writeAlpha(fragColor.a * texture(texId, texUV).r, 1.0);		"
"}												";


const char GlBackend::blitFragmentShaderTintmap[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform sampler2D texId;						" WAX_GL_SOURCE_SAMPLE
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " "flat in vec4 flatTint;  " WAX_GL_TINT_FUNC
"void main()									"
"{												"
"   vec4 fragColor = evalTint(tintmapBufferId, tintOfs, tintPos) * flatTint;	"
"   writeColor(sampleSource(texUV) * premul(fragColor), 1.0);	"
"}												";

const char GlBackend::blitFragmentShaderTintmap_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform sampler2D texId;						" WAX_GL_SOURCE_SAMPLE
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " "flat in vec4 flatTint;  " WAX_GL_TINT_FUNC
"void main()									"
"{												"
"   float fragA = (evalTint(tintmapBufferId, tintOfs, tintPos) * flatTint).a;	"
"   writeAlpha(sampleSource(texUV).a * fragA, 1.0);			"
"}												";

const char GlBackend::alphaBlitFragmentShaderTintmap[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " "flat in vec4 flatTint;  " WAX_GL_TINT_FUNC
"void main()									"
"{												"
"   vec4 fragColor = evalTint(tintmapBufferId, tintOfs, tintPos) * flatTint;	"
"   writeColor(premul(fragColor) * texture(texId, texUV).r, 1.0);	"
"}												";

const char GlBackend::alphaBlitFragmentShaderTintmap_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " "flat in vec4 flatTint;  " WAX_GL_TINT_FUNC
"void main()									"
"{												"
"   float fragA = (evalTint(tintmapBufferId, tintOfs, tintPos) * flatTint).a;	"
"   writeAlpha(fragA * texture(texId, texUV).r, 1.0);			"
"}												";


const char GlBackend::paletteBlitNearestVertexShader[] =

"#version 330 core\n"

WAX_GL_CANVAS_BLOCK

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

WAX_GL_CANVAS_BLOCK

"uniform samplerBuffer colorBufferId;						   "
"layout(location = 2) in int colorOfs;                     "
"uniform samplerBuffer extrasBufferId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 3) in int extrasOfs;                    "
"layout(location = 4) in vec2 tintmapOfs;                  "
"out vec2 texUV;                                           "
"flat out int tintOfs;  out vec2 tintPos;  "
"flat out vec4 flatTint;                                   "
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
"   flatTint = texelFetch(colorBufferId, colorOfs);        "
"}														";


const char GlBackend::paletteBlitNearestFragmentShader[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform sampler2D texId;						"
"uniform int clipToSource;					"	// Set for a ClipBlit: nothing outside the source is read.
"uniform sampler2D paletteId;					" WAX_GL_PALETTE_FUNC
"in vec2 texUV;									"
"in vec4 fragColor;								"
"float inside(vec2 uv)							"
"{												"
"   return (clipToSource != 0 && (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0))))) ? 0.0 : 1.0;	"
"}												"
"void main()									"
"{												"
"   writeColor(paletteLookup(texture(texId, texUV).r) * premul(fragColor) * inside(texUV), 1.0);	"
"}												";

const char GlBackend::paletteBlitNearestFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform sampler2D texId;						"
"uniform int clipToSource;					"	// Set for a ClipBlit: nothing outside the source is read.
"uniform sampler2D paletteId;						" WAX_GL_PALETTE_FUNC
"in vec2 texUV;									"
"in vec4 fragColor;								"
"float inside(vec2 uv)							"
"{												"
"   return (clipToSource != 0 && (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0))))) ? 0.0 : 1.0;	"
"}												"
"void main()									"
"{												"
"   writeAlpha(paletteLookup(texture(texId, texUV).r).a * fragColor.a * inside(texUV), 1.0);	"
"}												";


const char GlBackend::paletteBlitNearestFragmentShaderTintmap[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"uniform int clipToSource;					"	// Set for a ClipBlit: nothing outside the source is read.
"uniform sampler2D paletteId;					" WAX_GL_PALETTE_FUNC
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " "flat in vec4 flatTint;  " WAX_GL_TINT_FUNC
"float inside(vec2 uv)							"
"{												"
"   return (clipToSource != 0 && (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0))))) ? 0.0 : 1.0;	"
"}												"
"void main()									"
"{												"
"   vec4 fragColor = evalTint(tintmapBufferId, tintOfs, tintPos) * flatTint;	"
"   writeColor(paletteLookup(texture(texId, texUV).r) * premul(fragColor) * inside(texUV), 1.0);	"
"}												";

const char GlBackend::paletteBlitNearestFragmentShaderTintmap_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"uniform int clipToSource;					"	// Set for a ClipBlit: nothing outside the source is read.
"uniform sampler2D paletteId;						" WAX_GL_PALETTE_FUNC
"in vec2 texUV;									"
"flat in int tintOfs;  in vec2 tintPos;  " "flat in vec4 flatTint;  " WAX_GL_TINT_FUNC
"float inside(vec2 uv)							"
"{												"
"   return (clipToSource != 0 && (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0))))) ? 0.0 : 1.0;	"
"}												"
"void main()									"
"{												"
"   float fragA = (evalTint(tintmapBufferId, tintOfs, tintPos) * flatTint).a;	"
"   writeAlpha(paletteLookup(texture(texId, texUV).r).a * fragA * inside(texUV), 1.0);	"
"}												";



const char GlBackend::paletteBlitInterpolateVertexShader[] =

"#version 330 core\n"

WAX_GL_CANVAS_BLOCK

"uniform samplerBuffer extrasBufferId;						   "
"uniform samplerBuffer colorBufferId;					   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 2) in int colorOfs;                    "
"layout(location = 3) in int extrasOfs;                    "
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
"   fragColor = texelFetch(colorBufferId, colorOfs);				"
"}                                                         ";


const char GlBackend::paletteBlitInterpolateTintmapVertexShader[] =

"#version 330 core\n"

WAX_GL_CANVAS_BLOCK

"uniform samplerBuffer colorBufferId;						   "
"layout(location = 2) in int colorOfs;                     "
"uniform samplerBuffer extrasBufferId;						   "
"layout(location = 0) in ivec2 pos;                        "
"layout(location = 1) in vec2 texSize;					   "
"layout(location = 3) in int extrasOfs;                    "
"layout(location = 4) in vec2 tintmapOfs;                  "
"out vec2 uvFrac;                                         "
"flat out int tintOfs;  out vec2 tintPos;  "
"flat out vec4 flatTint;                                   "
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
"   tintOfs = int(tintmapOfs.x);  tintPos = vec2(pos);  "
"   flatTint = texelFetch(colorBufferId, colorOfs);        "
"}                                                         ";







const char GlBackend::paletteBlitInterpolateFragmentShader[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform sampler2D texId;						"
"uniform int clipToSource;					"	// Set for a ClipBlit: nothing outside the source is read.
"uniform sampler2D paletteId;						" WAX_GL_PALETTE_FUNC
"in vec2 uvFrac;								"
"in vec4 fragColor;								"
"float inside(vec2 uv)							"
"{												"
"   return (clipToSource != 0 && (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0))))) ? 0.0 : 1.0;	"
"}												"
"void main()									"
"{												"
"   vec2 uvBase = floor(uvFrac);	"		// Texels and weights from the same value, so they always agree.
"   vec2 uvf = uvFrac - uvBase;	"
"   vec2 texSize = vec2(textureSize(texId, 0));	"
"   vec2 texUV00 = (uvBase + 0.5) / texSize;	"		// Centers of the texels, which no rounding error can move.
"   vec2 texUV11 = (uvBase + 1.5) / texSize;	"
"   float index00 = texture(texId, texUV00).r;		"
"   float index01 = texture(texId, vec2(texUV11.x,texUV00.y) ).r;		"
"   float index10 = texture(texId, vec2(texUV00.x,texUV11.y) ).r;		"
"   float index11 = texture(texId, texUV11).r;		"
"   vec4 color00 = paletteLookup(index00) * inside(texUV00);	"
"   vec4 color01 = paletteLookup(index01) * inside(vec2(texUV11.x,texUV00.y));	"
"   vec4 color10 = paletteLookup(index10) * inside(vec2(texUV00.x,texUV11.y));	"
"   vec4 color11 = paletteLookup(index11) * inside(texUV11);	"
"   vec4 out0 = color00 * (1-uvf.x) + color01 * uvf.x;	"
"   vec4 out1 = color10 * (1-uvf.x) + color11 * uvf.x;	"
"   vec4 sampled = out0 * (1-uvf.y) + out1 * uvf.y;	"
"   writeColor(sampled * premul(fragColor), 1.0);	"
"}												";

const char GlBackend::paletteBlitInterpolateFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform sampler2D texId;						"
"uniform int clipToSource;					"	// Set for a ClipBlit: nothing outside the source is read.
"uniform sampler2D paletteId;						" WAX_GL_PALETTE_FUNC
"in vec2 uvFrac;								"
"in vec4 fragColor;								"
"float inside(vec2 uv)							"
"{												"
"   return (clipToSource != 0 && (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0))))) ? 0.0 : 1.0;	"
"}												"
"void main()									"
"{												"
"   vec2 uvBase = floor(uvFrac);	"		// Texels and weights from the same value, so they always agree.
"   vec2 uvf = uvFrac - uvBase;	"
"   vec2 texSize = vec2(textureSize(texId, 0));	"
"   vec2 texUV00 = (uvBase + 0.5) / texSize;	"		// Centers of the texels, which no rounding error can move.
"   vec2 texUV11 = (uvBase + 1.5) / texSize;	"
"   float index00 = texture(texId, texUV00).r;		"
"   float index01 = texture(texId, vec2(texUV11.x,texUV00.y) ).r;		"
"   float index10 = texture(texId, vec2(texUV00.x,texUV11.y) ).r;		"
"   float index11 = texture(texId, texUV11).r;		"
"   vec4 color00 = paletteLookup(index00) * inside(texUV00);	"
"   vec4 color01 = paletteLookup(index01) * inside(vec2(texUV11.x,texUV00.y));	"
"   vec4 color10 = paletteLookup(index10) * inside(vec2(texUV00.x,texUV11.y));	"
"   vec4 color11 = paletteLookup(index11) * inside(texUV11);	"
"   vec4 out0 = color00 * (1-uvf.x) + color01 * uvf.x;	"
"   vec4 out1 = color10 * (1-uvf.x) + color11 * uvf.x;	"
"   vec4 sampled = out0 * (1-uvf.y) + out1 * uvf.y;	"
"   writeAlpha(sampled.a * fragColor.a, 1.0);	"
"}												";


const char GlBackend::paletteBlitInterpolateFragmentShaderTintmap[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"uniform int clipToSource;					"	// Set for a ClipBlit: nothing outside the source is read.
"uniform sampler2D paletteId;						" WAX_GL_PALETTE_FUNC
"in vec2 uvFrac;								"
"flat in int tintOfs;  in vec2 tintPos;  " "flat in vec4 flatTint;  " WAX_GL_TINT_FUNC
"float inside(vec2 uv)							"
"{												"
"   return (clipToSource != 0 && (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0))))) ? 0.0 : 1.0;	"
"}												"
"void main()									"
"{												"
"   vec2 uvBase = floor(uvFrac);	"		// Texels and weights from the same value, so they always agree.
"   vec2 uvf = uvFrac - uvBase;	"
"   vec2 texSize = vec2(textureSize(texId, 0));	"
"   vec2 texUV00 = (uvBase + 0.5) / texSize;	"		// Centers of the texels, which no rounding error can move.
"   vec2 texUV11 = (uvBase + 1.5) / texSize;	"
"   float index00 = texture(texId, texUV00).r;		"
"   float index01 = texture(texId, vec2(texUV11.x,texUV00.y) ).r;		"
"   float index10 = texture(texId, vec2(texUV00.x,texUV11.y) ).r;		"
"   float index11 = texture(texId, texUV11).r;		"
"   vec4 color00 = paletteLookup(index00) * inside(texUV00);	"
"   vec4 color01 = paletteLookup(index01) * inside(vec2(texUV11.x,texUV00.y));	"
"   vec4 color10 = paletteLookup(index10) * inside(vec2(texUV00.x,texUV11.y));	"
"   vec4 color11 = paletteLookup(index11) * inside(texUV11);	"
"   vec4 out0 = color00 * (1-uvf.x) + color01 * uvf.x;	"
"   vec4 out1 = color10 * (1-uvf.x) + color11 * uvf.x;	"
"   vec4 sampled = out0 * (1-uvf.y) + out1 * uvf.y;	"
"   vec4 fragColor = evalTint(tintmapBufferId, tintOfs, tintPos) * flatTint;	"
"   writeColor(sampled * premul(fragColor), 1.0);	"
"}												";

const char GlBackend::paletteBlitInterpolateFragmentShaderTintmap_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform sampler2D texId;						"
"uniform samplerBuffer tintmapBufferId;			"
"uniform int clipToSource;					"	// Set for a ClipBlit: nothing outside the source is read.
"uniform sampler2D paletteId;						" WAX_GL_PALETTE_FUNC
"in vec2 uvFrac;								"
"flat in int tintOfs;  in vec2 tintPos;  " "flat in vec4 flatTint;  " WAX_GL_TINT_FUNC
"float inside(vec2 uv)							"
"{												"
"   return (clipToSource != 0 && (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0))))) ? 0.0 : 1.0;	"
"}												"
"void main()									"
"{												"
"   vec2 uvBase = floor(uvFrac);	"		// Texels and weights from the same value, so they always agree.
"   vec2 uvf = uvFrac - uvBase;	"
"   vec2 texSize = vec2(textureSize(texId, 0));	"
"   vec2 texUV00 = (uvBase + 0.5) / texSize;	"		// Centers of the texels, which no rounding error can move.
"   vec2 texUV11 = (uvBase + 1.5) / texSize;	"
"   float index00 = texture(texId, texUV00).r;		"
"   float index01 = texture(texId, vec2(texUV11.x,texUV00.y) ).r;		"
"   float index10 = texture(texId, vec2(texUV00.x,texUV11.y) ).r;		"
"   float index11 = texture(texId, texUV11).r;		"
"   vec4 color00 = paletteLookup(index00) * inside(texUV00);	"
"   vec4 color01 = paletteLookup(index01) * inside(vec2(texUV11.x,texUV00.y));	"
"   vec4 color10 = paletteLookup(index10) * inside(vec2(texUV00.x,texUV11.y));	"
"   vec4 color11 = paletteLookup(index11) * inside(texUV11);	"
"   vec4 out0 = color00 * (1-uvf.x) + color01 * uvf.x;	"
"   vec4 out1 = color10 * (1-uvf.x) + color11 * uvf.x;	"
"   vec4 sampled = out0 * (1-uvf.y) + out1 * uvf.y;	"
"   float fragA = (evalTint(tintmapBufferId, tintOfs, tintPos) * flatTint).a;	"
"   writeAlpha(sampled.a * fragA, 1.0);	"
"}												";


const char GlBackend::lineFromToVertexShader[] =

"#version 330 core\n"

WAX_GL_CANVAS_BLOCK

"uniform samplerBuffer colorBufferId;					"
"uniform samplerBuffer extrasBufferId;					"

"layout(location = 0) in ivec2 pos;                     "
"layout(location = 2) in int colorOfs;                  "
"layout(location = 3) in int extrasOfs;                 "
"layout(location = 4) in vec2 tintmapOfs;               "

"out vec4 fragColor;                                    "
"flat out int tintOfs;									"
"out vec2 tintPos;										"
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
"   tintOfs = int(tintmapOfs.x);						"
"   tintPos = vec2(pos);								"
"}                                                      ";


const char GlBackend::lineFromToFragmentShader[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform samplerBuffer tintmapBufferId;	"
"in vec4 fragColor;                     "
"flat in int tintOfs;					"
"in vec2 tintPos;						"
"flat in float s;							"
"flat in float w;							"
"flat in float slope;						"
"flat in float ifSteep;						"
"flat in float ifMild;						"
WAX_GL_TINT_FUNC
"float coverage() { return clamp(w - abs(gl_FragCoord.x*ifSteep + gl_FragCoord.y*ifMild - s - (gl_FragCoord.x*ifMild + gl_FragCoord.y*ifSteep) * slope), 0.0, 1.0); }	"
"vec4 lineColor() { return tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, tintPos) * fragColor : fragColor; }	"
"void main()                            "
"{										"
"   float cov = coverage();				"
"   writeColor(premul(lineColor()) * cov, cov);	"
"}                                      ";

const char GlBackend::lineFromToFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform samplerBuffer tintmapBufferId;	"
"in vec4 fragColor;                     "
"flat in int tintOfs;					"
"in vec2 tintPos;						"
"flat in float s;							"
"flat in float w;							"
"flat in float slope;						"
"flat in float ifSteep;						"
"flat in float ifMild;						"
WAX_GL_TINT_FUNC
"float coverage() { return clamp(w - abs(gl_FragCoord.x*ifSteep + gl_FragCoord.y*ifMild - s - (gl_FragCoord.x*ifMild + gl_FragCoord.y*ifSteep) * slope), 0.0, 1.0); }	"
"vec4 lineColor() { return tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, tintPos) * fragColor : fragColor; }	"
"void main()                            "
"{										"
"   float cov = coverage();				"
"   writeAlpha(lineColor().a * cov, cov);	"
"}                                      ";



const char GlBackend::aaFillVertexShader[] =

"#version 330 core\n"

WAX_GL_CANVAS_BLOCK

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

WAX_GL_CANVAS_BLOCK

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
WAX_GL_OUTPUT
"in vec4 fragColor;						"
"flat in vec4 rect;						"
"void main()                            "
"{										"
"	vec2 middleofs = abs(gl_FragCoord.xy - rect.xy);   "
"	vec2 coverage = clamp(rect.zw  - middleofs, 0.f, 1.f);  "
"	float cov = coverage.x * coverage.y;	"
"	writeColor(premul(fragColor) * cov, cov);	"
"}                                      ";

const char GlBackend::aaFillFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"in vec4 fragColor;						"
"flat in vec4 rect;						"
"void main()                            "
"{										"
"	vec2 middleofs = abs(gl_FragCoord.xy - rect.xy);   "
"	vec2 coverage = clamp(rect.zw  - middleofs, 0.f, 1.f);  "
"	float cov = coverage.x * coverage.y;	"
"	writeAlpha(fragColor.a * cov, cov);	"
"}                                      ";


const char GlBackend::aaFillFragmentShaderTintmap[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform samplerBuffer tintmapBufferId;	"
"in vec4 fragColor;						"
"flat in int tintOfs;  in vec2 tintPos;  " WAX_GL_TINT_FUNC
"flat in vec4 rect;						"
"void main()                            "
"{										"
"	vec2 middleofs = abs(gl_FragCoord.xy - rect.xy);   "
"	vec2 coverage = clamp(rect.zw  - middleofs, 0.f, 1.f);  "
"	float cov = coverage.x * coverage.y;	"
"	writeColor(premul(evalTint(tintmapBufferId, tintOfs, tintPos) * fragColor) * cov, cov);	"
"}                                      ";

const char GlBackend::aaFillFragmentShaderTintmap_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform samplerBuffer tintmapBufferId;	"
"in vec4 fragColor;						"
"flat in int tintOfs;  in vec2 tintPos;  " WAX_GL_TINT_FUNC
"flat in vec4 rect;						"
"void main()                            "
"{										"
"	vec2 middleofs = abs(gl_FragCoord.xy - rect.xy);   "
"	vec2 coverage = clamp(rect.zw  - middleofs, 0.f, 1.f);  "
"	float cov = coverage.x * coverage.y;	"
"	writeAlpha(evalTint(tintmapBufferId, tintOfs, tintPos).a * fragColor.a * cov, cov);	"
"}                                      ";


const char GlBackend::segmentsVertexShader[] =

"#version 330 core\n"

WAX_GL_CANVAS_BLOCK

"uniform samplerBuffer extrasBufferId;					"
"uniform samplerBuffer colorBufferId;					"
"layout(location = 2) in int colorOfs;                   "
"layout(location = 0) in ivec2 pos;                     "
"layout(location = 1) in vec2 uv;					    "
"layout(location = 3) in int extrasOfs;                 "
"layout(location = 4) in vec2 tintmapOfs;               "
"out vec2 mapUV;										"
"flat out vec2 tileSize;								"
"flat out int edgemapPitch;								"
"flat out int flatColorsOfs;							"
"flat out int tintTableOfs;								"
"flat out int tintOfs;									"
"out vec2 tintPos;										"
"flat out vec4 flatTint;								"

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
"   tileSize = texelFetch(extrasBufferId, extrasOfs + 1).xy;	"
"   mapUV = uv;											"
"   tintOfs = int(tintmapOfs.x);"
"   tintPos = vec2(pos);"
"   flatTint = texelFetch(colorBufferId, colorOfs);"
"}                                                      ";

// Segment colors are either flat or a tint in edgemap space. The segments together cover the
// whole pixel, each by the part of it between its edges. The edgemap buffer holds a
// table with the offset of each segment's tint block, -1 for flat colored segments.

#define WAX_GL_SEGCOLOR_FUNC \
"vec4 segColor(int seg, int flatColorsOfs, int tintTableOfs, vec2 epos)			" \
"{																				" \
"	int tofs = int(texelFetch(edgemapId, tintTableOfs + seg).x);					" \
"	return tofs < 0 ? texelFetch(edgemapId, flatColorsOfs + seg) : evalTint(edgemapId, tofs, epos);	" \
"}																				"

// Position in the edgemap of a pixel of a tiled edgemap, which may be outside of it, wrapped
// into it. uv.y is half a pixel back, so it is whole at pixel centers. tileSize is the size
// of the edgemap in columns and pixels, zero when not tiling.

#define WAX_GL_WRAP_FUNC \
"vec2 wrapUV(vec2 uv, vec2 tileSize)											" \
"{																				" \
"	if (tileSize.x > 0.0)														" \
"	{																			" \
"		vec2 cell = vec2(floor(uv.x), floor(uv.y + 0.5));						" \
"		uv += mod(cell, tileSize) - cell;										" \
"	}																			" \
"	return uv;																	" \
"}																				"

// A segment's premultiplied color pc covers part of the pixel. Usually it just counts by that
// part. On a canvas without alpha, Replace and Morph keep the color of what they give each
// segment, which is (canvas * (1 - m) + pc * m) / (1 - m + pc.a * m), where m is the morph
// factor (1 for Replace). That is averaged by area, so each segment adds its part of the second
// term and of how much of the canvas is kept (keep). A segment whose result is transparent gives
// black, like a transparent Replace does. Needs WAX_GL_OUTPUT.

#define WAX_GL_SEGPART_FUNC \
"bool keepsResultColors() { return blendInfo.x > 0.5 && blendInfo.x < 1.5 && blendInfo.z > 0.5; }	" \
"vec4 segPart(vec4 pc, float part, inout float keep)								" \
"{																				" \
"	if (keepsResultColors())													" \
"	{																			" \
"		float m = blendInfo.y;													" \
"		float resultAlpha = 1.0 - m + pc.a * m;									" \
"		if (resultAlpha <= 0.0)													" \
"			return vec4(0.0);													" \
"		keep += part * (1.0 - m) / resultAlpha;									" \
"		return pc * (part * m / resultAlpha);									" \
"	}																			" \
"	return pc * part;															" \
"}																				"

const char GlBackend::segmentsFragmentShader[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform samplerBuffer tintmapBufferId;			"
"uniform samplerBuffer edgemapId;				"
"in vec2 mapUV;									"
"flat in vec2 tileSize;							"
"flat in int edgemapPitch;						"
"flat in int flatColorsOfs;						"
"flat in int tintTableOfs;						"
"flat in int tintOfs;							"
"in vec2 tintPos;								"
"flat in vec4 flatTint;							"
WAX_GL_TINT_FUNC
WAX_GL_SEGCOLOR_FUNC
WAX_GL_WRAP_FUNC
WAX_GL_SEGPART_FUNC
"void main()									"
"{												"
"	vec2 texUV = wrapUV(mapUV, tileSize);"
"	vec4 tint = premul((tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, tintPos) : vec4(1.0)) * flatTint);"
"	vec4 acc = vec4(0.0);"
"	float keep = 0.0;"
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
"		float factor2 = min(clamp(x + adder, 0.f, 1.f), factor);"		// An edge above the one before is pushed down to it.
"		acc += segPart(premul(col) * tint, factor - factor2, keep);"
"		factor = factor2;"
"	}"
"	vec4 col = segColor($EDGES, flatColorsOfs, tintTableOfs, epos);"
"	acc += segPart(premul(col) * tint, factor, keep);"
"	if (keepsResultColors())"
"	{"
"		color = acc;"
"		colorFactor = vec4(1.0 - keep);"
"	}"
"	else"
"		writeColor(acc, 1.0);"
"}";


const char GlBackend::segmentsFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform samplerBuffer tintmapBufferId;			"
"uniform samplerBuffer edgemapId;				"
"in vec2 mapUV;									"
"flat in vec2 tileSize;							"
"flat in int edgemapPitch;						"
"flat in int flatColorsOfs;						"
"flat in int tintTableOfs;						"
"flat in int tintOfs;							"
"in vec2 tintPos;								"
"flat in vec4 flatTint;							"
WAX_GL_TINT_FUNC
WAX_GL_SEGCOLOR_FUNC
WAX_GL_WRAP_FUNC
"void main()									"
"{												"
"	vec2 texUV = wrapUV(mapUV, tileSize);"
"	vec4 acc = vec4(0.0);"
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
"		float factor2 = min(clamp(x + adder, 0.f, 1.f), factor);"		// An edge above the one before is pushed down to it.
"		acc += vec4(col.rgb * col.a, col.a) * (factor - factor2);"
"		factor = factor2;"
"	}"
"	vec4 col = segColor($EDGES, flatColorsOfs, tintTableOfs, epos);"
"	acc += vec4(col.rgb * col.a, col.a) * factor;"
"	vec4 tint = (tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, tintPos) : vec4(1.0)) * flatTint;"
"	writeAlpha(acc.a * tint.a, 1.0);"
"}";


// Shapes (Command::DrawShape), see GpuShapes for the entries read from the extras buffer. The
// coverage of each pixel by each segment is worked out from the geometry. The premultiplied
// colors of the drawn segments are summed by the part of the pixel they cover, and drawn as
// covering the part of the pixel they cover together. Pixels not covered by any drawn segment
// are left alone.

const char GlBackend::shapeVertexShader[] =

"#version 330 core\n"

WAX_GL_CANVAS_BLOCK

"uniform samplerBuffer colorBufferId;					"
"layout(location = 0) in ivec2 pos;                     "
"layout(location = 2) in int colorOfs;                  "
"layout(location = 3) in int extrasOfs;                 "
"layout(location = 4) in vec2 tintmapOfs;               "
"flat out int shapeOfs;									"
"flat out int tintOfs;									"
"out vec2 canvasPos;									"
"flat out vec4 flatTint;								"

"void main()											"
"{                                                      "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;              "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "
"   shapeOfs = extrasOfs;								"
"   tintOfs = int(tintmapOfs.x);						"
"   canvasPos = vec2(pos);								"
"   flatTint = texelFetch(colorBufferId, colorOfs);		"
"}                                                      ";

// Coverage of the segments of a shape at a pixel center, accumulated as color times alpha
// and alpha, weighted by coverage, in acc, and the coverage of drawn segments in covered.
// Needs extrasBufferId, colorBufferId and WAX_GL_TINT_FUNC.

#define WAX_GL_SHAPE_FUNCS \
"vec4 shapeEntry(int i)																" \
"{																					" \
"	return texelFetch(extrasBufferId, i);											" \
"}																					" \
"float spanCoverage(float a, float b, float p)										" \
"{																					" \
"	return clamp(min(p + 0.5, b) - max(p - 0.5, a), 0.0, 1.0);						" \
"}																					" \
"float rectCoverage(vec4 r, vec2 p)													" \
"{																					" \
"	return spanCoverage(r.x, r.z, p.x) * spanCoverage(r.y, r.w, p.y);				" \
"}																					" \
"float halfPlaneCoverage(float s, vec2 n)											" \
"{																					" \
"	vec2 an = abs(n);																" \
"	float a = max(an.x, an.y);														" \
"	float b = min(an.x, an.y);														" \
"	if (b < 0.001)																	" \
"		return clamp(s + 0.5, 0.0, 1.0);											" \
"	float t = s + 0.5 * (a + b);													" \
"	if (t <= 0.0)																	" \
"		return 0.0;																	" \
"	if (t >= a + b)																	" \
"		return 1.0;																	" \
"	if (t < b)																		" \
"		return t * t / (2.0 * a * b);												" \
"	if (t > a)																		" \
"		return 1.0 - (a + b - t) * (a + b - t) / (2.0 * a * b);						" \
"	return (t - 0.5 * b) / a;														" \
"}																					" \
"float circleCoverage(float radius, vec2 v, float d)								" \
"{																					" \
"	if (radius <= 0.0)																" \
"		return 0.0;																	" \
"	return d > 0.001 ? halfPlaneCoverage(radius - d, v / d) : clamp(radius + 0.5, 0.0, 1.0);	" \
"}																					" \
"float roundedRectCoverage(vec4 r, vec4 radii, vec2 p)								" \
"{																					" \
"	float c = rectCoverage(r, p);													" \
"	vec2 center;																	" \
"	float radius;																	" \
"	if (radii.x > 0.0 && p.x < r.x + radii.x && p.y < r.y + radii.x)				" \
"	{																				" \
"		radius = radii.x;															" \
"		center = vec2(r.x + radius, r.y + radius);									" \
"	}																				" \
"	else if (radii.y > 0.0 && p.x > r.z - radii.y && p.y < r.y + radii.y)			" \
"	{																				" \
"		radius = radii.y;															" \
"		center = vec2(r.z - radius, r.y + radius);									" \
"	}																				" \
"	else if (radii.z > 0.0 && p.x > r.z - radii.z && p.y > r.w - radii.z)			" \
"	{																				" \
"		radius = radii.z;															" \
"		center = vec2(r.z - radius, r.w - radius);									" \
"	}																				" \
"	else if (radii.w > 0.0 && p.x < r.x + radii.w && p.y > r.w - radii.w)			" \
"	{																				" \
"		radius = radii.w;															" \
"		center = vec2(r.x + radius, r.w - radius);									" \
"	}																				" \
"	else																			" \
"		return c;																	" \
"	vec2 v = p - center;															" \
"	return min(c, circleCoverage(radius, v, length(v)));							" \
"}																					" \
"void addSegment(int seg, float c, vec2 p, inout vec4 acc, inout float covered)		" \
"{																					" \
"	if (c <= 0.0)																	" \
"		return;																		" \
"	vec4 s = shapeEntry(seg);														" \
"	if (s.x < 0.0)																	" \
"		return;																		" \
"	vec4 col = texelFetch(colorBufferId, int(s.x));									" \
"	if (s.y >= 0.0)																	" \
"		col *= evalTint(colorBufferId, int(s.y), p);								" \
"	acc += vec4(col.rgb * col.a, col.a) * c;										" \
"	covered += c;																	" \
"}																					" \
"void boxCoverage(int ofs, vec2 p, inout vec4 acc, inout float covered)				" \
"{																					" \
"	float outer = roundedRectCoverage(shapeEntry(ofs + 1), shapeEntry(ofs + 2), p);	" \
"	float fill = outer;																" \
"	if ((int(shapeEntry(ofs).y) & 1) != 0)											" \
"		fill = min(outer, roundedRectCoverage(shapeEntry(ofs + 3), shapeEntry(ofs + 4), p));	" \
"	addSegment(ofs + 5, fill, p, acc, covered);										" \
"	addSegment(ofs + 6, outer - fill, p, acc, covered);								" \
"}																					" \
"float turnsFromTop(vec2 v)															" \
"{																					" \
"	vec2 a = abs(v);																" \
"	float t = a.x <= a.y ? atan(a.x / max(a.y, 1e-20)) : 1.5707963 - atan(a.y / max(a.x, 1e-20));	" \
"	t *= 0.15915494;																" \
"	if (v.y >= 0.0)																	" \
"		t = 0.5 - t;																" \
"	if (v.x < 0.0)																	" \
"		t = 1.0 - t;																" \
"	return fract(t);																" \
"}																					" \
"float squareSweep(vec2 v, float position)											" \
"{																					" \
"	vec2 u = vec2(sin(6.2831853 * position), -cos(6.2831853 * position));			" \
"	float left = 0.5 - v.x;															" \
"	float right = 0.5 + v.x;														" \
"	float top = 0.5 - v.y;															" \
"	float bottom = 0.5 + v.y;														" \
"	float tx = (u.x > 0.0 ? right : left) / max(abs(u.x), 1e-20);					" \
"	float ty = (u.y > 0.0 ? bottom : top) / max(abs(u.y), 1e-20);					" \
"	vec2 q = u * min(tx, ty);														" \
"	float a;																		" \
"	if (ty <= tx && u.y < 0.0 && position < 0.5)									" \
"		a = top * q.x;																" \
"	else if (tx < ty && u.x > 0.0)													" \
"		a = top * right + right * (top + q.y);										" \
"	else if (ty <= tx && u.y > 0.0)													" \
"		a = top * right + right * (top + bottom) + bottom * (right - q.x);			" \
"	else if (tx < ty)																" \
"		a = top * right + right * (top + bottom) + bottom * (right + left) + left * (bottom - q.y);	" \
"	else																			" \
"		a = 2.0 + top * q.x;														" \
"	return 0.5 * a;																	" \
"}																					" \
"void ringElements(int ofs, int n, int base, float band, vec2 p, vec2 v, float d, inout vec4 acc, inout float covered)	" \
"{																					" \
"	if (n == 1)																		" \
"	{																				" \
"		addSegment(ofs + int(shapeEntry(base).y), band, p, acc, covered);			" \
"		return;																		" \
"	}																				" \
"	if (abs(v.x) < 0.5 && abs(v.y) < 0.5)											" \
"	{																				" \
"		float firstSweep = squareSweep(v, shapeEntry(base).x);						" \
"		float sweep = firstSweep;													" \
"		for (int e = 0; e < n; e++)													" \
"		{																			" \
"			float nextSweep = e + 1 < n ? squareSweep(v, shapeEntry(base + e + 1).x) : firstSweep + 1.0;	" \
"			addSegment(ofs + int(shapeEntry(base + e).y), band * max(nextSweep - sweep, 0.0), p, acc, covered);	" \
"			sweep = nextSweep;														" \
"		}																			" \
"		return;																		" \
"	}																				" \
"	float theta = d > 0.0001 ? turnsFromTop(v) : 0.0;								" \
"	int lo = 0;																		" \
"	int hi = n;																		" \
"	while (lo < hi)																	" \
"	{																				" \
"		int mid = (lo + hi) / 2;													" \
"		if (shapeEntry(base + mid).x <= theta)										" \
"			lo = mid + 1;															" \
"		else																		" \
"			hi = mid;																" \
"	}																				" \
"	int first = lo > 0 ? lo - 1 : n - 1;											" \
"	float w = min(0.25, 0.70710678 / (6.2831853 * max(d, 0.0001)));					" \
"	float behind = fract(theta - shapeEntry(base + first).x);						" \
"	for (int i = 0; i < n - 1 && behind < w; i++)									" \
"	{																				" \
"		int prev = first > 0 ? first - 1 : n - 1;									" \
"		behind += fract(shapeEntry(base + first).x - shapeEntry(base + prev).x);	" \
"		first = prev;																" \
"	}																				" \
"	float g = 1.0;																	" \
"	float ahead = -behind;															" \
"	int k = first;																	" \
"	vec4 element = shapeEntry(base + k);											" \
"	for (int i = 0; i < n; i++)														" \
"	{																				" \
"		int next = k + 1 < n ? k + 1 : 0;											" \
"		vec4 nextElement = shapeEntry(base + next);									" \
"		ahead += fract(nextElement.x - element.x);									" \
"		float gEnd = 0.0;															" \
"		if (i < n - 1 && ahead < w)													" \
"		{																			" \
"			float a = 6.2831853 * nextElement.x;									" \
"			gEnd = min(g, halfPlaneCoverage(-d * sin(6.2831853 * ahead), vec2(cos(a), sin(a))));	" \
"		}																			" \
"		addSegment(ofs + int(element.y), band * (g - gEnd), p, acc, covered);		" \
"		if (gEnd <= 0.0)															" \
"			break;																	" \
"		g = gEnd;																	" \
"		k = next;																	" \
"		element = nextElement;														" \
"	}																				" \
"}																					" \
"void ringCoverage(int ofs, vec2 p, inout vec4 acc, inout float covered)			" \
"{																					" \
"	vec4 head = shapeEntry(ofs);													" \
"	int flags = int(head.y);														" \
"	vec4 circles = shapeEntry(ofs + 1);												" \
"	vec4 inner = shapeEntry(ofs + 2);												" \
"	vec4 rect = shapeEntry(ofs + 3);												" \
"	vec2 v = p - circles.xy;														" \
"	float d = length(v);															" \
"	bool bSquare = (flags & 1) != 0;												" \
"	float c0 = bSquare ? rectCoverage(rect, p) : circleCoverage(circles.z, v, d);	" \
"	float c1 = min(bSquare ? rectCoverage(shapeEntry(ofs + 4), p) : circleCoverage(circles.w, v, d), c0);	" \
"	float c2 = min(circleCoverage(inner.x, v, d), c1);								" \
"	float c3 = min(circleCoverage(inner.y, v, d), c2);								" \
"	float back = (flags & 2) != 0 ? max(rectCoverage(rect, p) - c0, 0.0) : 0.0;		" \
"	float outline = c0 - c1 + c2 - c3;												" \
"	float band = c1 - c2;															" \
"	if ((flags & 16) == 0)															" \
"	{																				" \
"		outline += band;															" \
"		band = 0.0;																	" \
"	}																				" \
"	addSegment(ofs + 5, back, p, acc, covered);										" \
"	addSegment(ofs + 6, outline, p, acc, covered);									" \
"	addSegment(ofs + 7, (flags & 4) != 0 ? c3 : 0.0, p, acc, covered);				" \
"	if (band > 0.0)																	" \
"		ringElements(ofs, int(head.z), ofs + int(head.w), band, p, v, d, acc, covered);	" \
"}																					" \
"float polygonCoverage(vec3 planes[6], int count)									" \
"{																					" \
"	bool bInside = true;															" \
"	for (int i = 0; i < count; i++)													" \
"	{																				" \
"		if (planes[i].z <= -0.7072)													" \
"			return 0.0;																" \
"		if (planes[i].z < 0.7072)													" \
"			bInside = false;														" \
"	}																				" \
"	if (bInside)																	" \
"		return 1.0;																	" \
"	vec2 v[10];																		" \
"	vec2 o[10];																		" \
"	int nv = 4;																		" \
"	v[0] = vec2(-0.5, -0.5);														" \
"	v[1] = vec2(0.5, -0.5);															" \
"	v[2] = vec2(0.5, 0.5);															" \
"	v[3] = vec2(-0.5, 0.5);															" \
"	for (int i = 0; i < count; i++)													" \
"	{																				" \
"		vec3 pl = planes[i];														" \
"		if (pl.z >= 0.7072)															" \
"			continue;																" \
"		int no = 0;																	" \
"		for (int k = 0; k < nv; k++)												" \
"		{																			" \
"			vec2 a = v[k];															" \
"			vec2 b = v[k + 1 < nv ? k + 1 : 0];										" \
"			float fa = pl.z + dot(a, pl.xy);										" \
"			float fb = pl.z + dot(b, pl.xy);										" \
"			if (fa >= 0.0)															" \
"				o[no++] = a;														" \
"			if ((fa >= 0.0) != (fb >= 0.0))											" \
"				o[no++] = a + (b - a) * (fa / (fa - fb));							" \
"		}																			" \
"		nv = no;																	" \
"		if (nv < 3)																	" \
"			return 0.0;																" \
"		for (int k = 0; k < nv; k++)												" \
"			v[k] = o[k];															" \
"	}																				" \
"	float area = 0.0;																" \
"	for (int k = 0; k < nv; k++)													" \
"	{																				" \
"		vec2 a = v[k];																" \
"		vec2 b = v[k + 1 < nv ? k + 1 : 0];											" \
"		area += a.x * b.y - a.y * b.x;												" \
"	}																				" \
"	return clamp(abs(area) * 0.5, 0.0, 1.0);										" \
"}																					" \
"void addCapPlanes(int cap, vec2 end, vec2 u, vec2 n, float r, vec2 p, inout vec3 planes[6], inout int count)	" \
"{																					" \
"	vec2 d = p - end;																" \
"	float a = dot(d, u);															" \
"	int shape = cap & 7;															" \
"	if (shape == 1 || shape == 3)													" \
"		planes[count++] = vec3(u, a);												" \
"	else if (shape == 2)															" \
"		planes[count++] = vec3(u, a + r);											" \
"	else if (shape == 4)															" \
"	{																				" \
"		float sx = u.x > 0.0 ? 1.0 : -1.0;											" \
"		planes[count++] = vec3(sx, 0.0, d.x * sx);									" \
"	}																				" \
"	else if (shape == 5)															" \
"	{																				" \
"		float sy = u.y > 0.0 ? 1.0 : -1.0;											" \
"		planes[count++] = vec3(0.0, sy, d.y * sy);									" \
"	}																				" \
"	else																			" \
"	{																				" \
"		float spear = float(cap >> 3) * 0.5 * r;									" \
"		float w = dot(d, n);														" \
"		vec2 g1 = r * u - spear * n;												" \
"		vec2 g2 = r * u + spear * n;												" \
"		float k = length(g1);														" \
"		planes[count++] = vec3(g1 / k, (r * a - spear * w) / k);					" \
"		planes[count++] = vec3(g2 / k, (r * a + spear * w) / k);					" \
"	}																				" \
"}																					" \
"void lineCoverage(int ofs, vec2 p, inout vec4 acc, inout float covered)			" \
"{																					" \
"	vec4 head = shapeEntry(ofs);													" \
"	vec4 ends = shapeEntry(ofs + 1);												" \
"	vec4 dir = shapeEntry(ofs + 2);													" \
"	vec2 u = dir.xy;																" \
"	vec2 n = vec2(-u.y, u.x);														" \
"	float r = dir.w;																" \
"	int capB = int(head.y);															" \
"	int capE = int(head.z);															" \
"	float w = dot(p - ends.xy, n);													" \
"	vec3 planes[6];																	" \
"	planes[0] = vec3(-n, r - w);													" \
"	planes[1] = vec3(n, r + w);														" \
"	int count = 2;																	" \
"	addCapPlanes(capB, ends.xy, u, n, r, p, planes, count);							" \
"	int countB = count;																" \
"	addCapPlanes(capE, ends.zw, -u, -n, r, p, planes, count);						" \
"	float c = polygonCoverage(planes, count);										" \
"	bool bRoundB = (capB & 7) == 3;													" \
"	bool bRoundE = (capE & 7) == 3;													" \
"	if (bRoundB || bRoundE)															" \
"	{																				" \
"		vec2 dB = p - ends.xy;														" \
"		vec2 dE = p - ends.zw;														" \
"		float discB = bRoundB ? circleCoverage(r, dB, length(dB)) : 0.0;			" \
"		float discE = bRoundE ? circleCoverage(r, dE, length(dE)) : 0.0;			" \
"		vec3 sub[6];																" \
"		int ns = 0;																	" \
"		for (int i = 2; i < countB; i++)											" \
"			sub[ns++] = planes[i];													" \
"		float regionB = max(polygonCoverage(sub, ns), discB);						" \
"		ns = 0;																		" \
"		for (int i = countB; i < count; i++)										" \
"			sub[ns++] = planes[i];													" \
"		float regionE = max(polygonCoverage(sub, ns), discE);						" \
"		c = max(c, max(min(discB, regionE), min(discE, regionB)));					" \
"	}																				" \
"	if (c <= 0.0)																	" \
"		return;																		" \
"	vec4 col = texelFetch(colorBufferId, int(head.w));								" \
"	acc += vec4(col.rgb * col.a, col.a) * c;										" \
"	covered += c;																	" \
"}																					" \
"float shapeCoverage(int ofs, vec2 p, out vec4 acc)									" \
"{																					" \
"	acc = vec4(0.0);																" \
"	float covered = 0.0;															" \
"	int kind = int(shapeEntry(ofs).x);												" \
"	if (kind == 1)																	" \
"		boxCoverage(ofs, p, acc, covered);											" \
"	else if (kind == 2)																" \
"		ringCoverage(ofs, p, acc, covered);											" \
"	else if (kind == 3)																" \
"		lineCoverage(ofs, p, acc, covered);											" \
"	return covered;																	" \
"}																					"

const char GlBackend::shapeFragmentShader[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform samplerBuffer extrasBufferId;			"
"uniform samplerBuffer colorBufferId;			"
"uniform samplerBuffer tintmapBufferId;			"
"flat in int shapeOfs;							"
"flat in int tintOfs;							"
"in vec2 canvasPos;								"
"flat in vec4 flatTint;							"
WAX_GL_TINT_FUNC
WAX_GL_SHAPE_FUNCS
"void main()									"
"{												"
"	vec4 acc;									"
"	float covered = shapeCoverage(shapeOfs, canvasPos, acc);	"
"	if (covered < 1.0 / 256.0)					"
"		discard;								"
"	vec4 tint = (tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, canvasPos) : vec4(1.0)) * flatTint;	"
"	writeColor(acc * premul(tint), covered);	"
"}";

const char GlBackend::shapeFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform samplerBuffer extrasBufferId;			"
"uniform samplerBuffer colorBufferId;			"
"uniform samplerBuffer tintmapBufferId;			"
"flat in int shapeOfs;							"
"flat in int tintOfs;							"
"in vec2 canvasPos;								"
"flat in vec4 flatTint;							"
WAX_GL_TINT_FUNC
WAX_GL_SHAPE_FUNCS
"void main()									"
"{												"
"	vec4 acc;									"
"	float covered = shapeCoverage(shapeOfs, canvasPos, acc);	"
"	if (covered < 1.0 / 256.0)					"
"		discard;								"
"	vec4 tint = (tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, canvasPos) : vec4(1.0)) * flatTint;	"
"	writeAlpha(acc.a * tint.a, covered);	"
"}";


// Warps (Command::WarpBlit and WarpDrawEdgemap), see Warpmap for the definition they follow.
// Each pixel finds the cell of the Warpmap it is in and interpolates the position in the source
// at its center, and how fast that changes along x and y of the canvas. Coverage is worked out
// from those, taking each border of the source and each edge of an edgemap as straight over the
// pixel. Extras of a warp, four entries:
//
//   0: position of Warpmap on canvas, cell size, number of edges.
//   1: size of Warpmap, size of source in pixels.
//   2: source offset in pixels of the source, periods in pixels of the source (0 if none).
//   3: edgemaps: edges per column, offsets of flat colors, tint table and raw samples in the
//      edgemap buffer. Blits: position of source rectangle in the surface.
//
// The cells of the Warpmap are in its buffer, see GlWarpmap.

const char GlBackend::warpVertexShader[] =

"#version 330 core\n"

WAX_GL_CANVAS_BLOCK

"uniform samplerBuffer colorBufferId;					"
"layout(location = 0) in ivec2 pos;                     "
"layout(location = 2) in int colorOfs;                  "
"layout(location = 3) in int extrasOfs;                 "
"layout(location = 4) in vec2 tintmapOfs;               "
"flat out int warpOfs;									"
"flat out int tintOfs;									"
"out vec2 canvasPos;									"
"flat out vec4 flatTint;								"

"void main()											"
"{                                                      "
"   gl_Position.x = pos.x*2/canvasWidth - 1.0;              "
"   gl_Position.y = (canvasYOfs + canvasYMul*pos.y)*2/canvasHeight - 1.0;    "
"   gl_Position.z = 0.0;                                    "
"   gl_Position.w = 1.0;                                    "
"   warpOfs = extrasOfs;								"
"   tintOfs = int(tintmapOfs.x);						"
"   canvasPos = vec2(pos);								"
"   flatTint = texelFetch(colorBufferId, colorOfs);		"
"}                                                      ";

// Needs extrasBufferId, warpmapId and warpOfs. warpHalfPlane() gives the part of the pixel where
// f + g.x * dx + g.y * dy >= 0, dx and dy going from -0.5 to 0.5 over it, which is linear within
// (a - b) / 2 of the center and a triangle in a corner further out. warpAxisCoverage() gives the
// part inside the source along one axis, with the copies of the source on a periodic axis.
// warpPosition() finds the cell with integers, as a division in floats can miss a whole number.
// warpNearestCopy() moves a center in the gap between copies on a periodic axis to the nearest
// copy, which is where colors are taken and edges evaluated.

#define WAX_GL_WARP_FUNCS \
"vec4 warpEntry(int i) { return texelFetch(extrasBufferId, warpOfs + i); }			" \
"float warpHalfPlane(float f, vec2 g)												" \
"{																					" \
"	vec2 ag = abs(g);																" \
"	float a = max(ag.x, ag.y);														" \
"	float b = min(ag.x, ag.y);														" \
"	if (2.0 * f >= a + b)															" \
"		return 1.0;																	" \
"	if (2.0 * f <= -(a + b))														" \
"		return 0.0;																	" \
"	if (2.0 * abs(f) <= a - b)														" \
"		return 0.5 + f / a;															" \
"	float t = a + b - 2.0 * abs(f);													" \
"	float corner = t * t / (8.0 * a * b);											" \
"	return f > 0.0 ? 1.0 - corner : corner;											" \
"}																					" \
"float warpInside(float size, float period) { return period > 0.0 ? min(size, period) : size; }	" \
"vec2 warpNearestCopy(vec2 s, vec2 size, vec2 period)								" \
"{																					" \
"	if (period.x > 0.0 && 2.0 * s.x > warpInside(size.x, period.x) + period.x)		" \
"		s.x -= period.x;															" \
"	if (period.y > 0.0 && 2.0 * s.y > warpInside(size.y, period.y) + period.y)		" \
"		s.y -= period.y;															" \
"	return s;																		" \
"}																					" \
"float warpAxisCoverage(float s, vec2 g, float size, float period)					" \
"{																					" \
"	if (period > 0.0 && period <= size)												" \
"		return 1.0;																	" \
"	float inside = warpInside(size, period);										" \
"	float ext = 0.5 * (abs(g.x) + abs(g.y));										" \
"	if (s - ext >= 0.0 && s + ext <= inside)										" \
"		return 1.0;																	" \
"	if (period <= 0.0)																" \
"		return warpHalfPlane(s, g) - warpHalfPlane(s - inside, g);					" \
"	float c = 0.0;																	" \
"	for (int k = -1; k <= 1; k++)													" \
"	{																				" \
"		float lo = float(k) * period;												" \
"		c += warpHalfPlane(s - lo, g) - warpHalfPlane(s - lo - inside, g);			" \
"	}																				" \
"	return min(c, 1.0);																" \
"}																					" \
"void warpPosition(vec2 p, out vec2 s, out vec2 dx, out vec2 dy)					" \
"{																					" \
"	vec4 e0 = warpEntry(0);															" \
"	vec4 e1 = warpEntry(1);															" \
"	vec4 e2 = warpEntry(2);															" \
"	vec2 q = p - e0.xy;																" \
"	int cellSize = int(e0.z);														" \
"	ivec2 cells = (ivec2(e1.xy) + cellSize - 1) / cellSize;							" \
"	ivec2 cell = clamp(ivec2(floor(max(q, vec2(0.0)))) / cellSize, ivec2(0), cells - 1);	" \
"	vec2 c0 = vec2(cell * cellSize);												" \
"	vec2 c1 = min(c0 + float(cellSize), e1.xy);										" \
"	vec2 f = (q - c0) / (c1 - c0);													" \
"	int i = (cell.y * cells.x + cell.x) * 2;										" \
"	vec4 top = texelFetch(warpmapId, i);											" \
"	vec4 bottom = texelFetch(warpmapId, i + 1);										" \
"	vec2 u = mix(mix(top.xy, top.zw, f.x), mix(bottom.xy, bottom.zw, f.x), f.y);	" \
"	s = u + e2.xy;        															" \
"	dx = mix(top.zw - top.xy, bottom.zw - bottom.xy, f.y) / (c1.x - c0.x);        	" \
"	dy = mix(bottom.xy - top.xy, bottom.zw - top.zw, f.x) / (c1.y - c0.y);        	" \
"	if (e2.z > 0.0)																	" \
"		s.x -= e2.z * floor(s.x / e2.z);											" \
"	if (e2.w > 0.0)																	" \
"		s.y -= e2.w * floor(s.y / e2.w);											" \
"}																					"

// Coverage of the segments of an edgemap. Edges are linear across the column at the center of
// the pixel (the nearest copy's for a center in the gap between copies on a periodic axis),
// so the border of the area below an edge is sy - edge(sx) = 0. Areas below the top of the
// source, each edge and the bottom of the source are measured, kept decreasing as an edge is
// pushed down to the edges above it, and their differences are the parts of the segments.
// SEGMENT is code run for each part, with the segment in e, the part in part and the position for
// colors in sc. Needs edgemapId.

#define WAX_GL_WARP_SEGMENTS(SEGMENT) \
"	vec2 s, dx, dy;																	" \
"	warpPosition(canvasPos, s, dx, dy);												" \
"	vec4 e0 = warpEntry(0);															" \
"	vec4 e1 = warpEntry(1);															" \
"	vec4 e2 = warpEntry(2);															" \
"	vec4 e3 = warpEntry(3);															" \
"	vec2 size = e1.zw;																" \
"	vec2 period = e2.zw;															" \
"	vec2 gx = vec2(dx.x, dy.x);														" \
"	vec2 gy = vec2(dx.y, dy.y);														" \
"	float covX = warpAxisCoverage(s.x, gx, size.x, period.x);						" \
"	if (covX <= 0.0)																" \
"		discard;																	" \
"	int nEdges = int(e0.w);															" \
"	int pitch = int(e3.x);															" \
"	int flatColorsOfs = int(e3.y);													" \
"	int tintTableOfs = int(e3.z);													" \
"	vec2 sc = warpNearestCopy(s, size, period);										" \
"	float ex = sc.x;																" \
"	int column = clamp(int(floor(ex)), 0, int(size.x) - 1);							" \
"	float frac = ex - float(column);												" \
"	int base = int(e3.w) + column * pitch;											" \
"	float insideY = warpInside(size.y, period.y);									" \
"	float ext = 0.5 * (abs(gy.x) + abs(gy.y));										" \
"	int kBeg = period.y > 0.0 ? -1 : 0;												" \
"	for (int k = kBeg; k <= -kBeg; k++)												" \
"	{																				" \
"		float sy = s.y - float(k) * period.y;										" \
"		if (sy + ext <= 0.0 || sy - ext >= insideY)									" \
"			continue;																" \
"		float above = warpHalfPlane(sy, gy);										" \
"		for (int e = 0; e <= nEdges; e++)											" \
"		{																			" \
"			float below;															" \
"			if (e < nEdges)															" \
"			{																		" \
"				vec2 lr = texelFetch(edgemapId, base + e).xy;						" \
"				float slope = lr.y - lr.x;											" \
"				below = min(above, warpHalfPlane(sy - lr.x - slope * frac, gy - slope * gx));	" \
"			}																		" \
"			else																	" \
"				below = min(above, warpHalfPlane(sy - insideY, gy));				" \
"			float part = (above - below) * covX;									" \
"			if (part > 0.0)															" \
SEGMENT \
"			above = below;															" \
"		}																			" \
"	}																				"

const char GlBackend::warpEdgemapFragmentShader[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform samplerBuffer extrasBufferId;			"
"uniform samplerBuffer tintmapBufferId;			"
"uniform samplerBuffer edgemapId;				"
"uniform samplerBuffer warpmapId;				"
"flat in int warpOfs;							"
"flat in int tintOfs;							"
"in vec2 canvasPos;								"
"flat in vec4 flatTint;							"
WAX_GL_TINT_FUNC
WAX_GL_SEGCOLOR_FUNC
WAX_GL_SEGPART_FUNC
WAX_GL_WARP_FUNCS
"void main()									"
"{												"
"	vec4 tint = premul((tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, canvasPos) : vec4(1.0)) * flatTint);	"
"	vec4 acc = vec4(0.0);						"
"	float keep = 0.0;							"
"	float covered = 0.0;						"
WAX_GL_WARP_SEGMENTS("{ acc += segPart(premul(segColor(e, flatColorsOfs, tintTableOfs, sc)) * tint, part, keep); covered += part; }")
"	if (covered < 1.0 / 256.0)					"
"		discard;								"
"	if (keepsResultColors())					"
"	{											"
"		color = acc;							"
"		colorFactor = vec4(covered - keep);		"
"	}											"
"	else										"
"		writeColor(acc, covered);				"
"}";

const char GlBackend::warpEdgemapFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform samplerBuffer extrasBufferId;			"
"uniform samplerBuffer tintmapBufferId;			"
"uniform samplerBuffer edgemapId;				"
"uniform samplerBuffer warpmapId;				"
"flat in int warpOfs;							"
"flat in int tintOfs;							"
"in vec2 canvasPos;								"
"flat in vec4 flatTint;							"
WAX_GL_TINT_FUNC
WAX_GL_SEGCOLOR_FUNC
WAX_GL_WARP_FUNCS
"void main()									"
"{												"
"	float acc = 0.0;							"
"	float covered = 0.0;						"
WAX_GL_WARP_SEGMENTS("{ acc += segColor(e, flatColorsOfs, tintTableOfs, sc).a * part; covered += part; }")
"	if (covered < 1.0 / 256.0)					"
"		discard;								"
"	vec4 tint = (tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, canvasPos) : vec4(1.0)) * flatTint;	"
"	writeAlpha(acc * tint.a, covered);			"
"}";

// The blit source is sampled at the position of the center of the pixel, with its sample method.
// A tiling surface wraps around as its texture is set up. Any other is read as if the edge pixels
// of the source rectangle continued outside it, which warpKeepInside() does by keeping positions
// clampInset (half a texel if bilinear, a hair if nearest) inside its edges, 0 for a tiling
// surface. sourceKind tells how to read it: 0 colors,
// 1 alpha only (Alpha_8 as white with alpha), 2 palette with nearest sampling and 3 palette with
// bilinear sampling, which looks up the four texels around the position. Gradients are given
// for mipmapped surfaces, which the derivatives of positions that wrap would get wrong.

#define WAX_GL_WARP_SAMPLE \
"uniform sampler2D texId;															" \
"uniform sampler2D paletteId;														" WAX_GL_PALETTE_FUNC \
"uniform int sourceKind;															" \
"uniform float clampInset;															" \
"vec2 warpKeepInside(vec2 p, vec2 begin, vec2 size)									" \
"{																					" \
"	vec2 center = begin + 0.5 * size;												" \
"	vec2 lo = min(begin + clampInset, center);										" \
"	vec2 hi = max(begin + size - clampInset, center);								" \
"	return clamp(p, lo, hi);														" \
"}																					" \
"vec4 warpSample(vec2 p, vec2 gx, vec2 gy)											" \
"{																					" \
"	vec2 texSize = vec2(textureSize(texId, 0));										" \
"	if (sourceKind == 2)															" \
"		return paletteLookup(textureLod(texId, p / texSize, 0.0).r);				" \
"	if (sourceKind == 3)															" \
"	{																				" \
"		vec2 q = p - 0.5;															" \
"		vec2 base = floor(q);														" \
"		vec2 f = q - base;															" \
"		vec2 uv00 = (base + 0.5) / texSize;											" \
"		vec2 uv11 = (base + 1.5) / texSize;											" \
"		vec4 c00 = paletteLookup(textureLod(texId, uv00, 0.0).r);					" \
"		vec4 c01 = paletteLookup(textureLod(texId, vec2(uv11.x, uv00.y), 0.0).r);	" \
"		vec4 c10 = paletteLookup(textureLod(texId, vec2(uv00.x, uv11.y), 0.0).r);	" \
"		vec4 c11 = paletteLookup(textureLod(texId, uv11, 0.0).r);					" \
"		return mix(mix(c00, c01, f.x), mix(c10, c11, f.x), f.y);					" \
"	}																				" \
"	vec4 c = textureGrad(texId, p / texSize, gx / texSize, gy / texSize);			" \
"	return sourceKind == 1 ? vec4(c.r) : c;											" \
"}																					"

#define WAX_GL_WARP_BLIT_COVERAGE \
"	vec2 s, dx, dy;																	" \
"	warpPosition(canvasPos, s, dx, dy);												" \
"	vec4 e1 = warpEntry(1);															" \
"	vec4 e2 = warpEntry(2);															" \
"	vec4 e3 = warpEntry(3);															" \
"	float cov = warpAxisCoverage(s.x, vec2(dx.x, dy.x), e1.z, e2.z);				" \
"	if (cov > 0.0)																	" \
"		cov *= warpAxisCoverage(s.y, vec2(dx.y, dy.y), e1.w, e2.w);					" \
"	if (cov < 1.0 / 256.0)															" \
"		discard;																	" \
"	vec2 sp = e3.xy + warpNearestCopy(s, e1.zw, e2.zw);								" \
"	if (clampInset > 0.0)															" \
"		sp = warpKeepInside(sp, e3.xy, e1.zw);										" \
"	vec4 c = warpSample(sp, dx, dy);												" \
"	vec4 tint = (tintOfs >= 0 ? evalTint(tintmapBufferId, tintOfs, canvasPos) : vec4(1.0)) * flatTint;	"

const char GlBackend::warpBlitFragmentShader[] =

"#version 330 core\n"
WAX_GL_OUTPUT
"uniform samplerBuffer extrasBufferId;			"
"uniform samplerBuffer tintmapBufferId;			"
"uniform samplerBuffer warpmapId;				"
"flat in int warpOfs;							"
"flat in int tintOfs;							"
"in vec2 canvasPos;								"
"flat in vec4 flatTint;							"
WAX_GL_TINT_FUNC
WAX_GL_WARP_FUNCS
WAX_GL_WARP_SAMPLE
"void main()									"
"{												"
WAX_GL_WARP_BLIT_COVERAGE
"	writeColor(c * premul(tint) * cov, cov);	"
"}";

const char GlBackend::warpBlitFragmentShader_A8[] =

"#version 330 core\n"
WAX_GL_OUTPUT_A8
"uniform samplerBuffer extrasBufferId;			"
"uniform samplerBuffer tintmapBufferId;			"
"uniform samplerBuffer warpmapId;				"
"flat in int warpOfs;							"
"flat in int tintOfs;							"
"in vec2 canvasPos;								"
"flat in vec4 flatTint;							"
WAX_GL_TINT_FUNC
WAX_GL_WARP_FUNCS
WAX_GL_WARP_SAMPLE
"void main()									"
"{												"
WAX_GL_WARP_BLIT_COVERAGE
"	writeAlpha(c.a * tint.a * cov, cov);		"
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

#endif //WAX_GLSHADERS_DOT_H
