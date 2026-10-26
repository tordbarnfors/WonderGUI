// Warped edgemaps. writeOutput() and premul() come from output.hlsl, tintColor() and
// evalTintBlock() from tint.hlsl and the warp functions from warp.hlsl, which are put
// in front of this file.
//
// Coverage of the segments: edges are linear across the column at the center of the
// pixel (the nearest copy's for a center in the gap between copies on a periodic
// axis), so the border of the area below an edge is sy - edge(sx) = 0. Areas below
// the top of the source, each edge and the bottom of the source are measured, kept
// decreasing as an edge is pushed down to the edges above it, and their differences
// are the parts of the segments. Edges are read from the raw samples of the edgemap,
// the left and right side of each column, see DX12Edgemap.


StructuredBuffer<float4> edgemap : register(t3);


// A segment is colored by a flat color or by a tint placed in the edgemap's own
// rectangle, see ps_segments.hlsl.

float4 segmentColor(uint seg, uint flatColorsOfs, uint tintTableOfs, float2 epos)
{
    float4 col;

    int blockOfs = int(edgemap[tintTableOfs + seg].x);

    [branch]
    if (blockOfs < 0)
        col = edgemap[flatColorsOfs + seg];
    else
        col = evalTintBlock(edgemap, uint(max(blockOfs, 0)), epos);

    return col;
}


// A segment's premultiplied color pc covers part of the pixel, see ps_segments.hlsl.

bool keepsResultColors()
{
    return outputMode == 1u && canvasKind == 1u;
}

float4 segmentPart(float4 pc, float part, inout float keep)
{
    float4 result = pc * part;

    if (keepsResultColors())
    {
        float m = morphFactor;
        float resultAlpha = 1.0f - m + pc.a * m;
        float scale = resultAlpha > 0.0f ? part / resultAlpha : 0.0f;

        keep += scale * (1.0f - m);
        result = pc * (scale * m);
    }

    return result;
}


PS_OUTPUT main(PS_INPUT input)
{
    float2 dx, dy;
    float2 s = warpPosition(input.warpOfs, input.position.xy, dx, dy);

    float4 e0 = extras[input.warpOfs];
    float4 e1 = extras[input.warpOfs + 1];
    float4 e2 = extras[input.warpOfs + 2];
    float4 e3 = extras[input.warpOfs + 3];

    float2 size = e1.zw;
    float2 period = e2.zw;
    float2 gx = float2(dx.x, dy.x);
    float2 gy = float2(dx.y, dy.y);

    uint nEdges = uint(e0.w);
    uint pitch = uint(e3.x);
    uint flatColorsOfs = uint(e3.y);
    uint tintTableOfs = uint(e3.z);

    // Colors are taken, and edges evaluated, at the position in the nearest copy.

    float2 sc = warpNearestCopy(s, size, period);

    uint column = uint(clamp(floor(sc.x), 0.0f, size.x - 1.0f));
    float fx = sc.x - float(column);
    uint base = uint(e3.w) + column * pitch;

    float covX = warpAxisCoverage(s.x, gx, size.x, period.x);

    float insideY = warpInside(size.y, period.y);
    float ext = 0.5f * (abs(gy.x) + abs(gy.y));
    int kBeg = period.y > 0.0f ? -1 : 0;

    float4 tint = premul(input.color * tintColor(input.position.xy));

    float4 acc = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float keep = 0.0f;
    float covered = 0.0f;

    [branch]
    if (covX > 0.0f)
    {
        [loop]
        for (int k = kBeg; k <= -kBeg; k++)
        {
            float sy = s.y - float(k) * period.y;

            [branch]
            if (sy + ext > 0.0f && sy - ext < insideY)
            {
                float above = warpHalfPlane(sy, gy);

                [loop]
                for (uint e = 0; e <= nEdges; e++)
                {
                    float below;

                    if (e < nEdges)
                    {
                        float2 lr = edgemap[base + e].xy;
                        float slope = lr.y - lr.x;
                        below = min(above, warpHalfPlane(sy - lr.x - slope * fx, gy - slope * gx));
                    }
                    else
                        below = min(above, warpHalfPlane(sy - insideY, gy));

                    float part = (above - below) * covX;

                    [branch]
                    if (part > 0.0f)
                    {
                        acc += segmentPart(premul(segmentColor(e, flatColorsOfs, tintTableOfs, sc)) * tint, part, keep);
                        covered += part;
                    }

                    above = below;
                }
            }
        }
    }

    if (covered < 1.0f / 256.0f)
        discard;

    PS_OUTPUT output = writeOutput(acc, covered);

    // Where segmentPart() has divided the segments by their result alphas, they are
    // output as they are, with how much of the canvas they keep: the uncovered part
    // and what the segments keep of it.

    if (keepsResultColors())
    {
        output.color = acc;
#ifdef WAX_DUAL_SOURCE
        float factor = covered - keep;
        output.factor = float4(factor, factor, factor, factor);
#endif
    }

    return output;
}
