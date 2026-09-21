cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // Not used here.
    float2 textureSize;     // Not used here.
    uint flags;             // Not used here.
    uint blurOfs;           // Not used here.
    uint edgemapEdges;      // Edges to walk, one fewer than the segments we render.
    uint edgemapPitch;      // Edges per column in the edgemap's buffer.
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


StructuredBuffer<float4> edgemap : register(t3);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texUV : TEXCOORD0;
    float2 colorstripUV : TEXCOORD1;
    nointerpolation float2 colorstripPitch : TEXCOORD2;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    // Walking down a column of the edgemap, one edge at a time. Each edge says
    // where it begins and how steeply it runs, which is enough to tell how much
    // of this pixel falls above it. The difference between one edge's coverage
    // and the next is how much of the pixel belongs to the segment between them.

    int nEdges = int(edgemapEdges);
    int pitch = int(edgemapPitch);

    int column = int(input.texUV.x) * pitch;

    int2 colorOfs = (int2) input.colorstripUV;
    int2 colorstripPitch = (int2) input.colorstripPitch;

    float totalAlpha = 0.0f;
    float3 rgbAcc = float3(0.0f, 0.0f, 0.0f);

    float factor = 1.0f;

    for (int i = 0; i < nEdges; i++)
    {
        // A segment's color is its horizontal colorstrip times its vertical one.
        // One of the two is white when only the other has a strip.

        float4 col = edgemap[colorOfs.x] * edgemap[colorOfs.y];

        colorOfs += colorstripPitch;

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

    float4 lastCol = edgemap[colorOfs.x] * edgemap[colorOfs.y];

    float lastFactor = factor * lastCol.a;

    totalAlpha += lastFactor;
    rgbAcc += lastCol.rgb * lastFactor;

    float4 outColor;

    outColor.a = totalAlpha;
    outColor.rgb = totalAlpha > 0.0f ? rgbAcc / totalAlpha : float3(0.0f, 0.0f, 0.0f);

    return outColor * input.color * tintmapColor(input.position.xy);
}
