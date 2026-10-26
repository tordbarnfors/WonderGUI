cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // Not used here.
    float2 textureSize;     // Size of blit source, in pixels.
    uint flags;             // Bit 0: source is alpha only. Others not used here.
};

// Warped blits. writeOutput() and premul() come from output.hlsl, tintColor() from
// tint.hlsl and the warp functions from warp.hlsl, which are put in front of this
// file.
//
// The source is sampled at the position of the center of the pixel, with its sample
// method. A tiling surface is wrapped around by the sampler, any other is kept within
// the source rectangle by warpBlitCoverage() (and never read through a clip sampler,
// coverage takes care of the borders of the source). Gradients are given for
// mipmapped surfaces, which the derivatives of positions that wrap would get wrong.
// Textures with alpha hold premultiplied colors, one without alpha (B8G8R8X8) reads
// alpha 1.


Texture2D blitSource : register(t2);
SamplerState blitSampler : register(s0);


PS_OUTPUT main(PS_INPUT input)
{
    float2 s, dx, dy;
    float cov = warpBlitCoverage(input.warpOfs, input.position.xy, s, dx, dy);

    if (cov < 1.0f / 256.0f)
        discard;

    float4 texel = blitSource.SampleGrad(blitSampler, s / textureSize, dx / textureSize, dy / textureSize);

    // An alpha only source has no color of its own, it just modulates the tint.

    if ((flags & 1u) != 0u)
        texel = texel.aaaa;

    float4 tint = premul(input.color * tintColor(input.position.xy));

    return writeOutput(texel * tint * cov, cov);
}
