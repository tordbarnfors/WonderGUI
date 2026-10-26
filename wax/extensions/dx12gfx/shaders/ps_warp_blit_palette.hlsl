cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // Not used here.
    float2 textureSize;     // Size of blit source, in pixels.
    uint flags;             // Bit 1: bilinear. Bit 2: tiling. Others not used here.
};

// Warped blits from a palette based source. writeOutput() and premul() come from
// output.hlsl, tintColor() from tint.hlsl and the warp functions from warp.hlsl,
// which are put in front of this file.
//
// The texels around the position of the center of the pixel are looked up in the
// palette before they are filtered, wrapped or clamped as the sampler would have,
// see ps_blit_palette.hlsl. A surface that isn't tiling is kept within the source
// rectangle by warpBlitCoverage(). Coverage takes care of the borders of the source.


Texture2D<uint> blitSource : register(t2);
StructuredBuffer<float4> palette : register(t3);


float4 paletteTexel(int2 texel)
{
    int2 size = int2(textureSize);
    int2 pos = texel;

    if ((flags & 4u) != 0u)
    {
        // Wrap, in unsigned since the compiler warns about signed modulus. A texel
        // left of or above the source wraps from the other side.

        uint2 usize = uint2(size);
        uint2 rest = uint2(abs(texel)) % usize;

        pos.x = (texel.x >= 0 || rest.x == 0u) ? int(rest.x) : int(usize.x - rest.x);
        pos.y = (texel.y >= 0 || rest.y == 0u) ? int(rest.y) : int(usize.y - rest.y);
    }

    pos = clamp(pos, int2(0, 0), size - int2(1, 1));

    uint index = blitSource.Load(int3(pos, 0));
    uint capacity = uint(palette[0].x);

    return palette[min(index, capacity - 1u) + 1u];
}


// Color at pos, in pixels of the source.

float4 samplePalette(float2 pos)
{
    float4 color;

    if ((flags & 2u) != 0u)
    {
        // Bilinear. Texel centers are at .5, so the four texels around us start
        // half a texel up and to the left.

        float2 base = pos - 0.5f;
        float2 baseFloor = floor(base);
        float2 weight = base - baseFloor;
        int2 texel = int2(baseFloor);

        float4 c00 = paletteTexel(texel);
        float4 c10 = paletteTexel(texel + int2(1, 0));
        float4 c01 = paletteTexel(texel + int2(0, 1));
        float4 c11 = paletteTexel(texel + int2(1, 1));

        color = lerp(lerp(c00, c10, weight.x), lerp(c01, c11, weight.x), weight.y);
    }
    else
        color = paletteTexel(int2(floor(pos)));

    return color;
}


PS_OUTPUT main(PS_INPUT input)
{
    float2 s, dx, dy;
    float cov = warpBlitCoverage(input.warpOfs, input.position.xy, s, dx, dy);

    if (cov < 1.0f / 256.0f)
        discard;

    return writeOutput(samplePalette(s) * premul(input.color * tintColor(input.position.xy)) * cov, cov);
}
