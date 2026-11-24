// Put after the pixel shaders that SdlGpuBackend (WAX_SDLGPU) blends with a second color
// (WAX_DUAL_SOURCE), by ../sdlgpugfx/compile_shaders.py, which has their main renamed waxMain.
// SDL_GPU has no dual-source blending, so this main outputs one of the two colors of output.hlsl
// or both put together, as sdlOutput says:
//
//   1: The second color, for the first of two draws of Replace and Morph, which blend
//      color + canvas * (1 - second color). This one leaves canvas * (1 - second color), the
//      second adds the color, drawn with the shader without WAX_DUAL_SOURCE.
//   2: color + 1 - second color, which Multiply multiplies the canvas by in one draw:
//      canvas * color + canvas * (1 - second color).
//
// The second color is the same for all channels, so the first draw blends with the source color
// as its factor.

#undef main

cbuffer SdlOutputInfo : register(b3)
{
    uint sdlOutput;         // 1 or 2, see above.
};

struct SDL_PS_OUTPUT
{
    float4 color : SV_Target0;
};

SDL_PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT result = waxMain(input);

    SDL_PS_OUTPUT output;
    output.color = (sdlOutput == 1u) ? result.factor : result.color + 1.0f - result.factor;
    return output;
}
