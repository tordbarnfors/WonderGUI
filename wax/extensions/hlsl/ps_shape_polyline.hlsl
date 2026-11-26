// Polylines, see GpuShapes::writePolyline(), with shape.hlsl and line.hlsl in front.
//
// A polyline is drawn as a quad around each segment, and the entries of a segment are
// what its quad is given: its first point, its direction and its last point, which is
// the first of the next segment.
//
//   x, y of point, direction of the polyline at it (of length 1, see below)
//   direction of segment x, y (of length 1), its length, its number from 0
//
// The entries of the last point follow those of the last segment, with a length of 0,
// and one entry comes before those of the first: half the thickness, cap of begin, cap
// of end (as for lines) and offset of the color.
//
// A segment covers the band between its sides from its first to its last point, where
// it ends flat. Beyond that is the round join: the part of the disc around the point
// that is beyond the ends of both segments, the wedge they leave open on the outside
// of the turn. Where two segments overlap on the inside of the turn, each has what is
// on its side of the line halving the angle between them, which is at right angle to
// the direction of the polyline at the point: the sum of the directions of the two
// segments, or the normal of the first if they are opposite. So segments and joins
// never overlap and what they cover can be added up.
//
// The quads of segments do overlap, so each pixel is drawn by the segment its center
// is on the side of, and gets what that segment, the ones before and after it and the
// joins between them cover of it. A pixel that more segments than those cover, as
// where they are shorter than a pixel or the polyline turns back within less than its
// thickness, loses what the others cover.


// A plane through a point with normal n, as its normal and how far inside it p is.

float3 planeThrough(float2 pt, float2 n, float2 p)
{
    return float3(n, dot(p - pt, n));
}


// Part of the pixel at p covered by the segment with entries at ofs.

float segmentCoverage(uint ofs, float4 head, float2 p)
{
    float4 from = extras[ofs];
    float4 dir = extras[ofs + 1];
    float4 to = extras[ofs + 2];
    float4 next = extras[ofs + 3];

    float2 u = dir.xy;
    float2 n = float2(-u.y, u.x);
    float r = head.x;

    bool bFirst = dir.w == 0.0f;
    bool bLast = next.z == 0.0f;

    int capB = int(head.y);
    int capE = int(head.z);

    float w = dot(p - from.xy, n);

    // Planes 0 and 1 are the sides, 2 and 3 the begin and 4 and 5 the end: a cap as for
    // lines, or the flat end at a joint and the line halving it.

    float3 planes[WAX_LINE_PLANES];
    planes[0] = float3(-n, r - w);
    planes[1] = float3(n, r + w);

    float3 capFirst;
    float3 capSecond;

    capPlanes(capB, from.xy, u, n, r, p, capFirst, capSecond);
    planes[2] = bFirst ? capFirst : planeThrough(from.xy, u, p);
    planes[3] = bFirst ? capSecond : planeThrough(from.xy, from.zw, p);

    capPlanes(capE, to.xy, -u, -n, r, p, capFirst, capSecond);
    planes[4] = bLast ? capFirst : planeThrough(to.xy, -u, p);
    planes[5] = bLast ? capSecond : planeThrough(to.xy, -to.zw, p);

    // A round cap is its circle, cut by the other end of the segment. Only one end can
    // have one, as a polyline has at least two segments.

    bool bRoundB = bFirst && (capB & 7) == 3;
    bool bRoundE = bLast && (capE & 7) == 3;

    float roundEnd = 0.0f;

    [branch]
    if (bRoundB || bRoundE)
    {
        float2 dR = p - (bRoundB ? from.xy : to.xy);
        float disc = circleCoverage(r, dR, length(dR));

        float3 other[WAX_LINE_PLANES] = { bRoundB ? planes[4] : planes[2], bRoundB ? planes[5] : planes[3],
                                          c_noPlane, c_noPlane, c_noPlane, c_noPlane };

        roundEnd = min(disc, planesCoverage(other, 2));
    }

    return max(planesCoverage(planes, WAX_LINE_PLANES), roundEnd);
}


// Part of the pixel at p covered by the round join at v, between a segment with
// direction ua and the next with ub.
//
// The join is the disc around v beyond the end of the first segment and before the
// begin of the second. Its arc is taken as straight where it is closest to the pixel,
// as circles of other shapes are, which is off for discs not much larger than a pixel.
// So it is also kept inside the tangents a quarter and three quarters of the way along
// the arc, which is within a few percent of the join for turns up to a right angle.

float joinCoverage(float2 v, float2 ua, float2 ub, float r, float2 p)
{
    float2 d = p - v;
    float dist = length(d);

    float c = 0.0f;

    [branch]
    if (dist < r + 0.7072f)
    {
        // The outside of the turn, as the normals of the segments on that side. A
        // polyline turning straight back has it at its end.

        float side = (ua.x * ub.y - ua.y * ub.x) < 0.0f ? 1.0f : -1.0f;
        float2 na = side * float2(-ua.y, ua.x);
        float2 nb = side * float2(-ub.y, ub.x);

        float2 mid = na + nb;
        float midLength = length(mid);
        mid = midLength > 1e-4f ? mid / midLength : ua;

        float2 q1 = normalize(na + mid);
        float2 q3 = normalize(mid + nb);

        float3 planes[WAX_LINE_PLANES];
        planes[0] = float3(ua, dot(d, ua));
        planes[1] = float3(-ub, -dot(d, ub));
        planes[2] = float3(-q1, r - dot(d, q1));
        planes[3] = float3(-q3, r - dot(d, q3));
        planes[4] = dist > 0.001f ? float3(-d / dist, r - dist) : c_noPlane;
        planes[5] = c_noPlane;

        c = planesCoverage(planes, 5);
    }

    return c;
}


PS_OUTPUT main(PS_INPUT input)
{
    // Loop variables are declared once, as FXC lets them live on after their loops (X3078).

    int k;

    float2 p = input.position.xy;
    uint ofs = input.shapeOfs;

    float4 from = extras[ofs];
    float4 dir = extras[ofs + 1];
    float4 to = extras[ofs + 2];
    float4 next = extras[ofs + 3];

    float4 head = extras[ofs - 2u * uint(dir.w) - 1u];

    bool bFirst = dir.w == 0.0f;
    bool bLast = next.z == 0.0f;

    // Which side of the lines halving the joints the pixel is on: of this segment if
    // not behind its begin and before its end.

    float begin = dot(p - from.xy, from.zw);
    float end = dot(p - to.xy, to.zw);

    if ((!bFirst && begin < 0.0f) || (!bLast && end >= 0.0f))
        discard;

    // The segment before, this one and the one after. The others only reach pixels the
    // line halving their joint passes through.

    float c = 0.0f;

    [loop]
    for (k = 0; k < 3; k++)
    {
        bool bDraw = k == 1 || (k == 0 && !bFirst && begin < 0.7072f) || (k == 2 && !bLast && end > -0.7072f);

        [branch]
        if (bDraw)
            c += segmentCoverage(uint(int(ofs) + 2 * (k - 1)), head, p);
    }

    // The joins at its begin and end.

    [loop]
    for (k = 0; k < 2; k++)
    {
        [branch]
        if (k == 0 ? !bFirst : !bLast)
        {
            uint at = ofs + 2u * uint(k);
            c += joinCoverage(extras[at].xy, extras[at - 1].xy, extras[at + 1].xy, head.x, p);
        }
    }

    c = min(c, 1.0f);

    float4 col = colors[uint(head.w)];

    return shapeOutput(input, float4(col.rgb * col.a, col.a) * c, c);
}
