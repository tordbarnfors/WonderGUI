# HLSL shaders

The shaders of DX12Backend (`../dx12gfx`), GlBackend (`../openglgfx`), MetalBackend
(`../metalgfx`) and VulkanBackend (`../vulkangfx`). All four draw with the same shaders, so a
change here changes them all.

## How they are put together

Each shader is a few of these files put one after the other, listed in the shader templates of
the backends:

- `constants.hlsl` first, for all but the mipmap shaders: the constants set between draws.
- `output.hlsl` and `tint.hlsl` next, for the pixel shaders that draw into canvases.
- `warp.hlsl` before the warp pixel shaders.
- Then the `vs_*.hlsl` or `ps_*.hlsl` file itself.

DX12Backend compiles them when it starts. `dx12gfx/wax_dx12shaders.cpp.template` lists them, and
`scripts/insert_quoted.rb` turns it into `wax_dx12shaders.cpp`, which is checked in:

    cd wax/extensions/dx12gfx
    ruby ../../../scripts/insert_quoted.rb wax_dx12shaders.cpp.template

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

The list of DX12Backend and that of VulkanBackend have to be kept in step when a shader is added.

## Rules that keep them working on all four

- **Constants** come from `constants.hlsl`. DX12 has them as three cbuffers of root constants,
  Vulkan and Metal as one block of push constants, whose members get their usual names with
  `#define`s.
  So no local variable or parameter may be named like a constant (`flags`, `tintOfs`...).
- **Registers** become Vulkan bindings with the same number, in one descriptor set: `t0`-`t4`
  are bindings 0-4, and `s0` is binding 5 (compiled with `-fvk-s-shift 5 0`). For Metal, `t0`-`t4`
  are shifted by one (`-fvk-t-shift 1 0`), which makes them buffers 1-5 and the blit source
  texture 3, with the constants in buffer 0 and `s0` sampler 0.
- **Vertex inputs** get their locations in the order they are declared, so they are always
  declared in the order `POSITION`, `COLOROFS`, `EXTRASOFS`, `TEXCOORD0`, leaving out the ones not
  needed from the end only.
- **Between vertex and pixel shader**, Vulkan matches by location, not by semantic as DX12 does,
  and locations also follow the order of declaration. So `PS_INPUT` declares the same members in
  the same order as the `VS_OUTPUT` of its vertex shader.
- **Dual-source blending** needs the second output as index 1 of location 0 in Vulkan, see
  `PS_OUTPUT` in `output.hlsl`.
- Code only for Vulkan, Metal and GL goes inside `#ifdef __spirv__`, which DXC defines when it
  compiles to SPIR-V, and code only for GL inside `#ifdef WAX_GLSL`.
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
  the rounded boxes were a chain of branches each waiting for a read: 1.5-1.7 times as slow on Intel
  (Mesa). So nothing may rely on the second operand not being evaluated, which FXC never allowed
  anyway.

All shaders should compile with DXC both ways, which can be checked without Windows: as DXIL
(`dxc -T ps_6_0 ...`), the closest we get to DX12's `D3DCompile()`, and as SPIR-V.
