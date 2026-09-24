cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // Not used here.
    float2 textureSize;     // Not used here.
    uint flags;             // Not used here.
    uint blurOfs;           // Not used here.
    uint edgemapEdges;      // Edges to walk, one fewer than the segments we render.
    uint edgemapPitch;      // Edges per column in the edgemap's buffer.
};

// The tint, tintColor() and the colors buffer come from tint.hlsl, which is put
// in front of this file.


StructuredBuffer<float4> edgemap : register(t3);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texUV : TEXCOORD0;
    nointerpolation uint2 segColorsOfs : TEXCOORD1;    // Flat colors and tint table in the edgemap buffer.
};


// A segment is colored either by a flat color or by a tint placed in the
// edgemap's own rectangle. The edgemap buffer has a table with the offset of
// each segment's tint block, -1 for a flat colored segment.

float4 segmentColor(uint seg, uint2 segColorsOfs, float2 epos)
{
    float4 col;

    int blockOfs = int(edgemap[segColorsOfs.y + seg].x);

    [branch]
    if (blockOfs < 0)
        col = edgemap[segColorsOfs.x + seg];
    else
        col = evalTintBlock(edgemap, uint(blockOfs), epos);

    return col;
}


float4 main(PS_INPUT input) : SV_TARGET
{
    // Walking down a column of the edgemap, one edge at a time. Each edge says
    // where it begins and how steeply it runs, which is enough to tell how much
    // of this pixel falls above it. The difference between one edge's coverage
    // and the next is how much of the pixel belongs to the segment between them.

    uint nEdges = edgemapEdges;
    uint pitch = edgemapPitch;

    uint column = uint(input.texUV.x) * pitch;

    // Pixel center in edgemap space, which is where segment tints are placed.
    // V is half a pixel back already, see DX12Backend::_drawEdgemap().

    float2 epos = float2(floor(input.texUV.x) + 0.5f, input.texUV.y + 0.5f);

    float totalAlpha = 0.0f;
    float3 rgbAcc = float3(0.0f, 0.0f, 0.0f);

    float factor = 1.0f;

    [loop]
    for (uint i = 0; i < nEdges; i++)
    {
        float4 col = segmentColor(i, input.segColorsOfs, epos);

        float4 edge = edgemap[column + i];

        float x = (input.texUV.y - edge.r) * edge.g;

        float adder = edge.g / 2.0f;

        if (x < 0.0f)
            adder = edge.b;
        else if (x + edge.g > 1.0f)
            adder = edge.a;

        float factor2 = clamp(x + adder, 0.0f, 1.0f);

        float useFactor = (factor - factor2) * col.a;

        totalAlpha += useFactor;
        rgbAcc += col.rgb * useFactor;

        factor = factor2;
    }

    // Whatever is left below the last edge belongs to the last segment.

    float4 lastCol = segmentColor(nEdges, input.segColorsOfs, epos);

    float lastFactor = factor * lastCol.a;

    totalAlpha += lastFactor;
    rgbAcc += lastCol.rgb * lastFactor;

    float4 outColor;

    outColor.a = totalAlpha;
    outColor.rgb = totalAlpha > 0.0f ? rgbAcc / totalAlpha : float3(0.0f, 0.0f, 0.0f);

    return outColor * input.color * tintColor(input.position.xy);
}
