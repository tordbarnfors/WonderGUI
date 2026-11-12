// writeOutput() and premul() come from output.hlsl, the tint, tintColor(),
// evalTintBlock() and the colors buffer from tint.hlsl, which are put in front of
// this file, and one of ps_shape_box.hlsl, ps_shape_ring.hlsl and ps_shape_line.hlsl
// after it.
//
// Shapes (Command::DrawShape and lines with caps), see GpuShapes for the entries read
// from the extras buffer. The coverage of each pixel by each segment is worked out
// from the geometry. Pixels get the premultiplied colors of the drawn segments
// covering them, weighted by coverage, and are covered by them together. Pixels not
// covered by any drawn segment are left alone.
//
// Each kind of shape has a pixel shader of its own. In one shader for all of them the
// rounded boxes, the cheapest, took 1.6 times as long on Intel (Mesa) as on their own,
// rings and lines 5-10% longer.


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


// Adds a segment covering c of the pixel, if drawn: color times alpha and alpha weighted
// by coverage to acc, coverage to covered. Tint offsets are kept from going negative for
// the same reason as in tintColor().

void addSegment(uint seg, float c, float2 p, inout float4 acc, inout float covered)
{
    [branch]
    if (c > 0.0f)
    {
        float4 s = extras[seg];

        [branch]
        if (s.x >= 0.0f)
        {
            float4 col = colors[uint(s.x)];

            [branch]
            if (s.y >= 0.0f)
                col *= evalTintBlock(colors, uint(max(s.y, 0.0f)), p);

            acc += float4(col.rgb * col.a, col.a) * c;
            covered += c;
        }
    }
}


// What a shader writes for a pixel, with the colors and coverage of the drawn segments.

PS_OUTPUT shapeOutput(PS_INPUT input, float4 acc, float covered)
{
    if (covered < 1.0f / 256.0f)
        discard;

    return writeOutput(acc * premul(input.color * tintColor(input.position.xy)), covered);
}
