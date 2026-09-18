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
#include <wg_dx12backend.h>

namespace wg {

const char DX12Backend::g_fillVS[] =

"cbuffer CanvasInfo : register(b0)\n"
"{\n"
"    float2 canvasScale;     // 2/canvasWidth, 2/canvasHeight, in pixels.\n"
"};\n"
"\n"
"StructuredBuffer<float4> colors : register(t0);\n"
"\n"
"\n"
"struct VS_INPUT\n"
"{\n"
"    float2 position : POSITION;     // Canvas pixels, origin top left.\n"
"    uint colorOfs : COLOROFS;       // Offset into the color buffer.\n"
"};\n"
"\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"    float4 position : SV_POSITION;\n"
"    float4 color : COLOR;\n"
"};\n"
"\n"
"\n"
"VS_OUTPUT main(VS_INPUT input)\n"
"{\n"
"    VS_OUTPUT output;\n"
"\n"
"    // Canvas pixels to clip space. Y is flipped, in clip space it points up.\n"
"\n"
"    output.position = float4(input.position.x * canvasScale.x - 1.0f,\n"
"                             1.0f - input.position.y * canvasScale.y,\n"
"                             0.0f, 1.0f);\n"
"\n"
"    output.color = colors[input.colorOfs];\n"
"\n"
"    return output;\n"
"}\n"
;

const char DX12Backend::g_fillPS[] =

"struct PS_INPUT //from the VS_OUTPUT\n"
"{\n"
"	\n"
"    float4 position : SV_POSITION;\n"
"    float4 color : COLOR;\n"
"	\n"
"};\n"
"\n"
"\n"
"float4 main(PS_INPUT input) : SV_TARGET\n"
"{\n"
"\n"
"   \n"
"    return input.color;\n"
"\n"
"}\n"
;

const char DX12Backend::g_blitVS[] =

"cbuffer CanvasInfo : register(b0)\n"
"{\n"
"    float2 canvasScale;     // 2/canvasWidth, 2/canvasHeight, in pixels.\n"
"    float2 textureSize;     // Size of blit source, in pixels.\n"
"    uint flags;             // Bit 0: source is alpha only.\n"
"};\n"
"\n"
"StructuredBuffer<float4> colors : register(t0);\n"
"StructuredBuffer<float4> extras : register(t1);\n"
"\n"
"\n"
"struct VS_INPUT\n"
"{\n"
"    float2 position : POSITION;     // Canvas pixels, origin top left.\n"
"    uint colorOfs : COLOROFS;       // Offset into the color buffer.\n"
"    uint extrasOfs : EXTRASOFS;     // Offset into the extras buffer.\n"
"};\n"
"\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"    float4 position : SV_POSITION;\n"
"    float4 color : COLOR;\n"
"    float2 texUV : TEXCOORD0;\n"
"};\n"
"\n"
"\n"
"VS_OUTPUT main(VS_INPUT input)\n"
"{\n"
"    VS_OUTPUT output;\n"
"\n"
"    // Canvas pixels to clip space. Y is flipped, in clip space it points up.\n"
"\n"
"    output.position = float4(input.position.x * canvasScale.x - 1.0f,\n"
"                             1.0f - input.position.y * canvasScale.y,\n"
"                             0.0f, 1.0f);\n"
"\n"
"    // First extras entry holds source and destination origin, second one the\n"
"    // transform. Source coordinate is the destination offset run through the\n"
"    // transform, which is how flipping and rotation are handled.\n"
"\n"
"    float4 srcDst = extras[input.extrasOfs];\n"
"    float4 transform = extras[input.extrasOfs + 1];\n"
"\n"
"    float2 src = srcDst.xy;\n"
"    float2 dst = srcDst.zw;\n"
"\n"
"    float2 ofs = input.position - dst;\n"
"\n"
"    output.texUV = float2((src.x + ofs.x * transform.x + ofs.y * transform.z) / textureSize.x,\n"
"                          (src.y + ofs.x * transform.y + ofs.y * transform.w) / textureSize.y);\n"
"\n"
"    output.color = colors[input.colorOfs];\n"
"\n"
"    return output;\n"
"}\n"
;

const char DX12Backend::g_blitPS[] =

"cbuffer CanvasInfo : register(b0)\n"
"{\n"
"    float2 canvasScale;\n"
"    float2 textureSize;\n"
"    uint flags;             // Bit 0: source is alpha only.\n"
"};\n"
"\n"
"Texture2D blitSource : register(t2);\n"
"SamplerState blitSampler : register(s0);\n"
"\n"
"\n"
"struct PS_INPUT\n"
"{\n"
"    float4 position : SV_POSITION;\n"
"    float4 color : COLOR;\n"
"    float2 texUV : TEXCOORD0;\n"
"};\n"
"\n"
"\n"
"float4 main(PS_INPUT input) : SV_TARGET\n"
"{\n"
"    float4 texel = blitSource.Sample(blitSampler, input.texUV);\n"
"\n"
"    // An alpha only source has no color of its own, it just modulates the tint.\n"
"\n"
"    if ((flags & 1) != 0)\n"
"    {\n"
"        float4 color = input.color;\n"
"        color.a *= texel.a;\n"
"        return color;\n"
"    }\n"
"\n"
"    return texel * input.color;\n"
"}\n"
;


}
