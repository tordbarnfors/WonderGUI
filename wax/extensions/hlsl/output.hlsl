// Output of the pixel shaders that draw into canvases. The shader templates put this
// file in front of each of them, after constants.hlsl and ahead of tint.hlsl.
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

// The constants (CanvasInfo, PixelInfo, WarpInfo) come from constants.hlsl.

// Vulkan wants the second color of dual-source blending as index 1 of location 0.

#ifdef __spirv__
#   define WAX_OUTPUT_COLOR     [[vk::location(0)]]
#   define WAX_OUTPUT_FACTOR    [[vk::location(0), vk::index(1)]]
#else
#   define WAX_OUTPUT_COLOR
#   define WAX_OUTPUT_FACTOR
#endif

struct PS_OUTPUT
{
    WAX_OUTPUT_COLOR float4 color : SV_Target0;
#ifdef WAX_DUAL_SOURCE
    WAX_OUTPUT_FACTOR float4 factor : SV_Target1;
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

