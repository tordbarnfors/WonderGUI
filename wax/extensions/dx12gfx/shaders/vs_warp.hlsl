cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // 2/canvasWidth, 2/canvasHeight, in pixels.
    float2 textureSize;     // Not used here.
    uint flags;             // Not used here.
    uint blurOfs;           // Not used here.
    uint edgemapEdges;      // Not used here.
    uint edgemapPitch;      // Not used here.
};

StructuredBuffer<float4> colors : register(t0);


struct VS_INPUT
{
    float2 position : POSITION;         // Canvas pixels, origin top left.
    uint colorOfs : COLOROFS;           // Offset into the color buffer, of the tint color.
    uint extrasOfs : EXTRASOFS;         // Offset into the extras buffer, of the warp.
    float2 texUV : TEXCOORD0;           // Not used here.
};


struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    nointerpolation uint warpOfs : TEXCOORD1;
};


VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Canvas pixels to clip space. Y is flipped, in clip space it points up.

    output.position = float4(input.position.x * canvasScale.x - 1.0f,
                             1.0f - input.position.y * canvasScale.y,
                             0.0f, 1.0f);

    output.color = colors[input.colorOfs];
    output.warpOfs = input.extrasOfs;

    return output;
}
