cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // Not used here.
    float2 textureSize;     // Not used here, the vertex shader has already used it.
    uint flags;             // Bit 0: source is alpha only.
    uint blurOfs;           // Where our 9 color matrices and 9 offsets start in extras.
};

// The tint, tintColor() and the colors buffer come from tint.hlsl, which is put
// in front of this file.


StructuredBuffer<float4> extras : register(t1);

Texture2D blitSource : register(t2);
SamplerState blitSampler : register(s0);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texUV : TEXCOORD0;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    // Nine taps around this one, each weighted per channel by its own row of the
    // brush's color matrix. The offsets are in texture coordinates, worked out
    // from the blur radius and the size of the source.
    //
    // The alpha row only weights the center tap, so the source's own alpha is
    // kept and only the colors are blurred. An alpha only source reads as white
    // with alpha (an A8_UNORM texture samples as black), like it does in blits.

    bool bAlphaOnly = (flags & 1u) != 0u;

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

    [unroll]
    for (uint i = 0u; i < 9u; i++)
    {
        float2 ofs = extras[blurOfs + 9u + i].xy;

        float4 tap = blitSource.Sample(blitSampler, input.texUV + ofs);
        if (bAlphaOnly)
            tap = float4(1.0f, 1.0f, 1.0f, tap.a);

        color += tap * extras[blurOfs + i];
    }

    return color * input.color * tintColor(input.position.xy);
}
