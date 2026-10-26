// writeOutput() and premul() come from output.hlsl, the tint, tintColor(),
// evalTintBlock() and the colors buffer from tint.hlsl, which are put in front of
// this file.
//
// Shapes (Command::DrawShape), see GpuShapes for the entries read from the extras
// buffer. The coverage of each pixel by each segment is worked out from the geometry.
// Pixels get the premultiplied colors of the drawn segments covering them, weighted
// by coverage, and are covered by them together. Pixels not covered by any drawn
// segment are left alone.


StructuredBuffer<float4> extras : register(t1);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    nointerpolation uint shapeOfs : TEXCOORD1;
};


float spanCoverage(float a, float b, float p)
{
    return clamp(min(p + 0.5f, b) - max(p - 0.5f, a), 0.0f, 1.0f);
}


float rectCoverage(float4 r, float2 p)
{
    return spanCoverage(r.x, r.z, p.x) * spanCoverage(r.y, r.w, p.y);
}


// Functions below return once, at their end. FXC can take an early return from a
// function as leaving its result uninitialized (warning X4000).

// Part of a pixel inside a straight edge, s from the pixel center along its normal n.

float halfPlaneCoverage(float s, float2 n)
{
    float2 an = abs(n);
    float a = max(an.x, an.y);
    float b = min(an.x, an.y);
    float t = s + 0.5f * (a + b);

    float c = 0.0f;                                     // Outside, t <= 0.

    if (b < 0.001f)
        c = clamp(s + 0.5f, 0.0f, 1.0f);
    else if (t >= a + b)
        c = 1.0f;
    else if (t > 0.0f)
    {
        if (t < b)
            c = t * t / (2.0f * a * b);
        else if (t > a)
            c = 1.0f - (a + b - t) * (a + b - t) / (2.0f * a * b);
        else
            c = (t - 0.5f * b) / a;
    }

    return c;
}


// Part of a pixel inside a circle, v from its center to the pixel center and d its length.

float circleCoverage(float radius, float2 v, float d)
{
    float c = 0.0f;

    if (radius > 0.0f)
        c = d > 0.001f ? halfPlaneCoverage(radius - d, v / d) : clamp(radius + 0.5f, 0.0f, 1.0f);

    return c;
}


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


// Adds a segment covering c of the pixel, if drawn: color times alpha and alpha weighted
// by coverage to acc, coverage to covered. Tint offsets are kept from going negative for
// the same reason as in tintColor().

void addSegment(uint seg, float c, float2 p, inout float4 acc, inout float covered)
{
    if (c <= 0.0f)
        return;

    float4 s = extras[seg];
    if (s.x < 0.0f)
        return;

    float4 col = colors[uint(s.x)];

    [branch]
    if (s.y >= 0.0f)
        col *= evalTintBlock(colors, uint(max(s.y, 0.0f)), p);

    acc += float4(col.rgb * col.a, col.a) * c;
    covered += c;
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


// Splits band, the coverage of the slice area, between the elements: those whose
// boundaries pass through the pixel get the part between them, the element at the
// pixel center the rest. If the center of the ring is inside the pixel, all elements
// get the part of the pixel within their angle.

void ringElements(uint ofs, int n, uint base, float band, float2 p, float2 v, float d, inout float4 acc, inout float covered)
{
    if (n == 1)
    {
        addSegment(ofs + uint(extras[base].y), band, p, acc, covered);
        return;
    }

    [branch]
    if (abs(v.x) < 0.5f && abs(v.y) < 0.5f)
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
        return;
    }

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


void ringCoverage(uint ofs, float2 p, inout float4 acc, inout float covered)
{
    float4 head = extras[ofs];
    int flags = int(head.y);
    float4 circles = extras[ofs + 1];
    float4 inner = extras[ofs + 2];
    float4 rect = extras[ofs + 3];

    float2 v = p - circles.xy;
    float d = length(v);
    bool bSquare = (flags & 1) != 0;

    float c0 = bSquare ? rectCoverage(rect, p) : circleCoverage(circles.z, v, d);
    float c1 = min(bSquare ? rectCoverage(extras[ofs + 4], p) : circleCoverage(circles.w, v, d), c0);
    float c2 = min(circleCoverage(inner.x, v, d), c1);
    float c3 = min(circleCoverage(inner.y, v, d), c2);

    float back = (flags & 2) != 0 ? max(rectCoverage(rect, p) - c0, 0.0f) : 0.0f;
    float outline = c0 - c1 + c2 - c3;
    float band = c1 - c2;

    if ((flags & 16) == 0)
    {
        outline += band;
        band = 0.0f;
    }

    addSegment(ofs + 5, back, p, acc, covered);
    addSegment(ofs + 6, outline, p, acc, covered);
    addSegment(ofs + 7, (flags & 4) != 0 ? c3 : 0.0f, p, acc, covered);

    if (band > 0.0f)
        ringElements(ofs, int(head.z), ofs + uint(head.w), band, p, v, d, acc, covered);
}


// Coverage of the drawn segments of a shape at a pixel center, their colors in acc.

// Lines with caps, see GpuShapes::writeLine(). Their straight edges are half-planes, each as
// its inward normal and how far inside it the pixel center is. The part of the pixel inside
// planes first up to last is worked out exactly, by clipping the pixel to each in turn. Round
// ends add their circle.

float polygonCoverage(float3 planes[6], int first, int last)
{
    // Loop variables are declared once, as FXC lets them live on after their loops (X3078).

    int i;
    int k;

    bool bOutside = false;
    bool bInside = true;

    for (i = first; i < last; i++)
    {
        if (planes[i].z <= -0.7072f)
            bOutside = true;
        if (planes[i].z < 0.7072f)
            bInside = false;
    }

    float c = bOutside ? 0.0f : 1.0f;

    [branch]
    if (!bOutside && !bInside)
    {
        float2 v[10];
        float2 o[10];
        int nv = 4;

        v[0] = float2(-0.5f, -0.5f);
        v[1] = float2(0.5f, -0.5f);
        v[2] = float2(0.5f, 0.5f);
        v[3] = float2(-0.5f, 0.5f);

        [loop]
        for (i = first; i < last && nv >= 3; i++)
        {
            float3 pl = planes[i];

            if (pl.z < 0.7072f)
            {
                int no = 0;

                [loop]
                for (k = 0; k < nv; k++)
                {
                    float2 a = v[k];
                    float2 b = v[k + 1 < nv ? k + 1 : 0];
                    float fa = pl.z + dot(a, pl.xy);
                    float fb = pl.z + dot(b, pl.xy);

                    if (fa >= 0.0f)
                    {
                        o[no] = a;
                        no++;
                    }

                    if ((fa >= 0.0f) != (fb >= 0.0f))
                    {
                        o[no] = a + (b - a) * (fa / (fa - fb));
                        no++;
                    }
                }

                nv = no;

                [loop]
                for (k = 0; k < nv; k++)
                    v[k] = o[k];
            }
        }

        float area = 0.0f;

        [loop]
        for (k = 0; k < nv && nv >= 3; k++)
        {
            float2 a = v[k];
            float2 b = v[k + 1 < nv ? k + 1 : 0];
            area += a.x * b.y - a.y * b.x;
        }

        c = clamp(abs(area) * 0.5f, 0.0f, 1.0f);
    }

    return c;
}


// Adds the half-planes of a cap, at end with u pointing along the line and n across it.
// Round caps add the straight cut through the end, their circle is added by lineCoverage().

void addCapPlanes(int cap, float2 end, float2 u, float2 n, float r, float2 p, inout float3 planes[6], inout int count)
{
    float2 d = p - end;
    float a = dot(d, u);
    int shape = cap & 7;

    if (shape == 1 || shape == 3)
    {
        planes[count] = float3(u, a);
        count++;
    }
    else if (shape == 2)
    {
        planes[count] = float3(u, a + r);
        count++;
    }
    else if (shape == 4)
    {
        float sx = u.x > 0.0f ? 1.0f : -1.0f;
        planes[count] = float3(sx, 0.0f, d.x * sx);
        count++;
    }
    else if (shape == 5)
    {
        float sy = u.y > 0.0f ? 1.0f : -1.0f;
        planes[count] = float3(0.0f, sy, d.y * sy);
        count++;
    }
    else
    {
        float spear = float(cap >> 3) * 0.5f * r;
        float w = dot(d, n);
        float2 g1 = r * u - spear * n;
        float2 g2 = r * u + spear * n;
        float k = length(g1);

        planes[count] = float3(g1 / k, (r * a - spear * w) / k);
        planes[count + 1] = float3(g2 / k, (r * a + spear * w) / k);
        count += 2;
    }
}


void lineCoverage(uint ofs, float2 p, inout float4 acc, inout float covered)
{
    float4 head = extras[ofs];
    float4 ends = extras[ofs + 1];
    float4 dir = extras[ofs + 2];

    float2 u = dir.xy;
    float2 n = float2(-u.y, u.x);
    float r = dir.w;
    int capB = int(head.y);
    int capE = int(head.z);

    float w = dot(p - ends.xy, n);

    float3 planes[6];
    planes[0] = float3(-n, r - w);
    planes[1] = float3(n, r + w);
    planes[2] = float3(0.0f, 0.0f, 0.0f);
    planes[3] = float3(0.0f, 0.0f, 0.0f);
    planes[4] = float3(0.0f, 0.0f, 0.0f);
    planes[5] = float3(0.0f, 0.0f, 0.0f);

    int count = 2;
    addCapPlanes(capB, ends.xy, u, n, r, p, planes, count);
    int countB = count;
    addCapPlanes(capE, ends.zw, -u, -n, r, p, planes, count);

    float c = polygonCoverage(planes, 0, count);

    // A round end is the band cut through the end, together with its circle cut by the other end.

    bool bRoundB = (capB & 7) == 3;
    bool bRoundE = (capE & 7) == 3;

    [branch]
    if (bRoundB || bRoundE)
    {
        float2 dB = p - ends.xy;
        float2 dE = p - ends.zw;
        float discB = bRoundB ? circleCoverage(r, dB, length(dB)) : 0.0f;
        float discE = bRoundE ? circleCoverage(r, dE, length(dE)) : 0.0f;

        float regionB = max(polygonCoverage(planes, 2, countB), discB);
        float regionE = max(polygonCoverage(planes, countB, count), discE);
        c = max(c, max(min(discB, regionE), min(discE, regionB)));
    }

    if (c > 0.0f)
    {
        float4 col = colors[uint(head.w)];
        acc += float4(col.rgb * col.a, col.a) * c;
        covered += c;
    }
}


float shapeCoverage(uint ofs, float2 p, out float4 acc)
{
    acc = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float covered = 0.0f;

    int kind = int(extras[ofs].x);

    if (kind == 1)
        boxCoverage(ofs, p, acc, covered);
    else if (kind == 2)
        ringCoverage(ofs, p, acc, covered);
    else if (kind == 3)
        lineCoverage(ofs, p, acc, covered);

    return covered;
}


PS_OUTPUT main(PS_INPUT input)
{
    float4 acc;

    float covered = shapeCoverage(input.shapeOfs, input.position.xy, acc);

    if (covered < 1.0f / 256.0f)
        discard;

    return writeOutput(acc * premul(input.color * tintColor(input.position.xy)), covered);
}
