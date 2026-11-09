// The constants (CanvasInfo, PixelInfo, WarpInfo) come from constants.hlsl. WarpInfo, where
// the Warpmap is drawn, is set for each warp by the backend.

StructuredBuffer<float4> colors : register(t0);


// The vertices of the Warpmap as they are, see DX12Warpmap. It is drawn with its triangles
// as index buffer, which makes SV_VertexID the index of the vertex.

struct WarpVertex
{
    int2 pos;               // Position in spx of the Warpmap.
    int2 uv;                // Position in spx of the source.
    int w;                  // Weight, 65536 for 1.0.
    int coverage;           // Coverage, 65536 for full.
};

StructuredBuffer<WarpVertex> vertices : register(t4);


struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    nointerpolation uint warpOfs : TEXCOORD1;
    noperspective float3 uvw : TEXCOORD2;
    noperspective float coverage : TEXCOORD3;
};


// u * w, v * w and w go on without perspective correction, so they are linear over the
// canvas, see warp.hlsl. u and v are relative to the first vertex, to keep the precision of
// floats, and the weight is brought down to 1.0 for 65536.

VS_OUTPUT main(uint id : SV_VertexID)
{
    VS_OUTPUT output;

    WarpVertex v = vertices[id];
    float2 p = float2(v.pos) / 64.0f + warpDest;

    // Canvas pixels to clip space. Y is flipped, in clip space it points up.

    output.position = float4(p.x * canvasScale.x - 1.0f, 1.0f - p.y * canvasScale.y, 0.0f, 1.0f);

    output.color = colors[warpColorOfs];
    output.warpOfs = warpExtrasOfs;

    float w = float(v.w) / 65536.0f;
    output.uvw = float3(float2(v.uv - warpRef) * (w / 64.0f), w);
    output.coverage = float(v.coverage) / 65536.0f;

    return output;
}
