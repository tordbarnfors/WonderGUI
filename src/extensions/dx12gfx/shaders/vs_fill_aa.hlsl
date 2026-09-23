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
    float4 rect : TEXCOORD0;        // Center of the rectangle in xy, its radius in zw.
};


VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Canvas pixels to clip space. Y is flipped, in clip space it points up.

    output.position = float4(input.position.x * canvasScale.x - 1.0f,
                             1.0f - input.position.y * canvasScale.y,
                             0.0f, 1.0f);

    output.color = colors[input.colorOfs];

    // The rectangle we are covering, as a center and a radius. Half a pixel is
    // added to the radius so that a pixel exactly on the edge comes out at full
    // coverage rather than half.

    output.rect = extras[input.extrasOfs];
    output.rect.zw += float2(0.5f, 0.5f);

    return output;
}
