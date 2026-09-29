cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;
    float2 textureSize;
    uint flags;             // Bit 0: source is alpha only. Bit 1: bilinear. Bit 3: clip, nothing outside the source is read. Bit 4: source has no alpha.
};

// The tint, tintColor() and the colors buffer come from tint.hlsl, which is put
// in front of this file.


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

    // A source without alpha (B8G8R8X8) reads alpha 1 also from the transparent
    // border that clips a ClipBlit. So its alpha is worked out from the position:
    // 1 inside and 0 outside with nearest sampling, and the part of the bilinear
    // footprint that lies inside with bilinear sampling, which is what the sampler
    // does with the colors.

    if ((flags & 24u) == 24u)
    {
        float width, height;
        blitSource.GetDimensions(width, height);
        float2 size = float2(width, height);

        if ((flags & 2u) != 0u)
        {
            float2 p = input.texUV * size;
            float2 coverage = saturate(min(p + 0.5f, size - p + 0.5f));
            texel.a = coverage.x * coverage.y;
        }
        else
            texel.a = (any(input.texUV < 0.0f) || any(input.texUV >= 1.0f)) ? 0.0f : 1.0f;
    }

    // An alpha only source has no color of its own, it just modulates the tint.

    if ((flags & 1) != 0)
    {
        float4 color = input.color;
        color.a *= texel.a;
        return color * tintColor(input.position.xy);
    }

    return texel * input.color * tintColor(input.position.xy);
}
