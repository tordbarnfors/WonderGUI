cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // 2/canvasWidth, 2/canvasHeight, in pixels.
    float2 textureSize;     // Size of blit source, in pixels.
    uint flags;             // Bit 0: source is alpha only.
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
    float2 texUV : TEXCOORD0;
};


VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Canvas pixels to clip space. Y is flipped, in clip space it points up.

    output.position = float4(input.position.x * canvasScale.x - 1.0f,
                             1.0f - input.position.y * canvasScale.y,
                             0.0f, 1.0f);

    // First extras entry holds source and destination origin, second one the
    // transform. Source coordinate is the destination offset run through the
    // transform, which is how flipping and rotation are handled.

    float4 srcDst = extras[input.extrasOfs];
    float4 transform = extras[input.extrasOfs + 1];

    float2 src = srcDst.xy;
    float2 dst = srcDst.zw;

    float2 ofs = input.position - dst;

    output.texUV = float2((src.x + ofs.x * transform.x + ofs.y * transform.z) / textureSize.x,
                          (src.y + ofs.x * transform.y + ofs.y * transform.w) / textureSize.y);

    output.color = colors[input.colorOfs];

    return output;
}
