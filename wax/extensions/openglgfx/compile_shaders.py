#!/usr/bin/env python3
#
# Turns the HLSL shaders in ../hlsl into GLSL 3.3 for GlBackend, in wax_glshaders.cpp, which is
# checked in, so building Wax doesn't need a shader compiler. DXC compiles each shader to SPIR-V,
# SPIRV-Cross (https://github.com/KhronosGroup/SPIRV-Cross) turns that into GLSL and this script
# fixes up what GLSL 3.3 doesn't have or does differently. Run it after changing a shader, with DXC
# and spirv-cross in the path or given with --dxc and --spirv-cross:
#
#     python3 compile_shaders.py [--dxc path/to/dxc] [--spirv-cross path/to/spirv-cross]
#
# The shaders are those of VulkanBackend, listed in ../vulkangfx/compile_shaders.py. Pixel shaders
# are compiled twice, the second time with WAX_DUAL_SOURCE for pipelines that blend with a second
# color. All are compiled with WAX_GLSL defined, see vs_warp.hlsl.
#
# What is fixed up, see GlBackend for the other side of it:
#
#   Buffers         GL 3.3 has no storage buffers. StructuredBuffer<float4> registers t0, t1 and
#                   t3 become buffer textures (samplerBuffer) named waxBuffer0, waxBuffer1 and
#                   waxBuffer3, read with texelFetch().
#   Blit source     The texture and sampler of t2 and s0 become a sampler named waxBlitSource, and
#                   waxBlitSource1 if there is another one only for textureSize().
#   Constants       The push constants of constants.hlsl, with glCanvas added at their end (how the
#                   canvas is flipped, see below), become one array of uniforms, uvec4
#                   waxConstants[6], with each member read from where std140 would have put it,
#                   floats through uintBitsToFloat(). GlBackend sets them per draw with one
#                   glUniform4uiv(), which unlike a uniform buffer per draw costs the same on all
#                   drivers. Integers go in as their bits, untouched.
#   Varyings        GL 3.3 matches them by name, not location, so the outputs of the vertex shaders
#                   and the inputs of the pixel shaders get the same names, from their semantics.
#   Discard         SPIR-V for Vulkan 1.3 makes discard a demote, which GLSL 3.3 doesn't have, so the
#                   SPIR-V is for Vulkan 1.1, where it is a kill. The shaders take their
#                   derivatives before they can discard, see warp.hlsl, so it makes no difference.
#   Y               GL draws into surfaces with y pointing up, from the first row of the texture,
#                   and into the window with y pointing up from its bottom row. Surfaces keep their
#                   first row at the top, as on the other backends, which makes them upside down
#                   to GL: the vertex shaders flip y by glCanvas.x for them. In the window it is
#                   gl_FragCoord that is upside down to the shaders, so they get the position as
#                   glCanvas.y + glCanvas.z * gl_FragCoord.y instead, and dFdy() times glCanvas.z.
#   Loops           A continue at the end of a loop is removed, see remove_redundant_continues().
#
# SPIRV-Cross writes its usual GLSL, which old drivers are most likely to take. With
# --force-temporary it would read each buffer element once rather than wherever it is used, which
# llvmpipe runs rounded boxes 25% faster with, but it writes almost every loop as for (;;) with a
# break and the shaders get half as long again. That was the GLSL the AMD driver of macOS crashed
# on.

import argparse
import importlib.util
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
HLSL = os.path.join(HERE, '..', 'hlsl')
OUTPUT = os.path.join(HERE, 'wax_glshaders.cpp')

# The shader lists of VulkanBackend, so there is one list to keep in step with the HLSL.

spec = importlib.util.spec_from_file_location('vulkan_shaders', os.path.join(HERE, '..', 'vulkangfx', 'compile_shaders.py'))
vulkan_shaders = importlib.util.module_from_spec(spec)
spec.loader.exec_module(vulkan_shaders)

VERTEX_SHADERS = vulkan_shaders.VERTEX_SHADERS
PIXEL_SHADERS = vulkan_shaders.PIXEL_SHADERS

DXC_OPTIONS = ['-nologo', '-spirv', '-fspv-target-env=vulkan1.1', '-O3', '-E', 'main', '-D', 'WAX_GLSL=1']
SPIRV_CROSS_OPTIONS = ['--version', '330', '--no-es', '--glsl-emit-push-constant-as-ubo']

LICENSE_FROM = os.path.join(HERE, '..', 'vulkangfx', 'wax_vulkanshaders.h')


def run(cmd, what):
	result = subprocess.run(cmd, capture_output=True, text=True)
	if result.returncode != 0:
		sys.stderr.write('%s failed:\n%s%s' % (what, result.stdout, result.stderr))
		sys.exit(1)
	return result.stdout


def fail(name, what):
	sys.stderr.write('%s: %s\n' % (name, what))
	sys.exit(1)


# Replaces each NAME._m0[EXPR] with texelFetch(NEW, int(EXPR)), finding the bracket that closes
# EXPR, which can hold brackets and reads of its own.

def replace_buffer_reads(name, glsl, old, new):
	pattern = old + '._m0['
	out = []
	pos = 0
	while True:
		start = glsl.find(pattern, pos)
		if start < 0:
			break
		if start > 0 and (glsl[start - 1].isalnum() or glsl[start - 1] == '_'):
			out.append(glsl[pos:start + len(pattern)])
			pos = start + len(pattern)
			continue
		depth = 1
		i = start + len(pattern)
		while depth > 0:
			if i >= len(glsl):
				fail(name, 'unbalanced brackets reading ' + old)
			if glsl[i] == '[':
				depth += 1
			elif glsl[i] == ']':
				depth -= 1
			i += 1
		expr = glsl[start + len(pattern):i - 1]
		out.append(glsl[pos:start])
		out.append('texelFetch(%s, int(%s))' % (new, replace_buffer_reads(name, expr, old, new)))
		pos = i
	out.append(glsl[pos:])
	return ''.join(out)


# Removes each continue that only ends the body of its loop, which SPIRV-Cross puts at the end of
# most loops, often in an if whose else breaks. Such a continue is followed by nothing but the ends
# of ifs and elses up to the end of the loop. The AMD driver of macOS crashed in a function named
# ppil_handle_continue on the shape shader, which had dozens of them.

def remove_redundant_continues(glsl):
	match = {}
	stack = []
	for i, c in enumerate(glsl):
		if c == '{':
			stack.append(i)
		elif c == '}':
			open = stack.pop()
			match[i] = open
			match[open] = i

	def header(open):
		# The kind of statement the block opened at open belongs to.
		text = glsl[:open].rstrip()
		if text.endswith('else'):
			return 'else'
		if text.endswith('do'):
			return 'do'
		if text.endswith(')'):
			depth = 0
			for i in range(len(text) - 1, -1, -1):
				if text[i] == ')':
					depth += 1
				elif text[i] == '(':
					depth -= 1
					if depth == 0:
						m = re.search(r'(\w+)\s*$', text[:i])
						return m.group(1) if m else ''
		return ''

	def skip_space(pos):
		while pos < len(glsl) and glsl[pos].isspace():
			pos += 1
		return pos

	def ends_loop(pos):
		# True if only ends of ifs and elses come from pos up to the end of a loop.
		while True:
			pos = skip_space(pos)
			if pos >= len(glsl) or glsl[pos] != '}':
				return False
			kind = header(match[pos])
			if kind in ('for', 'while', 'do'):
				return True
			if kind not in ('if', 'else'):
				return False
			pos = skip_space(pos + 1)
			while glsl.startswith('else', pos) and not (glsl[pos + 4].isalnum() or glsl[pos + 4] == '_'):
				pos = skip_space(pos + 4)
				if glsl.startswith('if', pos):
					pos = glsl.index('{', pos)
				if glsl[pos] != '{':
					return False
				pos = skip_space(match[pos] + 1)

	removed = []
	for m in re.finditer(r'\n[ \t]*continue;', glsl):
		if ends_loop(m.end()):
			removed.append(m)

	for m in reversed(removed):
		glsl = glsl[:m.start()] + glsl[m.end():]

	return glsl


def fix_up(name, glsl, bVertex):
	# Storage buffers of float4, int4 or uint4 to buffer textures.

	buffers = re.findall(r'layout\((?:binding = \d+, )?std430\) readonly buffer \w+\n\{\n    (\w+) _m0\[\];\n\} (\w+);\n', glsl)
	bindings = re.findall(r'layout\((?:binding = (\d+), )?std430\) readonly buffer \w+\n\{\n    \w+ _m0\[\];\n\} \w+;\n', glsl)

	for (elementType, instance), binding in zip(buffers, bindings):
		samplerType = { 'vec4': 'samplerBuffer', 'ivec4': 'isamplerBuffer', 'uvec4': 'usamplerBuffer' }.get(elementType)
		if samplerType is None:
			fail(name, 'buffer %s of %s, only buffers of vectors of four can be buffer textures' % (instance, elementType))
		if binding == '':
			fail(name, 'buffer %s without binding' % instance)
		newName = 'waxBuffer' + binding
		glsl = re.sub(r'layout\((?:binding = \d+, )?std430\) readonly buffer \w+\n\{\n    \w+ _m0\[\];\n\} ' + instance + r';\n',
					  'uniform %s %s;\n' % (samplerType, newName), glsl)
		glsl = replace_buffer_reads(name, glsl, instance, newName)

	if 'buffer ' in glsl and 'std430' in glsl:
		fail(name, 'storage buffer left')

	# SPIRV-Cross puts bindings in the GLSL, which GL 3.3 can't have without
	# GL_ARB_shading_language_420pack. GlBackend sets them instead.

	glsl = re.sub(r'#ifdef GL_ARB_shading_language_420pack\n#extension GL_ARB_shading_language_420pack : require\n#endif\n', '', glsl)
	glsl = re.sub(r'layout\(binding = \d+\) ', '', glsl)
	glsl = re.sub(r'layout\(binding = \d+, ', 'layout(', glsl)

	if 'binding' in glsl or '420pack' in glsl:
		fail(name, 'binding left')

	# The blit source.

	# SPIRV-Cross makes a sampler for each sampler state the texture is used with, and one without
	# for textureSize(), so there may be more than one. GlBackend points them all at the same
	# texture unit.

	combined = []
	for m in re.finditer(r'\bSPIRV_Cross_Combined\w+', glsl):
		if m.group(0) not in combined:
			combined.append(m.group(0))
	for i, old in enumerate(combined):
		glsl = re.sub(r'\b' + old + r'\b', 'waxBlitSource' + (str(i) if i > 0 else ''), glsl)

	# Constants.

	glsl, n = re.subn(r'layout\(std140\) uniform type_PushConstant_WaxConstants\n\{\n((?:    .*\n)*?)\} waxConstants;',
					  lambda m: 'layout(std140) uniform WaxConstants\n{\n' + m.group(1) + '    vec4 glCanvas;\n} waxConstants;', glsl)
	if n != 1:
		fail(name, 'no constants block found')

	# Varyings named by their semantics.

	if bVertex:
		glsl = re.sub(r'\bout_var_(\w+)', r'wax_\1', glsl)
	else:
		glsl = re.sub(r'\bin_var_(\w+)', r'wax_\1', glsl)

	# Y, see the top of this file.

	if bVertex:
		end = glsl.rstrip().rfind('}')
		if glsl.rfind('return;') > glsl.find('void main()'):
			fail(name, 'return in main, y would not be flipped')
		glsl = glsl[:end] + '    gl_Position.y *= waxConstants.glCanvas.x;\n' + glsl[end:]
	else:
		helpers = []
		if 'gl_FragCoord' in glsl:
			glsl = re.sub(r'\bgl_FragCoord\b', 'waxFragCoord()', glsl)
			helpers.append('vec4 waxFragCoord() { return vec4(gl_FragCoord.x, waxConstants.glCanvas.y + waxConstants.glCanvas.z * gl_FragCoord.y, gl_FragCoord.zw); }\n')
		if re.search(r'\bdFdy\(', glsl):
			glsl = re.sub(r'\bdFdy\(', 'waxDFdy(', glsl)
			for t in ('float', 'vec2', 'vec3', 'vec4'):
				helpers.append('%s waxDFdy(%s v) { return dFdy(v) * waxConstants.glCanvas.z; }\n' % (t, t))
		if helpers:
			pos = glsl.find('} waxConstants;\n') + len('} waxConstants;\n')
			glsl = glsl[:pos] + '\n' + ''.join(helpers) + glsl[pos:]

	return constants_to_array(name, remove_redundant_continues(glsl))


# The uniform block of constants to uvec4 waxConstants[], see the top of this file.

STD140 = { 'float': (4, 4), 'uint': (4, 4), 'int': (4, 4), 'vec2': (8, 8), 'uvec2': (8, 8), 'ivec2': (8, 8), 'vec4': (16, 16) }

def constants_to_array(name, glsl):
	m = re.search(r'layout\(std140\) uniform WaxConstants\n\{\n((?:    .*\n)*?)\} waxConstants;\n', glsl)
	if not m:
		fail(name, 'no constants block found')

	offset = 0
	access = {}

	for line in m.group(1).splitlines():
		t, member = line.split()
		member = member.rstrip(';')
		size, align = STD140[t]
		offset = (offset + align - 1) // align * align
		vec, comp = offset // 16, (offset % 16) // 4
		element = 'waxConstants[%d]' % vec
		swizzle = 'xyzw'[comp:comp + size // 4]

		if t == 'vec4':
			access[member] = 'uintBitsToFloat(%s)' % element
		elif t in ('float', 'vec2'):
			access[member] = 'uintBitsToFloat(%s.%s)' % (element, swizzle)
		elif t in ('uint', 'uvec2'):
			access[member] = '%s.%s' % (element, swizzle)
		else:
			access[member] = '%s(%s.%s)' % (t, element, swizzle)

		offset += size

	nVectors = (offset + 15) // 16
	if nVectors != 6:
		fail(name, 'constants take %d vectors, GlBackend sets 6' % nVectors)

	glsl = glsl[:m.start()] + 'uniform uvec4 waxConstants[%d];\n' % nVectors + glsl[m.end():]

	for member, expr in access.items():
		glsl = re.sub(r'\bwaxConstants\.' + member + r'\b', '(' + expr + ')', glsl)

	if 'waxConstants.' in glsl:
		fail(name, 'constant left unconverted')

	return glsl


def compile_shader(args, tmpdir, name, files, profile, extra):
	source = os.path.join(tmpdir, name + '.hlsl')
	spirv = os.path.join(tmpdir, name + '.spv')
	stage = 'vert' if profile.startswith('vs') else 'frag'
	glslPath = os.path.join(tmpdir, name + '.' + stage)

	with open(source, 'w') as f:
		for part in files:
			with open(os.path.join(HLSL, part)) as p:
				f.write('#line 1 "%s"\n' % part)
				f.write(p.read())
				f.write('\n')

	run([args.dxc] + DXC_OPTIONS + ['-T', profile] + extra + [source, '-Fo', spirv], 'Compiling ' + name)
	glsl = run([args.spirv_cross] + SPIRV_CROSS_OPTIONS + [spirv], 'Cross-compiling ' + name)
	glsl = fix_up(name, glsl, stage == 'vert')

	# What GL gets must be GLSL 3.3, which glslangValidator can check, if there is one.

	if args.glslang:
		with open(glslPath, 'w') as f:
			f.write(glsl)
		run([args.glslang, glslPath], 'Validating ' + name)

	return glsl


def main():
	parser = argparse.ArgumentParser(description='Turns the HLSL shaders into GLSL 3.3 for GlBackend.')
	parser.add_argument('--dxc', default=shutil.which('dxc') or 'dxc', help='DXC executable')
	parser.add_argument('--spirv-cross', default=shutil.which('spirv-cross') or 'spirv-cross', help='SPIRV-Cross executable')
	parser.add_argument('--glslang', default=shutil.which('glslangValidator'), help='glslangValidator executable, checks the output if given or in the path')
	args = parser.parse_args()

	shaders = []

	with tempfile.TemporaryDirectory() as tmpdir:
		for name, files in VERTEX_SHADERS:
			shaders.append((name, compile_shader(args, tmpdir, name, files, 'vs_6_0', [])))

		for name, files in PIXEL_SHADERS:
			shaders.append((name, compile_shader(args, tmpdir, name, files, 'ps_6_0', [])))
			shaders.append((name + 'Dual', compile_shader(args, tmpdir, name + 'Dual', files, 'ps_6_0', ['-D', 'WAX_DUAL_SOURCE=1'])))

	with open(LICENSE_FROM) as f:
		license = ''.join(f.readlines()[:21])

	with open(OUTPUT, 'w', newline='\n') as f:
		f.write(license)
		f.write('\n// Generated by compile_shaders.py from the HLSL shaders in ../hlsl. Don\'t edit, change the HLSL\n')
		f.write('// and run compile_shaders.py again.\n\n')
		f.write('#include <wax_glbackend.h>\n\nnamespace wax\n{\n\nconst GlBackend::ShaderSource GlBackend::s_shaderSources[] =\n{\n')
		for name, glsl in shaders:
			assert ')glsl"' not in glsl
			f.write('\t{ "%s", R"glsl(%s)glsl" },\n\n' % (name, glsl.rstrip('\n') + '\n'))
		f.write('};\n\nconst int GlBackend::s_nbShaderSources = int(sizeof(s_shaderSources) / sizeof(s_shaderSources[0]));\n\n} // namespace wax\n')

	print('Wrote %d shaders to %s' % (len(shaders), OUTPUT))


if __name__ == '__main__':
	main()
