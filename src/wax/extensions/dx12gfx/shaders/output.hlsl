// Output of the pixel shaders that draw into canvases, and the root constants they
// share. wax_dx12shaders.cpp.template puts this file in front of each of them, ahead
// of tint.hlsl.
//
// Canvases with alpha hold premultiplied colors. A pixel shader works out the
// premultiplied color of what it draws times the part of the pixel it covers (pc)
// and that part (cov), and hands them to writeOutput(). That passes them on as the
// blend mode of the pipeline needs them, the second color being the second source
// of dual-source blending, see DX12Backend::_createPipeline(). It is only output by
// pipelines that blend with it, which compile the shaders with WAX_DUAL_SOURCE.
//
//   outputMode 0: pc, with its alpha as the second color (Blend, Subtract and
//                 Multiply).
//   outputMode 1: pc times morphFactor (1 but for Morph), with the covered part
//                 times morphFactor as the second color, which tells the blender
//                 how much of the canvas to keep (Replace and Morph). Canvases
//                 without alpha keep the color of the result, which is the result
//                 divided by its alpha. The canvas is opaque, so that alpha only
//                 depends on the source, and the result can be divided by it here.
//                 A transparent Replace writes the color of the source, which
//                 premultiplied is black.
//   outputMode 2: the source faded towards white by the alpha of pc (Min).
//   outputMode 3: pc with its strongest channel as alpha, by which Add, Invert and
//                 Max raise the alpha of the canvas.
//
// Alpha only canvases (A8_UNORM) keep their value in alpha. Paint modes work on it
// as on other canvases, effect modes take the alpha of the source as the value,
// like an opaque source of that color. Mode 1 is also used by Multiply there.

cbuffer PixelInfo : register(b1)
{
    int tintOfs;            // Offset of the tint block in the color buffer, -1 for no tint. See tint.hlsl.
    uint outputMode;        // See above.
    uint canvasKind;        // 0: canvas with alpha, 1: canvas without alpha, 2: alpha only canvas.
    float morphFactor;      // 1 unless the blend mode is Morph.
};


struct PS_OUTPUT
{
    float4 color : SV_Target0;
#ifdef WAX_DUAL_SOURCE
    float4 factor : SV_Target1;
#endif
};


float4 premul(float4 c)
{
    return float4(c.rgb * c.a, c.a);
}


PS_OUTPUT writeOutput(float4 pc, float cov)
{
    float4 color;
    float4 factor;

    if (canvasKind == 2u)
    {
        float pa = pc.a;

        if (outputMode == 2u)
        {
            color = 1.0f - cov + pa;
            factor = 0.0f;
        }
        else if (outputMode == 1u)
        {
            color = pa * morphFactor;
            factor = cov * morphFactor;
        }
        else
        {
            color = pa;
            factor = pa;
        }
    }
    else if (outputMode == 3u)
    {
        color = float4(pc.rgb, max(pc.r, max(pc.g, pc.b)));
        factor = 0.0f;
    }
    else if (outputMode == 2u)
    {
        color = float4(1.0f - pc.a + pc.rgb, 1.0f);
        factor = 0.0f;
    }
    else if (outputMode == 1u)
    {
        float m = morphFactor;
        float k = 1.0f;

        if (canvasKind == 1u)
        {
            float resultAlpha = 1.0f - m + (cov > 0.0f ? pc.a / cov : 0.0f) * m;
            k = resultAlpha > 0.0f ? 1.0f / resultAlpha : 0.0f;
        }

        color = pc * (m * k);
        factor = cov * (1.0f - (1.0f - m) * k);
    }
    else
    {
        color = pc;
        factor = pc.a;
    }

    PS_OUTPUT output;

    output.color = color;
#ifdef WAX_DUAL_SOURCE
    output.factor = factor;
#endif

    return output;
}

