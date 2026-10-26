// writeOutput() and premul() come from output.hlsl, the tint, tintColor() and the
// colors buffer from tint.hlsl, which are put in front of this file.


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


PS_OUTPUT main(PS_INPUT input)
{
    return writeOutput(premul(input.color * tintColor(input.position.xy)), 1.0f);
}
