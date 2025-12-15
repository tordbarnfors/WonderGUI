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

"struct VS_INPUT\n"
"{\n"
"    float3 position : POSITION;\n"
"    float4 color : COLOR;\n"
"    \n"
"};\n"
"\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"	\n"
"    float4 position : SV_POSITION;\n"
"    float4 color : COLOR;\n"
"	\n"
"};\n"
"\n"
"\n"
"VS_OUTPUT main(VS_INPUT input)\n"
"{\n"
"    VS_OUTPUT output;\n"
"    output.position = float4(input.position, 1.0f);\n"
"    output.color = input.color;\n"
"    \n"
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
