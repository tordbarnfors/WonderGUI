// The tint, tintColor() and the colors buffer come from tint.hlsl, which is put
// in front of this file.


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


float4 main(PS_INPUT input) : SV_TARGET
{
    return input.color * tintColor(input.position.xy);
}
