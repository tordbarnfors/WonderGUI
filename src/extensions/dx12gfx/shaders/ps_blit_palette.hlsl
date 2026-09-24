cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // Not used here.
    float2 textureSize;     // Size of blit source, in pixels.
    uint flags;             // Bit 1: bilinear. Bit 2: tiling.
    uint blurOfs;           // Where a blur's 9 color matrices and 9 offsets start in extras.
};

// The tint, tintColor() and the colors buffer come from tint.hlsl, which is put
// in front of this file.


// A palette based source is a texture of 8 or 16 bit indexes and a palette of
// colors already converted to linear. The palette lookup has to come before any
// filtering, so we fetch texels ourselves rather than going through a sampler,
// and do what the sampler would have done: wrap or clamp at the edges, and for
// a bilinear surface blend the four texels around us.
//
// The palette's first entry holds its capacity, the colors follow. An index
// beyond the palette is clamped to its last entry rather than read from
// whatever lies past the end of the buffer.

Texture2D<uint> blitSource : register(t2);
StructuredBuffer<float4> palette : register(t3);


float4 paletteTexel(int2 texel)
{
    int2 size = int2(textureSize);

    if ((flags & 4u) != 0u)
    {
        // Wrap. Done in unsigned, since the compiler warns about signed modulus
        // and we treat warnings as errors. A texel left of or above the source
        // wraps from the other side.

        uint2 usize = uint2(size);
        uint2 rest = uint2(abs(texel)) % usize;

        texel.x = (texel.x >= 0 || rest.x == 0u) ? int(rest.x) : int(usize.x - rest.x);
        texel.y = (texel.y >= 0 || rest.y == 0u) ? int(rest.y) : int(usize.y - rest.y);
    }
    else
        texel = clamp(texel, int2(0, 0), size - int2(1, 1));

    uint index = blitSource.Load(int3(texel, 0));
    uint capacity = uint(palette[0].x);

    return palette[min(index, capacity - 1u) + 1u];
}


float4 samplePalette(float2 uv)
{
    // One return at the end: FXC takes an early return out of a function to
    // mean the result may be left uninitialized (X4000), and warnings are
    // errors for us.

    float2 pos = uv * textureSize;
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
    {
        // Nearest. A pixel lined up with the source lands exactly on a texel
        // edge, where rounding could tip it into the texel before. The sampler
        // snaps to a fraction of a texel for the same reason, so we nudge by as
        // much.

        color = paletteTexel(int2(floor(pos + 1.0f / 256.0f)));
    }

    return color;
}


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texUV : TEXCOORD0;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    return samplePalette(input.texUV) * input.color * tintColor(input.position.xy);
}
