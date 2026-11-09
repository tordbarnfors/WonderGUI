// The constants all shaders share, set by the backend between draws. Put in front of every
// shader by the shader templates of DX12Backend and VulkanBackend, which both use these
// shaders.
//
// DX12Backend sets them as root constants: CanvasInfo (b0) with the canvas and blit source,
// PixelInfo (b1) with the tint and how the pixel shaders hand what they draw to the blender,
// see output.hlsl, and WarpInfo (b2) with where a Warpmap is drawn, see vs_warp.hlsl.
//
// VulkanBackend has them as one block of push constants, which holds all three in the same
// order: CanvasInfo at byte 0, PixelInfo at byte 32 and WarpInfo at byte 48. There can only be
// one block of push constants, so its members are given their usual names with defines.
//
// Changing them means changing both backends: DX12Backend::_createRootSignature() and the
// structs and offsets in VulkanBackend.

#ifdef __spirv__

struct WaxConstants
{
    // CanvasInfo

    float2 canvasScale;     // 2/canvasWidth, 2/canvasHeight, in pixels.
    float2 textureSize;     // Size of blit source, in pixels.
    uint flags;             // Bits depend on the shader, see the blit and blur shaders.
    uint blurOfs;           // Where the blur brush is in the extras buffer.
    uint edgemapEdges;      // Edges of the edgemap being drawn.
    uint edgemapPitch;      // Entries per column of the edgemap being drawn.

    // PixelInfo

    int tintOfs;            // Offset of the tint block in the color buffer, -1 for no tint. See tint.hlsl.
    uint outputMode;        // See output.hlsl.
    uint canvasKind;        // 0: canvas with alpha, 1: canvas without alpha, 2: alpha only canvas.
    float morphFactor;      // 1 unless the blend mode is Morph.

    // WarpInfo

    float2 warpDest;        // Position of the Warpmap on the canvas, in pixels.
    uint warpColorOfs;      // Offset into the color buffer, of the tint color.
    uint warpExtrasOfs;     // Offset into the extras buffer, of the warp.
    int2 warpRef;           // u and v of the first vertex, in spx.
};

[[vk::push_constant]] WaxConstants waxConstants;

#define canvasScale     waxConstants.canvasScale
#define textureSize     waxConstants.textureSize
#define flags           waxConstants.flags
#define blurOfs         waxConstants.blurOfs
#define edgemapEdges    waxConstants.edgemapEdges
#define edgemapPitch    waxConstants.edgemapPitch
#define tintOfs         waxConstants.tintOfs
#define outputMode      waxConstants.outputMode
#define canvasKind      waxConstants.canvasKind
#define morphFactor     waxConstants.morphFactor
#define warpDest        waxConstants.warpDest
#define warpColorOfs    waxConstants.warpColorOfs
#define warpExtrasOfs   waxConstants.warpExtrasOfs
#define warpRef         waxConstants.warpRef

#else

cbuffer CanvasInfo : register(b0)
{
    float2 canvasScale;     // 2/canvasWidth, 2/canvasHeight, in pixels.
    float2 textureSize;     // Size of blit source, in pixels.
    uint flags;             // Bits depend on the shader, see the blit and blur shaders.
    uint blurOfs;           // Where the blur brush is in the extras buffer.
    uint edgemapEdges;      // Edges of the edgemap being drawn.
    uint edgemapPitch;      // Entries per column of the edgemap being drawn.
};

cbuffer PixelInfo : register(b1)
{
    int tintOfs;            // Offset of the tint block in the color buffer, -1 for no tint. See tint.hlsl.
    uint outputMode;        // See output.hlsl.
    uint canvasKind;        // 0: canvas with alpha, 1: canvas without alpha, 2: alpha only canvas.
    float morphFactor;      // 1 unless the blend mode is Morph.
};

cbuffer WarpInfo : register(b2)
{
    float2 warpDest;        // Position of the Warpmap on the canvas, in pixels.
    uint warpColorOfs;      // Offset into the color buffer, of the tint color.
    uint warpExtrasOfs;     // Offset into the extras buffer, of the warp.
    int2 warpRef;           // u and v of the first vertex, in spx.
};

#endif
