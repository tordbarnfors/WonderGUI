struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float4 lineInfo : TEXCOORD0;    // Offset, half width, slope, and whether it is steep.
};


float4 main(PS_INPUT input) : SV_TARGET
{
    float s = input.lineInfo.x;
    float w = input.lineInfo.y;
    float slope = input.lineInfo.z;

    // A steep line runs mainly vertically, so x and y swap roles. Carrying it as
    // a number rather than a branch keeps the maths to one expression.

    float ifSteep = input.lineInfo.w;
    float ifMild = 1.0f - ifSteep;

    float2 pos = input.position.xy;

    float along = pos.x * ifMild + pos.y * ifSteep;
    float across = pos.x * ifSteep + pos.y * ifMild;

    // Distance from the center of the line, against its half width. A pixel
    // further away than that is outside the line, one closer is fully inside.

    float4 color = input.color;

    color.a = input.color.a * clamp(w - abs(across - s - along * slope), 0.0f, 1.0f);

    return color;
}
