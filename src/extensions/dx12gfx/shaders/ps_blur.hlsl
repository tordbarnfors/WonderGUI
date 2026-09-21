cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // Not used here.
    float2 textureSize;     // Not used here, the vertex shader has already used it.
    uint flags;             // Not used here.
    uint blurOfs;           // Where our 9 color matrices and 9 offsets start in extras.
};

// The tintmap, if one is set: a color per pixel column and one per pixel row,
// multiplied together. They live in the color buffer, and the root constants
// say where they start, which pixel the first of them belongs to and how many
// there are. An axis with no colors has a count of zero and is left out.

cbuffer TintmapInfo : register(b1)
{
    int2 tintmapBegin;      // Where the horizontal and vertical colors start in the color buffer.
    int2 tintmapOrigin;     // Canvas pixel the first color of each axis belongs to.
    int2 tintmapCount;      // Number of colors on each axis, zero for none.
};

StructuredBuffer<float4> colors : register(t0);


float4 tintmapColor(float2 pixelPos)
{
    float4 tint = float4(1.0f, 1.0f, 1.0f, 1.0f);

    int2 ofs = int2(floor(pixelPos)) - tintmapOrigin;

    if (tintmapCount.x > 0)
        tint *= colors[tintmapBegin.x + clamp(ofs.x, 0, tintmapCount.x - 1)];

    if (tintmapCount.y > 0)
        tint *= colors[tintmapBegin.y + clamp(ofs.y, 0, tintmapCount.y - 1)];

    return tint;
}


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

        return alphaColor * tintmapColor(input.position.xy);
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

    return color * input.color * tintmapColor(input.position.xy);
}
