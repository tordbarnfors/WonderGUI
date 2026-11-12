#!/usr/bin/env python3
#
# Compiles the HLSL shaders in ../hlsl to SPIR-V for VulkanBackend, into spirv/<name>.spv, which
# are checked in, so building Wax doesn't need a shader compiler. CMakeLists.txt turns them into
# arrays in wax_vulkanshaders.cpp in the build directory, see wax_vulkanshaders.h for their names.
# Run it after changing a shader, with DXC (https://github.com/microsoft/DirectXShaderCompiler)
# in the path or given with --dxc:
#
#     python3 compile_shaders.py [--dxc path/to/dxc]
#
# Each shader is a few files of ../hlsl put one after the other, as DX12Backend has them, see
# ../hlsl/README.md. Pixel shaders are compiled twice, the second time with WAX_DUAL_SOURCE for
# pipelines that blend with a second color.
#
# Options:
#
#   -HV 2018            No short-circuiting of &&, || and ?:, as with FXC, see ../hlsl/README.md.
#   -fvk-s-shift 5 0    Samplers (s0) after the textures and buffers (t0-t4) in the descriptor set.
#   -fvk-invert-y       Vertex shaders: Vulkan's clip space has y pointing down, D3D's up.

import argparse
import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
HLSL = os.path.join(HERE, '..', 'hlsl')
OUTPUT = os.path.join(HERE, 'spirv')

VERTEX_SHADERS = [
	('fillVS',		['constants.hlsl', 'vs_fill.hlsl']),
	('fillAAVS',	['constants.hlsl', 'vs_fill_aa.hlsl']),
	('blitVS',		['constants.hlsl', 'vs_blit.hlsl']),
	('lineVS',		['constants.hlsl', 'vs_line.hlsl']),
	('segmentsVS',	['constants.hlsl', 'vs_segments.hlsl']),
	('shapeVS',		['constants.hlsl', 'vs_shape.hlsl']),
	('warpVS',		['constants.hlsl', 'vs_warp.hlsl']),
]

PIXEL = ['constants.hlsl', 'output.hlsl', 'tint.hlsl']

PIXEL_SHADERS = [
	('fillPS',				PIXEL + ['ps_fill.hlsl']),
	('fillAAPS',			PIXEL + ['ps_fill_aa.hlsl']),
	('blitPS',				PIXEL + ['ps_blit.hlsl']),
	('blurPS',				PIXEL + ['ps_blur.hlsl']),
	('paletteBlitPS',		PIXEL + ['ps_blit_palette.hlsl']),
	('paletteBlurPS',		PIXEL + ['ps_blur_palette.hlsl']),
	('linePS',				PIXEL + ['ps_line.hlsl']),
	('segmentsPS',			PIXEL + ['ps_segments.hlsl']),
	('shapeBoxPS',			PIXEL + ['shape.hlsl', 'ps_shape_box.hlsl']),
	('shapeRingPS',			PIXEL + ['shape.hlsl', 'ps_shape_ring.hlsl']),
	('shapeLinePS',			PIXEL + ['shape.hlsl', 'ps_shape_line.hlsl']),
	('warpEdgemapPS',		PIXEL + ['warp.hlsl', 'ps_warp_edgemap.hlsl']),
	('warpBlitPS',			PIXEL + ['warp.hlsl', 'ps_warp_blit.hlsl']),
	('warpPaletteBlitPS',	PIXEL + ['warp.hlsl', 'ps_warp_blit_palette.hlsl']),
]

COMMON_OPTIONS = ['-nologo', '-HV', '2018', '-spirv', '-fspv-target-env=vulkan1.3', '-fvk-s-shift', '5', '0', '-O3', '-E', 'main']


def compile_shader(dxc, tmpdir, name, files, profile, extra):
	source = os.path.join(tmpdir, name + '.hlsl')
	output = os.path.join(tmpdir, name + '.spv')

	with open(source, 'w') as f:
		for part in files:
			with open(os.path.join(HLSL, part)) as p:
				f.write('#line 1 "%s"\n' % part)
				f.write(p.read())
				f.write('\n')

	cmd = [dxc] + COMMON_OPTIONS + ['-T', profile] + extra + [source, '-Fo', output]
	result = subprocess.run(cmd, capture_output=True, text=True)
	if result.returncode != 0:
		sys.stderr.write('Compiling %s failed:\n%s%s' % (name, result.stdout, result.stderr))
		sys.exit(1)

	with open(output, 'rb') as f:
		data = f.read()

	assert len(data) % 4 == 0
	return data


def main():
	parser = argparse.ArgumentParser(description='Compiles the HLSL shaders of VulkanBackend to SPIR-V.')
	parser.add_argument('--dxc', default=shutil.which('dxc') or 'dxc', help='DXC executable')
	args = parser.parse_args()

	shaders = []

	with tempfile.TemporaryDirectory() as tmpdir:
		for name, files in VERTEX_SHADERS:
			shaders.append((name, compile_shader(args.dxc, tmpdir, name, files, 'vs_6_0', ['-fvk-invert-y'])))

		for name, files in PIXEL_SHADERS:
			shaders.append((name, compile_shader(args.dxc, tmpdir, name, files, 'ps_6_0', [])))
			shaders.append((name + 'Dual', compile_shader(args.dxc, tmpdir, name + 'Dual', files, 'ps_6_0', ['-D', 'WAX_DUAL_SOURCE=1'])))

	os.makedirs(OUTPUT, exist_ok=True)

	for name, data in shaders:
		with open(os.path.join(OUTPUT, name + '.spv'), 'wb') as f:
			f.write(data)

	print('Wrote %d shaders to %s' % (len(shaders), OUTPUT))


if __name__ == '__main__':
	main()
