// The tint, tintColor() and the colors buffer come from tint.hlsl, which is put
// in front of this file.


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float4 rect : TEXCOORD0;        // Center of the rectangle in xy, its radius in zw.
};


float4 main(PS_INPUT input) : SV_TARGET
{
    // How far this pixel's center is from the center of the rectangle, against
    // how far it may be before it stops being covered. Anything in between is a
    // pixel the edge runs through, and gets partial coverage.

    float4 color = input.color;

    float2 middleOfs = abs(input.position.xy - input.rect.xy);
    float2 alphas = clamp(input.rect.zw - middleOfs, 0.0f, 1.0f);

    color.a = input.color.a * alphas.x * alphas.y;

    return color * tintColor(input.position.xy);
}
