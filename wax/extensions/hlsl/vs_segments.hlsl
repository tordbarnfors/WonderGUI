// The constants (CanvasInfo, PixelInfo, WarpInfo) come from constants.hlsl.

StructuredBuffer<float4> colors : register(t0);
StructuredBuffer<float4> extras : register(t1);


struct VS_INPUT
{
    float2 position : POSITION;         // Canvas pixels, origin top left.
    uint colorOfs : COLOROFS;           // Offset into the color buffer.
    uint extrasOfs : EXTRASOFS;         // Offset into the extras buffer.
};


struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texUV : TEXCOORD0;                          // Column in x, distance down the column in y.
    nointerpolation uint2 segColorsOfs : TEXCOORD1;    // Flat colors and tint table in the edgemap buffer.
    nointerpolation float2 tileSize : TEXCOORD2;       // Size of a tiled edgemap in columns and pixels, zero when not tiled.
};


VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // Canvas pixels to clip space. Y is flipped, in clip space it points up.

    output.position = float4(input.position.x * canvasScale.x - 1.0f,
                             1.0f - input.position.y * canvasScale.y,
                             0.0f, 1.0f);

    output.color = colors[input.colorOfs];

    // First extras entry holds the column and the distance down it at the top
    // left corner of the edgemap, and where that corner is on the canvas, second
    // one the transform. As for blits, the offset from the corner is run through
    // the transform, which is how flipping and rotation are handled.

    float4 srcDst = extras[input.extrasOfs];
    float4 transform = extras[input.extrasOfs + 1];

    float2 ofs = input.position - srcDst.zw;

    output.texUV = float2(srcDst.x + ofs.x * transform.x + ofs.y * transform.z,
                          srcDst.y + ofs.x * transform.y + ofs.y * transform.w);

    // Third entry holds where the edgemap's flat segment colors and tint table
    // are, and its tile size, the same for the whole edgemap.

    float4 colorsAndTile = extras[input.extrasOfs + 2];

    output.segColorsOfs = uint2(colorsAndTile.xy);
    output.tileSize = colorsAndTile.zw;

    return output;
}
