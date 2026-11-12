// Rounded boxes (ShapeKind::RoundedBox), with shape.hlsl in front.


float roundedRectCoverage(float4 r, float4 radii, float2 p)
{
    // The rounded corner, if any, whose rounding the pixel center is in. Square corners are
    // left to rectCoverage(), which also covers the pixels just outside them partly.

    bool bCorner = true;
    float2 center = float2(0.0f, 0.0f);
    float radius = 0.0f;

    if (radii.x > 0.0f && p.x < r.x + radii.x && p.y < r.y + radii.x)
    {
        radius = radii.x;
        center = float2(r.x + radius, r.y + radius);
    }
    else if (radii.y > 0.0f && p.x > r.z - radii.y && p.y < r.y + radii.y)
    {
        radius = radii.y;
        center = float2(r.z - radius, r.y + radius);
    }
    else if (radii.z > 0.0f && p.x > r.z - radii.z && p.y > r.w - radii.z)
    {
        radius = radii.z;
        center = float2(r.z - radius, r.w - radius);
    }
    else if (radii.w > 0.0f && p.x < r.x + radii.w && p.y > r.w - radii.w)
    {
        radius = radii.w;
        center = float2(r.x + radius, r.w - radius);
    }
    else
        bCorner = false;

    float c = rectCoverage(r, p);

    if (bCorner)
    {
        float2 v = p - center;
        c = min(c, circleCoverage(radius, v, length(v)));
    }

    return c;
}


void boxCoverage(uint ofs, float2 p, inout float4 acc, inout float covered)
{
    float outer = roundedRectCoverage(extras[ofs + 1], extras[ofs + 2], p);
    float fill = outer;

    if ((int(extras[ofs].y) & 1) != 0)
        fill = min(outer, roundedRectCoverage(extras[ofs + 3], extras[ofs + 4], p));

    addSegment(ofs + 5, fill, p, acc, covered);
    addSegment(ofs + 6, outer - fill, p, acc, covered);
}


PS_OUTPUT main(PS_INPUT input)
{
    float4 acc = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float covered = 0.0f;

    boxCoverage(input.shapeOfs, input.position.xy, acc, covered);

    return shapeOutput(input, acc, covered);
}
