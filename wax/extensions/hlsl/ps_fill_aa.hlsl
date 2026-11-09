// writeOutput() and premul() come from output.hlsl, the tint, tintColor() and the
// colors buffer from tint.hlsl, which are put in front of this file.


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float4 rect : TEXCOORD0;        // Center of the rectangle in xy, its radius in zw.
};


PS_OUTPUT main(PS_INPUT input)
{
    // How far this pixel's center is from the center of the rectangle, against
    // how far it may be before it stops being covered. Anything in between is a
    // pixel the edge runs through, and gets partial coverage.

    float2 middleOfs = abs(input.position.xy - input.rect.xy);
    float2 alphas = clamp(input.rect.zw - middleOfs, 0.0f, 1.0f);

    float cov = alphas.x * alphas.y;

    return writeOutput(premul(input.color * tintColor(input.position.xy)) * cov, cov);
}
