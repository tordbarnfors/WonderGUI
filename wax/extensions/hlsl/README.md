# HLSL shaders

The shaders of DX12Backend (`../dx12gfx`) and VulkanBackend (`../vulkangfx`). Both backends draw
with the same shaders, so a change here changes both.

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

Both lists have to be kept in step when a shader is added.

## Rules that keep them working on both

- **Constants** come from `constants.hlsl`. DX12 has them as three cbuffers of root constants,
  Vulkan as one block of push constants, whose members get their usual names with `#define`s.
  So no local variable or parameter may be named like a constant (`flags`, `tintOfs`...).
- **Registers** become Vulkan bindings with the same number, in one descriptor set: `t0`-`t4`
  are bindings 0-4, and `s0` is binding 5 (compiled with `-fvk-s-shift 5 0`).
- **Vertex inputs** get their locations in the order they are declared, so they are always
  declared in the order `POSITION`, `COLOROFS`, `EXTRASOFS`, `TEXCOORD0`, leaving out the ones not
  needed from the end only.
- **Between vertex and pixel shader**, Vulkan matches by location, not by semantic as DX12 does,
  and locations also follow the order of declaration. So `PS_INPUT` declares the same members in
  the same order as the `VS_OUTPUT` of its vertex shader.
- **Dual-source blending** needs the second output as index 1 of location 0 in Vulkan, see
  `PS_OUTPUT` in `output.hlsl`.
- Code only for Vulkan goes inside `#ifdef __spirv__`, which DXC defines when it compiles to
  SPIR-V.

All shaders should compile with DXC both ways, which can be checked without Windows: as DXIL
(`dxc -T ps_6_0 ...`), the closest we get to DX12's `D3DCompile()`, and as SPIR-V.
