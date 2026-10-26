cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // 2/canvasWidth, 2/canvasHeight, in pixels.
    float2 textureSize;     // Not used here.
    uint flags;             // Not used here.
};

StructuredBuffer<float4> colors : register(t0);
StructuredBuffer<float4> extras : register(t1);


struct VS_INPUT
{
    float2 position : POSITION;     // Canvas pixels, origin top left.
    uint colorOfs : COLOROFS;       // Offset into the color buffer.
    uint extrasOfs : EXTRASOFS;     // Offset into the extras buffer.
};


struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float4 lineInfo : TEXCOORD0;    // Offset, half width, slope, and whether it is steep.
};


VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Canvas pixels to clip space. Y is flipped, in clip space it points up.

    output.position = float4(input.position.x * canvasScale.x - 1.0f,
                             1.0f - input.position.y * canvasScale.y,
                             0.0f, 1.0f);

    output.color = colors[input.colorOfs];

    // The line itself, which every vertex of the quad carries unchanged. The
    // pixel shader works out how much of each pixel the line covers.

    output.lineInfo = extras[input.extrasOfs];

    return output;
}
