
#include <metal_stdlib>
using namespace metal; 


//____ Vertex ___________________________________________________

typedef struct
{
    vector_int2     coord;
    int             colorOfs;              // Offset into color buffer.
    int             extrasOfs;              // Offset into extras buffer.
    vector_float2   uv;
    vector_float2   tintmapOfs;
    vector_float2   colorstripOfs;
} Vertex;

//____ Uniform ______________________________________________

typedef struct             // Uniform buffer object for canvas information.
{                               // DO NOT CHANGE ORDER OF MEMBERS!!!
    vector_float2   canvasDim;
    int        canvasYOfs;
    int        canvasYMul;

    vector_float4 flatTint;

    vector_int2     tintRectPos;
    vector_int2     tintRectSize;

    vector_float4   topLeftTint;
    vector_float4   topRightTint;
    vector_float4   bottomRightTint;
    vector_float4   bottomLeftTint;

    vector_int2     texSize;
} Uniform;


//____ BlurUniform ______________________________________________

typedef struct             // Uniform buffer object for fragment shader blur information.
{                               // DO NOT CHANGE ORDER OF MEMBERS!!!
    vector_float4     colorMtx[9];
    vector_float2     offset[9];
} BlurUniform;


//____ PlotFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float pointSize [[point_size]];
    float4 color;
} PlotFragInput;

//____ LineFromToFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float s;
    float w;
    float slope;
    float ifSteep;
    float ifMild;

} LineFromToFragInput;

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
    float2 tintmapUV;
} FillTintmapFragInput;

//____ FillAAFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float4 rect;
} FillAAFragInput;


//____ BlitFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float2 texUV;
} BlitFragInput;

//____ PaletteBlitInterpolateFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float2 texUV00;
    float2 texUV11;
    float2 uvFrac;
 } PaletteBlitInterpolateFragInput;

//____ SegmentsFragInput ______________________________________________

typedef struct 
{
    float4 position [[position]];
    float4 color;
    float2 texUV;
    int segments;
    int stripesOfs;
    float2 paletteOfs;
} SegmentsFragInput;



//____ plotVertexShader() ____________________________________________

vertex PlotFragInput
plotVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pExtras [[buffer(1)]],
             constant Uniform * pUniform[[buffer(2)]])
{
    PlotFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;


    int     eOfs = pVertices[vertexID].extrasOfs;
    out.color = pUniform->flatTint * pExtras[eOfs];

    out.pointSize = 1.f;

    return out;
}

//____ plotFragmentShader() ____________________________________________

fragment float4 plotFragmentShader(PlotFragInput in [[stage_in]])
{
    return in.color;
};

//____ plotFragmentShader_A8() ____________________________________________

fragment float4 plotFragmentShader_A8(PlotFragInput in [[stage_in]])
{
    return { in.color.a, 0.0, 0.0, 0.0 };
};


//____ lineFromToVertexShader() ____________________________________________

vertex LineFromToFragInput
lineFromToVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pColor [[buffer(1)]],
             constant vector_float4  *pExtras [[buffer(2)]],
             constant Uniform * pUniform[[buffer(3)]])
{
    LineFromToFragInput out;

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

//____ lineFromToFragmentShader() ____________________________________________

fragment float4 lineFromToFragmentShader(LineFromToFragInput in [[stage_in]])
{
   float4 outColor;

   outColor.rgb = in.color.rgb;
   outColor.a = in.color.a * clamp(in.w - abs(in.position.x*in.ifSteep + in.position.y*in.ifMild - in.s - (in.position.x*in.ifMild + in.position.y*in.ifSteep) * in.slope), 0.0, 1.0);

   return outColor;
};

//____ lineFromToFragmentShader_A8() ____________________________________________

fragment float4 lineFromToFragmentShader_A8(LineFromToFragInput in [[stage_in]])
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
    out.position.x = pos.x*2 / 512.0 - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / 512.0 - 1.0;


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
    out.color = pColor[colOfs];
    out.tintmapUV = pVertices[vertexID].tintmapOfs;

    return out;
}

//____ fillTintmapFragmentShader() ____________________________________________

fragment float4 fillTintmapFragmentShader(FillTintmapFragInput in [[stage_in]],
                                    constant float4  *pColor [[buffer(0)]])
{
    float4 colorFromColorstripX = pColor[(int)in.tintmapUV.x];
    float4 colorFromColorstripY = pColor[(int)in.tintmapUV.y];

    return colorFromColorstripX * colorFromColorstripY * in.color;
};

//____ fillTintmapFragmentShader_A8() ____________________________________________

fragment float4 fillTintmapFragmentShader_A8(FillTintmapFragInput in [[stage_in]],
                                    constant float4  *pColor [[buffer(0)]])
{
    float alphaFromColorstripX = pColor[(int)in.tintmapUV.x].a;
    float alphaFromColorstripY = pColor[(int)in.tintmapUV.y].a;

    return {alphaFromColorstripX * alphaFromColorstripY * in.color.a,0.0,0.0,0.0};
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


//____ fillGradientAAVertexShader() ____________________________________________

vertex FillAAFragInput
fillGradientAAVertexShader(uint vertexID [[vertex_id]],
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

    float2   tintOfs = (pos - float2(pUniform->tintRectPos)) / float2(pUniform->tintRectSize);
    float4   lineStartTint = pUniform->topLeftTint + (pUniform->bottomLeftTint - pUniform->topLeftTint) * tintOfs.y;
    float4   lineEndTint = pUniform->topRightTint + (pUniform->bottomRightTint - pUniform->topRightTint) * tintOfs.y;
    float4   gradientTint = lineStartTint + (lineEndTint - lineStartTint) * tintOfs.x;

    int     eOfs = pVertices[vertexID].extrasOfs;
    out.color = pUniform->flatTint * gradientTint * pExtras[eOfs];

    out.rect = pExtras[eOfs+1];
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

//____ blitGradientVertexShader() _______________________________________________


vertex BlitFragInput
blitGradientVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pExtras [[buffer(1)]],
             constant Uniform * pUniform[[buffer(2)]])
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

    float2   tintOfs = (pos - float2(pUniform->tintRectPos)) / float2(pUniform->tintRectSize);
    float4   lineStartTint = pUniform->topLeftTint + (pUniform->bottomLeftTint - pUniform->topLeftTint) * tintOfs.y;
    float4   lineEndTint = pUniform->topRightTint + (pUniform->bottomRightTint - pUniform->topRightTint) * tintOfs.y;
    float4   gradientTint = lineStartTint + (lineEndTint - lineStartTint) * tintOfs.x;

    out.color = pUniform->flatTint * gradientTint;

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


//____ paletteBlitInterpolateVertexShader() _______________________________________________

vertex PaletteBlitInterpolateFragInput
paletteBlitInterpolateVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pExtras [[buffer(1)]],
             constant Uniform * pUniform[[buffer(2)]])
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

//____ paletteBlitInterpolateGradientVertexShader() _______________________________________________

vertex PaletteBlitInterpolateFragInput
paletteBlitInterpolateGradientVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pExtras [[buffer(1)]],
             constant Uniform * pUniform[[buffer(2)]])
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

    float2   tintOfs = (pos - float2(pUniform->tintRectPos)) / float2(pUniform->tintRectSize);
    float4   lineStartTint = pUniform->topLeftTint + (pUniform->bottomLeftTint - pUniform->topLeftTint) * tintOfs.y;
    float4   lineEndTint = pUniform->topRightTint + (pUniform->bottomRightTint - pUniform->topRightTint) * tintOfs.y;
    float4   gradientTint = lineStartTint + (lineEndTint - lineStartTint) * tintOfs.x;

    out.color = pUniform->flatTint * gradientTint;

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

//____ blurFragmentShader() ____________________________________________

fragment float4 blurFragmentShader(BlitFragInput in [[stage_in]],
                                    texture2d<float> colorTexture [[ texture(0) ]],
                                    sampler textureSampler [[ sampler(0) ]],
                                    constant BlurUniform  *pBlurInfo [[buffer(1)]])
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

//____ segmentsVertexShader() _______________________________________________

vertex SegmentsFragInput
segmentsVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pExtras [[buffer(1)]],
             constant Uniform * pUniform[[buffer(2)]])
{
    SegmentsFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;


    int     eOfs = pVertices[vertexID].extrasOfs;

    vector_float4 extras = pExtras[eOfs];
    vector_float4 extras2 = pExtras[eOfs+1];

    out.segments = int(extras.x);
    out.stripesOfs = int(extras.y);
 
    float2 uv = pVertices[vertexID].uv;
    out.texUV = uv;

    float xTintOfs = uv.x/extras.z*1/extras2.z;
    float yTintOfs = uv.y/extras.w*1/extras2.w;

    out.paletteOfs = { extras2.x+xTintOfs, extras2.y+yTintOfs };
    out.color = pUniform->flatTint;

    return out;
}

//____ segmentsGradientVertexShader() _______________________________________________

vertex SegmentsFragInput
segmentsGradientVertexShader(uint vertexID [[vertex_id]],
             constant Vertex *pVertices [[buffer(0)]],
             constant vector_float4  *pExtras [[buffer(1)]],
             constant Uniform * pUniform[[buffer(2)]])
{
    SegmentsFragInput out;

    float2 pos = (vector_float2) pVertices[vertexID].coord.xy;

    vector_float2 canvasSize = pUniform->canvasDim;
    
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = pos.x*2 / canvasSize.x - 1.0;
    out.position.y = (pUniform->canvasYOfs + pUniform->canvasYMul*pos.y)*2 / canvasSize.y - 1.0;


    int     eOfs = pVertices[vertexID].extrasOfs;

    vector_float4 extras = pExtras[eOfs];
    vector_float4 extras2 = pExtras[eOfs+1];

    out.segments = int(extras.x);
    out.stripesOfs = int(extras.y);
 
    float2 uv = pVertices[vertexID].uv;
    out.texUV = uv;

    float xTintOfs = uv.x/extras.z*1/extras2.z;
    float yTintOfs = uv.y/extras.w*1/extras2.w;

    out.paletteOfs = { extras2.x+xTintOfs, extras2.y+yTintOfs };
 
    float2   tintOfs = (pos - float2(pUniform->tintRectPos)) / float2(pUniform->tintRectSize);
    float4   lineStartTint = pUniform->topLeftTint + (pUniform->bottomLeftTint - pUniform->topLeftTint) * tintOfs.y;
    float4   lineEndTint = pUniform->topRightTint + (pUniform->bottomRightTint - pUniform->topRightTint) * tintOfs.y;
    float4   gradientTint = lineStartTint + (lineEndTint - lineStartTint) * tintOfs.x;

    out.color = pUniform->flatTint * gradientTint;

    return out;
}


//____ segmentsFragmentShader() ____________________________________________

template <int EDGES>
inline float4 segFragShaderCore(SegmentsFragInput in,
                                    constant float4  *pEdgeStripes,
                                    texture2d<half> paletteTexture)
{

    constexpr sampler textureSampler (mag_filter::linear,
                                      min_filter::linear);

    float totalAlpha = 0.f;
    float3    rgbAcc = float3(0,0,0);

    float factor = 1.f;
    float2 palOfs = in.paletteOfs;
    for( int i = 0 ; i < EDGES ; i++ )
    {
        float4 col = (float4) paletteTexture.sample(textureSampler, palOfs);
        palOfs.x += 1/(float) $MAXSEG;

        float4 edge = pEdgeStripes[in.stripesOfs + int(in.texUV.x)*(in.segments-1)+i];

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

    float4 col = (float4) paletteTexture.sample(textureSampler, palOfs);
    float useFactor = factor*col.a;
    totalAlpha += useFactor;
    rgbAcc += col.rgb * useFactor;

    float4 out;
    out.a = totalAlpha * in.color.a;
    out.rgb = (rgbAcc/totalAlpha) * in.color.rgb;
    return out;
};


fragment float4 segmentsFragmentShader1(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<1>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader2(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<2>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader3(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<3>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader4(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<4>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader5(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<5>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader6(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<6>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader7(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<7>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<8>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader9(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<9>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader10(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<10>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader11(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<11>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader12(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<12>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader13(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<13>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader14(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<14>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader15(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore<15>(in,pEdgeStripes,paletteTexture);
};

//____ segmentsFragmentShader_A8() ____________________________________________

template <int EDGES>
inline float4 segFragShaderCore_A8(SegmentsFragInput in,
                                    constant float4  *pEdgeStripes,
                                    texture2d<half> paletteTexture)
{

    constexpr sampler textureSampler (mag_filter::linear,
                                      min_filter::linear);

    float totalAlpha = 0.f;

    float factor = 1.f;
    float2 palOfs = in.paletteOfs;
    for( int i = 0 ; i < EDGES ; i++ )
    {
        float alpha = (float) paletteTexture.sample(textureSampler, palOfs).a;
        palOfs.x += 1/$MAXSEG.f;

        float4 edge = pEdgeStripes[in.stripesOfs + int(in.texUV.x)*(in.segments-1)+i];

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

    float alpha = (float) paletteTexture.sample(textureSampler, palOfs).a;
    totalAlpha += factor*alpha;

    return { totalAlpha * in.color.a, 0.f, 0.f, 0.f };
};


fragment float4 segmentsFragmentShader1_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<1>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader2_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<2>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader3_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<3>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader4_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<4>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader5_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<5>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader6_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<6>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader7_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<7>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader8_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<8>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader9_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<9>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader10_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<10>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader11_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<11>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader12_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<12>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader13_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<13>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader14_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<14>(in,pEdgeStripes,paletteTexture);
};

fragment float4 segmentsFragmentShader15_A8(SegmentsFragInput in [[stage_in]],
                                    constant float4  *pEdgeStripes [[buffer(0)]],
                                    texture2d<half> paletteTexture [[ texture(2) ]])
{
    return segFragShaderCore_A8<15>(in,pEdgeStripes,paletteTexture);
};
