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
"\n"
"struct VS_INPUT\n"
"{\n"
"    float2 position : POSITION;     // Canvas pixels, origin top left.\n"
"    float4 color : COLOR;\n"
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
"    output.color = input.color;\n"
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


}
