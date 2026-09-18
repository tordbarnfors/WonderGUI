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
    float2 colorstripUV : COLORSTRIP;   // Where in the edgemap this corner's colors start.
};


struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texUV : TEXCOORD0;
    float2 colorstripUV : TEXCOORD1;
    nointerpolation float2 colorstripPitch : TEXCOORD2;
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
    output.colorstripUV = input.colorstripUV;

    // How far to step through the palette for each segment. Zero on an axis with
    // no colorstrip, which then keeps reading the same color.

    output.colorstripPitch = extras[input.extrasOfs].xy;

    return output;
}
