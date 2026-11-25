# HLSL shaders

The shaders of DX12Backend (`../dx12gfx`), GlBackend (`../openglgfx`), MetalBackend
(`../metalgfx`), VulkanBackend (`../vulkangfx`) and SdlGpuBackend (`../sdlgpugfx`). All five draw
with the same shaders, so a change here changes them all.

## How they are put together

Each shader is a few of these files put one after the other, listed in
`vulkangfx/compile_shaders.py`, which the scripts of the other backends take their list from:

- `constants.hlsl` first, for all but the mipmap shaders: the constants set between draws.
- `output.hlsl` and `tint.hlsl` next, for the pixel shaders that draw into canvases.
- `warp.hlsl` before the warp pixel shaders.
- `shape.hlsl` before the shape pixel shaders, one for each kind of shape.
- Then the `vs_*.hlsl` or `ps_*.hlsl` file itself.

DX12Backend gets them as bytecode, compiled by FXC of the Windows SDK into
`dx12gfx/wax_dx12shaders.cpp`, which is checked in. `dx12gfx/compile_shaders.py` makes it, and
finds FXC where the SDK is usually installed:

    python wax/extensions/dx12gfx/compile_shaders.py

FXC only runs on Windows. After changing a shader elsewhere, run the other three and leave this
one for someone on Windows: `compile_shaders.py --check` tells if `wax_dx12shaders.cpp` is made
from the shaders as they are, which CMake also warns about when configuring a DX12 build.

VulkanBackend gets them as SPIR-V, compiled by DXC into `vulkangfx/spirv`, which is checked in.
`vulkangfx/compile_shaders.py` lists them and compiles them all:

    python3 wax/extensions/vulkangfx/compile_shaders.py --dxc path/to/dxc

MetalBackend gets them as MSL, which `metalgfx/compile_shaders.py` makes from them with DXC and
SPIRV-Cross, using the list of `vulkangfx/compile_shaders.py`, into `metalgfx/wax_metalshaders.mm`,
which is checked in:

    python3 wax/extensions/metalgfx/compile_shaders.py --dxc path/to/dxc

GlBackend gets them as GLSL 3.3, which `openglgfx/compile_shaders.py` makes the same way, fixing
up what GLSL 3.3 lacks (see the top of the script), into `openglgfx/wax_glshaders.cpp`, which is
checked in. It checks the GLSL with glslangValidator if there is one:

    python3 wax/extensions/openglgfx/compile_shaders.py --dxc path/to/dxc

SdlGpuBackend gets them as SPIR-V, DXIL and MSL, one of which SDL_GPU takes depending on its
device, compiled by DXC (and SPIRV-Cross for MSL) into `sdlgpugfx/shaders`, which are checked in,
together with what each binds in `sdlgpugfx/wax_sdlgpushaders.cpp`. They are compiled with
`WAX_SDLGPU`, the pixel shaders with `sdlgpu.hlsl` in front of the shader itself, and the Dual
ones with `sdlgpu_dual.hlsl` after it, as SDL_GPU has no push constants, swizzles, border colors,
samplers of integer textures or dual-source blending, see those files.
`sdlgpugfx/compile_shaders.py` moves the registers to the slots SDL_GPU binds by:

    python3 wax/extensions/sdlgpugfx/compile_shaders.py --dxc path/to/dxc

DXC has to find libdxil (dxil.dll on Windows) next to it to sign the DXIL, which D3D12 needs.
The script stops if it is left unsigned.

A new shader is added to the list in `vulkangfx/compile_shaders.py`, and to
`GpuBackend::_vertexShaderName()` or `_pixelShaderName()` for the kind of pipeline it draws.

## Rules that keep them working on all five

- **Constants** come from `constants.hlsl`. DX12 has them as three cbuffers of root constants,
  Vulkan and Metal as one block of push constants, whose members get their usual names with
  `#define`s.
  So no local variable or parameter may be named like a constant (`flags`, `tintOfs`...).
- **Registers** become Vulkan bindings with the same number, in one descriptor set: `t0`-`t4`
  are bindings 0-4, and `s0` is binding 5 (compiled with `-fvk-s-shift 5 0`). For Metal, `t0`-`t4`
  are shifted by one (`-fvk-t-shift 1 0`), which makes them buffers 1-5 and the blit source
  texture 3, with the constants in buffer 0 and `s0` sampler 0. SdlGpuBackend binds by slots
  that `sdlgpugfx/compile_shaders.py` moves the registers to, and a storage buffer of a new
  register needs a name there and in `SdlGpuShaders::Resource`.
- **Vertex inputs** get their locations in the order they are declared, so they are always
  declared in the order `POSITION`, `COLOROFS`, `EXTRASOFS`, leaving out the ones not needed from
  the end only. A vertex shader declares only the inputs it reads: D3D12 takes no pipeline whose
  layout lacks an input the shader declares, and the backends give each kind of pipeline the inputs
  of `GpuBackend::_nbVertexAttributes()`.
- **Between vertex and pixel shader**, Vulkan matches by location, not by semantic as DX12 does,
  and locations also follow the order of declaration. So `PS_INPUT` declares the same members in
  the same order as the `VS_OUTPUT` of its vertex shader.
- **Dual-source blending** needs the second output as index 1 of location 0 in Vulkan, see
  `PS_OUTPUT` in `output.hlsl`.
- Code only for Vulkan, Metal and GL goes inside `#ifdef __spirv__`, which DXC defines when it
  compiles to SPIR-V, code only for GL inside `#ifdef WAX_GLSL` and code only for SdlGpuBackend
  inside `#ifdef WAX_SDLGPU`. SdlGpuBackend's SPIR-V is compiled with `__spirv__` too, so a
  `__spirv__` path that isn't for it checks `!defined(WAX_SDLGPU)`, as in `constants.hlsl`.
- **Functions return once, at their end.** FXC can take an early return as leaving the result
  uninitialized, and in the generated GLSL an early return turns into a `do { } while (false)`
  around the inlined function, a construct old drivers handle badly. The AMD driver of macOS
  crashed on the shape shader when it was full of them, together with a `continue` at the end
  of every loop (which `openglgfx/compile_shaders.py` now removes).
- **Storage buffers** must be `StructuredBuffer`s of `float4`, `int4` or `uint4` read by index for
  GL, which has them as buffer textures. Others need a `WAX_GLSL` path, as the vertices in
  `vs_warp.hlsl` have.

- **No short-circuiting.** DXC compiles them as HLSL 2018 (`-HV 2018`), where `&&`, `||` and
  `?:` evaluate both sides, as FXC does for DX12Backend. HLSL 2021, the default of DXC, makes each
  of them a branch, and drivers don't flatten a branch that reads a buffer, so the corner tests of
  the rounded boxes were a chain of branches each waiting for a read, which made them 1.5-1.7
  times as slow on Intel (Mesa). So nothing may rely on the second operand not being evaluated,
  which FXC never allowed anyway.

All shaders should compile with DXC both ways, which can be checked without Windows: as DXIL
(`dxc -T ps_6_0 ...`), the closest we get to FXC, and as SPIR-V.
