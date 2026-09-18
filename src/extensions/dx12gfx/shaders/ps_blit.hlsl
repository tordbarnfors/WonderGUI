cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;
    float2 textureSize;
    uint flags;             // Bit 0: source is alpha only.
};

Texture2D blitSource : register(t2);
SamplerState blitSampler : register(s0);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texUV : TEXCOORD0;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texel = blitSource.Sample(blitSampler, input.texUV);

    // An alpha only source has no color of its own, it just modulates the tint.

    if ((flags & 1) != 0)
    {
        float4 color = input.color;
        color.a *= texel.a;
        return color;
    }

    return texel * input.color;
}
