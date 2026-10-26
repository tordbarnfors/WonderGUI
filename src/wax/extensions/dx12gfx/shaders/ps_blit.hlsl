cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;
    float2 textureSize;
    uint flags;             // Bit 0: source is alpha only. Bit 1: bilinear. Bit 3: clip, nothing outside the source is read. Bit 4: source has no alpha.
};

// writeOutput() and premul() come from output.hlsl, the tint, tintColor() and the
// colors buffer from tint.hlsl, which are put in front of this file.
//
// Textures with alpha hold premultiplied colors, so texels are premultiplied.


Texture2D blitSource : register(t2);
SamplerState blitSampler : register(s0);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texUV : TEXCOORD0;
};


PS_OUTPUT main(PS_INPUT input)
{
    float4 texel = blitSource.Sample(blitSampler, input.texUV);

    // A source without alpha (B8G8R8X8) reads alpha 1 also from the transparent
    // border that clips a ClipBlit. So its alpha is worked out from the position:
    // 1 inside and 0 outside with nearest sampling, and the part of the bilinear
    // footprint that lies inside with bilinear sampling, which is what the sampler
    // does with the colors. The colors are therefore already premultiplied by it.

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

    float4 tint = premul(input.color * tintColor(input.position.xy));

    if ((flags & 1u) != 0u)
        texel = texel.a;

    return writeOutput(texel * tint, 1.0f);
}
