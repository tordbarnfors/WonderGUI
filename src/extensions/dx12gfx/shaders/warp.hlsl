// Warps (Command::WarpBlit and WarpDrawEdgemap), see Warpmap for the definition they
// follow. Shared by the warp pixel shaders, put in front of each of them after
// output.hlsl and tint.hlsl.
//
// The triangles of the Warpmap are drawn, and each pixel gets the position in the source
// at its center from the triangle that draws it, and how fast that changes along x and y
// of the canvas. Coverage is worked out from those, taking each border of the source and
// each edge of an edgemap as straight over the pixel. Extras of a warp, three entries:
//
//   0: size of source in pixels, number of edges, 0.
//   1: source offset plus u and v of the first vertex (brought into the periods) in pixels
//      of the source, periods in pixels of the source (0 if none).
//   2: edgemaps: edges per column, offsets of flat colors, tint table and raw samples in
//      the edgemap buffer. Blits: position of source rectangle in the surface, how far inside
//      its edges positions are kept (0 for a tiling surface, see warpBlitCoverage()).
//
// The mesh of the Warpmap is in its buffer, see DX12Warpmap and vs_warp.hlsl.
//
// Functions return once, at their end, see ps_shape.hlsl. Integer arithmetic is
// unsigned, the compiler warns about signed divisions.


StructuredBuffer<float4> extras : register(t1);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    nointerpolation uint warpOfs : TEXCOORD1;
    noperspective float3 uvw : TEXCOORD2;
    noperspective float coverage : TEXCOORD3;       // Of the vertices, multiplies the coverage of the pixel.
};


// Part of the pixel where f + g.x * dx + g.y * dy >= 0, dx and dy going from -0.5 to
// 0.5 over it, which is linear within (a - b) / 2 of the center and a triangle in a
// corner further out.

float warpHalfPlane(float f, float2 g)
{
    float2 ag = abs(g);
    float a = max(ag.x, ag.y);
    float b = min(ag.x, ag.y);

    float c;

    if (2.0f * f >= a + b)
        c = 1.0f;
    else if (2.0f * f <= -(a + b))
        c = 0.0f;
    else if (2.0f * abs(f) <= a - b)
        c = 0.5f + f / a;
    else
    {
        float t = a + b - 2.0f * abs(f);
        float corner = t * t / (8.0f * a * b);
        c = f > 0.0f ? 1.0f - corner : corner;
    }

    return c;
}


// Length of the source along an axis that is inside of each period.

float warpInside(float size, float period)
{
    return period > 0.0f ? min(size, period) : size;
}


// Position s moved to the nearest copy if in the gap between copies on a periodic
// axis, which is where colors are taken and edges evaluated.

float2 warpNearestCopy(float2 s, float2 size, float2 period)
{
    float2 r = s;

    if (period.x > 0.0f && 2.0f * s.x > warpInside(size.x, period.x) + period.x)
        r.x -= period.x;

    if (period.y > 0.0f && 2.0f * s.y > warpInside(size.y, period.y) + period.y)
        r.y -= period.y;

    return r;
}


// Part of the pixel inside the source along one axis, with the copies of the source
// on a periodic axis. s is the position at the center of the pixel, g how it changes
// along x and y.

float warpAxisCoverage(float s, float2 g, float size, float period)
{
    float inside = warpInside(size, period);
    float ext = 0.5f * (abs(g.x) + abs(g.y));

    float c;

    if ((period > 0.0f && period <= size) || (s - ext >= 0.0f && s + ext <= inside))
        c = 1.0f;
    else if (period <= 0.0f)
        c = warpHalfPlane(s, g) - warpHalfPlane(s - inside, g);
    else
    {
        c = 0.0f;

        [unroll]
        for (int k = -1; k <= 1; k++)
        {
            float lo = float(k) * period;
            c += warpHalfPlane(s - lo, g) - warpHalfPlane(s - lo - inside, g);
        }

        c = min(c, 1.0f);
    }

    return c;
}


// Position in the source at the center of the pixel, wrapped on periodic axes, and how it
// changes along x (dx) and y (dy) of the canvas, from u * w, v * w and w (uvw), which are
// linear over the canvas, so that their derivatives are exact. A pixel with its center
// beyond the horizon is discarded. Must be called before anything else that can discard,
// for the derivatives.

float2 warpPosition(uint ofs, float3 uvw, out float2 dx, out float2 dy)
{
    float3 gx = ddx(uvw);
    float3 gy = ddy(uvw);
    float w = uvw.z;

    if (w <= 0.0f)
        discard;

    float2 uv = uvw.xy / w;

    dx = (gx.xy - uv * gx.z) / w;
    dy = (gy.xy - uv * gy.z) / w;

    float4 e1 = extras[ofs + 1];
    float2 s = uv + e1.xy;

    if (e1.z > 0.0f)
        s.x -= e1.z * floor(s.x / e1.z);

    if (e1.w > 0.0f)
        s.y -= e1.w * floor(s.y / e1.w);

    return s;
}


// Part of the pixel covered by the source of a blit, the position in the surface to
// read in s and its derivatives in dx and dy. A surface that isn't tiling is read as if
// the edge pixels of the source rectangle continued outside it, by keeping positions half
// a texel (bilinear) or a hair (nearest) inside its edges.

float warpBlitCoverage(uint ofs, float3 uvw, float fade, out float2 s, out float2 dx, out float2 dy)
{
    float2 pos = warpPosition(ofs, uvw, dx, dy);

    float4 e0 = extras[ofs];
    float4 e1 = extras[ofs + 1];
    float4 e2 = extras[ofs + 2];

    float cov = warpAxisCoverage(pos.x, float2(dx.x, dy.x), e0.x, e1.z) * saturate(fade);

    if (cov > 0.0f)
        cov *= warpAxisCoverage(pos.y, float2(dx.y, dy.y), e0.y, e1.w);

    s = e2.xy + warpNearestCopy(pos, e0.xy, e1.zw);

    if (e2.z > 0.0f)
    {
        float2 center = e2.xy + 0.5f * e0.xy;
        s = clamp(s, min(e2.xy + e2.z, center), max(e2.xy + e0.xy - e2.z, center));
    }

    s += e2.w;              // Bias for Nearest sampling, see wg_dx12backend.cpp.

    return cov;
}
