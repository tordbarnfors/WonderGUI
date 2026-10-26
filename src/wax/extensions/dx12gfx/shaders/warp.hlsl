// Warps (Command::WarpBlit and WarpDrawEdgemap), see Warpmap for the definition they
// follow. Shared by the warp pixel shaders, put in front of each of them after
// output.hlsl and tint.hlsl.
//
// Each pixel finds the cell of the Warpmap it is in and interpolates the position in
// the source at its center, and how fast that changes along x and y of the canvas.
// Coverage is worked out from those, taking each border of the source and each edge
// of an edgemap as straight over the pixel. Extras of a warp, four entries:
//
//   0: position of Warpmap on canvas, cell size, number of edges.
//   1: size of Warpmap, size of source in pixels.
//   2: source offset in pixels of the source, periods in pixels of the source (0 if none).
//   3: edgemaps: edges per column, offsets of flat colors, tint table and raw samples in
//      the edgemap buffer. Blits: position of source rectangle in the surface, how far inside
//      its edges positions are kept (0 for a tiling surface, see warpBlitCoverage()).
//
// The cells of the Warpmap are in its buffer, see DX12Warpmap.
//
// Functions return once, at their end, see ps_shape.hlsl. Integer arithmetic is
// unsigned, the compiler warns about signed divisions.


StructuredBuffer<float4> extras : register(t1);
StructuredBuffer<float4> warpmap : register(t4);


struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    nointerpolation uint warpOfs : TEXCOORD1;
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


// Position in the source of the pixel center p, wrapped on periodic axes, and how it
// changes along x (dx) and y (dy) of the canvas. The cell is found with integers, as a
// division in floats can miss a whole number.

float2 warpPosition(uint ofs, float2 p, out float2 dx, out float2 dy)
{
    float4 e0 = extras[ofs];
    float4 e1 = extras[ofs + 1];
    float4 e2 = extras[ofs + 2];

    float2 q = p - e0.xy;
    uint cellSize = uint(e0.z);
    uint2 cells = (uint2(e1.xy) + cellSize - 1u) / cellSize;
    uint2 cell = min(uint2(floor(max(q, 0.0f))) / cellSize, cells - 1u);
    float2 c0 = float2(cell * cellSize);
    float2 c1 = min(c0 + float(cellSize), e1.xy);
    float2 f = (q - c0) / (c1 - c0);

    uint i = (cell.y * cells.x + cell.x) * 2u;
    float4 top = warpmap[i];
    float4 bottom = warpmap[i + 1u];

    float2 u = lerp(lerp(top.xy, top.zw, f.x), lerp(bottom.xy, bottom.zw, f.x), f.y);
    float2 s = u * e1.zw + e2.xy;

    dx = lerp(top.zw - top.xy, bottom.zw - bottom.xy, f.y) / (c1.x - c0.x) * e1.zw;
    dy = lerp(bottom.xy - top.xy, bottom.zw - top.zw, f.x) / (c1.y - c0.y) * e1.zw;

    if (e2.z > 0.0f)
        s.x -= e2.z * floor(s.x / e2.z);

    if (e2.w > 0.0f)
        s.y -= e2.w * floor(s.y / e2.w);

    return s;
}


// Part of the pixel covered by the source of a blit, the position in the surface to
// read in s and its derivatives in dx and dy. A surface that isn't tiling is read as if
// the edge pixels of the source rectangle continued outside it, by keeping positions half
// a texel (bilinear) or a hair (nearest) inside its edges.

float warpBlitCoverage(uint ofs, float2 p, out float2 s, out float2 dx, out float2 dy)
{
    float2 pos = warpPosition(ofs, p, dx, dy);

    float4 e1 = extras[ofs + 1];
    float4 e2 = extras[ofs + 2];
    float4 e3 = extras[ofs + 3];

    float cov = warpAxisCoverage(pos.x, float2(dx.x, dy.x), e1.z, e2.z);

    if (cov > 0.0f)
        cov *= warpAxisCoverage(pos.y, float2(dx.y, dy.y), e1.w, e2.w);

    s = e3.xy + warpNearestCopy(pos, e1.zw, e2.zw);

    if (e3.z > 0.0f)
    {
        float2 center = e3.xy + 0.5f * e1.zw;
        s = clamp(s, min(e3.xy + e3.z, center), max(e3.xy + e1.zw - e3.z, center));
    }

    return cov;
}
