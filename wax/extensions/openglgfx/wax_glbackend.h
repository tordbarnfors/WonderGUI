/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#ifndef WAX_GLBACKEND_DOT_H
#define WAX_GLBACKEND_DOT_H
#pragma once

#include <wax_geo.h>
#include <wax_util.h>

#ifdef WIN32
#	include <GL/glew.h>
#else
#	ifdef __APPLE__
#		define GL_SILENCE_DEPRECATION
#		include <OpenGL/gl3.h>
#	else
#		include <GL/glew.h>
#	endif
#endif

#include <wax_gpubackend.h>
#include <wax_glsurface.h>

#include <vector>

namespace wax
{
	class GlSurface;
	class GlBackend;
	typedef	StrongPtr<GlBackend> GlBackend_p;
	typedef	WeakPtr<GlBackend>	GlBackend_wp;

	//____ GlBackend ___________________________________________________________
	/**
	 * @brief Backend that renders with OpenGL 3.3.
	 *
	 * Built on GpuBackend, which turns the commands into what the shaders read. The shaders are
	 * the HLSL of wax/extensions/hlsl, turned into GLSL 3.3 by compile_shaders.py, see
	 * wax_glshaders.cpp.
	 *
	 * GL takes the vertices, colors and extras of a session best in one upload each, so the hooks
	 * GpuBackend calls only record what to do, and endSession() uploads the buffers and plays the
	 * recording back, see _execute(). The constants of each draw are recorded the same way and set
	 * as uniforms when they have changed for the program in use, see compile_shaders.py.
	 */

	class GlBackend : public GpuBackend
	{
		friend class GlSurface;

	public:

		//.____ Creation __________________________________________

		static GlBackend_p	create(int uboBindingPoint = 0 );		// uboBindingPoint isn't used, we have no uniform buffers.

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginRender() override;
		void	endRender() override;

		//.____ Misc _________________________________________________________

		bool	setDefaultCanvas(SizeSPX size, int scale, ColorSpace colorSpace = ColorSpace::sRGB);

		const CanvasInfo* canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;
		WarpmapFactory_p	warpmapFactory() override;

		bool	canBeBlitSource(const TypeInfo& type) const override;
		bool	canBeCanvas(const TypeInfo& type) const override;

		void	waitForCompletion() override;

		static void		setShaderBlob(Blob* pBlob) { s_pShaderPrograms = pBlob; }
		static Blob_p	shaderBlob() { return s_pShaderPrograms; }

	protected:
		GlBackend(int uboBindingPoint);
		~GlBackend();

		//.____ GpuBackend ____________________________________________________

		bool	_isRecording() const override;

		bool	_isNativeSurface( const Object * pObject ) const override;
		bool	_isNativeEdgemap( const Object * pObject ) const override;
		bool	_isNativeWarpmap( const Object * pObject ) const override;

		void	_beginSessionOnDevice( Surface * pCanvas ) override;
		void	_endSessionOnDevice() override;

		bool	_reserveFrameBuffers( int nVertices, int nColors, int nExtras, int nObjects ) override;

		bool	_bindCanvas( Surface * pCanvas, SizeI& size, CanvasKind& kind ) override;
		bool	_setPipeline( PipelineKind kind ) override;
		void	_setConstants( ConstantBlock block ) override;
		void	_draw( int firstVertex, int nVertices ) override;
		void	_setScissor( const RectI& rect ) override;
		void	_resetScissor() override;

		void	_prepareBlitSource( Surface * pSurface ) override;
		bool	_bindBlitTexture( Surface * pSurface ) override;
		void	_bindBlitSampler( Surface * pSurface, bool bClip ) override;
		bool	_bindEdgemap( GpuEdgemap * pEdgemap ) override;
		void	_drawWarpmap( Warpmap * pWarpmap, const RectI * pPatches, int nPatches ) override;

		//.____ Internal ______________________________________________________

		// Texture units of what the shaders read, see compile_shaders.py.

		enum TextureUnit
		{
			BlitSourceUnit = 0,				// waxBlitSource, sampler2D or usampler2D.
			ColorsUnit = 1,					// waxBuffer0, register t0.
			ExtrasUnit = 2,					// waxBuffer1, register t1.
			EdgemapOrPaletteUnit = 3		// waxBuffer3, register t3: the edgemap drawn or the palette of the blit source.
		};

		// The constants as the shaders have them, the three blocks of GpuBackend one after the other,
		// see constants.hlsl, and glCanvas, which compile_shaders.py adds: how y is flipped. Set as
		// uvec4 waxConstants[6].

		struct ShaderConstants
		{
			CanvasConstants	canvas;
			PixelConstants	pixel;
			WarpConstants	warp;
			float			glCanvas[4];		// Flip of clip space y, y of gl_FragCoord at 0, its multiplier, unused.
		};

		// What the hooks record, played back by _execute().

		enum class Op : uint8_t
		{
			BindCanvas,				// pObject: the surface, null for the default canvas.
			SetPipeline,			// a: PipelineKind, b: blend mode.
			SetConstants,			// a: index in m_constantSnapshots.
			Draw,					// a: first vertex, b: vertices.
			SetScissor,				// rect, in canvas pixels from the top.
			BindBlitTexture,		// pObject: the surface.
			BindSampler,			// a: index in m_samplers.
			BindEdgemap,			// pObject: the edgemap.
			DrawWarpmap				// pObject: the Warpmap, a: first patch in m_patches, b: patches.
		};

		struct Op_t
		{
			Op			op;
			int			a;
			int			b;
			Object *	pObject;		// Kept alive by m_sessionObjects.
			RectI		rect;
		};

		void	_record( Op op, int a = 0, int b = 0, Object * pObject = nullptr, const RectI& rect = RectI() );
		void	_holdForSession( Object * pObject );
		void	_pushConstants();

		void	_execute();
		void	_executeBindCanvas( GlSurface * pCanvas );
		void	_executeSetPipeline( PipelineKind kind, BlendMode blendMode );
		void	_executeScissor( const RectI& rect );
		void	_executeConstants();

		void	_loadPrograms();
		Blob_p	_generateProgramBlob();
		GLuint  _loadOrCompileProgram(int programNb, const char* pVertexShaderSource, const char* pFragmentShaderSource);
		void	_setUniforms(GLuint progId);

		static const char *	_shaderSource( const char * pName );

		SurfaceFactory_p	m_pSurfaceFactory;
		EdgemapFactory_p	m_pEdgemapFactory;
		WarpmapFactory_p	m_pWarpmapFactory;

		bool			m_bFullyInitialized = false;
		bool			m_bRendering = false;			// Between beginRender() and endRender(), ready to render.

		// Recording, see the class description.

		std::vector<Op_t>			m_ops;
		std::vector<RectI>			m_patches;				// Of DrawWarpmap.
		std::vector<Object_p>		m_sessionObjects;		// Objects the recording points at.
		std::vector<ShaderConstants>	m_constantSnapshots;	// What the draws have.

		ShaderConstants				m_constants;			// As set by the hooks.
		bool						m_bConstantsChanged = true;

		bool						m_bCanvasBound = false;	// A canvas has been bound in the recording.
		SizeI						m_targetSize;			// Of the canvas being recorded for, in pixels.
		CanvasKind					m_targetKind = CanvasKind::WithAlpha;

		std::vector<Vertex>			m_vertices;				// GpuBackend's buffers, see _reserveFrameBuffers().
		std::vector<Color>			m_colors;
		std::vector<Extras>			m_extras;

		// Playing back.

		GlSurface *		m_pExecCanvas = nullptr;
		bool			m_bExecDefaultCanvas = false;
		bool			m_bExecCanvasOK = false;			// Framebuffer complete, draws go through.
		SizeI			m_execTargetSize;
		CanvasKind		m_execTargetKind = CanvasKind::WithAlpha;
		GLuint			m_execProgram = 0;
		int				m_execProgramIndex = -1;			// kind * 2 + dual source, of m_execProgram.
		int				m_execConstants = -1;				// What the draws get, in m_constantSnapshots.
		int				m_execBlendKey = -1;				// Blend state last set, see _executeSetPipeline().

		// GL objects

		GLuint			m_programs[int(PipelineKind::Size)][2] = {};	// [kind][dual source]
		GLint			m_constantsLocations[int(PipelineKind::Size) * 2] = {};	// Of waxConstants, [kind * 2 + dual source].
		int				m_programConstants[int(PipelineKind::Size) * 2] = {};	// What each program has been given this session, -1 for nothing.
		GLuint			m_samplers[16] = {};				// [mipmapped*8 + clip*4 + tiling*2 + bilinear]

		GLuint			m_framebufferId = 0;
		GLuint			m_vertexArrayId = 0;
		GLuint			m_vertexBufferId = 0;
		GLuint			m_colorBufferId = 0;
		GLuint			m_colorBufferTex = 0;
		GLuint			m_extrasBufferId = 0;
		GLuint			m_extrasBufferTex = 0;

		bool			m_bProgramBinariesSupported = false;

		CanvasInfo		m_defaultCanvas;
		CanvasInfo		m_dummyCanvas;
		bool			m_bDefaultCanvasHasAlpha = true;	// The default framebuffer might not have alpha.

		// GL states saved between beginRender() and endRender().

		GLboolean	m_glDepthTest;
		GLboolean   m_glScissorTest;
		GLboolean	m_glBlendEnabled;
		GLboolean	m_glSRGBEnabled;
		GLboolean	m_glCullFace;
		GLint		m_glBlendSrc;
		GLint		m_glBlendDst;
		GLint		m_glViewport[4];
		GLint		m_glScissorBox[4];
		GLint		m_glReadFrameBuffer;
		GLint		m_glDrawFrameBuffer;

		// Program binaries

		const static int c_nbPrograms = int(PipelineKind::Size) * 2;
		const static int c_versionNb = 124;		// ... 122: Bias of warp positions for Nearest sampling. 123: GLSL generated from the HLSL, on GpuBackend. 124: A shape shader per kind, plain min() and max().

		struct ProgramBlobEntry
		{
			int		offset;
			GLsizei	size;
			GLenum	binaryFormat;
		};

		struct ProgramBlobHeader
		{
			int					version;
			int					nbPrograms;
			ProgramBlobEntry	programs[c_nbPrograms];
		};

		static Blob_p	s_pShaderPrograms;

		// Shaders, see wax_glshaders.cpp.

		struct ShaderSource
		{
			const char *	pName;
			const char *	pSource;
		};

		static const ShaderSource	s_shaderSources[];
		static const int			s_nbShaderSources;

		static void onGlError(GLenum errorCode, const Object * pObject, const TypeInfo* pClassType, const char * func, const char * file, int line);
	};

} // namespace wax
#endif //WAX_GLBACKEND_DOT_H
