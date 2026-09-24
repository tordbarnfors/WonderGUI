cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // Not used here.
    float2 textureSize;     // Not used here, the vertex shader has already used it.
    uint flags;             // Not used here.
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
    // An alpha only source has no color to blur, so its alpha is blurred instead
    // and the tint supplies the color. The red row of the matrix is used as the
    // weight, the three rows being the same for an ordinary brush.

    if ((flags & 1u) != 0u)
    {
        float alpha = 0.0f;

        [unroll]
        for (uint j = 0u; j < 9u; j++)
        {
            float2 ofs = extras[blurOfs + 9u + j].xy;

            alpha += blitSource.Sample(blitSampler, input.texUV + ofs).a * extras[blurOfs + j].x;
        }

        float4 alphaColor = input.color;
        alphaColor.a *= alpha;

        return alphaColor * tintColor(input.position.xy);
    }

    // Nine taps around this one, each weighted per channel by its own row of the
    // brush's color matrix. The offsets are in texture coordinates, worked out
    // from the blur radius and the size of the source.

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

    [unroll]
    for (uint i = 0u; i < 9u; i++)
    {
        float2 ofs = extras[blurOfs + 9u + i].xy;

        color += blitSource.Sample(blitSampler, input.texUV + ofs) * extras[blurOfs + i];
    }

    // A blur has no alpha of its own. What comes out is opaque, and the tint or
    // tintmap is what decides how much of it reaches the canvas.

    color.a = 1.0f;

    return color * input.color * tintColor(input.position.xy);
}
