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

#include <wax_backend.h>
#include <wax_gpushapes.h>
#include <wax_tinttools.h>
#include <wax_glsurface.h>

namespace wax
{
	class GlSurface;
	class GlBackend;
	typedef	StrongPtr<GlBackend> GlBackend_p;
	typedef	WeakPtr<GlBackend>	GlBackend_wp;

	class GlBackend : public Backend
	{
		friend class GlSurface;

	public:

		//.____ Creation __________________________________________

		static GlBackend_p	create(int uboBindingPoint = 0 );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginRender() override;
		void	endRender() override;

		void	beginSession( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo = nullptr ) override;
		void	endSession() override;

		void	setCanvas(Surface* pSurface) override;
		void	setCanvas(CanvasRef ref) override;

		void	setObjects(Object* const * pBeg, Object* const * pEnd) override;

		void	setRects(const RectSPX* pBeg, const RectSPX* pEnd) override;
		void	setColors(const HiColor* pBeg, const HiColor* pEnd) override;
		void	setTransforms(const Transform* pBeg, const Transform* pEnd) override;

		void	processCommands(const uint16_t* pBeg, const uint16_t* pEnd, int version) override;


		//.____ Misc _________________________________________________________

		bool	setDefaultCanvas(SizeSPX size, int scale, ColorSpace colorSpace = ColorSpace::sRGB);

		const CanvasInfo* canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;
		WarpmapFactory_p	warpmapFactory() override;

		int		maxEdges() const override;

		bool	canBeBlitSource(const TypeInfo& type) const override;
		bool	canBeCanvas(const TypeInfo& type) const override;

		void	waitForCompletion() override;

		static void		setShaderBlob(Blob* pBlob) { s_pShaderPrograms = pBlob; }
		static Blob_p	shaderBlob() { return s_pShaderPrograms; }

	protected:
		GlBackend(int uboBindingPoint);
		~GlBackend();

		const static int	c_maxSegments = 16;
		static const int	c_segmentsTintTexMapSize = 64 * 10;				// Number of segments tint palettes that fit into segmentsTintTexMap.

		static const int 	s_flipCornerOrder[GfxFlip_size][4];

		void	_loadPrograms(int uboBindingPoint);
		Blob_p	_generateProgramBlob();
		GLuint  _loadOrCompileProgram(int programNb, const char* pVertexShaderSource, const char* pFragmentShaderSource);

		void	_setUniforms(GLuint progId, int uboBindingPoint);

		void	_setBlendMode(BlendMode mode);
		void	_setBlitSource(GlSurface* pSurf);
		void	_setPartialCoverage(bool bPartialCoverage);
		void	_resetCommandStates();
		void	_uploadCanvasUBO();


		void	_initTables();
		float	_scaleThickness(float thickeness, float slope);

		void	_setCanvas(Surface* pSurface);
		void	_setPaletteInfo(GLuint prog, GlSurface* pSurf);
		void	_setRgbxClip(GLuint prog, GlSurface* pSurf, bool bClipBlit);

		enum CommandGL
		{
			SetCanvas,
			StateChange,
			Lines,
			CappedLines,	// Lines with caps, drawn by the shape shader.
			StraightFill,
			SubpixelFill,
			Blit,
			ClipBlit,		// A Blit that leaves everything outside the source alone.
			Blur,
			Edgemap,
			Shape,
			WarpEdgemap,
			WarpBlit
		};

		enum BlurSource			// Kind of blit source, selects the blur shader.
		{
			Normal,
			Alpha,				// Alpha_8, read as white with alpha.
			Palette				// Palette based, each tap looked up in the palette.
		};


		SurfaceFactory_p	m_pSurfaceFactory = nullptr;
		EdgemapFactory_p	m_pEdgemapFactory = nullptr;
		WarpmapFactory_p	m_pWarpmapFactory;

		Object* const * m_pObjectsBeg = nullptr;		// Object table, commands refer to objects by index.
		Object* const * m_pObjectsEnd = nullptr;

		const RectSPX* m_pRectsBeg = nullptr;
		const RectSPX* m_pRectsEnd = nullptr;
		const RectSPX* m_pRectsPtr = nullptr;

		const HiColor* m_pColorsBeg = nullptr;
		const HiColor* m_pColorsEnd = nullptr;
		const HiColor* m_pColorsPtr = nullptr;

		const Transform* m_pTransformsBeg = nullptr;
		const Transform* m_pTransformsEnd = nullptr;


		float	m_lineThicknessTable[17];

		//

		bool	m_bProgramBinariesSupported = false;



		GLuint			m_framebufferId;
		int				m_nSegments;								// Number of segments for current segment command.

		GLsync          m_idleSync = 0;

		bool			m_bFullyInitialized = false;

		CoordF			m_blitSourceSize;
		SampleMethod	m_blitSourceSampleMethod;

		// "Active" members are updated and used in the rendering loop in endSession.

		bool			m_bActiveCanvasIsA8 = false;
		bool			m_bActiveCanvasHasAlpha = true;									// False for canvases that don't store alpha, like XRGB_8.
		bool			m_bReplaceBlending = false;										// Blending enabled for Replace, see _setPartialCoverage().
		GlSurface_p		m_pActiveCanvas = nullptr;                                      // Currently active canvas in OpenGL, not to confuse with m_pCanvas which might not be active yet.
		SizeI			m_activeCanvasSize;                                             // Currently active canvas size in OpenGL, " -
		bool			m_bMipmappedActiveCanvas = false;                               // Set if currently active canvas is a surface that is mipmapped.
		BlendMode		m_activeBlendMode = BlendMode::Blend;

		GlSurface_p		m_pActiveBlitSource = nullptr;									// Currently active blit source in OpenGL, not to confuse with m_pBlitSource which might not be active yet.
		bool			m_bTintIsActive = false;

		GLfloat			m_activeMorphFactor = 0.5f;

		// Device programs

		GLuint  m_fillProg[2];									// [RGB/A_8 dest]
		GLuint  m_fillTintmapProg[2];							// [RGB/A_8 dest]

		GLuint  m_aaFillProg[2];								// [RGB/A_8 dest]
		GLuint  m_aaFillTintmapProg[2];							// [RGB/A_8 dest]

		GLuint  m_blurProg[3][2][2];							// [BlurSource][tint][RGB/A_8 dest]
		GLuint  m_blitProg[2];									// [RGB/A_8 dest]
		GLuint  m_blitTintmapProg[2];							// [RGB/A_8 dest]

		GLuint  m_alphaBlitProg[2];								// [RGB/A_8 dest]
		GLuint  m_alphaBlitTintmapProg[2];						// [RGB/A_8 dest]

		GLuint  m_paletteBlitNearestProg[2];						// [RGB/A_8 dest]
		GLuint  m_paletteBlitNearestTintmapProg[2];				// [RGB/A_8 dest]

		GLuint  m_paletteBlitInterpolateProg[2];					// [RGB/A_8 dest]
		GLuint  m_paletteBlitInterpolateTintmapProg[2];			// [RGB/A_8 dest]

		GLuint  m_lineFromToProg[2];							// [RGB/A_8 dest]

		GLuint	m_segmentsProg[c_maxSegments][2];			// [nb segments][RGB/A_8 dest]

		GLuint	m_shapeProg[2];									// [RGB/A_8 dest]
		GLuint	m_warpEdgemapProg[2];							// [RGB/A_8 dest]
		GLuint	m_warpBlitProg[2];								// [RGB/A_8 dest]

		GLuint	m_blitProgMatrix[PixelFormat_size][2][2][2];	// [source format][interpolation][tintgradient][RGB/A_8 dest]

		//

		const static int c_nbPrograms = 44 + (c_maxSegments-1) * 2;
		const static int c_versionNb = 117;		// 104: Tints replaced Tintmaps. 105: Tint color applied together with tint. 106: Palette ClipBlit clipping. 107: Palette read by index, capacity wide. 108: Blur for Alpha_8 sources and canvases. 109: Alpha of sources without alpha in ClipBlits. 110: Bilinear palette taps and weights from the same value. 111: Palette sampled with texture(), not texelFetch(). 112: Shape programs. 113: Premultiplied canvases, output by blend mode, tinted lines. 114: Tiled edgemaps. 115: Edgemap segments where edges cross and on canvases without alpha. 116: Effect modes on premultiplied colors in one pass. 117: Warps.

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


		//

		struct VertexGL
		{
			CoordI	coord;
			CoordF	uv;								// Actually contains blitSourceSize in most cases.
			int		colorsOfs;						// Offset into colorBuffer for color incl flat tint.
			int		extrasOfs;						// Offset into extrasBuffer for extra data needed by shader.
			CoordF	tintmapOfs;						// x is offset of tint block in colorBuffer, -1 for none.
			CoordF	colorstripOfs;					// Not used.
		};

		struct ColorGL
		{
			float	r, g, b, a;
		};


		//

		CanvasInfo			m_defaultCanvas;
		CanvasInfo			m_dummyCanvas;
		
		//

		struct BlurUniform
		{
			float   colorMtx[9][4];
			float   offset[9][2];
		};

		struct CanvasUBO
		{
			float	width;
			float	height;
			float	yOfs;
			float	yMul;
			float	blendInfo[4];		// How fragment shaders output to the blender, see WAX_GL_OUTPUT.
		};


		BlurUniform	m_activeBlurInfo;
		spx			m_activeBlurRadius = 64;
		Blurbrush_p	m_pActiveBlurbrush;

		GLint		m_blurUniformLocation[3][2][2][2];				// [BlurSource][tint][RGB/A_8 dest][colorMtx/offset]



		// Buffers

		CanvasUBO	m_canvasUBO;
		GLuint		m_canvasUBOId;				// Binds to binding point 1


		GLuint  m_vertexArrayId;
		GLuint  m_vertexBufferId;

		GLuint	m_colorBufferTex;
		GLuint	m_colorBufferId;


		GLuint	m_extrasBufferTex;
		GLuint	m_extrasBufferId;


		// GL states saved between BeginRender() and EndRender().

		GLboolean	m_glDepthTest;
		GLboolean   m_glScissorTest;
		GLboolean	m_glBlendEnabled;
		GLboolean	m_glSRGBEnabled;
		GLint		m_glBlendSrc;
		GLint		m_glBlendDst;
		GLint		m_glViewport[4];
		GLint		m_glScissorBox[4];
		GLint		m_glReadFrameBuffer;
		GLint		m_glDrawFrameBuffer;

		//

		static Blob_p	s_pShaderPrograms;

		static int s_bindingPointCanvasUBO;

		//

		static const char fillVertexShader[];
		static const char fillTintmapVertexShader[];
		static const char fillFragmentShader[];
		static const char fillFragmentShader_A8[];
		static const char fillFragmentShaderTintmap[];
		static const char fillFragmentShaderTintmap_A8[];

		static const char blitVertexShader[];
		static const char blitTintmapVertexShader[];
		static const char blitFragmentShader[];
		static const char blitFragmentShader_A8[];
		static const char blitFragmentShaderTintmap[];
		static const char blitFragmentShaderTintmap_A8[];
		static const char * const blurFragmentShaders[3][2][2];		// [BlurSource][tint][RGB/A_8 dest]
		static const char lineFromToVertexShader[];
		static const char lineFromToFragmentShader[];
		static const char lineFromToFragmentShader_A8[];
		static const char aaFillVertexShader[];
		static const char aaFillTintmapVertexShader[];

		static const char aaFillFragmentShader[];
		static const char aaFillFragmentShader_A8[];
		static const char aaFillFragmentShaderTintmap[];
		static const char aaFillFragmentShaderTintmap_A8[];


		static const char segmentsVertexShader[];
		static const char segmentsFragmentShader[];
		static const char segmentsFragmentShader_A8[];

		static const char shapeVertexShader[];
		static const char shapeFragmentShader[];
		static const char shapeFragmentShader_A8[];
		static const char warpVertexShader[];
		static const char warpEdgemapFragmentShader[];
		static const char warpEdgemapFragmentShader_A8[];
		static const char warpBlitFragmentShader[];
		static const char warpBlitFragmentShader_A8[];

		static const char alphaBlitFragmentShader[];
		static const char alphaBlitFragmentShader_A8[];
		static const char alphaBlitFragmentShaderTintmap[];
		static const char alphaBlitFragmentShaderTintmap_A8[];

		static const char paletteBlitNearestVertexShader[];
		static const char paletteBlitNearestTintmapVertexShader[];

		static const char paletteBlitNearestFragmentShader[];
		static const char paletteBlitNearestFragmentShader_A8[];
		static const char paletteBlitNearestFragmentShaderTintmap[];
		static const char paletteBlitNearestFragmentShaderTintmap_A8[];


		static const char paletteBlitInterpolateVertexShader[];
		static const char paletteBlitInterpolateTintmapVertexShader[];

		static const char paletteBlitInterpolateFragmentShader[];
		static const char paletteBlitInterpolateFragmentShader_A8[];
		static const char paletteBlitInterpolateFragmentShaderTintmap[];
		static const char paletteBlitInterpolateFragmentShaderTintmap_A8[];


		//

		static void onGlError(GLenum errorCode, const Object * pObject, const TypeInfo* pClassType, const char * func, const char * file, int line);

		//

		VertexGL*	m_pVertexBuffer = nullptr;
		int			m_nVertices = 0;

		ColorGL*	m_pColorBuffer = nullptr;
		int			m_nColors = 0;

		GLfloat*	m_pExtrasBuffer = nullptr;
		int			m_extrasBufferSize = 0;

		int*		m_pCommandQueue = nullptr;
		int			m_commandQueueSize = 0;

		std::vector<Object*> m_objects;

		int			m_tintColorOfs = -1;		// Offset in m_pColorBuffer for tintColor if flat tint active.

		int			m_tintOfs = -1;				// Offset in m_pColorBuffer for tint block (see TintTools) if Tint is active.

		int			m_colorBufferCapacity = 0;	// Entries allocated in m_pColorBuffer.
		int			m_sessionColors = 0;		// Colors in session according to SessionInfo, reserve for tint blocks growing the buffer.

		void		_reserveColors(int used, int extra);

		std::vector<GpuShapes::Segment>	m_shapeSegments;		// Segments of shape being processed.

		struct CappedLine
		{
			CoordSPX	begin;
			CoordSPX	end;
			spx			thickness;
			uint16_t	caps;
			HiColor		color;
		};

		std::vector<CappedLine>	m_cappedLines;		// Lines with caps of line command being processed.


	};

} // namespace wax
#endif //WAX_GLDEVICE_DOT_H

