// Lines with caps, see GpuShapes::writeLine(), with shape.hlsl in front. Their
// straight edges are half-planes, each as its inward normal and how far inside it the
// pixel center is. The part of the pixel inside planes first up to last is worked out
// exactly, by clipping the pixel to each in turn. Round ends add their circle.


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


PS_OUTPUT main(PS_INPUT input)
{
    float4 acc = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float covered = 0.0f;

    lineCoverage(input.shapeOfs, input.position.xy, acc, covered);

    return shapeOutput(input, acc, covered);
}
