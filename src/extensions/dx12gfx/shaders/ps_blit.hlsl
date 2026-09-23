cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;
    float2 textureSize;
    uint flags;             // Bit 0: source is alpha only.
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
    float4 texel = blitSource.Sample(blitSampler, input.texUV);

    // An alpha only source has no color of its own, it just modulates the tint.

    if ((flags & 1) != 0)
    {
        float4 color = input.color;
        color.a *= texel.a;
        return color * tintmapColor(input.position.xy);
    }

    return texel * input.color * tintmapColor(input.position.xy);
}
