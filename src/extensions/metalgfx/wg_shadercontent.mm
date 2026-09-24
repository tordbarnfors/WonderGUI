
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
    float2 texUV00;
    float2 texUV11;
    float2 uvFrac;
 } PaletteBlitInterpolateFragInput;

//____ PaletteBlitInterpolateTintmapFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float2 texUV00;
    float2 texUV11;
    float2 uvFrac;
    int tintOfs [[flat]];
 } PaletteBlitInterpolateTintmapFragInput;


//____ SegmentsFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
	float4 color;
    float2 texUV;
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

    return out;
}

//____ lineFragmentShader() ____________________________________________

fragment float4 lineFragmentShader(LineFragInput in [[stage_in]])
{
   float4 outColor;

   outColor.rgb = in.color.rgb;
   outColor.a = in.color.a * clamp(in.w - abs(in.position.x*in.ifSteep + in.position.y*in.ifMild - in.s - (in.position.x*in.ifMild + in.position.y*in.ifSteep) * in.slope), 0.0, 1.0);

   return outColor;
};

//____ lineFragmentShader_A8() ____________________________________________

fragment float4 lineFragmentShader_A8(LineFragInput in [[stage_in]])
{
   float4 outColor;
   outColor.r = in.color.a * clamp(in.w - abs(in.position.x*in.ifSteep + in.position.y*in.ifMild - in.s - (in.position.x*in.ifMild + in.position.y*in.ifSteep) * in.slope), 0.0, 1.0);

   return outColor;
};


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

fragment float4 fillFragmentShader(FillFragInput in [[stage_in]])
{
    return in.color;
};

//____ fillFragmentShader_A8() ____________________________________________

fragment float4 fillFragmentShader_A8(FillFragInput in [[stage_in]])
{
    return {in.color.a,0.0,0.0,0.0};
};


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

fragment float4 fillTintmapFragmentShader(FillTintmapFragInput in [[stage_in]],
                                    constant float4  *pColor [[buffer(0)]])
{
    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    return tintColor * in.color;
};

//____ fillTintmapFragmentShader_A8() ____________________________________________

fragment float4 fillTintmapFragmentShader_A8(FillTintmapFragInput in [[stage_in]],
                                    constant float4  *pColor [[buffer(0)]])
{
    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    return {tintAlpha * in.color.a,0.0,0.0,0.0};
};

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

fragment float4 fillAAFragmentShader(FillAAFragInput in [[stage_in]])
{
    float4 color = in.color;
    float2 middleofs = abs(in.position.xy - in.rect.xy);
    float2 alphas = clamp(in.rect.zw  - middleofs, 0.f, 1.f);
    color.a = in.color.a * alphas.x * alphas.y;

    return color;
};

//____ fillAAFragmentShader_A8() ____________________________________________

fragment float4 fillAAFragmentShader_A8(FillAAFragInput in [[stage_in]])
{
    float2 middleofs = abs(in.position.xy - in.rect.xy);
    float2 alphas = clamp(in.rect.zw  - middleofs, 0.f, 1.f);
    
    return { in.color.a * alphas.x * alphas.y, 0.0, 0.0, 0.0 };
};


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

fragment float4 fillAATintmapFragmentShader(FillAATintmapFragInput in [[stage_in]],
                                    constant float4  *pColor [[buffer(0)]])
{
    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    float4 color = tintColor * in.color;

    float2 middleofs = abs(in.position.xy - in.rect.xy);
    float2 alphas = clamp(in.rect.zw  - middleofs, 0.f, 1.f);
    color.a = color.a * alphas.x * alphas.y;

    return color;
};

//____ fillAATintmapFragmentShader_A8() ____________________________________________

fragment float4 fillAATintmapFragmentShader_A8(FillAATintmapFragInput in [[stage_in]],
                                        constant float4  *pColor [[buffer(0)]])
{
    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    float alpha = tintAlpha * in.color.a;

    float2 middleofs = abs(in.position.xy - in.rect.xy);
    float2 alphas = clamp(in.rect.zw  - middleofs, 0.f, 1.f);
    
    return { alpha * alphas.x * alphas.y, 0.0, 0.0, 0.0 };
};



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

fragment float4 blitFragmentShader(BlitFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]])
{
    const half4 colorSample = colorTexture.sample(textureSampler, in.texUV);

    return float4(colorSample) * in.color;
};

//____ blitFragmentShader_A8() ____________________________________________

fragment float4 blitFragmentShader_A8(BlitFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]])
{
    const float colorSample = float(colorTexture.sample(textureSampler, in.texUV).a);

    return { colorSample * in.color.a, 0.0, 0.0, 0.0 };
};

//____ rgbxBlitFragmentShader() ____________________________________________

fragment float4 rgbxBlitFragmentShader(BlitFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]])
{
    half4 colorSample = colorTexture.sample(textureSampler, in.texUV);
    colorSample.a = (half) 1.0;

    return float4(colorSample) * in.color;
};

//____ rgbxBlitFragmentShader_A8() ____________________________________________

fragment float4 rgbxBlitFragmentShader_A8(BlitFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]])
{
    return { in.color.a, 0.0, 0.0, 0.0 };
};


//____ alphaBlitFragmentShader() ____________________________________________

fragment float4 alphaBlitFragmentShader(BlitFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]])
{
    float4 color = in.color;
    color.a *= colorTexture.sample(textureSampler, in.texUV).r;

    return color;
};

//____ alphaBlitFragmentShader_A8() ____________________________________________

fragment float4 alphaBlitFragmentShader_A8(BlitFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]])
{
    const float colorSample = float(colorTexture.sample(textureSampler, in.texUV).r);

    return { colorSample * in.color.a, 0.0, 0.0, 0.0 };
};

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

fragment float4 blitTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]])

{
    const half4 colorSample = colorTexture.sample(textureSampler, in.texUV);

    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    return float4(colorSample) * tintColor * in.color;
};

//____ blitTintmapFragmentShader_A8() ____________________________________________

fragment float4 blitTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]])
{
    const float colorSample = float(colorTexture.sample(textureSampler, in.texUV).a);

    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    return { colorSample * tintAlpha * in.color.a, 0.0, 0.0, 0.0 };
};

//____ rgbxBlitTintmapFragmentShader() ____________________________________________

fragment float4 rgbxBlitTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]])
{
    half4 colorSample = colorTexture.sample(textureSampler, in.texUV);
    colorSample.a = (half) 1.0;

    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    return float4(colorSample) * tintColor * in.color;
};

//____ rgbxBlitTintmapFragmentShader_A8() ____________________________________________

fragment float4 rgbxBlitTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]])
{
    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    return { tintAlpha * in.color.a, 0.0, 0.0, 0.0 };
};


//____ alphaBlitTintmapFragmentShader() ____________________________________________

fragment float4 alphaBlitTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]])
{
    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    float4 color = tintColor * in.color;

    color.a *= colorTexture.sample(textureSampler, in.texUV).r;

    return color;
};

//____ alphaBlitTintmapFragmentShader_A8() ____________________________________________

fragment float4 alphaBlitTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                    texture2d<half> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]])
{
    const float colorSample = float(colorTexture.sample(textureSampler, in.texUV).r);

    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    return { colorSample * tintAlpha * in.color.a, 0.0, 0.0, 0.0 };
};




//____ paletteBlitNearestFragmentShader() ____________________________________________

fragment float4 paletteBlitNearestFragmentShader(BlitFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    texture2d<half> paletteTexture [[ texture(1) ]],
                                    sampler textureSampler [[ sampler(0) ]])
{
    constexpr sampler paletteSampler (mag_filter::nearest,
                                      min_filter::nearest);

    const float colorIndex = colorTexture.sample(textureSampler, in.texUV).r;
    const half4 colorSample = paletteTexture.sample(paletteSampler, {colorIndex,0.5f} );

//    return in.color;
    return float4(colorSample) * in.color;
};

//____ paletteBlitNearestFragmentShader_A8() ____________________________________________

fragment float4 paletteBlitNearestFragmentShader_A8(BlitFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    texture2d<half> paletteTexture [[ texture(1) ]],
                                    sampler textureSampler [[ sampler(0) ]])
{
    constexpr sampler paletteSampler (mag_filter::nearest,
                                      min_filter::nearest);

    const float colorIndex = colorTexture.sample(textureSampler, in.texUV).r;
    const float colorSample = paletteTexture.sample(paletteSampler, {colorIndex,0.5f} ).a;

    return { colorSample * in.color.a, 0.0, 0.0, 0.0 };
};


//____ paletteBlitNearestTintmapFragmentShader() ____________________________________________

fragment float4 paletteBlitNearestTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    texture2d<half> paletteTexture [[ texture(1) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]])
{
    constexpr sampler paletteSampler (mag_filter::nearest,
                                      min_filter::nearest);

    const float colorIndex = colorTexture.sample(textureSampler, in.texUV).r;
    const half4 colorSample = paletteTexture.sample(paletteSampler, {colorIndex,0.5f} );

    float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    return float4(colorSample) * tintColor * in.color;
};

//____ paletteBlitNearestTintmapFragmentShader_A8() ____________________________________________

fragment float4 paletteBlitNearestTintmapFragmentShader_A8(BlitTintmapFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    texture2d<half> paletteTexture [[ texture(1) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]])
{
    constexpr sampler paletteSampler (mag_filter::nearest,
                                      min_filter::nearest);

    const float colorIndex = colorTexture.sample(textureSampler, in.texUV).r;
    const float colorSample = paletteTexture.sample(paletteSampler, {colorIndex,0.5f} ).a;

    float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

    return { colorSample * tintAlpha * in.color.a, 0.0, 0.0, 0.0 };
};



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
    out.texUV00 = texUV/ (float2) pUniform->texSize;
    out.texUV11 = (texUV+1)/ (float2) pUniform->texSize;

    out.color = pUniform->flatTint;

    return out;
}

//____ paletteBlitInterpolateFragmentShader() ____________________________________________

fragment float4 paletteBlitInterpolateFragmentShader(PaletteBlitInterpolateFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    texture2d<half> paletteTexture [[ texture(1) ]],
                                    sampler textureSampler [[ sampler(0) ]])
{
    constexpr sampler paletteSampler (mag_filter::nearest,
                                      min_filter::nearest);

   float index00 = colorTexture.sample(textureSampler, in.texUV00).r;
   float index01 = colorTexture.sample(textureSampler, float2(in.texUV11.x,in.texUV00.y) ).r;
   float index10 = colorTexture.sample(textureSampler, float2(in.texUV00.x,in.texUV11.y) ).r;
   float index11 = colorTexture.sample(textureSampler, in.texUV11).r;
   half4 color00 = paletteTexture.sample(paletteSampler, float2(index00,0.5f));
   half4 color01 = paletteTexture.sample(paletteSampler, float2(index01,0.5f));
   half4 color10 = paletteTexture.sample(paletteSampler, float2(index10,0.5f));
   half4 color11 = paletteTexture.sample(paletteSampler, float2(index11,0.5f));

   half4 out0 = color00 * (1-fract(in.uvFrac.x)) + color01 * fract(in.uvFrac.x);
   half4 out1 = color10 * (1-fract(in.uvFrac.x)) + color11 * fract(in.uvFrac.x);
   half4 colorSample = (out0 * (1-fract(in.uvFrac.y)) + out1 * fract(in.uvFrac.y));

    return float4(colorSample) * in.color;
};

//____ paletteBlitInterpolateFragmentShader_A8() ____________________________________________

fragment float4 paletteBlitInterpolateFragmentShader_A8(PaletteBlitInterpolateFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    texture2d<half> paletteTexture [[ texture(1) ]],
                                    sampler textureSampler [[ sampler(0) ]])
{
    constexpr sampler paletteSampler (mag_filter::nearest,
                                      min_filter::nearest);

   float index00 = colorTexture.sample(textureSampler, in.texUV00).r;
   float index01 = colorTexture.sample(textureSampler, float2(in.texUV11.x,in.texUV00.y) ).r;
   float index10 = colorTexture.sample(textureSampler, float2(in.texUV00.x,in.texUV11.y) ).r;
   float index11 = colorTexture.sample(textureSampler, in.texUV11).r;
   float color00 = paletteTexture.sample(paletteSampler, float2(index00,0.5f)).a;
   float color01 = paletteTexture.sample(paletteSampler, float2(index01,0.5f)).a;
   float color10 = paletteTexture.sample(paletteSampler, float2(index10,0.5f)).a;
   float color11 = paletteTexture.sample(paletteSampler, float2(index11,0.5f)).a;

   float out0 = color00 * (1-fract(in.uvFrac.x)) + color01 * fract(in.uvFrac.x);
   float out1 = color10 * (1-fract(in.uvFrac.x)) + color11 * fract(in.uvFrac.x);
   float colorSample = (out0 * (1-fract(in.uvFrac.y)) + out1 * fract(in.uvFrac.y));

   return { colorSample * in.color.a, 0.0, 0.0, 0.0 };
};

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
    out.texUV00 = texUV/ (float2) pUniform->texSize;
    out.texUV11 = (texUV+1)/ (float2) pUniform->texSize;

    out.tintOfs = pVertices[vertexID].tintOfs;
    out.color = pUniform->flatTint;

    return out;
}

//____ paletteBlitInterpolateTintmapFragmentShader() ____________________________________________

fragment float4 paletteBlitInterpolateTintmapFragmentShader(PaletteBlitInterpolateTintmapFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    texture2d<half> paletteTexture [[ texture(1) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]])
{
    constexpr sampler paletteSampler (mag_filter::nearest,
                                      min_filter::nearest);

   float index00 = colorTexture.sample(textureSampler, in.texUV00).r;
   float index01 = colorTexture.sample(textureSampler, float2(in.texUV11.x,in.texUV00.y) ).r;
   float index10 = colorTexture.sample(textureSampler, float2(in.texUV00.x,in.texUV11.y) ).r;
   float index11 = colorTexture.sample(textureSampler, in.texUV11).r;
   half4 color00 = paletteTexture.sample(paletteSampler, float2(index00,0.5f));
   half4 color01 = paletteTexture.sample(paletteSampler, float2(index01,0.5f));
   half4 color10 = paletteTexture.sample(paletteSampler, float2(index10,0.5f));
   half4 color11 = paletteTexture.sample(paletteSampler, float2(index11,0.5f));

   half4 out0 = color00 * (1-fract(in.uvFrac.x)) + color01 * fract(in.uvFrac.x);
   half4 out1 = color10 * (1-fract(in.uvFrac.x)) + color11 * fract(in.uvFrac.x);
   half4 colorSample = (out0 * (1-fract(in.uvFrac.y)) + out1 * fract(in.uvFrac.y));

   float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

   return float4(colorSample) * tintColor * in.color;
};

//____ paletteBlitInterpolateTintmapFragmentShader_A8() ____________________________________________

fragment float4 paletteBlitInterpolateTintmapFragmentShader_A8(PaletteBlitInterpolateTintmapFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    texture2d<half> paletteTexture [[ texture(1) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]])
{
    constexpr sampler paletteSampler (mag_filter::nearest,
                                      min_filter::nearest);

   float index00 = colorTexture.sample(textureSampler, in.texUV00).r;
   float index01 = colorTexture.sample(textureSampler, float2(in.texUV11.x,in.texUV00.y) ).r;
   float index10 = colorTexture.sample(textureSampler, float2(in.texUV00.x,in.texUV11.y) ).r;
   float index11 = colorTexture.sample(textureSampler, in.texUV11).r;
   float color00 = paletteTexture.sample(paletteSampler, float2(index00,0.5f)).a;
   float color01 = paletteTexture.sample(paletteSampler, float2(index01,0.5f)).a;
   float color10 = paletteTexture.sample(paletteSampler, float2(index10,0.5f)).a;
   float color11 = paletteTexture.sample(paletteSampler, float2(index11,0.5f)).a;

   float out0 = color00 * (1-fract(in.uvFrac.x)) + color01 * fract(in.uvFrac.x);
   float out1 = color10 * (1-fract(in.uvFrac.x)) + color11 * fract(in.uvFrac.x);
   float colorSample = (out0 * (1-fract(in.uvFrac.y)) + out1 * fract(in.uvFrac.y));

   float tintAlpha = evalTint(pColor, in.tintOfs, in.position.xy).a;

   return { colorSample * tintAlpha * in.color.a, 0.0, 0.0, 0.0 };
};



//____ blurFragmentShader() ____________________________________________

fragment float4 blurFragmentShader(BlitFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant BlurUniform  *pBlurInfo [[buffer(2)]])
{

   float4 color = colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[0] ) * pBlurInfo->colorMtx[0];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[1] ) * pBlurInfo->colorMtx[1];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[2] ) * pBlurInfo->colorMtx[2];

   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[3] ) * pBlurInfo->colorMtx[3];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[4] ) * pBlurInfo->colorMtx[4];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[5] ) * pBlurInfo->colorMtx[5];

   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[6] ) * pBlurInfo->colorMtx[6];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[7] ) * pBlurInfo->colorMtx[7];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[8] ) * pBlurInfo->colorMtx[8];

   color.a = 1.f;
    return color * in.color;
};


//____ blurTintmapFragmentShader() ____________________________________________

fragment float4 blurTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant float4  *pColor [[buffer(0)]],
                                    constant BlurUniform  *pBlurInfo [[buffer(2)]])
{

   float4 color = colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[0] ) * pBlurInfo->colorMtx[0];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[1] ) * pBlurInfo->colorMtx[1];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[2] ) * pBlurInfo->colorMtx[2];

   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[3] ) * pBlurInfo->colorMtx[3];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[4] ) * pBlurInfo->colorMtx[4];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[5] ) * pBlurInfo->colorMtx[5];

   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[6] ) * pBlurInfo->colorMtx[6];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[7] ) * pBlurInfo->colorMtx[7];
   color += colorTexture.sample(textureSampler, in.texUV + pBlurInfo->offset[8] ) * pBlurInfo->colorMtx[8];

   color.a = 1.f;

   float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

    return color * tintColor * in.color;
};



//____ paletteBlurFragmentShader() ____________________________________________
//
// Blur from a palette based source. Each tap is looked up in the palette before
// it is weighted, the way paletteBlitNearestFragmentShader() does it. A palette
// source is always read through a nearest sampler, so the taps don't blend
// indexes.

inline float4 paletteBlurCore(float2 texUV, texture2d<float> colorTexture, texture2d<half> paletteTexture,
							  sampler textureSampler, constant BlurUniform* pBlurInfo)
{
	constexpr sampler paletteSampler (mag_filter::nearest,
									  min_filter::nearest);

	float4 color = float4(0,0,0,0);

	for( int i = 0 ; i < 9 ; i++ )
	{
		float colorIndex = colorTexture.sample(textureSampler, texUV + pBlurInfo->offset[i] ).r;
		color += float4(paletteTexture.sample(paletteSampler, {colorIndex,0.5f} )) * pBlurInfo->colorMtx[i];
	}

	color.a = 1.f;
	return color;
}

fragment float4 paletteBlurFragmentShader(BlitFragInput in [[stage_in]],
									texture2d<float> colorTexture [[ texture(0) ]],
									texture2d<half> paletteTexture [[ texture(1) ]],
									sampler textureSampler [[ sampler(0) ]],
									constant BlurUniform  *pBlurInfo [[buffer(2)]])
{
	return paletteBlurCore(in.texUV, colorTexture, paletteTexture, textureSampler, pBlurInfo) * in.color;
};

//____ paletteBlurTintmapFragmentShader() _____________________________________

fragment float4 paletteBlurTintmapFragmentShader(BlitTintmapFragInput in [[stage_in]],
									texture2d<float> colorTexture [[ texture(0) ]],
									texture2d<half> paletteTexture [[ texture(1) ]],
									sampler textureSampler [[ sampler(0) ]],
									constant float4  *pColor [[buffer(0)]],
									constant BlurUniform  *pBlurInfo [[buffer(2)]])
{
	float4 color = paletteBlurCore(in.texUV, colorTexture, paletteTexture, textureSampler, pBlurInfo);

	float4 tintColor = evalTint(pColor, in.tintOfs, in.position.xy);

	return color * tintColor * in.color;
};



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
    out.texUV = pVertices[vertexID].uv;
	out.tintOfs = pVertices[vertexID].tintOfs;
	out.color = pUniform->flatTint;
	return out;
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

template <int EDGES>
inline float4 segFragShaderCore(SegmentsFragInput in,
                                    constant float4  *pEdgemap)
{
    float totalAlpha = 0.f;
    float3    rgbAcc = float3(0,0,0);


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
        float factor2 = clamp(x + adder, 0.f, 1.f);

        float useFactor = (factor - factor2)*col.a;
        totalAlpha += useFactor;
        rgbAcc += col.rgb * useFactor;
        factor = factor2;
    }

	float4 col = segColor(EDGES, in, pEdgemap, epos);

	float useFactor = factor*col.a;
    totalAlpha += useFactor;
	rgbAcc += col.rgb * useFactor;

    float4 out;
	out.a = totalAlpha; // * in.color.a;
	out.rgb = totalAlpha > 0.f ? rgbAcc/totalAlpha : float3(0,0,0);	// Fully transparent would be 0/0.
	return out;
};


fragment float4 segmentsFragmentShader1(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<1>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader2(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<2>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader3(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<3>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader4(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<4>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader5(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<5>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader6(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<6>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader7(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<7>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader8(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<8>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader9(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<9>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader10(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<10>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader11(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<11>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader12(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<12>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader13(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<13>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader14(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<14>(in,pEdgemap) * in.color;
};

fragment float4 segmentsFragmentShader15(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore<15>(in,pEdgemap) * in.color;
};


fragment float4 segmentsTintmapFragmentShader1(SegmentsFragInput in [[stage_in]],
											   constant float4  *pColor [[buffer(0)]],
											   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<1>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader2(SegmentsFragInput in [[stage_in]],
											   constant float4  *pColor [[buffer(0)]],
											   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<2>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader3(SegmentsFragInput in [[stage_in]],
											   constant float4  *pColor [[buffer(0)]],
											   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<3>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader4(SegmentsFragInput in [[stage_in]],
											   constant float4  *pColor [[buffer(0)]],
											   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<4>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader5(SegmentsFragInput in [[stage_in]],
											   constant float4  *pColor [[buffer(0)]],
											   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<5>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader6(SegmentsFragInput in [[stage_in]],
											   constant float4  *pColor [[buffer(0)]],
											   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<6>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader7(SegmentsFragInput in [[stage_in]],
											   constant float4  *pColor [[buffer(0)]],
											   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<7>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader8(SegmentsFragInput in [[stage_in]],
											   constant float4  *pColor [[buffer(0)]],
											   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<8>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader9(SegmentsFragInput in [[stage_in]],
											   constant float4  *pColor [[buffer(0)]],
											   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<9>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader10(SegmentsFragInput in [[stage_in]],
												constant float4  *pColor [[buffer(0)]],
												constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<10>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader11(SegmentsFragInput in [[stage_in]],
												constant float4  *pColor [[buffer(0)]],
												constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<11>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader12(SegmentsFragInput in [[stage_in]],
												constant float4  *pColor [[buffer(0)]],
												constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<12>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader13(SegmentsFragInput in [[stage_in]],
												constant float4  *pColor [[buffer(0)]],
												constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<13>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader14(SegmentsFragInput in [[stage_in]],
												constant float4  *pColor [[buffer(0)]],
												constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<14>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};

fragment float4 segmentsTintmapFragmentShader15(SegmentsFragInput in [[stage_in]],
												constant float4  *pColor [[buffer(0)]],
												constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore<15>(in,pEdgemap) * evalTint(pColor, in.tintOfs, in.position.xy) * in.color;
};



//____ segmentsFragmentShader_A8() ____________________________________________

template <int EDGES>
inline float4 segFragShaderCore_A8(SegmentsFragInput in,
                                    constant float4  *pEdgemap)
{
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
		float factor2 = clamp(x + adder, 0.f, 1.f);

		totalAlpha += (factor - factor2)*alpha;

		factor = factor2;
	}

	float alpha = segColor(EDGES, in, pEdgemap, epos).a;
	totalAlpha += factor*alpha;

	return { totalAlpha, 0.f, 0.f, 0.f };
};



fragment float4 segmentsFragmentShader1_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<1>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader2_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<2>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader3_A8(SegmentsFragInput in [[stage_in]],
								   constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<3>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader4_A8(SegmentsFragInput in [[stage_in]],
								   constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<4>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader5_A8(SegmentsFragInput in [[stage_in]],
								   constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<5>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader6_A8(SegmentsFragInput in [[stage_in]],
								   constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<6>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader7_A8(SegmentsFragInput in [[stage_in]],
								   constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<7>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader8_A8(SegmentsFragInput in [[stage_in]],
								   constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<8>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader9_A8(SegmentsFragInput in [[stage_in]],
								   constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<9>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader10_A8(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<10>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader11_A8(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<11>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader12_A8(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<12>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader13_A8(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<13>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader14_A8(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<14>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsFragmentShader15_A8(SegmentsFragInput in [[stage_in]],
									constant float4  *pEdgemap [[buffer(3)]])
{
    return segFragShaderCore_A8<15>(in,pEdgemap) * in.color.a;
};

fragment float4 segmentsTintmapFragmentShader1_A8(SegmentsFragInput in [[stage_in]],
												  constant float4  *pColor [[buffer(0)]],
												  constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<1>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader2_A8(SegmentsFragInput in [[stage_in]],
												  constant float4  *pColor [[buffer(0)]],
												  constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<2>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader3_A8(SegmentsFragInput in [[stage_in]],
												  constant float4  *pColor [[buffer(0)]],
												  constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<3>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader4_A8(SegmentsFragInput in [[stage_in]],
												  constant float4  *pColor [[buffer(0)]],
												  constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<4>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader5_A8(SegmentsFragInput in [[stage_in]],
												  constant float4  *pColor [[buffer(0)]],
												  constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<5>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader6_A8(SegmentsFragInput in [[stage_in]],
												  constant float4  *pColor [[buffer(0)]],
												  constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<6>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader7_A8(SegmentsFragInput in [[stage_in]],
												  constant float4  *pColor [[buffer(0)]],
												  constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<7>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader8_A8(SegmentsFragInput in [[stage_in]],
												  constant float4  *pColor [[buffer(0)]],
												  constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<8>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader9_A8(SegmentsFragInput in [[stage_in]],
												  constant float4  *pColor [[buffer(0)]],
												  constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<9>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader10_A8(SegmentsFragInput in [[stage_in]],
												   constant float4  *pColor [[buffer(0)]],
												   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<10>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader11_A8(SegmentsFragInput in [[stage_in]],
												   constant float4  *pColor [[buffer(0)]],
												   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<11>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader12_A8(SegmentsFragInput in [[stage_in]],
												   constant float4  *pColor [[buffer(0)]],
												   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<12>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader13_A8(SegmentsFragInput in [[stage_in]],
												   constant float4  *pColor [[buffer(0)]],
												   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<13>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader14_A8(SegmentsFragInput in [[stage_in]],
												   constant float4  *pColor [[buffer(0)]],
												   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<14>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

fragment float4 segmentsTintmapFragmentShader15_A8(SegmentsFragInput in [[stage_in]],
												   constant float4  *pColor [[buffer(0)]],
												   constant float4  *pEdgemap [[buffer(3)]])
{
	return segFragShaderCore_A8<15>(in,pEdgemap) * (evalTint(pColor, in.tintOfs, in.position.xy).a * in.color.a);
};

