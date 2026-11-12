// Rings (ShapeKind::Ring), with shape.hlsl in front.


// Direction of v in turns clockwise from 12 o'clock, 0.0 -> 1.0. Worked out from the angle to
// the vertical axis instead of by atan2(), which with fast math can put directions along an
// axis in the wrong quadrant, as -0.0 doesn't count as negative while y / -0.0 does.

float turnsFromTop(float2 v)
{
    float2 a = abs(v);
    float t = a.x <= a.y ? atan(a.x / max(a.y, 1e-20f)) : 1.5707963f - atan(a.y / max(a.x, 1e-20f));
    t *= 0.15915494f;                                   // From the vertical axis, 0.0 -> 0.25.

    if (v.y >= 0.0f)
        t = 0.5f - t;                                   // Below the center.
    if (v.x < 0.0f)
        t = 1.0f - t;                                   // Left of the center.

    return frac(t);
}


// For a pixel with the center of the ring inside it, v from the center to the pixel center:
// the part of the pixel swept by a ray from the center, turning clockwise from 12 o'clock to
// position (in turns). The ray passes the right half of the top side, the right side, the
// bottom side, the left side and the left half of the top side, sweeping a triangle between
// the center and each of them.

float squareSweep(float2 v, float position)
{
    float2 u = float2(sin(6.2831853f * position), -cos(6.2831853f * position));

    float left = 0.5f - v.x;                            // Distances from the center to the sides.
    float right = 0.5f + v.x;
    float top = 0.5f - v.y;
    float bottom = 0.5f + v.y;

    // Where the ray leaves the pixel, through the left or right side if tx < ty.

    float tx = (u.x > 0.0f ? right : left) / max(abs(u.x), 1e-20f);
    float ty = (u.y > 0.0f ? bottom : top) / max(abs(u.y), 1e-20f);
    float2 q = u * min(tx, ty);

    // Halves of the top side are told apart by position, as positions of 1.0 point straight up
    // too and are where the sweep ends.

    float a;                                            // Twice the area.
    if (ty <= tx && u.y < 0.0f && position < 0.5f)
        a = top * q.x;
    else if (tx < ty && u.x > 0.0f)
        a = top * right + right * (top + q.y);
    else if (ty <= tx && u.y > 0.0f)
        a = top * right + right * (top + bottom) + bottom * (right - q.x);
    else if (tx < ty)
        a = top * right + right * (top + bottom) + bottom * (right + left) + left * (bottom - q.y);
    else
        a = 2.0f + top * q.x;

    return 0.5f * a;
}


// The elements around the pixel when the center of the ring is outside of it, by angle.

void ringElementsAround(uint ofs, int n, uint base, float band, float2 p, float2 v, float d, inout float4 acc, inout float covered)
{
    float theta = d > 0.0001f ? turnsFromTop(v) : 0.0f;

    uint lo = 0;                                        // Unsigned, FXC warns of slow signed divides.
    uint hi = uint(n);

    [loop]
    while (lo < hi)
    {
        uint mid = (lo + hi) / 2;
        if (extras[base + mid].x <= theta)
            lo = mid + 1;
        else
            hi = mid;
    }

    int first = lo > 0 ? int(lo) - 1 : n - 1;
    float w = min(0.25f, 0.70710678f / (6.2831853f * max(d, 0.0001f)));
    float behind = frac(theta - extras[base + uint(first)].x);

    [loop]
    for (int i = 0; i < n - 1 && behind < w; i++)
    {
        int prev = first > 0 ? first - 1 : n - 1;
        behind += frac(extras[base + uint(first)].x - extras[base + uint(prev)].x);
        first = prev;
    }

    float g = 1.0f;
    float ahead = -behind;
    int k = first;
    float4 element = extras[base + uint(k)];

    [loop]
    for (int j = 0; j < n; j++)
    {
        int next = k + 1 < n ? k + 1 : 0;
        float4 nextElement = extras[base + uint(next)];
        ahead += frac(nextElement.x - element.x);

        float gEnd = 0.0f;
        if (j < n - 1 && ahead < w)
        {
            float a = 6.2831853f * nextElement.x;
            gEnd = min(g, halfPlaneCoverage(-d * sin(6.2831853f * ahead), float2(cos(a), sin(a))));
        }

        addSegment(ofs + uint(element.y), band * (g - gEnd), p, acc, covered);

        if (gEnd <= 0.0f)
            break;

        g = gEnd;
        k = next;
        element = nextElement;
    }
}


// Splits band, the coverage of the slice area, between the elements: those whose
// boundaries pass through the pixel get the part between them, the element at the
// pixel center the rest. If the center of the ring is inside the pixel, all elements
// get the part of the pixel within their angle.

void ringElements(uint ofs, int n, uint base, float band, float2 p, float2 v, float d, inout float4 acc, inout float covered)
{
    [branch]
    if (n == 1)
        addSegment(ofs + uint(extras[base].y), band, p, acc, covered);
    else if (abs(v.x) < 0.5f && abs(v.y) < 0.5f)
    {
        float firstSweep = squareSweep(v, extras[base].x);
        float sweep = firstSweep;

        [loop]
        for (int e = 0; e < n; e++)
        {
            float nextSweep = e + 1 < n ? squareSweep(v, extras[base + uint(e + 1)].x) : firstSweep + 1.0f;
            addSegment(ofs + uint(extras[base + uint(e)].y), band * max(nextSweep - sweep, 0.0f), p, acc, covered);
            sweep = nextSweep;
        }
    }
    else
        ringElementsAround(ofs, n, base, band, p, v, d, acc, covered);
}


void ringCoverage(uint ofs, float2 p, inout float4 acc, inout float covered)
{
    float4 head = extras[ofs];
    int ringFlags = int(head.y);
    float4 circles = extras[ofs + 1];
    float4 inner = extras[ofs + 2];
    float4 rect = extras[ofs + 3];

    float2 v = p - circles.xy;
    float d = length(v);
    bool bSquare = (ringFlags & 1) != 0;

    float c0 = bSquare ? rectCoverage(rect, p) : circleCoverage(circles.z, v, d);
    float c1 = min(bSquare ? rectCoverage(extras[ofs + 4], p) : circleCoverage(circles.w, v, d), c0);
    float c2 = min(circleCoverage(inner.x, v, d), c1);
    float c3 = min(circleCoverage(inner.y, v, d), c2);

    float back = (ringFlags & 2) != 0 ? max(rectCoverage(rect, p) - c0, 0.0f) : 0.0f;
    float outline = c0 - c1 + c2 - c3;
    float band = c1 - c2;

    if ((ringFlags & 16) == 0)
    {
        outline += band;
        band = 0.0f;
    }

    addSegment(ofs + 5, back, p, acc, covered);
    addSegment(ofs + 6, outline, p, acc, covered);
    addSegment(ofs + 7, (ringFlags & 4) != 0 ? c3 : 0.0f, p, acc, covered);

    if (band > 0.0f)
        ringElements(ofs, int(head.z), ofs + uint(head.w), band, p, v, d, acc, covered);
}


PS_OUTPUT main(PS_INPUT input)
{
    float4 acc = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float covered = 0.0f;

    ringCoverage(input.shapeOfs, input.position.xy, acc, covered);

    return shapeOutput(input, acc, covered);
}
