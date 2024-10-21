/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/
#ifndef WG_GLBACKEND_DOT_H
#define WG_GLBACKEND_DOT_H
#pragma once

#include <wg_geo.h>
#include <wg_gfxutil.h>

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

#include <wg_gfxbackend.h>
#include <wg_glsurface.h>

namespace wg
{
	class GlSurface;
	class GlBackend;
	typedef	StrongPtr<GlBackend> GlBackend_p;
	typedef	WeakPtr<GlBackend>	GlBackend_wp;

	class GlBackend : public GfxBackend
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

		void	beginSession(const SessionInfo* pSession) override;
		void	endSession() override;

		void	setCanvas(Surface* pSurface) override;
		void	setCanvas(CanvasRef ref) override;

		void	setObjects(Object** pBeg, Object** pEnd) override;

		void	setCoords(spx* pBeg, spx* pEnd) override;
		void	setColors(HiColor* pBeg, HiColor* pEnd) override;
		void	setTransforms(Transform* pBeg, Transform* pEnd) override;

		void	processCommands(int32_t* pBeg, int32_t* pEnd) override;


		//.____ Misc _________________________________________________________

		bool	setDefaultCanvas(SizeSPX size, int scale);

		const CanvasInfo* canvasInfo(CanvasRef ref) const override;

		SurfaceFactory_p	surfaceFactory() override;
		EdgemapFactory_p	edgemapFactory() override;

		int		maxEdges() const override;

		const TypeInfo& surfaceType(void) const override;

	protected:
		GlBackend(int uboBindingPoint);
		~GlBackend();

		const static int	c_maxSegments = 16;
		static const int	c_segmentsTintTexMapSize = 64 * 10;				// Number of segments tint palettes that fit into segmentsTintTexMap.


		void	_loadPrograms(int uboBindingPoint);
		Blob_p	_generateProgramBlob();
		GLuint  _loadOrCompileProgram(int programNb, const char* pVertexShaderSource, const char* pFragmentShaderSource);

		void	_setUniforms(GLuint progId, int uboBindingPoint);

		void	_setBlendMode(BlendMode mode);
		void	_setBlitSource(GlSurface* pSurf);


		void	_initTables();
		float	_scaleThickness(float thickeness, float slope);

		void	_setCanvas(Surface* pSurface);

		enum CommandGL
		{
			SetCanvas,
			StateChange,
			Plot,
			Lines,
			StraightFill,
			SubpixelFill,
			Blit,
			Blur,
			Edgemap

		};


		SurfaceFactory_p	m_pSurfaceFactory = nullptr;
		EdgemapFactory_p	m_pEdgemapFactory = nullptr;

		Object** m_pObjectsBeg = nullptr;
		Object** m_pObjectsEnd = nullptr;

		spx* m_pCoordsBeg = nullptr;
		spx* m_pCoordsEnd = nullptr;
		spx* m_pCoordsPtr = nullptr;

		HiColor* m_pColorsBeg = nullptr;
		HiColor* m_pColorsEnd = nullptr;
		HiColor* m_pColorsPtr = nullptr;

		Transform* m_pTransformsBeg = nullptr;
		Transform* m_pTransformsEnd = nullptr;


		float	m_lineThicknessTable[17];

		float	m_sRGBtoLinearTable[256];
		float	m_linearToLinearTable[256];

		//

		bool	m_bProgramBinariesSupported = false;



		GLuint			m_framebufferId;
		int				m_nSegments;								// Number of segments for current segment command.

		GLsync          m_idleSync = 0;

		bool			m_bFullyInitialized = false;
		
		CanvasInit		m_beginRenderOp = CanvasInit::Keep;

		CoordF			m_blitSourceSize;
		SampleMethod	m_blitSourceSampleMethod;

		Surface_p		m_pCanvas = nullptr;

		// "Active" members are updated and used in the rendering loop in endSession.

		bool			m_bActiveCanvasIsA8 = false;
		GlSurface_p		m_pActiveCanvas = nullptr;                                      // Currently active canvas in OpenGL, not to confuse with m_pCanvas which might not be active yet.
		SizeI			m_activeCanvasSize;                                             // Currently active canvas size in OpenGL, " -
		bool			m_bMipmappedActiveCanvas = false;                               // Set if currently active canvas is a surface that is mipmapped.
		BlendMode		m_activeBlendMode = BlendMode::Blend;

		GlSurface_p		m_pActiveBlitSource = nullptr;									// Currently active blit source in OpenGL, not to confuse with m_pBlitSource which might not be active yet.
		bool			m_bTintmapIsActive = false;

		GLfloat			m_activeMorphFactor = 0.5f;

		// Device programs

		GLuint  m_fillProg[2];									// [RGB/A_8 dest]
		GLuint  m_fillTintmapProg[2];							// [RGB/A_8 dest]

		GLuint  m_aaFillProg[2];								// [RGB/A_8 dest]
		GLuint  m_aaFillTintmapProg[2];							// [RGB/A_8 dest]

		GLuint  m_blurProg[2];									// [tintgradient]
		GLuint  m_blitProg[2];									// [RGB/A_8 dest]
		GLuint  m_blitTintmapProg[2];							// [RGB/A_8 dest]

		GLuint  m_alphaBlitProg[2];								// [RGB/A_8 dest]
		GLuint  m_alphaBlitTintmapProg[2];						// [RGB/A_8 dest]

		GLuint  m_paletteBlitNearestProg[2];						// [RGB/A_8 dest]
		GLuint  m_paletteBlitNearestTintmapProg[2];				// [RGB/A_8 dest]

		GLuint  m_paletteBlitInterpolateProg[2];					// [RGB/A_8 dest]
		GLuint  m_paletteBlitInterpolateTintmapProg[2];			// [RGB/A_8 dest]

		GLuint  m_plotProg[2];									// [RGB/A_8 dest]
		GLuint  m_lineFromToProg[2];							// [RGB/A_8 dest]

		GLuint	m_segmentsProg[c_maxSegments][2];			// [nb segments][RGB/A_8 dest]

		GLuint	m_blitProgMatrix[PixelFormat_size][2][2][2];	// [source format][interpolation][tintgradient][RGB/A_8 dest]

		//

		const static int c_nbPrograms = 30 + (c_maxSegments-1) * 4;
		const static int c_versionNb = 101;					//

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
			CoordF	tintmapOfs;
			CoordF	colorstripOfs;					// For Edgemaps only.
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
		};


		BlurUniform	m_activeBlurInfo;
		Blurbrush_p	m_pActiveBlurbrush;

		GLint		m_blurUniformLocation[2][2];



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
		static const char blurFragmentShader[];
		static const char blurFragmentShaderTintmap[];
		static const char plotVertexShader[];
		static const char plotFragmentShader[];
		static const char plotFragmentShader_A8[];
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

		bool		m_bTintmap = false;
		RectI		m_tintmapRect;				// Measured in pixels.
		int			m_tintmapBeginX	= -1;		// Offset in m_pColorBuffer
		int			m_tintmapEndX	= -1;		// " -
		int			m_tintmapBeginY = -1;
		int			m_tintmapEndY	= -1;


	};

} // namespace wg
#endif //WG_GLGFXDEVICE_DOT_H

