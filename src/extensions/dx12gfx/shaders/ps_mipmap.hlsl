// Draws a mip level from the one above it. The source view holds that level
// only, and a pixel's centre lands between four of its texels, so the bilinear
// sampler averages them. An sRGB view converts to linear on the way in and the
// render target back on the way out, so the average is taken in linear space.

Texture2D source : register(t2);
SamplerState linearSampler : register(s0);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texUV : TEXCOORD;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    return source.SampleLevel(linearSampler, input.texUV, 0);
}
