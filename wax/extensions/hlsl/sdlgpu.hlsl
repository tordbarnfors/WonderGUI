// The texels of blit sources for SdlGpuBackend (WAX_SDLGPU), put in front of its pixel shaders
// after tint.hlsl by ../sdlgpugfx/compile_shaders.py. The other backends read their sources
// through swizzles and border colors that SDL_GPU doesn't have, so the shaders do it:
//
// - Alpha_8 is R8, as the A8 of SDL_GPU can't be rendered into on Metal and is R8 with a swizzle
//   on Vulkan, which a render target must not have. Its value is moved to alpha (flags bit 0).
// - A source without alpha (B8G8R8A8 for XRGB_8) reads alpha 1, whatever the unused byte holds
//   (bit 4).
// - Palette based sources hold their indexes in R8_UNORM or R16_UNORM rather than R8_UINT or
//   R16_UINT, as SDL_GPU only samples textures of floats, and a texture goes with a sampler.
//   waxPaletteIndex() turns what is read into the index, by the largest index there can be,
//   which SdlGpuSurface puts in the header of the palette (y), after its capacity (x).
// - Clipping (bit 3) can't read transparent black outside the source, there is only clamping to
//   the edge. Clamping repeats the edge texels in the part of the footprint that lies outside,
//   where a border has nothing, so a border gives the clamped texel times the part of the
//   footprint inside. Exact for nearest and bilinear sampling of the first mip level.
//
// The constants come from constants.hlsl.

float waxClipCoverage(float2 uv)
{
    float cov;

    if ((flags & 2u) != 0u)
    {
        float2 p = uv * textureSize;
        float2 coverage = saturate(min(p + 0.5f, textureSize - p + 0.5f));
        cov = coverage.x * coverage.y;
    }
    else
        cov = (any(uv < 0.0f) || any(uv >= 1.0f)) ? 0.0f : 1.0f;

    return cov;
}

uint waxPaletteIndex(float value, float maxIndex)
{
    return uint(value * maxIndex + 0.5f);
}

float4 waxSourceChannels(float4 texel)
{
    if ((flags & 1u) != 0u)
        texel.a = texel.r;

    if ((flags & 16u) != 0u)
        texel.a = 1.0f;

    return texel;
}

float4 waxSourceTexel(float4 texel, float2 uv)
{
    texel = waxSourceChannels(texel);

    if ((flags & 8u) != 0u)
        texel *= waxClipCoverage(uv);

    return texel;
}
