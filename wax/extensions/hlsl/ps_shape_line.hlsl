PS_OUTPUT main(PS_INPUT input)
{
    float4 acc = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float covered = 0.0f;

    lineCoverage(input.shapeOfs, input.position.xy, acc, covered);

    return shapeOutput(input, acc, covered);
}
