
#include <metal_stdlib>
using namespace metal; 


//____ Vertex ___________________________________________________

typedef struct
{
    vector_int2     coord;
    int             colorOfs;              // Offset into color buffer.
    int             extrasOfs;              // Offset into extras buffer.
    vector_float2   uv;
    int             tintOfs;               // Offset of tint block in color buffer, -1 for none.
    int             padding;
} Vertex;

//____ Uniform ______________________________________________

typedef struct             // Uniform buffer object for canvas information.
{                               // DO NOT CHANGE ORDER OF MEMBERS!!!
    vector_float2   canvasDim;
    int        canvasYOfs;
    int        canvasYMul;

    vector_float4 flatTint;

    vector_int2     texSize;
} Uniform;


//____ BlurUniform ______________________________________________

typedef struct             // Uniform buffer object for fragment shader blur information.
{                               // DO NOT CHANGE ORDER OF MEMBERS!!!
    vector_float4     colorMtx[9];
    vector_float2     offset[9];
} BlurUniform;


//____ Output ______________________________________________________________
//
// Canvases with alpha hold premultiplied colors. A fragment shader works out the premultiplied
// color of what it draws times the part of the pixel it covers (pc) and that part (cov), and
// hands them to writeColor(), or their alphas to writeAlpha() on Alpha_8 canvases. These pass
// them on as the blend mode of the pipeline needs them, the second color being the second source
// of dual-source blending (see MetalBackend::_compileRenderPipeline()). Each pipeline sets the
// function constants:
//
//   c_outputMode 0: pc, with its alpha as the second color (Blend, Subtract and Multiply).
//   c_outputMode 1: pc times m, with the covered part times m as the second color, which tells
//                   the blender how much of the canvas to keep. m is the morph factor for Morph
//                   (c_morph) and 1 for Replace. Canvases without alpha (c_noAlpha) keep the color
//                   of the result, which is the result divided by its alpha. The canvas is opaque,
//                   so that alpha only depends on the source, and the result can be divided by
//                   it here. A transparent Replace writes the color of the source, which
//                   premultiplied is black.
//   c_outputMode 2: the source faded towards white by the alpha of pc (Min).
//   c_outputMode 3: pc with its strongest channel as alpha, by which Add, Invert and Max raise
//                   the alpha of the canvas.
//
// On Alpha_8 canvases, which hold alpha in the red channel, effect modes take the alpha of the
// source as the value, like an opaque source of that color, and mode 1 is also used by Multiply.

constant int	c_outputMode	[[function_constant(0)]];
constant bool	c_noAlpha		[[function_constant(1)]];
constant bool	c_morph			[[function_constant(2)]];

typedef struct
{
	float4	color	[[color(0), index(0)]];
	float4	factor	[[color(0), index(1)]];
} FragOutput;

inline float4 premul(float4 c)
{
	return float4(c.rgb * c.a, c.a);
}

inline FragOutput writeColor(float4 pc, float cov, float morphFactor)
{
	FragOutput out;

	if( c_outputMode == 3 )
	{
		out.color = float4(pc.rgb, max(pc.r, max(pc.g, pc.b)));
		out.factor = float4(0.f);
	}
	else if( c_outputMode == 2 )
	{
		out.color = float4(float3(1.f - pc.a) + pc.rgb, 1.f);
		out.factor = float4(0.f);
	}
	else if( c_outputMode == 1 )
	{
		float m = c_morph ? morphFactor : 1.f;
		float k = 1.f;

		if( c_noAlpha )
		{
			float resultAlpha = 1.f - m + (cov > 0.f ? pc.a / cov : 0.f) * m;
			k = resultAlpha > 0.f ? 1.f / resultAlpha : 0.f;
		}

		out.color = pc * (m * k);
		out.factor = float4(cov * (1.f - (1.f - m) * k));
	}
	else
	{
		out.color = pc;
		out.factor = float4(pc.a);
	}

	return out;
}

inline FragOutput writeAlpha(float pa, float cov, float morphFactor)
{
	FragOutput out;

	if( c_outputMode == 2 )
	{
		out.color = float4(1.f - cov + pa);
		out.factor = float4(0.f);
	}
	else if( c_outputMode == 1 )
	{
		float m = c_morph ? morphFactor : 1.f;

		out.color = float4(pa * m);
		out.factor = float4(cov * m);
	}
	else
	{
		out.color = float4(pa);
		out.factor = float4(pa);
	}

	return out;
}


//____ LineFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float s;
    float w;
    float slope;
    float ifSteep;
    float ifMild;
    int tintOfs [[flat]];

} LineFragInput;

//____ FillFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
} FillFragInput;

//____ FillTintmapFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    int tintOfs [[flat]];
} FillTintmapFragInput;

//____ FillAAFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float4 rect;
} FillAAFragInput;

//____ FillAATintmapFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float4 rect;
    int tintOfs [[flat]];
} FillAATintmapFragInput;


//____ BlitFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float2 texUV;
} BlitFragInput;

//____ BlitTintmapFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float2 texUV;
    int tintOfs [[flat]];
} BlitTintmapFragInput;

//____ PaletteBlitInterpolateFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float2 uvFrac;
 } PaletteBlitInterpolateFragInput;

//____ PaletteBlitInterpolateTintmapFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float2 uvFrac;
    int tintOfs [[flat]];
 } PaletteBlitInterpolateTintmapFragInput;


//____ SegmentsFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
	float4 color;
    float2 texUV;
	float2	tileSize [[flat]];			// Size of a tiled edgemap in columns and pixels, zero when not tiled.
	int		edgemapPitch [[flat]];
	int		flatColorsOfs [[flat]];		// Flat segment colors in edgemap buffer.
	int		tintTableOfs [[flat]];		// Table of segment tint block offsets in edgemap buffer, -1 for flat.
	int		tintOfs [[flat]];			// Device tint block in color buffer, -1 for none.
} SegmentsFragInput;


//____ Tint ______________________________________________________________
//
// Evaluates a tint block (see TintTools::writeGpuTintBlock()) at a position,
// canvas pixels for the device tint, edgemap pixels for segment tints.
//
//   [0]            (nLayers, 0, 0, 0)
//   per layer:
//     [+0]         (shape, spread, nStops, weight)
//     [+1]         geometry. Linear: t = x*a + y*b + c. Radial: (centerX, centerY, invRadiusX, invRadiusY).
//     [+2]         (1 if stop colors are sRGB encoded else 0, 0, 0, 0)
//     [+3]...      stop positions, four per entry, unused ones 2.0.
//     then         one color per stop.

template <typename BUF>
inline float tintStopPos(BUF pBuf, int posOfs, int i)
{
    return pBuf[posOfs + (i >> 2)][i & 3];
}

template <typename BUF>
float4 evalTint(BUF pBuf, int ofs, float2 pos)
{
    int nLayers = int(pBuf[ofs].x);
    int p = ofs + 1;
    float4 result = float4(0.0);

    for( int l = 0 ; l < nLayers ; l++ )
    {
        float4 info = pBuf[p];
        float4 geo = pBuf[p + 1];
        float srgb = pBuf[p + 2].x;

        int nStops = int(info.z);
        int posOfs = p + 3;
        int colOfs = posOfs + (nStops + 3) / 4;

        if( nStops > 0 )
        {
            float t = info.x < 0.5f ? geo.x * pos.x + geo.y * pos.y + geo.z : length((pos - geo.xy) * geo.zw);
            t = clamp(t, -1e7f, 1e7f);

            int spread = int(info.y);
            if( spread == 0 )
                t = clamp(t, 0.f, 1.f);
            else if( spread == 1 )
                t = t - floor(t);
            else
            {
                float r = t - 2.f * floor(t * 0.5f);
                t = r > 1.f ? 2.f - r : r;
            }

            int k = -1;
            for( int i = 0 ; i < nStops ; i++ )
            {
                if( tintStopPos(pBuf, posOfs, i) <= t )
                    k = i;
            }

            float4 c;
            if( k < 0 )
                c = pBuf[colOfs];
            else if( k == nStops - 1 )
                c = pBuf[colOfs + k];
            else
            {
                float p0 = tintStopPos(pBuf, posOfs, k);
                float p1 = tintStopPos(pBuf, posOfs, k + 1);
                c = mix(pBuf[colOfs + k], pBuf[colOfs + k + 1], float4((t - p0) / (p1 - p0)));
            }

            if( srgb > 0.5f )
            {
                float3 lo = c.rgb / 12.92f;
                float3 hi = pow((c.rgb + 0.055f) / 1.055f, float3(2.4f));
                c.rgb = select(hi, lo, c.rgb <= float3(0.04045f));
            }

            result += c * info.w;
        }

        p = colOfs + nStops;
    }

    return result;
}


//____ lineVertexShader() ____________________________________________

vertex LineFragInput
lineVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pColor [[buffer(1)]],
             constant vector_float4  *pExtras [[buffer(2)]],
             constant Uniform * pUniform[[buffer(3)]])
{
    LineFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;


    int     colOfs = pVertices[vertexID].colorOfs;
    out.color = pUniform->flatTint * pColor[colOfs];

    int     eOfs = pVertices[vertexID].extrasOfs;
    float4 x = pExtras[eOfs];
    out.s = x.x;
    out.w = x.y;
    out.slope = x.z;
    out.ifSteep = x.w;
    out.ifMild = 1.0 - out.ifSteep;
    out.tintOfs = pVertices[vertexID].tintOfs;

    return out;
}

//____ lineFragmentShader() ____________________________________________

inline float lineCoverage(LineFragInput in)
{
    return clamp(in.w - abs(in.position.x*in.ifSteep + in.position.y*in.ifMild - in.s - (in.position.x*in.ifMild + in.position.y*in.ifSteep) * in.slope), 0.0, 1.0);
}

inline float4 lineColor(LineFragInput in, constant float4 * pColor)
{
    return in.tintOfs >= 0 ? evalTint(pColor, in.tintOfs, in.position.xy) * in.color : in.color;
}

fragment FragOutput lineFragmentShader(LineFragInput in [[stage_in]],
                                       constant float4 * pColor [[buffer(0)]],
                                       constant float & morphFactor [[buffer(5)]])
{
    float cov = lineCoverage(in);
    return writeColor(premul(lineColor(in, pColor)) * cov, cov, morphFactor);
}

//____ lineFragmentShader_A8() ____________________________________________

fragment FragOutput lineFragmentShader_A8(LineFragInput in [[stage_in]],
                                          constant float4 * pColor [[buffer(0)]],
                                          constant float & morphFactor [[buffer(5)]])
{
    float cov = lineCoverage(in);
    return writeAlpha(lineColor(in, pColor).a * cov, cov, morphFactor);
}


//____ fillVertexShader() ____________________________________________

vertex FillFragInput
fillVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pColor [[buffer(1)]],
             constant vector_float4  *pExtras [[buffer(2)]],
             constant Uniform * pUniform[[buffer(3)]])
{
    FillFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;


    int     colOfs = pVertices[vertexID].colorOfs;
    out.color = pUniform->flatTint * pColor[colOfs];

    return out;
}


//____ fillFragmentShader() ____________________________________________

fragment FragOutput fillFragmentShader(FillFragInput in [[stage_in]],
                                       constant float & morphFactor [[buffer(5)]])
{
    return writeColor(premul(in.color), 1.0, morphFactor);
}

//____ fillFragmentShader_A8() ____________________________________________

fragment FragOutput fillFragmentShader_A8(FillFragInput in [[stage_in]],
                                          constant float & morphFactor [[buffer(5)]])
{
    return writeAlpha(in.color.a, 1.0, morphFactor);
}


//____ fillTintmapVertexShader() ____________________________________________

vertex FillTintmapFragInput
fillTintmapVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pColor [[buffer(1)]],
             constant vector_float4  *pExtras [[buffer(2)]],
             constant Uniform * pUniform[[buffer(3)]])
{
    FillTintmapFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;

    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;

	
    int     colOfs = pVertices[vertexID].colorOfs;
    out.color = pUniform->flatTint * pColor[colOfs];
    out.tintOfs = pVertices[vertexID].tintOfs;

    return out;
}

//____ fillTintmapFragmentShader() ____________________________________________

fragment FragOutput fillTintmapFragmentShader(FillTintmapFragInput in [[stage_in]],
                                              constant float4 * pColor [[buffer(0)]],
                                              constant float & morphFactor [[buffer(5)]])
{
    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    return writeColor(premul(tintColor * in.color), 1.0, morphFactor);
}

//____ fillTintmapFragmentShader_A8() ____________________________________________

fragment FragOutput fillTintmapFragmentShader_A8(FillTintmapFragInput in [[stage_in]],
                                                 constant float4 * pColor [[buffer(0)]],
                                                 constant float & morphFactor [[buffer(5)]])
{
    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    return writeAlpha(tintAlpha * in.color.a, 1.0, morphFactor);
}

//____ fillAAVertexShader() ____________________________________________

vertex FillAAFragInput
fillAAVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pColor [[buffer(1)]],
             constant vector_float4  *pExtras [[buffer(2)]],
             constant Uniform * pUniform[[buffer(3)]])
{
    FillAAFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;


    int     colOfs = pVertices[vertexID].colorOfs;
    out.color = pUniform->flatTint * pColor[colOfs];

    int     eOfs = pVertices[vertexID].extrasOfs;
    out.rect = pExtras[eOfs];
//    out.rect.y = pUniform->canvasYOfs + pUniform->canvasYMul*out.rect.y;
    out.rect.zw += float2(0.5f,0.5f);

    return out;
}

//____ fillAAFragmentShader() ____________________________________________

fragment FragOutput fillAAFragmentShader(FillAAFragInput in [[stage_in]],
                                         constant float & morphFactor [[buffer(5)]])
{
    float2 middleofs = abs(in.position.xy - in.rect.xy);
    float2 alphas = clamp(in.rect.zw  - middleofs, 0.f, 1.f);
    float cov = alphas.x * alphas.y;

    return writeColor(premul(in.color) * cov, cov, morphFactor);
}

//____ fillAAFragmentShader_A8() ____________________________________________

fragment FragOutput fillAAFragmentShader_A8(FillAAFragInput in [[stage_in]],
                                            constant float & morphFactor [[buffer(5)]])
{
    float2 middleofs = abs(in.position.xy - in.rect.xy);
    float2 alphas = clamp(in.rect.zw  - middleofs, 0.f, 1.f);
    float cov = alphas.x * alphas.y;

    return writeAlpha(in.color.a * cov, cov, morphFactor);
}


//____ fillAATintmapVertexShader() ____________________________________________

vertex FillAATintmapFragInput
fillAATintmapVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pColor [[buffer(1)]],
             constant vector_float4  *pExtras [[buffer(2)]],
             constant Uniform * pUniform[[buffer(3)]])
{
    FillAATintmapFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;

    int     colOfs = pVertices[vertexID].colorOfs;
    out.color = pUniform->flatTint * pColor[colOfs];
    out.tintOfs = pVertices[vertexID].tintOfs;

    int     eOfs = pVertices[vertexID].extrasOfs;
    out.rect = pExtras[eOfs];
    out.rect.zw += float2(0.5f,0.5f);

    return out;
}

//____ fillAATintmapFragmentShader() ____________________________________________

fragment FragOutput fillAATintmapFragmentShader(FillAATintmapFragInput in [[stage_in]],
                                                constant float4 * pColor [[buffer(0)]],
                                                constant float & morphFactor [[buffer(5)]])
{
    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    float2 middleofs = abs(in.position.xy - in.rect.xy);
    float2 alphas = clamp(in.rect.zw  - middleofs, 0.f, 1.f);
    float cov = alphas.x * alphas.y;

    return writeColor(premul(tintColor * in.color) * cov, cov, morphFactor);
}

//____ fillAATintmapFragmentShader_A8() ____________________________________________

fragment FragOutput fillAATintmapFragmentShader_A8(FillAATintmapFragInput in [[stage_in]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    float2 middleofs = abs(in.position.xy - in.rect.xy);
    float2 alphas = clamp(in.rect.zw  - middleofs, 0.f, 1.f);
    float cov = alphas.x * alphas.y;

    return writeAlpha(tintAlpha * in.color.a * cov, cov, morphFactor);
}



//____ blitVertexShader() _______________________________________________

vertex BlitFragInput
blitVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pColor [[buffer(1)]],
             constant vector_float4  *pExtras [[buffer(2)]],
             constant Uniform * pUniform[[buffer(3)]])
{
    BlitFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;


    int     eOfs = pVertices[vertexID].extrasOfs;

    vector_float4 srcDst = pExtras[eOfs];
    vector_float4 transform = pExtras[eOfs+1];
    vector_float2 src = srcDst.xy;
    vector_float2 dst = srcDst.zw;

    out.texUV.x = (src.x + 0.0001f + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z) / pUniform->texSize.x;      //TODO: Replace this ugly +0.02f fix with whatever is correct.
    out.texUV.y = (src.y + 0.0001f + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w) / pUniform->texSize.y;      //TODO: Replace this ugly +0.02f fix with whatever is correct.

    out.color = pUniform->flatTint;

    return out;
}

//____ blitFragmentShader() ____________________________________________

fragment FragOutput blitFragmentShader(BlitFragInput in [[stage_in]],
                                       texture2d<half> colorTexture [[ texture(0) ]],
                                       sampler textureSampler [[ sampler(0) ]],
                                       constant float & morphFactor [[buffer(5)]])
{
    const half4 colorSample = colorTexture.sample(textureSampler, in.texUV);

    return writeColor(float4(colorSample) * premul(in.color), 1.0, morphFactor);
}

//____ blitFragmentShader_A8() ____________________________________________

fragment FragOutput blitFragmentShader_A8(BlitFragInput in [[stage_in]],
                                          texture2d<half> colorTexture [[ texture(0) ]],
                                          sampler textureSampler [[ sampler(0) ]],
                                          constant float & morphFactor [[buffer(5)]])
{
    const float colorSample = float(colorTexture.sample(textureSampler, in.texUV).a);

    return writeAlpha(colorSample * in.color.a, 1.0, morphFactor);
}

//____ insideSource() ________________________________________________________
//
// For a ClipBlit from a palette based source. Transparent black outside the
// index texture would read as palette index 0, so the palette shaders clip
// their taps themselves: 0.0 outside the source when clipping, else 1.0.

inline float insideSource(float2 uv, int clipToSource)
{
    return (clipToSource != 0 && (any(uv < float2(0.0)) || any(uv >= float2(1.0)))) ? 0.0 : 1.0;
}

//____ rgbxCoverage() ________________________________________________________
//
// Alpha for a source without alpha (RGBX), which is opaque inside but must be
// transparent outside the source in a ClipBlit. The sampler returns zero there,
// but RGBX shaders can't use the sampled alpha, since the X bytes are undefined.
// So the alpha is worked out from the position instead: 1 inside and 0 outside
// for nearest sampling, and the part of the bilinear footprint that is inside for
// bilinear sampling, which is what the sampler does with the colors. The colors
// are therefore already premultiplied by it.
//
// clipFlags: bit 0 = clip to source (ClipBlit of a non-tiling source), bit 1 = bilinear.

inline float rgbxCoverage(float2 uv, float2 size, int clipFlags)
{
    if( (clipFlags & 1) == 0 )
        return 1.0;

    if( (clipFlags & 2) == 0 )
        return insideSource(uv, 1);

    float2 p = uv * size;
    float2 coverage = saturate(min(p + 0.5, size - p + 0.5));
    return coverage.x * coverage.y;
}

//____ rgbxBlitFragmentShader() ____________________________________________

fragment FragOutput rgbxBlitFragmentShader(BlitFragInput in [[stage_in]],
                                           texture2d<half> colorTexture [[ texture(0) ]],
                                           sampler textureSampler [[ sampler(0) ]],
                                           constant int & clipFlags [[buffer(4)]],
                                           constant float & morphFactor [[buffer(5)]])
{
    half4 colorSample = colorTexture.sample(textureSampler, in.texUV);
    colorSample.a = (half) rgbxCoverage(in.texUV, float2(colorTexture.get_width(), colorTexture.get_height()), clipFlags);

    return writeColor(float4(colorSample) * premul(in.color), 1.0, morphFactor);
}

//____ rgbxBlitFragmentShader_A8() ____________________________________________

fragment FragOutput rgbxBlitFragmentShader_A8(BlitFragInput in [[stage_in]],
                                              texture2d<half> colorTexture [[ texture(0) ]],
                                              sampler textureSampler [[ sampler(0) ]],
                                              constant int & clipFlags [[buffer(4)]],
                                              constant float & morphFactor [[buffer(5)]])
{
    float alpha = rgbxCoverage(in.texUV, float2(colorTexture.get_width(), colorTexture.get_height()), clipFlags);

    return writeAlpha(alpha * in.color.a, 1.0, morphFactor);
}


//____ alphaBlitFragmentShader() ____________________________________________

fragment FragOutput alphaBlitFragmentShader(BlitFragInput in [[stage_in]],
                                            texture2d<half> colorTexture [[ texture(0) ]],
                                            sampler textureSampler [[ sampler(0) ]],
                                            constant float & morphFactor [[buffer(5)]])
{
    const float alpha = float(colorTexture.sample(textureSampler, in.texUV).r);

    return writeColor(premul(in.color) * alpha, 1.0, morphFactor);
}

//____ alphaBlitFragmentShader_A8() ____________________________________________

fragment FragOutput alphaBlitFragmentShader_A8(BlitFragInput in [[stage_in]],
                                               texture2d<half> colorTexture [[ texture(0) ]],
                                               sampler textureSampler [[ sampler(0) ]],
                                               constant float & morphFactor [[buffer(5)]])
{
    const float colorSample = float(colorTexture.sample(textureSampler, in.texUV).r);

    return writeAlpha(colorSample * in.color.a, 1.0, morphFactor);
}

//____ blitTintmapVertexShader() _______________________________________________


vertex BlitTintmapFragInput
blitTintmapVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pColor [[buffer(1)]],
             constant vector_float4  *pExtras [[buffer(2)]],
             constant Uniform * pUniform[[buffer(3)]])
{
    BlitTintmapFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;


    int     eOfs = pVertices[vertexID].extrasOfs;

    vector_float4 srcDst = pExtras[eOfs];
    vector_float4 transform = pExtras[eOfs+1];
    vector_float2 src = srcDst.xy;
    vector_float2 dst = srcDst.zw;

    out.texUV.x = (src.x + 0.0001f + (pos.x - dst.x) * transform.x + (pos.y - dst.y) * transform.z) / pUniform->texSize.x;      //TODO: Replace this ugly +0.02f fix with whatever is correct.
    out.texUV.y = (src.y + 0.0001f + (pos.x - dst.x) * transform.y + (pos.y - dst.y) * transform.w) / pUniform->texSize.y;      //TODO: Replace this ugly +0.02f fix with whatever is correct.

    out.tintOfs = pVertices[vertexID].tintOfs;
    out.color = pUniform->flatTint;

    return out;
}

//____ blitTintmapFragmentShader() ____________________________________________

fragment FragOutput blitTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                              texture2d<half> colorTexture [[ texture(0) ]],
                                              sampler textureSampler [[ sampler(0) ]],
                                              constant float4 * pColor [[buffer(0)]],
                                              constant float & morphFactor [[buffer(5)]])
{
    const half4 colorSample = colorTexture.sample(textureSampler, in.texUV);

    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    return writeColor(float4(colorSample) * premul(tintColor * in.color), 1.0, morphFactor);
}

//____ blitTintmapFragmentShader_A8() ____________________________________________

fragment FragOutput blitTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                                 texture2d<half> colorTexture [[ texture(0) ]],
                                                 sampler textureSampler [[ sampler(0) ]],
                                                 constant float4 * pColor [[buffer(0)]],
                                                 constant float & morphFactor [[buffer(5)]])
{
    const float colorSample = float(colorTexture.sample(textureSampler, in.texUV).a);

    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    return writeAlpha(colorSample * tintAlpha * in.color.a, 1.0, morphFactor);
}

//____ rgbxBlitTintmapFragmentShader() ____________________________________________

fragment FragOutput rgbxBlitTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                                  texture2d<half> colorTexture [[ texture(0) ]],
                                                  sampler textureSampler [[ sampler(0) ]],
                                                  constant float4 * pColor [[buffer(0)]],
                                                  constant int & clipFlags [[buffer(4)]],
                                                  constant float & morphFactor [[buffer(5)]])
{
    half4 colorSample = colorTexture.sample(textureSampler, in.texUV);
    colorSample.a = (half) rgbxCoverage(in.texUV, float2(colorTexture.get_width(), colorTexture.get_height()), clipFlags);

    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    return writeColor(float4(colorSample) * premul(tintColor * in.color), 1.0, morphFactor);
}

//____ rgbxBlitTintmapFragmentShader_A8() ____________________________________________

fragment FragOutput rgbxBlitTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                                     texture2d<half> colorTexture [[ texture(0) ]],
                                                     sampler textureSampler [[ sampler(0) ]],
                                                     constant float4 * pColor [[buffer(0)]],
                                                     constant int & clipFlags [[buffer(4)]],
                                                     constant float & morphFactor [[buffer(5)]])
{
    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;
    float alpha = rgbxCoverage(in.texUV, float2(colorTexture.get_width(), colorTexture.get_height()), clipFlags);

    return writeAlpha(alpha * tintAlpha * in.color.a, 1.0, morphFactor);
}


//____ alphaBlitTintmapFragmentShader() ____________________________________________

fragment FragOutput alphaBlitTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                                   texture2d<half> colorTexture [[ texture(0) ]],
                                                   sampler textureSampler [[ sampler(0) ]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    const float alpha = float(colorTexture.sample(textureSampler, in.texUV).r);

    return writeColor(premul(tintColor * in.color) * alpha, 1.0, morphFactor);
}

//____ alphaBlitTintmapFragmentShader_A8() ____________________________________________

fragment FragOutput alphaBlitTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                                      texture2d<half> colorTexture [[ texture(0) ]],
                                                      sampler textureSampler [[ sampler(0) ]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
    const float colorSample = float(colorTexture.sample(textureSampler, in.texUV).r);

    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    return writeAlpha(colorSample * tintAlpha * in.color.a, 1.0, morphFactor);
}




//____ paletteLookup() ________________________________________________________
//
// Palette entry for an index read from an index texture. Index_8 has an R8Unorm
// texture, Index_16 an RG8Unorm one with the low byte in red and the high byte in
// green (an R16Unorm index would lose precision as half). Green reads as zero
// from R8Unorm, so both work the same.
//
// The palette texture is up to 256 texels wide with as many rows as needed for
// paletteCapacity() entries, see MetalSurface::_createAndSyncTextures(). Indexes
// beyond the palette texture get its last entry, like DX12 does. Entries are
// premultiplied, see MetalSurface::_premultipliedPalette().

inline half4 paletteLookup(texture2d<half> paletteTexture, float2 normIndex)
{
    uint index = uint(normIndex.r * 255.0f + 0.5f) + uint(normIndex.g * 255.0f + 0.5f) * 256;
    uint width = paletteTexture.get_width();
    index = min(index, width * paletteTexture.get_height() - 1);
    return paletteTexture.read(uint2(index % width, index / width));
}

//____ paletteBlitNearestFragmentShader() ____________________________________________

fragment FragOutput paletteBlitNearestFragmentShader(BlitFragInput in [[stage_in]],
                                                     texture2d<float> colorTexture [[ texture(0) ]],
                                                     texture2d<half> paletteTexture [[ texture(1) ]],
                                                     sampler textureSampler [[ sampler(0) ]],
                                                     constant int & clipToSource [[buffer(4)]],
                                                     constant float & morphFactor [[buffer(5)]])
{
    const float2 colorIndex = float2(colorTexture.sample(textureSampler, in.texUV).rg);
    const half4 colorSample = paletteLookup(paletteTexture, colorIndex) * half(insideSource(in.texUV, clipToSource));

    return writeColor(float4(colorSample) * premul(in.color), 1.0, morphFactor);
}

//____ paletteBlitNearestFragmentShader_A8() ____________________________________________

fragment FragOutput paletteBlitNearestFragmentShader_A8(BlitFragInput in [[stage_in]],
                                                        texture2d<float> colorTexture [[ texture(0) ]],
                                                        texture2d<half> paletteTexture [[ texture(1) ]],
                                                        sampler textureSampler [[ sampler(0) ]],
                                                        constant int & clipToSource [[buffer(4)]],
                                                        constant float & morphFactor [[buffer(5)]])
{
    const float2 colorIndex = float2(colorTexture.sample(textureSampler, in.texUV).rg);
    const float colorSample = paletteLookup(paletteTexture, colorIndex).a * insideSource(in.texUV, clipToSource);

    return writeAlpha(colorSample * in.color.a, 1.0, morphFactor);
}


//____ paletteBlitNearestTintmapFragmentShader() ____________________________________________

fragment FragOutput paletteBlitNearestTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                                            texture2d<float> colorTexture [[ texture(0) ]],
                                                            texture2d<half> paletteTexture [[ texture(1) ]],
                                                            sampler textureSampler [[ sampler(0) ]],
                                                            constant int & clipToSource [[buffer(4)]],
                                                            constant float4 * pColor [[buffer(0)]],
                                                            constant float & morphFactor [[buffer(5)]])
{
    const float2 colorIndex = float2(colorTexture.sample(textureSampler, in.texUV).rg);
    const half4 colorSample = paletteLookup(paletteTexture, colorIndex) * half(insideSource(in.texUV, clipToSource));

    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    return writeColor(float4(colorSample) * premul(tintColor * in.color), 1.0, morphFactor);
}

//____ paletteBlitNearestTintmapFragmentShader_A8() ____________________________________________

fragment FragOutput paletteBlitNearestTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                                               texture2d<float> colorTexture [[ texture(0) ]],
                                                               texture2d<half> paletteTexture [[ texture(1) ]],
                                                               sampler textureSampler [[ sampler(0) ]],
                                                               constant int & clipToSource [[buffer(4)]],
                                                               constant float4 * pColor [[buffer(0)]],
                                                               constant float & morphFactor [[buffer(5)]])
{
    const float2 colorIndex = float2(colorTexture.sample(textureSampler, in.texUV).rg);
    const float colorSample = paletteLookup(paletteTexture, colorIndex).a * insideSource(in.texUV, clipToSource);

    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    return writeAlpha(colorSample * tintAlpha * in.color.a, 1.0, morphFactor);
}



//____ paletteBlitInterpolateVertexShader() _______________________________________________

vertex PaletteBlitInterpolateFragInput
paletteBlitInterpolateVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pColor [[buffer(1)]],
             constant vector_float4  *pExtras [[buffer(2)]],
             constant Uniform * pUniform[[buffer(3)]])
{
    PaletteBlitInterpolateFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;


    int     eOfs = pVertices[vertexID].extrasOfs;

    vector_float4 srcDst = pExtras[eOfs];
    vector_float4 transform = pExtras[eOfs+1];
    vector_float2 src = srcDst.xy;
    vector_float2 dst = srcDst.zw;

    float2 texUV = src + (pos-dst) * transform.xw + (pos.yx - dst.yx) * transform.zy;
    texUV -= 0.5f;

    out.uvFrac = texUV;

    out.color = pUniform->flatTint;

    return out;
}

//____ paletteTaps() _________________________________________________________
//
// The four texels around a bilinear sample from a palette based source and the
// weights between them, all from the same value so they always agree. The texels
// are given by their centers, which no rounding error can move to another texel.

inline void paletteTaps(float2 uvFrac, float2 texSize, thread float2& uv00, thread float2& uv11, thread float2& weight)
{
    float2 base = floor(uvFrac);
    weight = uvFrac - base;
    uv00 = (base + 0.5) / texSize;
    uv11 = (base + 1.5) / texSize;
}

//____ paletteBlitInterpolateFragmentShader() ____________________________________________

fragment FragOutput paletteBlitInterpolateFragmentShader(PaletteBlitInterpolateFragInput in [[stage_in]],
                                                         texture2d<float> colorTexture [[ texture(0) ]],
                                                         texture2d<half> paletteTexture [[ texture(1) ]],
                                                         sampler textureSampler [[ sampler(0) ]],
                                                         constant int & clipToSource [[buffer(4)]],
                                                         constant float & morphFactor [[buffer(5)]])
{
   float2 texUV00, texUV11, uvf;
   paletteTaps(in.uvFrac, float2(colorTexture.get_width(), colorTexture.get_height()), texUV00, texUV11, uvf);

   float2 index00 = float2(colorTexture.sample(textureSampler, texUV00).rg);
   float2 index01 = float2(colorTexture.sample(textureSampler, float2(texUV11.x,texUV00.y) ).rg);
   float2 index10 = float2(colorTexture.sample(textureSampler, float2(texUV00.x,texUV11.y) ).rg);
   float2 index11 = float2(colorTexture.sample(textureSampler, texUV11).rg);
   half4 color00 = paletteLookup(paletteTexture, index00) * half(insideSource(texUV00, clipToSource));
   half4 color01 = paletteLookup(paletteTexture, index01) * half(insideSource(float2(texUV11.x,texUV00.y), clipToSource));
   half4 color10 = paletteLookup(paletteTexture, index10) * half(insideSource(float2(texUV00.x,texUV11.y), clipToSource));
   half4 color11 = paletteLookup(paletteTexture, index11) * half(insideSource(texUV11, clipToSource));

   half4 out0 = color00 * (1-uvf.x) + color01 * uvf.x;
   half4 out1 = color10 * (1-uvf.x) + color11 * uvf.x;
   half4 colorSample = (out0 * (1-uvf.y) + out1 * uvf.y);

   return writeColor(float4(colorSample) * premul(in.color), 1.0, morphFactor);
}

//____ paletteBlitInterpolateFragmentShader_A8() ____________________________________________

fragment FragOutput paletteBlitInterpolateFragmentShader_A8(PaletteBlitInterpolateFragInput in [[stage_in]],
                                                            texture2d<float> colorTexture [[ texture(0) ]],
                                                            texture2d<half> paletteTexture [[ texture(1) ]],
                                                            sampler textureSampler [[ sampler(0) ]],
                                                            constant int & clipToSource [[buffer(4)]],
                                                            constant float & morphFactor [[buffer(5)]])
{
   float2 texUV00, texUV11, uvf;
   paletteTaps(in.uvFrac, float2(colorTexture.get_width(), colorTexture.get_height()), texUV00, texUV11, uvf);

   float2 index00 = float2(colorTexture.sample(textureSampler, texUV00).rg);
   float2 index01 = float2(colorTexture.sample(textureSampler, float2(texUV11.x,texUV00.y) ).rg);
   float2 index10 = float2(colorTexture.sample(textureSampler, float2(texUV00.x,texUV11.y) ).rg);
   float2 index11 = float2(colorTexture.sample(textureSampler, texUV11).rg);
   float color00 = paletteLookup(paletteTexture, index00).a * insideSource(texUV00, clipToSource);
   float color01 = paletteLookup(paletteTexture, index01).a * insideSource(float2(texUV11.x,texUV00.y), clipToSource);
   float color10 = paletteLookup(paletteTexture, index10).a * insideSource(float2(texUV00.x,texUV11.y), clipToSource);
   float color11 = paletteLookup(paletteTexture, index11).a * insideSource(texUV11, clipToSource);

   float out0 = color00 * (1-uvf.x) + color01 * uvf.x;
   float out1 = color10 * (1-uvf.x) + color11 * uvf.x;
   float colorSample = (out0 * (1-uvf.y) + out1 * uvf.y);

   return writeAlpha(colorSample * in.color.a, 1.0, morphFactor);
}

//____ paletteBlitInterpolateTintmapVertexShader() _______________________________________________

vertex PaletteBlitInterpolateTintmapFragInput
paletteBlitInterpolateTintmapVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pColor [[buffer(1)]],
             constant vector_float4  *pExtras [[buffer(2)]],
             constant Uniform * pUniform[[buffer(3)]])
{
    PaletteBlitInterpolateTintmapFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;


    int     eOfs = pVertices[vertexID].extrasOfs;

    vector_float4 srcDst = pExtras[eOfs];
    vector_float4 transform = pExtras[eOfs+1];
    vector_float2 src = srcDst.xy;
    vector_float2 dst = srcDst.zw;

    float2 texUV = src + (pos-dst) * transform.xw + (pos.yx - dst.yx) * transform.zy;
    texUV -= 0.5f;

    out.uvFrac = texUV;

    out.tintOfs = pVertices[vertexID].tintOfs;
    out.color = pUniform->flatTint;

    return out;
}

//____ paletteBlitInterpolateTintmapFragmentShader() ____________________________________________

fragment FragOutput paletteBlitInterpolateTintmapFragmentShader(PaletteBlitInterpolateTintmapFragInput in [[stage_in]],
                                                                texture2d<float> colorTexture [[ texture(0) ]],
                                                                texture2d<half> paletteTexture [[ texture(1) ]],
                                                                sampler textureSampler [[ sampler(0) ]],
                                                                constant int & clipToSource [[buffer(4)]],
                                                                constant float4 * pColor [[buffer(0)]],
                                                                constant float & morphFactor [[buffer(5)]])
{
   float2 texUV00, texUV11, uvf;
   paletteTaps(in.uvFrac, float2(colorTexture.get_width(), colorTexture.get_height()), texUV00, texUV11, uvf);

   float2 index00 = float2(colorTexture.sample(textureSampler, texUV00).rg);
   float2 index01 = float2(colorTexture.sample(textureSampler, float2(texUV11.x,texUV00.y) ).rg);
   float2 index10 = float2(colorTexture.sample(textureSampler, float2(texUV00.x,texUV11.y) ).rg);
   float2 index11 = float2(colorTexture.sample(textureSampler, texUV11).rg);
   half4 color00 = paletteLookup(paletteTexture, index00) * half(insideSource(texUV00, clipToSource));
   half4 color01 = paletteLookup(paletteTexture, index01) * half(insideSource(float2(texUV11.x,texUV00.y), clipToSource));
   half4 color10 = paletteLookup(paletteTexture, index10) * half(insideSource(float2(texUV00.x,texUV11.y), clipToSource));
   half4 color11 = paletteLookup(paletteTexture, index11) * half(insideSource(texUV11, clipToSource));

   half4 out0 = color00 * (1-uvf.x) + color01 * uvf.x;
   half4 out1 = color10 * (1-uvf.x) + color11 * uvf.x;
   half4 colorSample = (out0 * (1-uvf.y) + out1 * uvf.y);

   float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

   return writeColor(float4(colorSample) * premul(tintColor * in.color), 1.0, morphFactor);
}

//____ paletteBlitInterpolateTintmapFragmentShader_A8() ____________________________________________

fragment FragOutput paletteBlitInterpolateTintmapFragmentShader_A8(PaletteBlitInterpolateTintmapFragInput in [[stage_in]],
                                                                   texture2d<float> colorTexture [[ texture(0) ]],
                                                                   texture2d<half> paletteTexture [[ texture(1) ]],
                                                                   sampler textureSampler [[ sampler(0) ]],
                                                                   constant int & clipToSource [[buffer(4)]],
                                                                   constant float4 * pColor [[buffer(0)]],
                                                                   constant float & morphFactor [[buffer(5)]])
{
   float2 texUV00, texUV11, uvf;
   paletteTaps(in.uvFrac, float2(colorTexture.get_width(), colorTexture.get_height()), texUV00, texUV11, uvf);

   float2 index00 = float2(colorTexture.sample(textureSampler, texUV00).rg);
   float2 index01 = float2(colorTexture.sample(textureSampler, float2(texUV11.x,texUV00.y) ).rg);
   float2 index10 = float2(colorTexture.sample(textureSampler, float2(texUV00.x,texUV11.y) ).rg);
   float2 index11 = float2(colorTexture.sample(textureSampler, texUV11).rg);
   float color00 = paletteLookup(paletteTexture, index00).a * insideSource(texUV00, clipToSource);
   float color01 = paletteLookup(paletteTexture, index01).a * insideSource(float2(texUV11.x,texUV00.y), clipToSource);
   float color10 = paletteLookup(paletteTexture, index10).a * insideSource(float2(texUV00.x,texUV11.y), clipToSource);
   float color11 = paletteLookup(paletteTexture, index11).a * insideSource(texUV11, clipToSource);

   float out0 = color00 * (1-uvf.x) + color01 * uvf.x;
   float out1 = color10 * (1-uvf.x) + color11 * uvf.x;
   float colorSample = (out0 * (1-uvf.y) + out1 * uvf.y);

   float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

   return writeAlpha(colorSample * tintAlpha * in.color.a, 1.0, morphFactor);
}



//____ blurCore() ____________________________________________________________
//
// Nine taps around texUV, each weighted per channel by its own row of the brush's
// color matrix. The alpha row only weights the center tap, so the source's own
// alpha is kept and only the colors are blurred. Taps are premultiplied, like the
// textures, so their colors are divided by their alphas first, which gives a
// straight color.
//
// An Alpha_8 source (R8Unorm texture) reads as white with alpha, the way it does
// in blits. An RGBX source has undefined alpha bytes, so its alpha is 1.

enum class BlurSource { Normal, Alpha, RGBX };

inline float4 blurCore(float2 texUV, texture2d<float> colorTexture, sampler textureSampler,
					   constant BlurUniform* pBlurInfo, BlurSource source)
{
	float4 color = float4(0,0,0,0);

	for( int i = 0 ; i < 9 ; i++ )
	{
		float4 tap = colorTexture.sample(textureSampler, texUV + pBlurInfo->offset[i]);

		if( source == BlurSource::Alpha )
			tap = float4(1.0, 1.0, 1.0, tap.r);
		else if( source == BlurSource::RGBX )
			tap.a = 1.0;
		else
			tap.rgb = tap.a > 0.0 ? tap.rgb / tap.a : float3(0.0);

		color += tap * pBlurInfo->colorMtx[i];
	}

	return float4(saturate(color.rgb), color.a);
}

//____ paletteBlurCore() _____________________________________________________
//
// As blurCore(), but for a palette based source. Each tap is looked up in the
// palette before it is weighted, the way paletteBlitNearestFragmentShader() does it.
// A palette source is always read through a nearest sampler, so the taps don't
// blend indexes.

inline float4 paletteBlurCore(float2 texUV, texture2d<float> colorTexture, texture2d<half> paletteTexture,
							  sampler textureSampler, constant BlurUniform* pBlurInfo)
{
	float4 color = float4(0,0,0,0);

	for( int i = 0 ; i < 9 ; i++ )
	{
		float2 colorIndex = float2(colorTexture.sample(textureSampler, texUV + pBlurInfo->offset[i] ).rg);
		float4 tap = float4(paletteLookup(paletteTexture, colorIndex));
		tap.rgb = tap.a > 0.0 ? tap.rgb / tap.a : float3(0.0);

		color += tap * pBlurInfo->colorMtx[i];
	}

	return float4(saturate(color.rgb), color.a);
}

//____ blurFragmentShader() __________________________________________

fragment FragOutput blurFragmentShader(BlitFragInput in [[stage_in]],
                                       texture2d<float> colorTexture [[ texture(0) ]],
                                       sampler textureSampler [[ sampler(0) ]],
                                       constant BlurUniform * pBlurInfo [[buffer(2)]],
                                       constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::Normal);
	return writeColor(premul(color * in.color), 1.0, morphFactor);
}

//____ blurFragmentShader_A8() _______________________________________

fragment FragOutput blurFragmentShader_A8(BlitFragInput in [[stage_in]],
                                          texture2d<float> colorTexture [[ texture(0) ]],
                                          sampler textureSampler [[ sampler(0) ]],
                                          constant BlurUniform * pBlurInfo [[buffer(2)]],
                                          constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::Normal);
	return writeAlpha(color.a * in.color.a, 1.0, morphFactor);
}

//____ blurTintmapFragmentShader() ___________________________________

fragment FragOutput blurTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                              texture2d<float> colorTexture [[ texture(0) ]],
                                              sampler textureSampler [[ sampler(0) ]],
                                              constant float4 * pColor [[buffer(0)]],
                                              constant BlurUniform * pBlurInfo [[buffer(2)]],
                                              constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::Normal);
	float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);
	return writeColor(premul(color * tintColor * in.color), 1.0, morphFactor);
}

//____ blurTintmapFragmentShader_A8() ________________________________

fragment FragOutput blurTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                                 texture2d<float> colorTexture [[ texture(0) ]],
                                                 sampler textureSampler [[ sampler(0) ]],
                                                 constant float4 * pColor [[buffer(0)]],
                                                 constant BlurUniform * pBlurInfo [[buffer(2)]],
                                                 constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::Normal);
	float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);
	return writeAlpha(color.a * tintColor.a * in.color.a, 1.0, morphFactor);
}

//____ alphaBlurFragmentShader() _____________________________________

fragment FragOutput alphaBlurFragmentShader(BlitFragInput in [[stage_in]],
                                            texture2d<float> colorTexture [[ texture(0) ]],
                                            sampler textureSampler [[ sampler(0) ]],
                                            constant BlurUniform * pBlurInfo [[buffer(2)]],
                                            constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::Alpha);
	return writeColor(premul(color * in.color), 1.0, morphFactor);
}

//____ alphaBlurFragmentShader_A8() __________________________________

fragment FragOutput alphaBlurFragmentShader_A8(BlitFragInput in [[stage_in]],
                                               texture2d<float> colorTexture [[ texture(0) ]],
                                               sampler textureSampler [[ sampler(0) ]],
                                               constant BlurUniform * pBlurInfo [[buffer(2)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::Alpha);
	return writeAlpha(color.a * in.color.a, 1.0, morphFactor);
}

//____ alphaBlurTintmapFragmentShader() ______________________________

fragment FragOutput alphaBlurTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                                   texture2d<float> colorTexture [[ texture(0) ]],
                                                   sampler textureSampler [[ sampler(0) ]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant BlurUniform * pBlurInfo [[buffer(2)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::Alpha);
	float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);
	return writeColor(premul(color * tintColor * in.color), 1.0, morphFactor);
}

//____ alphaBlurTintmapFragmentShader_A8() ___________________________

fragment FragOutput alphaBlurTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                                      texture2d<float> colorTexture [[ texture(0) ]],
                                                      sampler textureSampler [[ sampler(0) ]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant BlurUniform * pBlurInfo [[buffer(2)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::Alpha);
	float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);
	return writeAlpha(color.a * tintColor.a * in.color.a, 1.0, morphFactor);
}

//____ rgbxBlurFragmentShader() ______________________________________

fragment FragOutput rgbxBlurFragmentShader(BlitFragInput in [[stage_in]],
                                           texture2d<float> colorTexture [[ texture(0) ]],
                                           sampler textureSampler [[ sampler(0) ]],
                                           constant BlurUniform * pBlurInfo [[buffer(2)]],
                                           constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::RGBX);
	return writeColor(premul(color * in.color), 1.0, morphFactor);
}

//____ rgbxBlurFragmentShader_A8() ___________________________________

fragment FragOutput rgbxBlurFragmentShader_A8(BlitFragInput in [[stage_in]],
                                              texture2d<float> colorTexture [[ texture(0) ]],
                                              sampler textureSampler [[ sampler(0) ]],
                                              constant BlurUniform * pBlurInfo [[buffer(2)]],
                                              constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::RGBX);
	return writeAlpha(color.a * in.color.a, 1.0, morphFactor);
}

//____ rgbxBlurTintmapFragmentShader() _______________________________

fragment FragOutput rgbxBlurTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                                  texture2d<float> colorTexture [[ texture(0) ]],
                                                  sampler textureSampler [[ sampler(0) ]],
                                                  constant float4 * pColor [[buffer(0)]],
                                                  constant BlurUniform * pBlurInfo [[buffer(2)]],
                                                  constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::RGBX);
	float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);
	return writeColor(premul(color * tintColor * in.color), 1.0, morphFactor);
}

//____ rgbxBlurTintmapFragmentShader_A8() ____________________________

fragment FragOutput rgbxBlurTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                                     texture2d<float> colorTexture [[ texture(0) ]],
                                                     sampler textureSampler [[ sampler(0) ]],
                                                     constant float4 * pColor [[buffer(0)]],
                                                     constant BlurUniform * pBlurInfo [[buffer(2)]],
                                                     constant float & morphFactor [[buffer(5)]])
{
	float4 color = blurCore(in.texUV, colorTexture, textureSampler, pBlurInfo, BlurSource::RGBX);
	float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);
	return writeAlpha(color.a * tintColor.a * in.color.a, 1.0, morphFactor);
}

//____ paletteBlurFragmentShader() ___________________________________

fragment FragOutput paletteBlurFragmentShader(BlitFragInput in [[stage_in]],
                                              texture2d<float> colorTexture [[ texture(0) ]],
                                              texture2d<half> paletteTexture [[ texture(1) ]],
                                              sampler textureSampler [[ sampler(0) ]],
                                              constant BlurUniform * pBlurInfo [[buffer(2)]],
                                              constant float & morphFactor [[buffer(5)]])
{
	float4 color = paletteBlurCore(in.texUV, colorTexture, paletteTexture, textureSampler, pBlurInfo);
	return writeColor(premul(color * in.color), 1.0, morphFactor);
}

//____ paletteBlurFragmentShader_A8() ________________________________

fragment FragOutput paletteBlurFragmentShader_A8(BlitFragInput in [[stage_in]],
                                                 texture2d<float> colorTexture [[ texture(0) ]],
                                                 texture2d<half> paletteTexture [[ texture(1) ]],
                                                 sampler textureSampler [[ sampler(0) ]],
                                                 constant BlurUniform * pBlurInfo [[buffer(2)]],
                                                 constant float & morphFactor [[buffer(5)]])
{
	float4 color = paletteBlurCore(in.texUV, colorTexture, paletteTexture, textureSampler, pBlurInfo);
	return writeAlpha(color.a * in.color.a, 1.0, morphFactor);
}

//____ paletteBlurTintmapFragmentShader() ____________________________

fragment FragOutput paletteBlurTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                                     texture2d<float> colorTexture [[ texture(0) ]],
                                                     texture2d<half> paletteTexture [[ texture(1) ]],
                                                     sampler textureSampler [[ sampler(0) ]],
                                                     constant float4 * pColor [[buffer(0)]],
                                                     constant BlurUniform * pBlurInfo [[buffer(2)]],
                                                     constant float & morphFactor [[buffer(5)]])
{
	float4 color = paletteBlurCore(in.texUV, colorTexture, paletteTexture, textureSampler, pBlurInfo);
	float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);
	return writeColor(premul(color * tintColor * in.color), 1.0, morphFactor);
}

//____ paletteBlurTintmapFragmentShader_A8() _________________________

fragment FragOutput paletteBlurTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                                        texture2d<float> colorTexture [[ texture(0) ]],
                                                        texture2d<half> paletteTexture [[ texture(1) ]],
                                                        sampler textureSampler [[ sampler(0) ]],
                                                        constant float4 * pColor [[buffer(0)]],
                                                        constant BlurUniform * pBlurInfo [[buffer(2)]],
                                                        constant float & morphFactor [[buffer(5)]])
{
	float4 color = paletteBlurCore(in.texUV, colorTexture, paletteTexture, textureSampler, pBlurInfo);
	float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);
	return writeAlpha(color.a * tintColor.a * in.color.a, 1.0, morphFactor);
}



//____ segmentsVertexShader() _______________________________________________

vertex SegmentsFragInput
segmentsVertexShader(uint vertexID [[vertex_id]],
					 constant Vertex *pVertices [[buffer(0)]],
					 constant vector_float4  *pColor [[buffer(1)]],
					 constant vector_float4  *pExtras [[buffer(2)]],
					 constant Uniform * pUniform[[buffer(3)]])
{
    SegmentsFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;

    int	eOfs = pVertices[vertexID].extrasOfs;

    vector_float4 extras = pExtras[eOfs];

	out.flatColorsOfs = int(extras.x);
	out.tintTableOfs = int(extras.y);
	out.edgemapPitch = int(extras.z);
	out.tileSize = pExtras[eOfs + 1].xy;
    out.texUV = pVertices[vertexID].uv;
	out.tintOfs = pVertices[vertexID].tintOfs;
	out.color = pUniform->flatTint;
	return out;
}

//____ wrapUV() ____________________________________________________________
//
// Position in the edgemap of a pixel of a tiled edgemap, which may be outside of it,
// wrapped into it. uv.y is half a pixel back, so it is whole at pixel centers.

inline float2 wrapUV(float2 uv, float2 tileSize)
{
	if( tileSize.x > 0.f )
	{
		float2 cell = float2(floor(uv.x), floor(uv.y + 0.5f));
		uv += cell - tileSize * floor(cell / tileSize) - cell;
	}
	return uv;
}

//____ segColor() __________________________________________________________
//
// A segment is colored by a flat color or by a tint placed in the edgemap's own
// rectangle. The edgemap buffer has a table with the offset of each segment's
// tint block, -1 for flat colored segments.

inline float4 segColor(int seg, SegmentsFragInput in, constant float4 * pEdgemap, float2 epos)
{
	int tofs = int(pEdgemap[in.tintTableOfs + seg].x);
	return tofs < 0 ? pEdgemap[in.flatColorsOfs + seg] : evalTint(pEdgemap, tofs, epos);
}

//____ segmentsFragmentShader() ____________________________________________

// A segment's premultiplied color pc, which covers part of the pixel. Usually just pc weighted
// by that part. On a canvas without alpha, Replace and Morph keep the color of what they give
// each segment, which is (canvas * (1 - m) + pc * m) / (1 - m + pc.a * m), where m is the morph
// factor (1 for Replace). That is averaged by area, so each segment adds its part of the second
// term and of how much of the canvas is kept (keep). A segment whose result is transparent gives
// black, like a transparent Replace does.

inline float4 segPart(float4 pc, float part, float morphFactor, thread float& keep)
{
	if( c_outputMode == 1 && c_noAlpha )
	{
		float m = c_morph ? morphFactor : 1.f;
		float resultAlpha = 1.f - m + pc.a * m;

		if( resultAlpha <= 0.f )
			return float4(0.f);

		keep += part * (1.f - m) / resultAlpha;
		return pc * (part * m / resultAlpha);
	}

	return pc * part;
}

// The segments together cover the whole pixel, each the part of it between its edges.
// Returns their colors premultiplied, tinted and weighted by those parts, see segPart().

template <int EDGES>
inline float4 segFragShaderCore(SegmentsFragInput in,
                                    constant float4  *pEdgemap, float4 tint, float morphFactor, thread float& keep)
{
	in.texUV = wrapUV(in.texUV, in.tileSize);

    float4 acc = float4(0.f);

    float factor = 1.f;
	float2 epos = float2(floor(in.texUV.x) + 0.5f, in.texUV.y + 0.5f);	// Pixel center in edgemap space.
    for( int i = 0 ; i < EDGES ; i++ )
    {
		float4 col = segColor(i, in, pEdgemap, epos);

        float4 edge = pEdgemap[int(in.texUV.x)*in.edgemapPitch+i];

        float x = (in.texUV.y - edge.r) * edge.g;
        float adder = edge.g / 2.f;
        if (x < 0.f)
            adder = edge.b;
        else if (x + edge.g > 1.f)
			adder = edge.a;
        float factor2 = min(clamp(x + adder, 0.f, 1.f), factor);		// An edge above the one before is pushed down to it.

        acc += segPart(premul(col) * tint, factor - factor2, morphFactor, keep);
        factor = factor2;
    }

	float4 col = segColor(EDGES, in, pEdgemap, epos);
	acc += segPart(premul(col) * tint, factor, morphFactor, keep);

	return acc;
};

// Output of the segments shaders. On canvases without alpha, Replace and Morph keep the color
// of the result of each segment, so segPart() has already divided them by their alphas and
// summed up how much of the canvas they keep. Other pipelines get the sum of the segments as
// any other shader does.

template <int EDGES>
inline FragOutput writeSegments(SegmentsFragInput in, constant float4 * pEdgemap, float4 tint, float morphFactor)
{
	float keep = 0.f;
	float4 pc = segFragShaderCore<EDGES>(in, pEdgemap, tint, morphFactor, keep);

	if( c_outputMode == 1 && c_noAlpha )
	{
		FragOutput out;
		out.color = pc;
		out.factor = float4(1.f - keep);
		return out;
	}

	return writeColor(pc, 1.0, morphFactor);
}


fragment FragOutput segmentsFragmentShader1(SegmentsFragInput in [[stage_in]],
                                            constant float4 * pEdgemap [[buffer(3)]],
                                            constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<1>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader2(SegmentsFragInput in [[stage_in]],
                                            constant float4 * pEdgemap [[buffer(3)]],
                                            constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<2>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader3(SegmentsFragInput in [[stage_in]],
                                            constant float4 * pEdgemap [[buffer(3)]],
                                            constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<3>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader4(SegmentsFragInput in [[stage_in]],
                                            constant float4 * pEdgemap [[buffer(3)]],
                                            constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<4>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader5(SegmentsFragInput in [[stage_in]],
                                            constant float4 * pEdgemap [[buffer(3)]],
                                            constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<5>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader6(SegmentsFragInput in [[stage_in]],
                                            constant float4 * pEdgemap [[buffer(3)]],
                                            constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<6>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader7(SegmentsFragInput in [[stage_in]],
                                            constant float4 * pEdgemap [[buffer(3)]],
                                            constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<7>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader8(SegmentsFragInput in [[stage_in]],
                                            constant float4 * pEdgemap [[buffer(3)]],
                                            constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<8>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader9(SegmentsFragInput in [[stage_in]],
                                            constant float4 * pEdgemap [[buffer(3)]],
                                            constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<9>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader10(SegmentsFragInput in [[stage_in]],
                                             constant float4 * pEdgemap [[buffer(3)]],
                                             constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<10>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader11(SegmentsFragInput in [[stage_in]],
                                             constant float4 * pEdgemap [[buffer(3)]],
                                             constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<11>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader12(SegmentsFragInput in [[stage_in]],
                                             constant float4 * pEdgemap [[buffer(3)]],
                                             constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<12>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader13(SegmentsFragInput in [[stage_in]],
                                             constant float4 * pEdgemap [[buffer(3)]],
                                             constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<13>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader14(SegmentsFragInput in [[stage_in]],
                                             constant float4 * pEdgemap [[buffer(3)]],
                                             constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<14>(in, pEdgemap, premul(in.color), morphFactor);
}

fragment FragOutput segmentsFragmentShader15(SegmentsFragInput in [[stage_in]],
                                             constant float4 * pEdgemap [[buffer(3)]],
                                             constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<15>(in, pEdgemap, premul(in.color), morphFactor);
}


fragment FragOutput segmentsTintmapFragmentShader1(SegmentsFragInput in [[stage_in]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float4 * pEdgemap [[buffer(3)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<1>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader2(SegmentsFragInput in [[stage_in]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float4 * pEdgemap [[buffer(3)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<2>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader3(SegmentsFragInput in [[stage_in]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float4 * pEdgemap [[buffer(3)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<3>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader4(SegmentsFragInput in [[stage_in]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float4 * pEdgemap [[buffer(3)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<4>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader5(SegmentsFragInput in [[stage_in]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float4 * pEdgemap [[buffer(3)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<5>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader6(SegmentsFragInput in [[stage_in]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float4 * pEdgemap [[buffer(3)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<6>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader7(SegmentsFragInput in [[stage_in]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float4 * pEdgemap [[buffer(3)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<7>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader8(SegmentsFragInput in [[stage_in]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float4 * pEdgemap [[buffer(3)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<8>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader9(SegmentsFragInput in [[stage_in]],
                                                   constant float4 * pColor [[buffer(0)]],
                                                   constant float4 * pEdgemap [[buffer(3)]],
                                                   constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<9>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader10(SegmentsFragInput in [[stage_in]],
                                                    constant float4 * pColor [[buffer(0)]],
                                                    constant float4 * pEdgemap [[buffer(3)]],
                                                    constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<10>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader11(SegmentsFragInput in [[stage_in]],
                                                    constant float4 * pColor [[buffer(0)]],
                                                    constant float4 * pEdgemap [[buffer(3)]],
                                                    constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<11>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader12(SegmentsFragInput in [[stage_in]],
                                                    constant float4 * pColor [[buffer(0)]],
                                                    constant float4 * pEdgemap [[buffer(3)]],
                                                    constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<12>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader13(SegmentsFragInput in [[stage_in]],
                                                    constant float4 * pColor [[buffer(0)]],
                                                    constant float4 * pEdgemap [[buffer(3)]],
                                                    constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<13>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader14(SegmentsFragInput in [[stage_in]],
                                                    constant float4 * pColor [[buffer(0)]],
                                                    constant float4 * pEdgemap [[buffer(3)]],
                                                    constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<14>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader15(SegmentsFragInput in [[stage_in]],
                                                    constant float4 * pColor [[buffer(0)]],
                                                    constant float4 * pEdgemap [[buffer(3)]],
                                                    constant float & morphFactor [[buffer(5)]])
{
	return writeSegments<15>(in, pEdgemap, premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), morphFactor);
}



//____ segmentsFragmentShader_A8() ____________________________________________

template <int EDGES>
inline float segFragShaderCore_A8(SegmentsFragInput in,
                                    constant float4  *pEdgemap)
{
	in.texUV = wrapUV(in.texUV, in.tileSize);

	float totalAlpha = 0.f;

	float factor = 1.f;
	float2 epos = float2(floor(in.texUV.x) + 0.5f, in.texUV.y + 0.5f);	// Pixel center in edgemap space.
	for( int i = 0 ; i < EDGES ; i++ )
	{
		float alpha = segColor(i, in, pEdgemap, epos).a;

		float4 edge = pEdgemap[int(in.texUV.x)*in.edgemapPitch+i];

		float x = (in.texUV.y - edge.r) * edge.g;
		float adder = edge.g / 2.f;
		if (x < 0.f)
			adder = edge.b;
		else if (x + edge.g > 1.f)
			adder = edge.a;
		float factor2 = min(clamp(x + adder, 0.f, 1.f), factor);		// An edge above the one before is pushed down to it.

		totalAlpha += (factor - factor2)*alpha;

		factor = factor2;
	}

	float alpha = segColor(EDGES, in, pEdgemap, epos).a;
	totalAlpha += factor*alpha;

	return totalAlpha;
};



fragment FragOutput segmentsFragmentShader1_A8(SegmentsFragInput in [[stage_in]],
                                               constant float4 * pEdgemap [[buffer(3)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<1>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader2_A8(SegmentsFragInput in [[stage_in]],
                                               constant float4 * pEdgemap [[buffer(3)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<2>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader3_A8(SegmentsFragInput in [[stage_in]],
                                               constant float4 * pEdgemap [[buffer(3)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<3>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader4_A8(SegmentsFragInput in [[stage_in]],
                                               constant float4 * pEdgemap [[buffer(3)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<4>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader5_A8(SegmentsFragInput in [[stage_in]],
                                               constant float4 * pEdgemap [[buffer(3)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<5>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader6_A8(SegmentsFragInput in [[stage_in]],
                                               constant float4 * pEdgemap [[buffer(3)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<6>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader7_A8(SegmentsFragInput in [[stage_in]],
                                               constant float4 * pEdgemap [[buffer(3)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<7>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader8_A8(SegmentsFragInput in [[stage_in]],
                                               constant float4 * pEdgemap [[buffer(3)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<8>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader9_A8(SegmentsFragInput in [[stage_in]],
                                               constant float4 * pEdgemap [[buffer(3)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<9>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader10_A8(SegmentsFragInput in [[stage_in]],
                                                constant float4 * pEdgemap [[buffer(3)]],
                                                constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<10>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader11_A8(SegmentsFragInput in [[stage_in]],
                                                constant float4 * pEdgemap [[buffer(3)]],
                                                constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<11>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader12_A8(SegmentsFragInput in [[stage_in]],
                                                constant float4 * pEdgemap [[buffer(3)]],
                                                constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<12>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader13_A8(SegmentsFragInput in [[stage_in]],
                                                constant float4 * pEdgemap [[buffer(3)]],
                                                constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<13>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader14_A8(SegmentsFragInput in [[stage_in]],
                                                constant float4 * pEdgemap [[buffer(3)]],
                                                constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<14>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsFragmentShader15_A8(SegmentsFragInput in [[stage_in]],
                                                constant float4 * pEdgemap [[buffer(3)]],
                                                constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<15>(in,pEdgemap) * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader1_A8(SegmentsFragInput in [[stage_in]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant float4 * pEdgemap [[buffer(3)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<1>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader2_A8(SegmentsFragInput in [[stage_in]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant float4 * pEdgemap [[buffer(3)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<2>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader3_A8(SegmentsFragInput in [[stage_in]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant float4 * pEdgemap [[buffer(3)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<3>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader4_A8(SegmentsFragInput in [[stage_in]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant float4 * pEdgemap [[buffer(3)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<4>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader5_A8(SegmentsFragInput in [[stage_in]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant float4 * pEdgemap [[buffer(3)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<5>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader6_A8(SegmentsFragInput in [[stage_in]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant float4 * pEdgemap [[buffer(3)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<6>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader7_A8(SegmentsFragInput in [[stage_in]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant float4 * pEdgemap [[buffer(3)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<7>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader8_A8(SegmentsFragInput in [[stage_in]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant float4 * pEdgemap [[buffer(3)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<8>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader9_A8(SegmentsFragInput in [[stage_in]],
                                                      constant float4 * pColor [[buffer(0)]],
                                                      constant float4 * pEdgemap [[buffer(3)]],
                                                      constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<9>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader10_A8(SegmentsFragInput in [[stage_in]],
                                                       constant float4 * pColor [[buffer(0)]],
                                                       constant float4 * pEdgemap [[buffer(3)]],
                                                       constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<10>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader11_A8(SegmentsFragInput in [[stage_in]],
                                                       constant float4 * pColor [[buffer(0)]],
                                                       constant float4 * pEdgemap [[buffer(3)]],
                                                       constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<11>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader12_A8(SegmentsFragInput in [[stage_in]],
                                                       constant float4 * pColor [[buffer(0)]],
                                                       constant float4 * pEdgemap [[buffer(3)]],
                                                       constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<12>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader13_A8(SegmentsFragInput in [[stage_in]],
                                                       constant float4 * pColor [[buffer(0)]],
                                                       constant float4 * pEdgemap [[buffer(3)]],
                                                       constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<13>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader14_A8(SegmentsFragInput in [[stage_in]],
                                                       constant float4 * pColor [[buffer(0)]],
                                                       constant float4 * pEdgemap [[buffer(3)]],
                                                       constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<14>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

fragment FragOutput segmentsTintmapFragmentShader15_A8(SegmentsFragInput in [[stage_in]],
                                                       constant float4 * pColor [[buffer(0)]],
                                                       constant float4 * pEdgemap [[buffer(3)]],
                                                       constant float & morphFactor [[buffer(5)]])
{
	return writeAlpha(segFragShaderCore_A8<15>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, 1.0, morphFactor);
}

//____ Shapes ______________________________________________________________
//
// Shapes (Command::DrawShape), see GpuShapes for the entries read from the extras buffer.
// The coverage of each pixel by each segment is worked out from the geometry. Pixels get the
// premultiplied colors of the drawn segments covering them, weighted by coverage, and are
// covered by them together. Pixels not covered by any drawn segment are left alone.

typedef struct
{
	float4	position [[position]];
	float4	color;						// Tint color.
	int		shapeOfs [[flat]];			// Entry of shape in extras buffer.
	int		tintOfs [[flat]];			// Device tint block in color buffer, -1 for none.
} ShapeFragInput;

vertex ShapeFragInput
shapeVertexShader(uint vertexID [[vertex_id]],
				  constant Vertex *pVertices [[buffer(0)]],
				  constant vector_float4  *pColor [[buffer(1)]],
				  constant vector_float4  *pExtras [[buffer(2)]],
				  constant Uniform * pUniform[[buffer(3)]])
{
	ShapeFragInput out;

	float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

	vector_float2 canvasSize = pUniform->canvasDim;

	out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
	out.position.x = pos.x*2 / canvasSize.x - 1.0;
	out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;

	out.color = pUniform->flatTint;
	out.shapeOfs = pVertices[vertexID].extrasOfs;
	out.tintOfs = pVertices[vertexID].tintOfs;
	return out;
}

inline float spanCoverage(float a, float b, float p)
{
	return clamp(min(p + 0.5f, b) - max(p - 0.5f, a), 0.f, 1.f);
}

inline float rectCoverage(float4 r, float2 p)
{
	return spanCoverage(r.x, r.z, p.x) * spanCoverage(r.y, r.w, p.y);
}

// Part of a pixel inside a straight edge, s from the pixel center along its normal n.

inline float halfPlaneCoverage(float s, float2 n)
{
	float2 an = abs(n);
	float a = max(an.x, an.y);
	float b = min(an.x, an.y);

	if( b < 0.001f )
		return clamp(s + 0.5f, 0.f, 1.f);

	float t = s + 0.5f * (a + b);

	if( t <= 0.f )
		return 0.f;
	if( t >= a + b )
		return 1.f;
	if( t < b )
		return t * t / (2.f * a * b);
	if( t > a )
		return 1.f - (a + b - t) * (a + b - t) / (2.f * a * b);
	return (t - 0.5f * b) / a;
}

// Part of a pixel inside a circle, v from its center to the pixel center and d its length.

inline float circleCoverage(float radius, float2 v, float d)
{
	if( radius <= 0.f )
		return 0.f;

	return d > 0.001f ? halfPlaneCoverage(radius - d, v / d) : clamp(radius + 0.5f, 0.f, 1.f);
}

inline float roundedRectCoverage(float4 r, float4 radii, float2 p)
{
	float c = rectCoverage(r, p);

	// The rounded corner, if any, whose rounding the pixel center is in. Square corners are
	// left to rectCoverage(), which also covers the pixels just outside them partly.

	float2 center;
	float radius;

	if( radii.x > 0.f && p.x < r.x + radii.x && p.y < r.y + radii.x )
	{
		radius = radii.x;
		center = float2(r.x + radius, r.y + radius);
	}
	else if( radii.y > 0.f && p.x > r.z - radii.y && p.y < r.y + radii.y )
	{
		radius = radii.y;
		center = float2(r.z - radius, r.y + radius);
	}
	else if( radii.z > 0.f && p.x > r.z - radii.z && p.y > r.w - radii.z )
	{
		radius = radii.z;
		center = float2(r.z - radius, r.w - radius);
	}
	else if( radii.w > 0.f && p.x < r.x + radii.w && p.y > r.w - radii.w )
	{
		radius = radii.w;
		center = float2(r.x + radius, r.w - radius);
	}
	else
		return c;

	float2 v = p - center;
	return min(c, circleCoverage(radius, v, length(v)));
}

// Adds a segment covering c of the pixel, if drawn: color times alpha and alpha weighted
// by coverage to acc, coverage to covered.

inline void addSegment(constant float4 * pShape, constant float4 * pColor, int seg, float c, float2 p, thread float4& acc, thread float& covered)
{
	if( c <= 0.f )
		return;

	float4 s = pShape[seg];
	if( s.x < 0.f )
		return;

	float4 col = pColor[int(s.x)];
	if( s.y >= 0.f )
		col *= evalTint(pColor, int(s.y), p);

	acc += float4(col.rgb * col.a, col.a) * c;
	covered += c;
}

inline void boxCoverage(constant float4 * pShape, constant float4 * pColor, int ofs, float2 p, thread float4& acc, thread float& covered)
{
	float outer = roundedRectCoverage(pShape[ofs + 1], pShape[ofs + 2], p);
	float fill = outer;

	if( (int(pShape[ofs].y) & 1) != 0 )
		fill = min(outer, roundedRectCoverage(pShape[ofs + 3], pShape[ofs + 4], p));

	addSegment(pShape, pColor, ofs + 5, fill, p, acc, covered);
	addSegment(pShape, pColor, ofs + 6, outer - fill, p, acc, covered);
}

// Direction of v in turns clockwise from 12 o'clock, 0.0 -> 1.0. Worked out from the angle to
// the vertical axis instead of by atan2(), which with fast math can put directions along an
// axis in the wrong quadrant, as -0.0 doesn't count as negative while y / -0.0 does.

inline float turnsFromTop(float2 v)
{
	float2 a = abs(v);
	float t = a.x <= a.y ? atan(a.x / max(a.y, 1e-20f)) : 1.5707963f - atan(a.y / max(a.x, 1e-20f));
	t *= 0.15915494f;								// From the vertical axis, 0.0 -> 0.25.

	if( v.y >= 0.f )
		t = 0.5f - t;								// Below the center.
	if( v.x < 0.f )
		t = 1.f - t;								// Left of the center.

	return fract(t);
}

// For a pixel with the center of the ring inside it, v from the center to the pixel center:
// the part of the pixel swept by a ray from the center, turning clockwise from 12 o'clock to
// position (in turns). The ray passes the right half of the top side, the right side, the
// bottom side, the left side and the left half of the top side, sweeping a triangle between
// the center and each of them.

inline float squareSweep(float2 v, float position)
{
	float2 u = float2(sin(6.2831853f * position), -cos(6.2831853f * position));

	float left = 0.5f - v.x;						// Distances from the center to the sides.
	float right = 0.5f + v.x;
	float top = 0.5f - v.y;
	float bottom = 0.5f + v.y;

	// Where the ray leaves the pixel, through the left or right side if tx < ty.

	float tx = (u.x > 0.f ? right : left) / max(abs(u.x), 1e-20f);
	float ty = (u.y > 0.f ? bottom : top) / max(abs(u.y), 1e-20f);
	float2 q = u * min(tx, ty);

	// Halves of the top side are told apart by position, as positions of 1.0 point straight up
	// too and are where the sweep ends.

	float a;										// Twice the area.
	if( ty <= tx && u.y < 0.f && position < 0.5f )
		a = top * q.x;
	else if( tx < ty && u.x > 0.f )
		a = top * right + right * (top + q.y);
	else if( ty <= tx && u.y > 0.f )
		a = top * right + right * (top + bottom) + bottom * (right - q.x);
	else if( tx < ty )
		a = top * right + right * (top + bottom) + bottom * (right + left) + left * (bottom - q.y);
	else
		a = 2.f + top * q.x;

	return 0.5f * a;
}

// Splits band, the coverage of the slice area, between the elements: those whose boundaries
// pass through the pixel get the part between them, the element at the pixel center the rest.
// If the center of the ring is inside the pixel, all elements get the part of the pixel within
// their angle.

inline void ringElements(constant float4 * pShape, constant float4 * pColor, int ofs, int n, int base, float band, float2 p, float2 v, float d, thread float4& acc, thread float& covered)
{
	if( n == 1 )
	{
		addSegment(pShape, pColor, ofs + int(pShape[base].y), band, p, acc, covered);
		return;
	}

	if( abs(v.x) < 0.5f && abs(v.y) < 0.5f )
	{
		float firstSweep = squareSweep(v, pShape[base].x);
		float sweep = firstSweep;

		for( int e = 0; e < n; e++ )
		{
			float nextSweep = e + 1 < n ? squareSweep(v, pShape[base + e + 1].x) : firstSweep + 1.f;
			addSegment(pShape, pColor, ofs + int(pShape[base + e].y), band * max(nextSweep - sweep, 0.f), p, acc, covered);
			sweep = nextSweep;
		}
		return;
	}

	float theta = d > 0.0001f ? turnsFromTop(v) : 0.f;

	int lo = 0;
	int hi = n;
	while( lo < hi )
	{
		int mid = (lo + hi) / 2;
		if( pShape[base + mid].x <= theta )
			lo = mid + 1;
		else
			hi = mid;
	}

	int first = lo > 0 ? lo - 1 : n - 1;
	float w = min(0.25f, 0.70710678f / (6.2831853f * max(d, 0.0001f)));
	float behind = fract(theta - pShape[base + first].x);

	for( int i = 0 ; i < n - 1 && behind < w ; i++ )
	{
		int prev = first > 0 ? first - 1 : n - 1;
		behind += fract(pShape[base + first].x - pShape[base + prev].x);
		first = prev;
	}

	float g = 1.f;
	float ahead = -behind;
	int k = first;
	float4 element = pShape[base + k];

	for( int i = 0 ; i < n ; i++ )
	{
		int next = k + 1 < n ? k + 1 : 0;
		float4 nextElement = pShape[base + next];
		ahead += fract(nextElement.x - element.x);

		float gEnd = 0.f;
		if( i < n - 1 && ahead < w )
		{
			float a = 6.2831853f * nextElement.x;
			gEnd = min(g, halfPlaneCoverage(-d * sin(6.2831853f * ahead), float2(cos(a), sin(a))));
		}

		addSegment(pShape, pColor, ofs + int(element.y), band * (g - gEnd), p, acc, covered);

		if( gEnd <= 0.f )
			break;

		g = gEnd;
		k = next;
		element = nextElement;
	}
}

inline void ringCoverage(constant float4 * pShape, constant float4 * pColor, int ofs, float2 p, thread float4& acc, thread float& covered)
{
	float4 head = pShape[ofs];
	int flags = int(head.y);
	float4 circles = pShape[ofs + 1];
	float4 inner = pShape[ofs + 2];
	float4 rect = pShape[ofs + 3];

	float2 v = p - circles.xy;
	float d = length(v);
	bool bSquare = (flags & 1) != 0;

	float c0 = bSquare ? rectCoverage(rect, p) : circleCoverage(circles.z, v, d);
	float c1 = min(bSquare ? rectCoverage(pShape[ofs + 4], p) : circleCoverage(circles.w, v, d), c0);
	float c2 = min(circleCoverage(inner.x, v, d), c1);
	float c3 = min(circleCoverage(inner.y, v, d), c2);

	float back = (flags & 2) != 0 ? max(rectCoverage(rect, p) - c0, 0.f) : 0.f;
	float outline = c0 - c1 + c2 - c3;
	float band = c1 - c2;

	if( (flags & 16) == 0 )
	{
		outline += band;
		band = 0.f;
	}

	addSegment(pShape, pColor, ofs + 5, back, p, acc, covered);
	addSegment(pShape, pColor, ofs + 6, outline, p, acc, covered);
	addSegment(pShape, pColor, ofs + 7, (flags & 4) != 0 ? c3 : 0.f, p, acc, covered);

	if( band > 0.f )
		ringElements(pShape, pColor, ofs, int(head.z), ofs + int(head.w), band, p, v, d, acc, covered);
}

// Lines with caps, see GpuShapes::writeLine(). Their straight edges are half-planes, each as its
// inward normal and how far inside it the pixel center is. The part of the pixel inside them all
// is worked out exactly, by clipping the pixel to each in turn. Round ends add their circle.

inline float polygonCoverage(thread const float3 * planes, int count)
{
	bool bInside = true;
	for( int i = 0 ; i < count ; i++ )
	{
		if( planes[i].z <= -0.7072f )
			return 0.f;
		if( planes[i].z < 0.7072f )
			bInside = false;
	}
	if( bInside )
		return 1.f;

	float2 v[10];
	float2 o[10];
	int nv = 4;
	v[0] = float2(-0.5f, -0.5f);
	v[1] = float2(0.5f, -0.5f);
	v[2] = float2(0.5f, 0.5f);
	v[3] = float2(-0.5f, 0.5f);

	for( int i = 0 ; i < count ; i++ )
	{
		float3 pl = planes[i];
		if( pl.z >= 0.7072f )
			continue;

		int no = 0;
		for( int k = 0 ; k < nv ; k++ )
		{
			float2 a = v[k];
			float2 b = v[k + 1 < nv ? k + 1 : 0];
			float fa = pl.z + dot(a, pl.xy);
			float fb = pl.z + dot(b, pl.xy);
			if( fa >= 0.f )
				o[no++] = a;
			if( (fa >= 0.f) != (fb >= 0.f) )
				o[no++] = a + (b - a) * (fa / (fa - fb));
		}

		nv = no;
		if( nv < 3 )
			return 0.f;
		for( int k = 0 ; k < nv ; k++ )
			v[k] = o[k];
	}

	float area = 0.f;
	for( int k = 0 ; k < nv ; k++ )
	{
		float2 a = v[k];
		float2 b = v[k + 1 < nv ? k + 1 : 0];
		area += a.x * b.y - a.y * b.x;
	}
	return clamp(abs(area) * 0.5f, 0.f, 1.f);
}

// Adds the half-planes of a cap, at end with u pointing along the line and n across it. Round
// caps add the straight cut through the end, their circle is added by lineCoverage().

inline void addCapPlanes(int cap, float2 end, float2 u, float2 n, float r, float2 p, thread float3 * planes, thread int& count)
{
	float2 d = p - end;
	float a = dot(d, u);
	int shape = cap & 7;

	if( shape == 1 || shape == 3 )
		planes[count++] = float3(u, a);
	else if( shape == 2 )
		planes[count++] = float3(u, a + r);
	else if( shape == 4 )
	{
		float sx = u.x > 0.f ? 1.f : -1.f;
		planes[count++] = float3(sx, 0.f, d.x * sx);
	}
	else if( shape == 5 )
	{
		float sy = u.y > 0.f ? 1.f : -1.f;
		planes[count++] = float3(0.f, sy, d.y * sy);
	}
	else
	{
		float spear = float(cap >> 3) * 0.5f * r;
		float w = dot(d, n);
		float2 g1 = r * u - spear * n;
		float2 g2 = r * u + spear * n;
		float k = length(g1);
		planes[count++] = float3(g1 / k, (r * a - spear * w) / k);
		planes[count++] = float3(g2 / k, (r * a + spear * w) / k);
	}
}

inline void lineCoverage(constant float4 * pShape, constant float4 * pColor, int ofs, float2 p, thread float4& acc, thread float& covered)
{
	float4 head = pShape[ofs];
	float4 ends = pShape[ofs + 1];
	float4 dir = pShape[ofs + 2];

	float2 u = dir.xy;
	float2 n = float2(-u.y, u.x);
	float r = dir.w;
	int capB = int(head.y);
	int capE = int(head.z);

	float w = dot(p - ends.xy, n);

	float3 planes[6];
	planes[0] = float3(-n, r - w);
	planes[1] = float3(n, r + w);
	int count = 2;
	addCapPlanes(capB, ends.xy, u, n, r, p, planes, count);
	int countB = count;
	addCapPlanes(capE, ends.zw, -u, -n, r, p, planes, count);

	float c = polygonCoverage(planes, count);

	// A round end is the band cut through the end, together with its circle cut by the other end.

	bool bRoundB = (capB & 7) == 3;
	bool bRoundE = (capE & 7) == 3;

	if( bRoundB || bRoundE )
	{
		float2 dB = p - ends.xy;
		float2 dE = p - ends.zw;
		float discB = bRoundB ? circleCoverage(r, dB, length(dB)) : 0.f;
		float discE = bRoundE ? circleCoverage(r, dE, length(dE)) : 0.f;

		float regionB = max(polygonCoverage(planes + 2, countB - 2), discB);
		float regionE = max(polygonCoverage(planes + countB, count - countB), discE);
		c = max(c, max(min(discB, regionE), min(discE, regionB)));
	}

	if( c <= 0.f )
		return;

	float4 col = pColor[int(head.w)];
	acc += float4(col.rgb * col.a, col.a) * c;
	covered += c;
}

// Coverage of the drawn segments of a shape at a pixel center, their colors in acc.

inline float shapeCoverage(constant float4 * pShape, constant float4 * pColor, int ofs, float2 p, thread float4& acc)
{
	acc = float4(0.f);
	float covered = 0.f;

	int kind = int(pShape[ofs].x);
	if( kind == 1 )
		boxCoverage(pShape, pColor, ofs, p, acc, covered);
	else if( kind == 2 )
		ringCoverage(pShape, pColor, ofs, p, acc, covered);
	else if( kind == 3 )
		lineCoverage(pShape, pColor, ofs, p, acc, covered);

	return covered;
}

fragment FragOutput shapeFragmentShader(ShapeFragInput in [[stage_in]],
                                        constant float4 * pColor [[buffer(0)]],
                                        constant float4 * pExtras [[buffer(1)]],
                                        constant float & morphFactor [[buffer(5)]])
{
	float4 acc;
	float covered = shapeCoverage(pExtras, pColor, in.shapeOfs, in.position.xy, acc);
	if( covered < 1.f / 256.f )
		discard_fragment();

	return writeColor(acc * premul(in.color), covered, morphFactor);
}

fragment FragOutput shapeFragmentShader_A8(ShapeFragInput in [[stage_in]],
                                           constant float4 * pColor [[buffer(0)]],
                                           constant float4 * pExtras [[buffer(1)]],
                                           constant float & morphFactor [[buffer(5)]])
{
	float4 acc;
	float covered = shapeCoverage(pExtras, pColor, in.shapeOfs, in.position.xy, acc);
	if( covered < 1.f / 256.f )
		discard_fragment();

	return writeAlpha(acc.a * in.color.a, covered, morphFactor);
}

fragment FragOutput shapeTintmapFragmentShader(ShapeFragInput in [[stage_in]],
                                               constant float4 * pColor [[buffer(0)]],
                                               constant float4 * pExtras [[buffer(1)]],
                                               constant float & morphFactor [[buffer(5)]])
{
	float4 acc;
	float covered = shapeCoverage(pExtras, pColor, in.shapeOfs, in.position.xy, acc);
	if( covered < 1.f / 256.f )
		discard_fragment();

	return writeColor(acc * premul(evalTint(pColor, in.tintOfs, in.position.xy) * in.color), covered, morphFactor);
}

fragment FragOutput shapeTintmapFragmentShader_A8(ShapeFragInput in [[stage_in]],
                                                  constant float4 * pColor [[buffer(0)]],
                                                  constant float4 * pExtras [[buffer(1)]],
                                                  constant float & morphFactor [[buffer(5)]])
{
	float4 acc;
	float covered = shapeCoverage(pExtras, pColor, in.shapeOfs, in.position.xy, acc);
	if( covered < 1.f / 256.f )
		discard_fragment();

	return writeAlpha(acc.a * evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a, covered, morphFactor);
}


//____ Warps _______________________________________________________________
//
// Warps (Command::WarpBlit and WarpDrawEdgemap), see Warpmap for the definition they follow.
// Each pixel finds the cell of the Warpmap it is in and interpolates the position in the source
// at its center, and how fast that changes along x and y of the canvas. Coverage is worked out
// from those, taking each border of the source and each edge of an edgemap as straight over the
// pixel. Extras of a warp, four entries:
//
//   0: position of Warpmap on canvas, cell size, number of edges.
//   1: size of Warpmap, size of source in pixels.
//   2: source offset in pixels of the source, periods in pixels of the source (0 if none).
//   3: edgemaps: edges per column, offsets of flat colors, tint table and raw samples in the
//      edgemap buffer. Blits: position of source rectangle in the surface, how far inside its
//      edges positions are kept (0 for a tiling surface, see warpBlitCoverage()).
//
// The cells of the Warpmap are in its buffer, see MetalWarpmap.

typedef struct
{
	float4	position [[position]];
	float4	color;						// Tint color.
	int		warpOfs [[flat]];			// Entry of warp in extras buffer.
	int		tintOfs [[flat]];			// Device tint block in color buffer, -1 for none.
} WarpFragInput;

vertex WarpFragInput
warpVertexShader(uint vertexID [[vertex_id]],
				 constant Vertex *pVertices [[buffer(0)]],
				 constant vector_float4  *pColor [[buffer(1)]],
				 constant vector_float4  *pExtras [[buffer(2)]],
				 constant Uniform * pUniform[[buffer(3)]])
{
	WarpFragInput out;

	float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

	vector_float2 canvasSize = pUniform->canvasDim;

	out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
	out.position.x = pos.x*2 / canvasSize.x - 1.0;
	out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;

	out.color = pUniform->flatTint;
	out.warpOfs = pVertices[vertexID].extrasOfs;
	out.tintOfs = pVertices[vertexID].tintOfs;
	return out;
}

// Part of the pixel where f + g.x * dx + g.y * dy >= 0, dx and dy going from -0.5 to 0.5 over
// it, which is linear within (a - b) / 2 of the center and a triangle in a corner further out.

inline float warpHalfPlane(float f, float2 g)
{
	float2 ag = abs(g);
	float a = max(ag.x, ag.y);
	float b = min(ag.x, ag.y);

	if( 2.f * f >= a + b )
		return 1.f;
	if( 2.f * f <= -(a + b) )
		return 0.f;
	if( 2.f * abs(f) <= a - b )
		return 0.5f + f / a;

	float t = a + b - 2.f * abs(f);
	float corner = t * t / (8.f * a * b);
	return f > 0.f ? 1.f - corner : corner;
}

// Length of the source along an axis that is inside of each period.

inline float warpInside(float size, float period)
{
	return period > 0.f ? min(size, period) : size;
}

// Position s moved to the nearest copy if in the gap between copies on a periodic axis, which is
// where colors are taken and edges evaluated.

inline float2 warpNearestCopy(float2 s, float2 size, float2 period)
{
	if( period.x > 0.f && 2.f * s.x > warpInside(size.x, period.x) + period.x )
		s.x -= period.x;
	if( period.y > 0.f && 2.f * s.y > warpInside(size.y, period.y) + period.y )
		s.y -= period.y;
	return s;
}

// Part of the pixel inside the source along one axis, with the copies of the source on a
// periodic axis. s is the position at the center of the pixel, g how it changes along x and y.

inline float warpAxisCoverage(float s, float2 g, float size, float period)
{
	if( period > 0.f && period <= size )
		return 1.f;

	float inside = warpInside(size, period);
	float ext = 0.5f * (abs(g.x) + abs(g.y));

	if( s - ext >= 0.f && s + ext <= inside )
		return 1.f;

	if( period <= 0.f )
		return warpHalfPlane(s, g) - warpHalfPlane(s - inside, g);

	float c = 0.f;
	for( int k = -1 ; k <= 1 ; k++ )
	{
		float lo = float(k) * period;
		c += warpHalfPlane(s - lo, g) - warpHalfPlane(s - lo - inside, g);
	}
	return min(c, 1.f);
}

// Position in the source of the pixel center p, wrapped on periodic axes, and how it changes
// along x (dx) and y (dy) of the canvas. The cell is found with integers, as a division in
// floats can miss a whole number.

inline float2 warpPosition(constant float4 * pExtras, int ofs, constant float4 * pWarpmap, float2 p, thread float2& dx, thread float2& dy)
{
	float4 e0 = pExtras[ofs];
	float4 e1 = pExtras[ofs + 1];
	float4 e2 = pExtras[ofs + 2];

	float2 q = p - e0.xy;
	int cellSize = int(e0.z);
	int2 cells = (int2(e1.xy) + cellSize - 1) / cellSize;
	int2 cell = clamp(int2(floor(max(q, float2(0.f)))) / cellSize, int2(0), cells - 1);
	float2 c0 = float2(cell * cellSize);
	float2 c1 = min(c0 + float(cellSize), e1.xy);
	float2 f = (q - c0) / (c1 - c0);

	int i = (cell.y * cells.x + cell.x) * 2;
	float4 top = pWarpmap[i];
	float4 bottom = pWarpmap[i + 1];

	float2 u = mix(mix(top.xy, top.zw, f.x), mix(bottom.xy, bottom.zw, f.x), f.y);
	float2 s = u + e2.xy;
	dx = mix(top.zw - top.xy, bottom.zw - bottom.xy, f.y) / (c1.x - c0.x);
	dy = mix(bottom.xy - top.xy, bottom.zw - top.zw, f.x) / (c1.y - c0.y);

	if( e2.z > 0.f )
		s.x -= e2.z * floor(s.x / e2.z);
	if( e2.w > 0.f )
		s.y -= e2.w * floor(s.y / e2.w);

	return s;
}

//____ Warped edgemaps ______________________________________________________
//
// Parts of the pixel covered by the segments of an edgemap, in parts[], their sum returned.
// Edges are linear across the column at the center of the pixel (the nearest copy's for a
// center in the gap between copies on a periodic axis), so the border of the area below an edge
// is sy - edge(sx) = 0. Areas below the top of the source, each edge and the bottom of the
// source are measured, kept decreasing as an edge is pushed down to the edges above it, and
// their differences are the parts of the segments.

inline float warpSegmentParts(constant float4 * pExtras, int ofs, constant float4 * pEdgemap, float2 s, float2 dx, float2 dy, thread float * parts)
{
	float4 e0 = pExtras[ofs];
	float4 e1 = pExtras[ofs + 1];
	float4 e2 = pExtras[ofs + 2];
	float4 e3 = pExtras[ofs + 3];

	int nEdges = int(e0.w);

	for( int e = 0 ; e <= nEdges ; e++ )
		parts[e] = 0.f;

	float2 size = e1.zw;
	float2 period = e2.zw;
	float2 gx = float2(dx.x, dy.x);
	float2 gy = float2(dx.y, dy.y);

	float covX = warpAxisCoverage(s.x, gx, size.x, period.x);
	if( covX <= 0.f )
		return 0.f;

	int pitch = int(e3.x);

	float ex = warpNearestCopy(s, size, period).x;

	int column = clamp(int(floor(ex)), 0, int(size.x) - 1);
	float frac = ex - float(column);
	int base = int(e3.w) + column * pitch;

	float insideY = warpInside(size.y, period.y);
	float ext = 0.5f * (abs(gy.x) + abs(gy.y));
	int kBeg = period.y > 0.f ? -1 : 0;

	float covered = 0.f;

	for( int k = kBeg ; k <= -kBeg ; k++ )
	{
		float sy = s.y - float(k) * period.y;
		if( sy + ext <= 0.f || sy - ext >= insideY )
			continue;

		float above = warpHalfPlane(sy, gy);

		for( int e = 0 ; e <= nEdges ; e++ )
		{
			float below;
			if( e < nEdges )
			{
				float2 lr = pEdgemap[base + e].xy;
				float slope = lr.y - lr.x;
				below = min(above, warpHalfPlane(sy - lr.x - slope * frac, gy - slope * gx));
			}
			else
				below = min(above, warpHalfPlane(sy - insideY, gy));

			float part = (above - below) * covX;
			parts[e] += part;
			covered += part;
			above = below;
		}
	}

	return covered;
}

// Color of a segment at position epos in the edgemap, see segColor().

inline float4 warpSegColor(int seg, constant float4 * pEdgemap, int flatColorsOfs, int tintTableOfs, float2 epos)
{
	int tofs = int(pEdgemap[tintTableOfs + seg].x);
	return tofs < 0 ? pEdgemap[flatColorsOfs + seg] : evalTint(pEdgemap, tofs, epos);
}

// Segments are mixed with segPart() as for edgemaps. On canvases without alpha, Replace and
// Morph keep the colors of the results of the segments, see writeSegments(), here for the
// covered part of the pixel only.

fragment FragOutput warpEdgemapFragmentShader(WarpFragInput in [[stage_in]],
											  constant float4 * pColor [[buffer(0)]],
											  constant float4 * pExtras [[buffer(1)]],
											  constant float4 * pEdgemap [[buffer(3)]],
											  constant float & morphFactor [[buffer(5)]],
											  constant float4 * pWarpmap [[buffer(6)]])
{
	float2 dx, dy;
	float2 s = warpPosition(pExtras, in.warpOfs, pWarpmap, in.position.xy, dx, dy);

	float parts[16];						// MetalBackend::c_maxSegments.
	float covered = warpSegmentParts(pExtras, in.warpOfs, pEdgemap, s, dx, dy, parts);
	if( covered < 1.f / 256.f )
		discard_fragment();

	int nEdges = int(pExtras[in.warpOfs].w);
	float4 e3 = pExtras[in.warpOfs + 3];
	int flatColorsOfs = int(e3.y);
	int tintTableOfs = int(e3.z);
	float2 sc = warpNearestCopy(s, pExtras[in.warpOfs + 1].zw, pExtras[in.warpOfs + 2].zw);

	float4 tint = premul((in.tintOfs >= 0 ? evalTint(pColor, in.tintOfs, in.position.xy) : float4(1.f)) * in.color);

	float4 acc = float4(0.f);
	float keep = 0.f;

	for( int e = 0 ; e <= nEdges ; e++ )
	{
		if( parts[e] > 0.f )
			acc += segPart(premul(warpSegColor(e, pEdgemap, flatColorsOfs, tintTableOfs, sc)) * tint, parts[e], morphFactor, keep);
	}

	if( c_outputMode == 1 && c_noAlpha )
	{
		FragOutput out;
		out.color = acc;
		out.factor = float4(covered - keep);
		return out;
	}

	return writeColor(acc, covered, morphFactor);
}

fragment FragOutput warpEdgemapFragmentShader_A8(WarpFragInput in [[stage_in]],
												 constant float4 * pColor [[buffer(0)]],
												 constant float4 * pExtras [[buffer(1)]],
												 constant float4 * pEdgemap [[buffer(3)]],
												 constant float & morphFactor [[buffer(5)]],
												 constant float4 * pWarpmap [[buffer(6)]])
{
	float2 dx, dy;
	float2 s = warpPosition(pExtras, in.warpOfs, pWarpmap, in.position.xy, dx, dy);

	float parts[16];						// MetalBackend::c_maxSegments.
	float covered = warpSegmentParts(pExtras, in.warpOfs, pEdgemap, s, dx, dy, parts);
	if( covered < 1.f / 256.f )
		discard_fragment();

	int nEdges = int(pExtras[in.warpOfs].w);
	float4 e3 = pExtras[in.warpOfs + 3];
	int flatColorsOfs = int(e3.y);
	int tintTableOfs = int(e3.z);
	float2 sc = warpNearestCopy(s, pExtras[in.warpOfs + 1].zw, pExtras[in.warpOfs + 2].zw);

	float acc = 0.f;

	for( int e = 0 ; e <= nEdges ; e++ )
	{
		if( parts[e] > 0.f )
			acc += warpSegColor(e, pEdgemap, flatColorsOfs, tintTableOfs, sc).a * parts[e];
	}

	float4 tint = (in.tintOfs >= 0 ? evalTint(pColor, in.tintOfs, in.position.xy) : float4(1.f)) * in.color;

	return writeAlpha(acc * tint.a, covered, morphFactor);
}

//____ Warped blits _________________________________________________________
//
// The source is sampled at the position of the center of the pixel, with its sample method.
// A tiling surface wraps around as the sampler is set up. Any other is read as if the edge pixels
// of the source rectangle continued outside it, by keeping positions half a texel (bilinear) or a
// hair (nearest) inside its edges. Gradients are given for mipmapped surfaces, which the
// derivatives of positions that wrap would get wrong.

// Part of the pixel covered by the source, the position in the surface to read in s.

inline float warpBlitCoverage(constant float4 * pExtras, int ofs, constant float4 * pWarpmap, float2 p, thread float2& s, thread float2& dx, thread float2& dy)
{
	s = warpPosition(pExtras, ofs, pWarpmap, p, dx, dy);

	float4 e1 = pExtras[ofs + 1];
	float4 e2 = pExtras[ofs + 2];
	float4 e3 = pExtras[ofs + 3];

	float cov = warpAxisCoverage(s.x, float2(dx.x, dy.x), e1.z, e2.z);
	if( cov > 0.f )
		cov *= warpAxisCoverage(s.y, float2(dx.y, dy.y), e1.w, e2.w);

	s = e3.xy + warpNearestCopy(s, e1.zw, e2.zw);

	if( e3.z > 0.f )
	{
		float2 center = e3.xy + 0.5f * e1.zw;
		s = clamp(s, min(e3.xy + e3.z, center), max(e3.xy + e1.zw - e3.z, center));
	}

	return cov;
}

// sourceKind: 0 colors, 1 alpha only (Alpha_8, as white with alpha), 2 colors without alpha (RGBX).

inline float4 warpSample(texture2d<float> colorTexture, sampler textureSampler, float2 p, float2 dx, float2 dy, int sourceKind)
{
	float2 texSize = float2(colorTexture.get_width(), colorTexture.get_height());
	float4 c = colorTexture.sample(textureSampler, p / texSize, gradient2d(dx / texSize, dy / texSize));

	if( sourceKind == 1 )
		return float4(c.r);
	if( sourceKind == 2 )
		c.a = 1.f;
	return c;
}

// Palette based sources are read through a nearest sampler and looked up texel by texel.
// sourceKind: 0 nearest, 1 bilinear, which mixes the four texels around the position.

inline float4 warpPaletteSample(texture2d<float> colorTexture, texture2d<half> paletteTexture, sampler textureSampler, float2 p, int sourceKind)
{
	float2 texSize = float2(colorTexture.get_width(), colorTexture.get_height());

	if( sourceKind == 0 )
		return float4(paletteLookup(paletteTexture, colorTexture.sample(textureSampler, p / texSize, level(0)).rg));

	float2 uv00, uv11, f;
	paletteTaps(p - 0.5f, texSize, uv00, uv11, f);

	float4 c00 = float4(paletteLookup(paletteTexture, colorTexture.sample(textureSampler, uv00, level(0)).rg));
	float4 c01 = float4(paletteLookup(paletteTexture, colorTexture.sample(textureSampler, float2(uv11.x, uv00.y), level(0)).rg));
	float4 c10 = float4(paletteLookup(paletteTexture, colorTexture.sample(textureSampler, float2(uv00.x, uv11.y), level(0)).rg));
	float4 c11 = float4(paletteLookup(paletteTexture, colorTexture.sample(textureSampler, uv11, level(0)).rg));

	return mix(mix(c00, c01, f.x), mix(c10, c11, f.x), f.y);
}

inline float4 warpTint(WarpFragInput in, constant float4 * pColor)
{
	return (in.tintOfs >= 0 ? evalTint(pColor, in.tintOfs, in.position.xy) : float4(1.f)) * in.color;
}

fragment FragOutput warpBlitFragmentShader(WarpFragInput in [[stage_in]],
										   texture2d<float> colorTexture [[ texture(0) ]],
										   sampler textureSampler [[ sampler(0) ]],
										   constant float4 * pColor [[buffer(0)]],
										   constant float4 * pExtras [[buffer(1)]],
										   constant int & sourceKind [[buffer(4)]],
										   constant float & morphFactor [[buffer(5)]],
										   constant float4 * pWarpmap [[buffer(6)]])
{
	float2 s, dx, dy;
	float cov = warpBlitCoverage(pExtras, in.warpOfs, pWarpmap, in.position.xy, s, dx, dy);
	if( cov < 1.f / 256.f )
		discard_fragment();

	float4 c = warpSample(colorTexture, textureSampler, s, dx, dy, sourceKind);

	return writeColor(c * premul(warpTint(in, pColor)) * cov, cov, morphFactor);
}

fragment FragOutput warpBlitFragmentShader_A8(WarpFragInput in [[stage_in]],
											  texture2d<float> colorTexture [[ texture(0) ]],
											  sampler textureSampler [[ sampler(0) ]],
											  constant float4 * pColor [[buffer(0)]],
											  constant float4 * pExtras [[buffer(1)]],
											  constant int & sourceKind [[buffer(4)]],
											  constant float & morphFactor [[buffer(5)]],
											  constant float4 * pWarpmap [[buffer(6)]])
{
	float2 s, dx, dy;
	float cov = warpBlitCoverage(pExtras, in.warpOfs, pWarpmap, in.position.xy, s, dx, dy);
	if( cov < 1.f / 256.f )
		discard_fragment();

	float4 c = warpSample(colorTexture, textureSampler, s, dx, dy, sourceKind);

	return writeAlpha(c.a * warpTint(in, pColor).a * cov, cov, morphFactor);
}

fragment FragOutput warpPaletteBlitFragmentShader(WarpFragInput in [[stage_in]],
												  texture2d<float> colorTexture [[ texture(0) ]],
												  texture2d<half> paletteTexture [[ texture(1) ]],
												  sampler textureSampler [[ sampler(0) ]],
												  constant float4 * pColor [[buffer(0)]],
												  constant float4 * pExtras [[buffer(1)]],
												  constant int & sourceKind [[buffer(4)]],
												  constant float & morphFactor [[buffer(5)]],
												  constant float4 * pWarpmap [[buffer(6)]])
{
	float2 s, dx, dy;
	float cov = warpBlitCoverage(pExtras, in.warpOfs, pWarpmap, in.position.xy, s, dx, dy);
	if( cov < 1.f / 256.f )
		discard_fragment();

	float4 c = warpPaletteSample(colorTexture, paletteTexture, textureSampler, s, sourceKind);

	return writeColor(c * premul(warpTint(in, pColor)) * cov, cov, morphFactor);
}

fragment FragOutput warpPaletteBlitFragmentShader_A8(WarpFragInput in [[stage_in]],
													 texture2d<float> colorTexture [[ texture(0) ]],
													 texture2d<half> paletteTexture [[ texture(1) ]],
													 sampler textureSampler [[ sampler(0) ]],
													 constant float4 * pColor [[buffer(0)]],
													 constant float4 * pExtras [[buffer(1)]],
													 constant int & sourceKind [[buffer(4)]],
													 constant float & morphFactor [[buffer(5)]],
													 constant float4 * pWarpmap [[buffer(6)]])
{
	float2 s, dx, dy;
	float cov = warpBlitCoverage(pExtras, in.warpOfs, pWarpmap, in.position.xy, s, dx, dy);
	if( cov < 1.f / 256.f )
		discard_fragment();

	float4 c = warpPaletteSample(colorTexture, paletteTexture, textureSampler, s, sourceKind);

	return writeAlpha(c.a * warpTint(in, pColor).a * cov, cov, morphFactor);
}


//____ Mipmap generation ______________________________________________________
//
// Draws a mip level from the one above it, see MetalBackend::_generateMipmaps().
// One triangle covers the whole level. The source view holds the level above
// only, and a pixel's centre lands between four of its texels, so the bilinear
// sampler averages them. An _sRGB view converts to linear on the way in and the
// render target back on the way out, so the average is taken in linear space.

typedef struct
{
	float4 position [[position]];
	float2 texUV;
} MipmapFragInput;

vertex MipmapFragInput mipmapVertexShader(uint vertexID [[vertex_id]])
{
	MipmapFragInput out;

	out.texUV = float2(float((vertexID << 1) & 2), float(vertexID & 2));
	out.position = float4(out.texUV.x * 2.0 - 1.0, 1.0 - out.texUV.y * 2.0, 0.0, 1.0);

	return out;
}

fragment float4 mipmapFragmentShader(MipmapFragInput in [[stage_in]],
									 texture2d<float> source [[texture(0)]])
{
	constexpr sampler linearSampler(filter::linear, address::clamp_to_edge);

	return source.sample(linearSampler, in.texUV, level(0));
}
