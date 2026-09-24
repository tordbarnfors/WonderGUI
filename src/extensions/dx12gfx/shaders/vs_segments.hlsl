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
StructuredBuffer<float4> extras : register(t1);


struct VS_INPUT
{
    float2 position : POSITION;         // Canvas pixels, origin top left.
    uint colorOfs : COLOROFS;           // Offset into the color buffer.
    uint extrasOfs : EXTRASOFS;         // Offset into the extras buffer.
    float2 texUV : TEXCOORD0;           // Column in x, distance down the column in y.
};


struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texUV : TEXCOORD0;
    nointerpolation uint2 segColorsOfs : TEXCOORD1;    // Flat colors and tint table in the edgemap buffer.
};


VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Canvas pixels to clip space. Y is flipped, in clip space it points up.

    output.position = float4(input.position.x * canvasScale.x - 1.0f,
                             1.0f - input.position.y * canvasScale.y,
                             0.0f, 1.0f);

    output.color = colors[input.colorOfs];

    output.texUV = input.texUV;

    // Where the edgemap's flat segment colors and tint table are, the same for
    // the whole edgemap.

    output.segColorsOfs = uint2(extras[input.extrasOfs].xy);

    return output;
}
