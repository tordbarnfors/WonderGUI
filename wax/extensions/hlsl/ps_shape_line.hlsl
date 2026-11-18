// Lines with caps, see GpuShapes::writeLine(), with shape.hlsl in front. Their
// straight edges are half-planes, each as its inward normal and how far inside it the
// pixel center is. The part of the pixel inside all of them is worked out exactly, see
// planesCoverage(). Round ends add their circle.
//
// No array here is indexed by anything but constants once loops are unrolled. Clipping
// the pixel to each plane in turn, as this did before, kept the corners in arrays
// indexed at run time, which drivers keep in scratch memory or read through long
// chains of selects: Mesa made 4760 instructions of it on Intel and could only run it
// as SIMD8.


// Planes of a line, see lineCoverage().

#define WAX_LINE_PLANES     6


// A plane that holds the whole pixel, for unused slots.

static const float3 c_noPlane = float3(1.0f, 0.0f, 1.0f);


// Lines less than this apart in direction (the sine of the angle between them) are taken
// as parallel. At 1e-6 that changes no pixel by more than about that much, while lines that
// should be exactly parallel can come out up to some 1e-7 apart, from normals that are a
// little more or less than 1 long or from multiplications fused into additions.

static const float c_parallel = 1e-6f;


// Clips the part [lo, hi] of a line to where c + d * t >= 0, with rd = 1 / d. Along a
// parallel edge (d of no size) the line is all in if cPar >= 0, or all out, cPar being c
// for the edge exactly parallel. When cPar is 0 the lines are on top of each other, and the
// line is in if bTie.

void clipSpan(float c, float cPar, float d, float rd, bool bTie, inout float lo, inout float hi)
{
    float t = -c * rd;

    if (d > c_parallel)
        lo = max(lo, t);
    else if (d < -c_parallel)
        hi = min(hi, t);
    else if (cPar < 0.0f || (cPar == 0.0f && !bTie))
        hi = -1e10f;
}


float safeRcp(float d)
{
    return 1.0f / (abs(d) > c_parallel ? d : 1.0f);
}


// The part of the pixel inside planes, by Green's theorem: the part inside them is convex,
// and each of its edges lies on an edge of the pixel or on the line of a plane, as the part
// of that line inside all the others. Its area is half the sum over those edges of their
// length times how far inside them the pixel center is, which may be negative. Lines are
// measured by t from where they come closest to the center, q = -z * n + t * (-n.y, n.x),
// which runs the same way around for every edge.
//
// Lines on top of each other: an edge of the pixel and a plane facing the same way count
// once, as the edge, and so do two planes, as the one first in order. Lines facing each
// other both count, and cancel out.
//
// The pixel edges are left, right, top and bottom, each 0.5 from the center, and start out
// from t = -0.5 to 0.5.

// Clips the edges of the pixel by planes[0] and returns its z times the length of its line
// inside the pixel and planes 1 to count - 1, of which the first nAfter come after it in order.

float lineTerm(float3 planes[WAX_LINE_PLANES], int count, int nAfter, inout float pixLo[4], inout float pixHi[4])
{
    int m;

    float3 a = planes[0];
    float rx = safeRcp(a.x);
    float ry = safeRcp(a.y);

    // Along a pixel edge, the plane is parallel when it is along x or y, and then taken as
    // exactly so, with sx or sy for its normal.

    float sx = a.x > 0.0f ? 1.0f : -1.0f;
    float sy = a.y > 0.0f ? 1.0f : -1.0f;

    clipSpan(a.z - 0.5f * a.x, a.z - 0.5f * sx, a.y, ry, true, pixLo[0], pixHi[0]);
    clipSpan(a.z + 0.5f * a.x, a.z + 0.5f * sx, -a.y, -ry, true, pixLo[1], pixHi[1]);
    clipSpan(a.z - 0.5f * a.y, a.z - 0.5f * sy, -a.x, -rx, true, pixLo[2], pixHi[2]);
    clipSpan(a.z + 0.5f * a.y, a.z + 0.5f * sy, a.x, rx, true, pixLo[3], pixHi[3]);

    float lo = -1e10f;
    float hi = 1e10f;

    clipSpan(0.5f - a.z * a.x, 0.5f - a.z * sx, -a.y, -ry, a.x < 0.0f, lo, hi);
    clipSpan(0.5f + a.z * a.x, 0.5f + a.z * sx, a.y, ry, a.x > 0.0f, lo, hi);
    clipSpan(0.5f - a.z * a.y, 0.5f - a.z * sy, a.x, rx, a.y < 0.0f, lo, hi);
    clipSpan(0.5f + a.z * a.y, 0.5f + a.z * sy, -a.x, -rx, a.y > 0.0f, lo, hi);

    // Planes parallel to this one are a.z + b.z apart if facing it, b.z - a.z if not, which
    // is the same seen from either.

    [unroll]
    for (m = 1; m < count; m++)
    {
        float3 b = planes[m];
        float d = a.x * b.y - a.y * b.x;
        float k = dot(a.xy, b.xy);
        bool bFacing = k < 0.0f;

        clipSpan(b.z - a.z * k, bFacing ? a.z + b.z : b.z - a.z, d, safeRcp(d), m <= nAfter || bFacing, lo, hi);
    }

    return a.z * max(hi - lo, 0.0f);
}


float pixelTerm(float pixLo[4], float pixHi[4])
{
    return 0.5f * (max(pixHi[0] - pixLo[0], 0.0f) + max(pixHi[1] - pixLo[1], 0.0f) +
                   max(pixHi[2] - pixLo[2], 0.0f) + max(pixHi[3] - pixLo[3], 0.0f));
}


// Planes moved one slot to the front, the first to the back.

void rotatePlanes(inout float3 planes[WAX_LINE_PLANES], int count)
{
    int m;
    float3 first = planes[0];

    [unroll]
    for (m = 0; m < count - 1; m++)
        planes[m] = planes[m + 1];

    planes[count - 1] = first;
}


// Part of the pixel inside planes 0 to count - 1, count being a constant, for a few planes.

float regionCoverage(float3 planes[WAX_LINE_PLANES], int count)
{
    // Loop variables are declared once, as FXC lets them live on after their loops (X3078).

    int i;

    float pixLo[4] = { -0.5f, -0.5f, -0.5f, -0.5f };
    float pixHi[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
    float sum = 0.0f;

    [unroll]
    for (i = 0; i < count; i++)
    {
        sum += lineTerm(planes, count, count - 1 - i, pixLo, pixHi);
        rotatePlanes(planes, count);
    }

    return clamp(0.5f * (sum + pixelTerm(pixLo, pixHi)), 0.0f, 1.0f);
}


// The same for all planes of a line, in a loop. Unrolled it takes too many registers for
// Intel GPUs to run the shader as SIMD16, and it is only needed where four or more planes
// cross a pixel, which takes lines thinner than 1.5 pixels or spears.

float regionCoverageLoop(float3 planes[WAX_LINE_PLANES])
{
    int i;

    float pixLo[4] = { -0.5f, -0.5f, -0.5f, -0.5f };
    float pixHi[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
    float sum = 0.0f;

    [loop]
    for (i = 0; i < WAX_LINE_PLANES; i++)
    {
        sum += lineTerm(planes, WAX_LINE_PLANES, WAX_LINE_PLANES - 1 - i, pixLo, pixHi);
        rotatePlanes(planes, WAX_LINE_PLANES);
    }

    return clamp(0.5f * (sum + pixelTerm(pixLo, pixHi)), 0.0f, 1.0f);
}


// Part of the pixel inside planes 0 to count - 1, count being a constant.
//
// Most pixels are inside or outside all planes, or on the edge of only one, whose part is
// halfPlaneCoverage(). Planes holding the whole pixel make no difference to the rest, so
// regionCoverage() only gets those crossing it when there are two or three, gathered at the
// front, as it takes about count * count steps. With more it gets all planes of the line,
// count being 6 then.

float planesCoverage(float3 planes[WAX_LINE_PLANES], int count)
{
    int i;
    int j;

    bool bOutside = false;
    int nCrossing = 0;
    float3 crossing[WAX_LINE_PLANES] = { c_noPlane, c_noPlane, c_noPlane, c_noPlane, c_noPlane, c_noPlane };

    [unroll]
    for (i = 0; i < count; i++)
    {
        float3 pl = planes[i];

        if (pl.z <= -0.7072f)
            bOutside = true;

        if (abs(pl.z) < 0.7072f)
        {
            [unroll]
            for (j = 0; j < count; j++)
            {
                if (j <= i && nCrossing == j)
                    crossing[j] = pl;
            }

            nCrossing++;
        }
    }

    float c = bOutside ? 0.0f : 1.0f;

    [branch]
    if (!bOutside && nCrossing > 0)
    {
        [branch]
        if (nCrossing == 1)
            c = halfPlaneCoverage(crossing[0].z, crossing[0].xy);
        else if (nCrossing == 2)
            c = regionCoverage(crossing, 2);
        else if (count > 2 && nCrossing == 3)
            c = regionCoverage(crossing, 3);
        else if (count > 3)
            c = regionCoverageLoop(crossing);
    }

    return c;
}


// The planes of a cap, at end with u pointing along the line and n across it. Spears have
// two, other caps leave the second unused. Round caps get the straight cut through the
// end, their circle is added by lineCoverage(). Caps are never None, see writeLine().

void capPlanes(int cap, float2 end, float2 u, float2 n, float r, float2 p, out float3 first, out float3 second)
{
    float2 d = p - end;
    float a = dot(d, u);
    int shape = cap & 7;

    first = float3(u, a);                               // Flat and Round.
    second = c_noPlane;

    if (shape == 2)
    {
        first = float3(u, a + r);
    }
    else if (shape == 4)
    {
        float sx = u.x > 0.0f ? 1.0f : -1.0f;
        first = float3(sx, 0.0f, d.x * sx);
    }
    else if (shape == 5)
    {
        float sy = u.y > 0.0f ? 1.0f : -1.0f;
        first = float3(0.0f, sy, d.y * sy);
    }
    else if (shape >= 6)
    {
        float spear = float(cap >> 3) * 0.5f * r;
        float w = dot(d, n);
        float2 g1 = r * u - spear * n;
        float2 g2 = r * u + spear * n;
        float k = length(g1);

        first = float3(g1 / k, (r * a - spear * w) / k);
        second = float3(g2 / k, (r * a + spear * w) / k);
    }
}


// Planes 0 and 1 are the sides of the line, 2 and 3 the cap at its begin, 4 and 5 that at
// its end.

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

    float3 planes[WAX_LINE_PLANES];
    planes[0] = float3(-n, r - w);
    planes[1] = float3(n, r + w);
    capPlanes(capB, ends.xy, u, n, r, p, planes[2], planes[3]);
    capPlanes(capE, ends.zw, -u, -n, r, p, planes[4], planes[5]);

    // A round end is the band cut through the end, together with its circle cut by the other
    // end, which with both ends round is the other circle and the cut through the other end.
    // A round end has only that cut as plane. Calling planesCoverage() for both ends here
    // took too many registers to run the shader as SIMD16 on Intel.

    bool bRoundB = (capB & 7) == 3;
    bool bRoundE = (capE & 7) == 3;

    float roundEnds = 0.0f;

    [branch]
    if (bRoundB && bRoundE)
    {
        float2 dB = p - ends.xy;
        float2 dE = p - ends.zw;
        float discB = circleCoverage(r, dB, length(dB));
        float discE = circleCoverage(r, dE, length(dE));

        float regionB = max(halfPlaneCoverage(planes[2].z, planes[2].xy), discB);
        float regionE = max(halfPlaneCoverage(planes[4].z, planes[4].xy), discE);
        roundEnds = max(min(discB, regionE), min(discE, regionB));
    }
    else if (bRoundB || bRoundE)
    {
        float2 dR = p - (bRoundB ? ends.xy : ends.zw);
        float disc = circleCoverage(r, dR, length(dR));

        float3 other[WAX_LINE_PLANES] = { bRoundB ? planes[4] : planes[2], bRoundB ? planes[5] : planes[3],
                                          c_noPlane, c_noPlane, c_noPlane, c_noPlane };

        roundEnds = min(disc, planesCoverage(other, 2));
    }

    float c = max(planesCoverage(planes, WAX_LINE_PLANES), roundEnds);

    if (c > 0.0f)
    {
        float4 col = colors[uint(head.w)];
        acc += float4(col.rgb * col.a, col.a) * c;
        covered += c;
    }
}


PS_OUTPUT main(PS_INPUT input)
{
    float4 acc = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float covered = 0.0f;

    lineCoverage(input.shapeOfs, input.position.xy, acc, covered);

    return shapeOutput(input, acc, covered);
}
