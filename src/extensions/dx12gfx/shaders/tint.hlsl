// The tint, shared by all pixel shaders that apply one. wg_dx12shaders.cpp.template
// puts this file in front of each of them, D3DCompile has no include handler here.
//
// A tint block is a run of float4 entries, see TintTools::writeGpuTintBlock():
//
//   [0]                 (nLayers, 0, 0, 0)
//   per layer:
//     [+0]              (shape, spread, nStops, weight)
//     [+1]              geometry. Linear: (a, b, c, -) with t = a*x + b*y + c.
//                                 Radial: (centerX, centerY, invRadiusX, invRadiusY).
//     [+2]              (1 if stop colors are sRGB encoded else 0, 0, 0, 0)
//     [+3]...           stop positions, four per entry, unused ones 2.0.
//     then              one color per stop.
//
// Positions are canvas pixels, the device tint is evaluated at pixel centers. Its
// block lives in the color buffer and the root constants say where, -1 for none.

cbuffer TintInfo : register(b1)
{
    int tintOfs;            // Offset of the tint block in the color buffer, -1 for no tint.
    int3 tintPadding;
};

StructuredBuffer<float4> colors : register(t0);


float tintStopPos(StructuredBuffer<float4> buf, uint posOfs, uint i)
{
    float4 v = buf[posOfs + (i >> 2)];
    uint c = i & 3;

    return c == 0 ? v.x : (c == 1 ? v.y : (c == 2 ? v.z : v.w));
}


float3 tintSRGBToLinear(float3 c)
{
    float3 lo = c / 12.92f;
    float3 hi = pow(abs((c + 0.055f) / 1.055f), 2.4f);        // abs() keeps FXC from warning, c is never negative.

    return lerp(hi, lo, step(c, 0.04045f));
}


float4 evalTintBlock(StructuredBuffer<float4> buf, uint ofs, float2 pos)
{
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

    uint nLayers = uint(buf[ofs].x);
    uint p = ofs + 1;

    [loop]
    for (uint l = 0; l < nLayers; l++)
    {
        float4 info = buf[p];
        float4 geo = buf[p + 1];
        float srgb = buf[p + 2].x;

        uint nStops = uint(info.z);
        uint posOfs = p + 3;
        uint colOfs = posOfs + ((nStops + 3) >> 2);

        [branch]
        if (nStops > 0)
        {
            // Position along the gradient, with spread applied.

            float t;

            if (info.x < 0.5f)
                t = geo.x * pos.x + geo.y * pos.y + geo.z;
            else
                t = length((pos - geo.xy) * geo.zw);

            t = clamp(t, -1e7f, 1e7f);

            uint spread = uint(info.y);

            if (spread == 0)
                t = saturate(t);
            else if (spread == 1)
                t = t - floor(t);
            else
            {
                float r = t - 2.0f * floor(t * 0.5f);
                t = r > 1.0f ? 2.0f - r : r;
            }

            // Last stop at or before t. Stops with equal positions make a hard edge.

            int k = -1;

            [loop]
            for (uint i = 0; i < nStops; i++)
            {
                if (tintStopPos(buf, posOfs, i) <= t)
                    k = int(i);
            }

            float4 c;

            if (k < 0)
                c = buf[colOfs];
            else if (uint(k) == nStops - 1)
                c = buf[colOfs + uint(k)];
            else
            {
                uint uk = uint(k);
                float p0 = tintStopPos(buf, posOfs, uk);
                float p1 = tintStopPos(buf, posOfs, uk + 1);

                c = lerp(buf[colOfs + uk], buf[colOfs + uk + 1], (t - p0) / (p1 - p0));
            }

            if (srgb > 0.5f)
                c.rgb = tintSRGBToLinear(c.rgb);

            result += c * info.w;
        }

        p = colOfs + nStops;
    }

    return result;
}


// Color of the device tint at a canvas pixel. White when there is no tint.

float4 tintColor(float2 pixelPos)
{
    float4 tint = float4(1.0f, 1.0f, 1.0f, 1.0f);

    [branch]
    if (tintOfs >= 0)
        tint = evalTintBlock(colors, uint(tintOfs), pixelPos);

    return tint;
}

