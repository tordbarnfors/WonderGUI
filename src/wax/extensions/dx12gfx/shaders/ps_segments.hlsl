cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // Not used here.
    float2 textureSize;     // Not used here.
    uint flags;             // Not used here.
    uint blurOfs;           // Not used here.
    uint edgemapEdges;      // Edges to walk, one fewer than the segments we render.
    uint edgemapPitch;      // Edges per column in the edgemap's buffer.
};

// writeOutput() and premul() come from output.hlsl, the tint, tintColor() and the
// colors buffer from tint.hlsl, which are put in front of this file.


StructuredBuffer<float4> edgemap : register(t3);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texUV : TEXCOORD0;
    nointerpolation uint2 segColorsOfs : TEXCOORD1;    // Flat colors and tint table in the edgemap buffer.
    nointerpolation float2 tileSize : TEXCOORD2;       // Size of a tiled edgemap in columns and pixels, zero when not tiled.
};


// Position in the edgemap of a pixel of a tiled edgemap, which may be outside of
// it, wrapped into it. uv.y is half a pixel back, so it is whole at pixel centers.

float2 wrapUV(float2 uv, float2 tileSize)
{
    if (tileSize.x > 0.0f)
    {
        float2 cell = float2(floor(uv.x), floor(uv.y + 0.5f));
        uv += cell - tileSize * floor(cell / tileSize) - cell;
    }

    return uv;
}


// A segment is colored either by a flat color or by a tint placed in the
// edgemap's own rectangle. The edgemap buffer has a table with the offset of
// each segment's tint block, -1 for a flat colored segment. That one is kept
// from being read as an offset for the same reason as in tintColor().

float4 segmentColor(uint seg, uint2 segColorsOfs, float2 epos)
{
    float4 col;

    int blockOfs = int(edgemap[segColorsOfs.y + seg].x);

    [branch]
    if (blockOfs < 0)
        col = edgemap[segColorsOfs.x + seg];
    else
        col = evalTintBlock(edgemap, uint(max(blockOfs, 0)), epos);

    return col;
}


// A segment's premultiplied color pc covers part of the pixel. Usually it just counts
// by that part. On a canvas without alpha, Replace and Morph keep the color of what
// they give each segment, which is (canvas * (1 - m) + pc * m) / (1 - m + pc.a * m),
// where m is the morph factor (1 for Replace). That is averaged by area, so each
// segment adds its part of the second term and of how much of the canvas is kept
// (keep). A segment whose result is transparent gives black, like a transparent
// Replace does.

bool keepsResultColors()
{
    return outputMode == 1u && canvasKind == 1u;
}

float4 segmentPart(float4 pc, float part, inout float keep)
{
    float4 result = pc * part;

    if (keepsResultColors())
    {
        float m = morphFactor;
        float resultAlpha = 1.0f - m + pc.a * m;
        float scale = resultAlpha > 0.0f ? part / resultAlpha : 0.0f;

        keep += scale * (1.0f - m);
        result = pc * (scale * m);
    }

    return result;
}


PS_OUTPUT main(PS_INPUT input)
{
    // Walking down a column of the edgemap, one edge at a time. Each edge says
    // where it begins and how steeply it runs, which is enough to tell how much
    // of this pixel falls above it. The difference between one edge's coverage
    // and the next is how much of the pixel belongs to the segment between them.
    // The segments together cover the whole pixel, their premultiplied and tinted
    // colors weighted by those parts, see segmentPart().

    uint nEdges = edgemapEdges;
    uint pitch = edgemapPitch;

    float2 texUV = wrapUV(input.texUV, input.tileSize);

    uint column = uint(texUV.x) * pitch;

    // Pixel center in edgemap space, which is where segment tints are placed.
    // V is half a pixel back already, see DX12Backend::_drawEdgemap().

    float2 epos = float2(floor(texUV.x) + 0.5f, texUV.y + 0.5f);

    float4 tint = premul(input.color * tintColor(input.position.xy));

    float4 acc = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float keep = 0.0f;

    float factor = 1.0f;

    [loop]
    for (uint i = 0; i < nEdges; i++)
    {
        float4 col = segmentColor(i, input.segColorsOfs, epos);

        float4 edge = edgemap[column + i];

        float x = (texUV.y - edge.r) * edge.g;

        float adder = edge.g / 2.0f;

        if (x < 0.0f)
            adder = edge.b;
        else if (x + edge.g > 1.0f)
            adder = edge.a;

        // An edge above the one before is pushed down to it.

        float factor2 = min(clamp(x + adder, 0.0f, 1.0f), factor);

        acc += segmentPart(premul(col) * tint, factor - factor2, keep);

        factor = factor2;
    }

    // Whatever is left below the last edge belongs to the last segment.

    float4 lastCol = segmentColor(nEdges, input.segColorsOfs, epos);

    acc += segmentPart(premul(lastCol) * tint, factor, keep);

    // Where segmentPart() has divided the segments by their result alphas, they are
    // output as they are, with how much of the canvas they keep.

    PS_OUTPUT output = writeOutput(acc, 1.0f);

    if (keepsResultColors())
    {
        output.color = acc;
#ifdef WAX_DUAL_SOURCE
        output.factor = float4(1.0f - keep, 1.0f - keep, 1.0f - keep, 1.0f - keep);
#endif
    }

    return output;
}
