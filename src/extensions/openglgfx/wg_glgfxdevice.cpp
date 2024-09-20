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

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <wg_glgfxdevice.h>
#include <wg_glsurface.h>
#include <wg_glsurfacefactory.h>
#include <wg_gledgemap.h>
#include <wg_gledgemapfactory.h>
#include <wg_gfxbase.h>
#include <wg_gfxutil.h>

using namespace std;
using namespace wg::Util;

namespace wg
{
	const TypeInfo GlGfxDevice::TYPEINFO = { "GlGfxDevice", &GfxDevice::TYPEINFO };

	GlGfxDevice *	GlGfxDevice::s_pActiveDevice = nullptr;

	Blob_p			GlGfxDevice::s_pShaderPrograms = nullptr;


#define LOG_GLERROR(check) { GLenum err = check; if(err != 0) onGlError(err, this, &TYPEINFO, __func__, __FILE__, __LINE__ ); }

#define LOG_INIT_GLERROR(check) { GLenum err = check; if(err != 0) { onGlError(err, this, &TYPEINFO, __func__, __FILE__, __LINE__ ); m_bFullyInitialized = false; } }


	//____ onGlError() _______________________________________________________________

	void GlGfxDevice::onGlError(GLenum errorCode, const Object * pObject, const TypeInfo* pClassType, const char * func, const char * file, int line)
	{
		char	buffer[1024];

		const char * pErrorName;

		switch( errorCode )
		{
			case GL_NO_ERROR:
				pErrorName = "GL_NO_ERROR";
				break;

			case GL_INVALID_ENUM:
				pErrorName = "GL_INVALID_ENUM";
				break;

			case GL_INVALID_VALUE:
				pErrorName = "GL_INVALID_VALUE";
				break;

			case GL_INVALID_OPERATION:
				pErrorName = "GL_INVALID_OPERATION";
				break;

			case GL_INVALID_FRAMEBUFFER_OPERATION:
				pErrorName = "GL_INVALID_FRAMEBUFFER_OPERATION";
				break;

			case GL_OUT_OF_MEMORY:
				pErrorName = "GL_OUT_OF_MEMORY";
				break;
/*
			case GL_STACK_UNDERFLOW:
				pErrorName = "GL_STACK_UNDERFLOW";
				break;

			case GL_STACK_OVERFLOW:
				pErrorName = "GL_STACK_OVERFLOW";
				break;
*/
			default:
				pErrorName = "UNKNOWN GL ERROR! (should not happen)";
				break;
		}


		snprintf( buffer, 1024, "OpenGL error 0x%x: %s", errorCode, pErrorName );
		GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::OpenGL, buffer, pObject, pClassType, func, file, line);
	}


	//____ create() _______________________________________________________________

	GlGfxDevice_p GlGfxDevice::create( int uboBindingPoint)
	{
		GlGfxDevice_p p(new GlGfxDevice( uboBindingPoint ));

		if( !p->m_bFullyInitialized )
			return GlGfxDevice_p(nullptr);

		return p;
	}

	//____ _setDrawUniforms() __________________________________________________

	void GlGfxDevice::_setDrawUniforms(GLuint progId, int uboBindingPoint)
	{
		GLint extrasIdLoc = glGetUniformLocation(progId, "extrasId");
		glUseProgram(progId);
		glUniform1i(extrasIdLoc, 1);		// Needs to be set. Texture unit 1 is used for extras buffer.

	}

	//____ _setBlitUniforms() __________________________________________________

	void GlGfxDevice::_setBlitUniforms(GLuint progId, int uboBindingPoint)
	{
		GLint extrasIdLoc = glGetUniformLocation(progId, "extrasId");
		GLint texIdLoc = glGetUniformLocation(progId, "texId");

		glUseProgram(progId);
		glUniform1i(extrasIdLoc, 1);		// Needs to be set. Texture unit 1 is used for extras buffer.
		glUniform1i(texIdLoc, 0);			// Needs to be set. Texture unit 0 is used for textures.
	}

	//____ _setPaletteBlitUniforms() __________________________________________________

	void GlGfxDevice::_setPaletteBlitUniforms(GLuint progId, int uboBindingPoint)
	{
		GLint extrasIdLoc = glGetUniformLocation(progId, "extrasId");
		GLint texIdLoc = glGetUniformLocation(progId, "texId");
		GLint paletteIdLoc = glGetUniformLocation(progId, "paletteId");

		glUseProgram(progId);
		glUniform1i(texIdLoc, 0);			// Needs to be set. Texture unit 0 is used for textures.
		glUniform1i(extrasIdLoc, 1);		// Needs to be set. Texture unit 1 is used for extras buffer.
		glUniform1i(paletteIdLoc, 2);			// Needs to be set. Texture unit 2 is used for palette.
	}

	//____ constructor _____________________________________________________________________

	GlGfxDevice::GlGfxDevice( int uboBindingPoint )
	{
		// Get version number, set flags

		GLint major, minor;
		glGetIntegerv(GL_MAJOR_VERSION, &major);
		glGetIntegerv(GL_MINOR_VERSION, &minor);

		if((major == 4 && minor >= 1) || major > 4)
		{
			GLint formats = 0;
			glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
			m_bProgramBinariesSupported = (formats >= 1);
		}
		
		//

		m_bFullyInitialized = true;

		_initTables();

		_loadPrograms(uboBindingPoint);

		if (!s_pShaderPrograms && m_bProgramBinariesSupported)
			s_pShaderPrograms = _generateProgramBlob();


		// Fill in our m_blitProgMatrix

		for (int i = 0; i < PixelFormat_size; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				m_blitProgMatrix[i][0][0][j] = m_blitProg[j];
				m_blitProgMatrix[i][1][0][j] = m_blitProg[j];
				m_blitProgMatrix[i][0][1][j] = m_blitGradientProg[j];
				m_blitProgMatrix[i][1][1][j] = m_blitGradientProg[j];
			}
		}


		m_blitProgMatrix[(int)PixelFormat::Alpha_8][0][0][0] = m_alphaBlitProg[0];
		m_blitProgMatrix[(int)PixelFormat::Alpha_8][1][0][0] = m_alphaBlitProg[0];

		m_blitProgMatrix[(int)PixelFormat::Alpha_8][0][1][0] = m_alphaBlitGradientProg[0];
		m_blitProgMatrix[(int)PixelFormat::Alpha_8][1][1][0] = m_alphaBlitGradientProg[0];

		m_blitProgMatrix[(int)PixelFormat::Index_8_linear][0][0][0] = m_paletteBlitNearestProg[0];
		m_blitProgMatrix[(int)PixelFormat::Index_8_linear][1][0][0] = m_paletteBlitInterpolateProg[0];

		m_blitProgMatrix[(int)PixelFormat::Index_8_linear][0][1][0] = m_paletteBlitNearestGradientProg[0];
		m_blitProgMatrix[(int)PixelFormat::Index_8_linear][1][1][0] = m_paletteBlitInterpolateGradientProg[0];

		m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][0][0][0] = m_paletteBlitNearestProg[0];
		m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][1][0][0] = m_paletteBlitInterpolateProg[0];

		m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][0][1][0] = m_paletteBlitNearestGradientProg[0];
		m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][1][1][0] = m_paletteBlitInterpolateGradientProg[0];


		m_blitProgMatrix[(int)PixelFormat::Alpha_8][0][0][1] = m_alphaBlitProg[1];
		m_blitProgMatrix[(int)PixelFormat::Alpha_8][1][0][1] = m_alphaBlitProg[1];

		m_blitProgMatrix[(int)PixelFormat::Alpha_8][0][1][1] = m_alphaBlitGradientProg[1];
		m_blitProgMatrix[(int)PixelFormat::Alpha_8][1][1][1] = m_alphaBlitGradientProg[1];

		m_blitProgMatrix[(int)PixelFormat::Index_8_linear][0][0][1] = m_paletteBlitNearestProg[1];
		m_blitProgMatrix[(int)PixelFormat::Index_8_linear][1][0][1] = m_paletteBlitInterpolateProg[1];

		m_blitProgMatrix[(int)PixelFormat::Index_8_linear][0][1][1] = m_paletteBlitNearestGradientProg[1];
		m_blitProgMatrix[(int)PixelFormat::Index_8_linear][1][1][1] = m_paletteBlitInterpolateGradientProg[1];

		m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][0][0][1] = m_paletteBlitNearestProg[1];
		m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][1][0][1] = m_paletteBlitInterpolateProg[1];

		m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][0][1][1] = m_paletteBlitNearestGradientProg[1];
		m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][1][1][1] = m_paletteBlitInterpolateGradientProg[1];

		LOG_INIT_GLERROR(glGetError());

		// Create texture for segment colors

		for (int seg = 0; seg < c_maxSegments; seg++)
		{
			for (int i = 0; i < 16 * 2 * 2 * 4; i++)
			{
				m_segmentsTintTexMap[seg][i] = 65535;
			}
		}


		glGenTextures(1, &m_segmentsTintTexId);
		glBindTexture(GL_TEXTURE_2D, m_segmentsTintTexId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2 * c_maxSegments, 2 * c_segmentsTintTexMapSize, 0, GL_BGRA, GL_UNSIGNED_SHORT, nullptr);

		// Create the buffers we need for FrameBuffer and Vertices

		glGenBuffers(1, &m_vertexBufferId);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
		glBufferData(GL_ARRAY_BUFFER, c_vertexBufferSize * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

		// Create and initialize our VertexArray

		glGenVertexArrays(1, &m_vertexArrayId);
		glBindVertexArray(m_vertexArrayId);

		glVertexAttribIPointer(		// Attribute: Coord
			0,						// attribute number, must match the layout in the shader.
			2,						// size
			GL_INT,					// type
			sizeof(Vertex),			// stride
			(void*)0				// array buffer offset
		);

		glVertexAttribPointer(		// Attribute: uv
			1,						// attribute number, must match the layout in the shader.
			2,						// size
			GL_FLOAT,				// type
			GL_TRUE,				// normalized?
			sizeof(Vertex),			// stride
			(void*)(sizeof(CoordI))  // array buffer offset
		);

		glVertexAttribIPointer(		// Attribute: extrasOfs
			2,						// attribute number, must match the layout in the shader.
			1,						// size
			GL_INT,					// type
			sizeof(Vertex),			// stride
			(void*)(sizeof(CoordI) + sizeof(CoordF))  // array buffer offset
		);

		glVertexAttribIPointer(		// Attribute: canvasInfoOfs
			3,						// attribute number, must match the layout in the shader.
			1,						// size
			GL_INT,					// type
			sizeof(Vertex),			// stride
			(void*)(sizeof(CoordI) + sizeof(CoordF) + sizeof(int))  // array buffer offset
		);

		glVertexAttribIPointer(		// Attribute: tintInfoOfs
			4,						// attribute number, must match the layout in the shader.
			1,						// size
			GL_INT,					// type
			sizeof(Vertex),			// stride
			(void*)(sizeof(CoordI) + sizeof(CoordF) + sizeof(int)*2)  // array buffer offset
		);


		LOG_INIT_GLERROR(glGetError());



		LOG_INIT_GLERROR(glGetError());

		// Create a TextureBufferObject for providing extra data to our shaders

		glGenBuffers(1, &m_extrasBufferId);
		glBindBuffer(GL_TEXTURE_BUFFER, m_extrasBufferId);
		glBufferData(GL_TEXTURE_BUFFER, c_extrasBufferSize * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

		LOG_INIT_GLERROR(glGetError());

		glGenTextures(1, &m_extrasBufferTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, m_extrasBufferTex);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_extrasBufferId);
		glActiveTexture(GL_TEXTURE0);

		LOG_INIT_GLERROR(glGetError());
	}

	//____ Destructor ______________________________________________________________

	GlGfxDevice::~GlGfxDevice()
	{
		LOG_GLERROR(glGetError());

		glDeleteProgram(m_fillProg[0]);
		glDeleteProgram(m_fillProg[1]);
		glDeleteProgram(m_fillGradientProg[0]);
		glDeleteProgram(m_fillGradientProg[1]);
		glDeleteProgram(m_aaFillProg[0]);
		glDeleteProgram(m_aaFillProg[1]);
		glDeleteProgram(m_aaFillGradientProg[0]);
		glDeleteProgram(m_aaFillGradientProg[1]);

		glDeleteProgram(m_blurProg[0]);
		glDeleteProgram(m_blurProg[1]);

		glDeleteProgram(m_blitProg[0]);
		glDeleteProgram(m_blitProg[1]);
		glDeleteProgram(m_blitGradientProg[0]);
		glDeleteProgram(m_blitGradientProg[1]);
		glDeleteProgram(m_alphaBlitProg[0]);
		glDeleteProgram(m_alphaBlitProg[1]);
		glDeleteProgram(m_alphaBlitGradientProg[0]);
		glDeleteProgram(m_alphaBlitGradientProg[1]);
		glDeleteProgram(m_paletteBlitNearestProg[0]);
		glDeleteProgram(m_paletteBlitNearestProg[1]);
		glDeleteProgram(m_paletteBlitNearestGradientProg[0]);
		glDeleteProgram(m_paletteBlitNearestGradientProg[1]);
		glDeleteProgram(m_paletteBlitInterpolateProg[0]);
		glDeleteProgram(m_paletteBlitInterpolateProg[1]);
		glDeleteProgram(m_paletteBlitInterpolateGradientProg[0]);
		glDeleteProgram(m_paletteBlitInterpolateGradientProg[1]);

		glDeleteProgram(m_plotProg[0]);
		glDeleteProgram(m_plotProg[1]);
		glDeleteProgram(m_lineFromToProg[0]);
		glDeleteProgram(m_lineFromToProg[1]);

		for (int i = 1; i < c_maxSegments; i++)
		{
			glDeleteProgram(m_segmentsProg[i][0][0]);
			glDeleteProgram(m_segmentsProg[i][0][1]);
			glDeleteProgram(m_segmentsProg[i][1][0]);
			glDeleteProgram(m_segmentsProg[i][1][1]);
		}

		glDeleteTextures(1, &m_extrasBufferTex);
		glDeleteTextures(1, &m_segmentsTintTexId);

		glDeleteBuffers(1, &m_vertexBufferId);
		glDeleteBuffers(1, &m_extrasBufferId);

		glDeleteVertexArrays(1, &m_vertexArrayId);

		if( m_idleSync != 0 )
			glDeleteSync(m_idleSync);

		LOG_GLERROR(glGetError());
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GlGfxDevice::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ surfaceType() _______________________________________________________

	const TypeInfo& GlGfxDevice::surfaceType( void ) const
	{
		return GlSurface::TYPEINFO;
	}

	//____ surfaceFactory() ______________________________________________________

	SurfaceFactory_p GlGfxDevice::surfaceFactory()
	{
		if( !m_pSurfaceFactory )
			m_pSurfaceFactory = GlSurfaceFactory::create();

		return m_pSurfaceFactory;
	}

	//____ edgemapFactory() ______________________________________________________

	EdgemapFactory_p GlGfxDevice::edgemapFactory()
	{
		if (!m_pEdgemapFactory)
			m_pEdgemapFactory = GlEdgemapFactory::create();

		return m_pEdgemapFactory;
	}


	//____ setDefaultCanvas() ___________________________________________

	bool GlGfxDevice::setDefaultCanvas(SizeSPX size, int scale)
	{
		m_defaultCanvas.ref = CanvasRef::Default;		// Starts as Undefined until this method is called.
		m_defaultCanvas.size = size;
		m_defaultCanvas.scale = scale;
		return true;
	}

	//____ canvas() ___________________________________________________________

	const CanvasInfo GlGfxDevice::canvas(CanvasRef ref) const
	{
		if (ref == CanvasRef::Default)
			return m_defaultCanvas;
		else
		{
			//TODO: Error handling!
			return m_dummyCanvas;
		}
	}

	//____ _canvasWasChanged() ________________________________________________

	void GlGfxDevice::_canvasWasChanged()
	{

		_renderLayerWasChanged();
	}

	//____ _renderLayerWasChanged() ___________________________________________

	void GlGfxDevice::_renderLayerWasChanged()
	{
		Surface* pRenderSurface;

		bool bClear = false;
		if (m_renderLayer > 0 && m_layerSurfaces[m_renderLayer] == nullptr)
		{
			m_layerSurfaces[m_renderLayer] = GlSurface::create( WGBP(Surface,
																_.size = roundToPixels(m_canvas.size), 
																_.format = m_pCanvasLayers->layerFormat(m_renderLayer), 
																_.canvas = true ));
			bClear = true;
		}

		if (!m_canvas.pSurface && m_renderLayer == 0)
		{
			m_canvasYstart = roundToPixels(m_canvas.size.h);
			m_canvasYmul = -1;
			pRenderSurface = nullptr;
		}
		else
		{
			m_canvasYstart = 0;
			m_canvasYmul = 1;

			pRenderSurface = m_layerSurfaces[m_renderLayer];
		}

		m_tintInfo.flatTint[0] = m_tintColor.r / 4096.f;
		m_tintInfo.flatTint[1] = m_tintColor.g / 4096.f;
		m_tintInfo.flatTint[2] = m_tintColor.b / 4096.f;
		m_tintInfo.flatTint[3] = m_tintColor.a / 4096.f;

		m_tintInfo.tintRect = (RectF) roundToPixels(m_tintGradientRect);

		m_tintInfo.topLeftTint[0] = m_tintGradient.topLeft.r / 4096.f;
		m_tintInfo.topLeftTint[1] = m_tintGradient.topLeft.g / 4096.f;
		m_tintInfo.topLeftTint[2] = m_tintGradient.topLeft.b / 4096.f;
		m_tintInfo.topLeftTint[3] = m_tintGradient.topLeft.a / 4096.f;

		m_tintInfo.topRightTint[0] = m_tintGradient.topRight.r / 4096.f;
		m_tintInfo.topRightTint[1] = m_tintGradient.topRight.g / 4096.f;
		m_tintInfo.topRightTint[2] = m_tintGradient.topRight.b / 4096.f;
		m_tintInfo.topRightTint[3] = m_tintGradient.topRight.a / 4096.f;

		m_tintInfo.bottomRightTint[0] = m_tintGradient.bottomRight.r / 4096.f;
		m_tintInfo.bottomRightTint[1] = m_tintGradient.bottomRight.g / 4096.f;
		m_tintInfo.bottomRightTint[2] = m_tintGradient.bottomRight.b / 4096.f;
		m_tintInfo.bottomRightTint[3] = m_tintGradient.bottomRight.a / 4096.f;

		m_tintInfo.bottomLeftTint[0] = m_tintGradient.bottomLeft.r / 4096.f;
		m_tintInfo.bottomLeftTint[1] = m_tintGradient.bottomLeft.g / 4096.f;
		m_tintInfo.bottomLeftTint[2] = m_tintGradient.bottomLeft.b / 4096.f;
		m_tintInfo.bottomLeftTint[3] = m_tintGradient.bottomLeft.a / 4096.f;


		// Do we need to end command and execute buffer if we
		// put commands in the queue instead?

		_endCommand();
		_executeBuffer();

		//

		_setCanvas(static_cast<GlSurface*>(pRenderSurface), roundToPixels(m_canvas.size.w), roundToPixels(m_canvas.size.h));

//		if( m_blendMode != m_activeBlendMode)
			_setBlendMode(m_blendMode);

		_setMorphFactor(m_morphFactor);

		m_bGradientActive = m_bTintGradient;

		if( m_pBlitSource != m_pActiveBlitSource )
			_setBlitSource(static_cast<GlSurface*>(m_pBlitSource.rawPtr()));


	//	_endCommand();
	//	_beginStateCommand(Command::SetCanvas, 2);
	//	m_commandBuffer[m_commandOfs++] = m_canvasSize.w;
	//	m_commandBuffer[m_commandOfs++] = m_canvasSize.h;
	//	m_surfaceBuffer[m_surfaceOfs++] = (GlSurface*) pRenderSurface;

		if (bClear)
			_clearRenderLayer();
	}

	//____ _clipListWasChanged() ______________________________________________

	void GlGfxDevice::_clipListWasChanged()
	{
		m_clipCurrOfs = -1;
	}

	//____ setTintColor() __________________________________________________________________

	void GlGfxDevice::setTintColor(HiColor color)
	{
		if (color == m_tintColor)
			return;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return;
		}

		GfxDeviceGen1::setTintColor(color);

		m_tintInfo.flatTint[0] = color.r / 4096.f;
		m_tintInfo.flatTint[1] = color.g / 4096.f;
		m_tintInfo.flatTint[2] = color.b / 4096.f;
		m_tintInfo.flatTint[3] = color.a / 4096.f;

		_writeTintInfo();

	}

	//____ setTintGradient() __________________________________________________________________

	void GlGfxDevice::setTintGradient(const RectSPX& rect, const Gradient& gradient)
	{
		if (m_bTintGradient && gradient == m_tintGradient && rect == m_tintGradientRect)
			return;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return;
		}

		GfxDeviceGen1::setTintGradient(rect, gradient);

		m_tintInfo.tintRect = (RectF) roundToPixels(rect);

		m_tintInfo.topLeftTint[0] = gradient.topLeft.r / 4096.f;
		m_tintInfo.topLeftTint[1] = gradient.topLeft.g / 4096.f;
		m_tintInfo.topLeftTint[2] = gradient.topLeft.b / 4096.f;
		m_tintInfo.topLeftTint[3] = gradient.topLeft.a / 4096.f;

		m_tintInfo.topRightTint[0] = gradient.topRight.r / 4096.f;
		m_tintInfo.topRightTint[1] = gradient.topRight.g / 4096.f;
		m_tintInfo.topRightTint[2] = gradient.topRight.b / 4096.f;
		m_tintInfo.topRightTint[3] = gradient.topRight.a / 4096.f;

		m_tintInfo.bottomRightTint[0] = gradient.bottomRight.r / 4096.f;
		m_tintInfo.bottomRightTint[1] = gradient.bottomRight.g / 4096.f;
		m_tintInfo.bottomRightTint[2] = gradient.bottomRight.b / 4096.f;
		m_tintInfo.bottomRightTint[3] = gradient.bottomRight.a / 4096.f;

		m_tintInfo.bottomLeftTint[0] = gradient.bottomLeft.r / 4096.f;
		m_tintInfo.bottomLeftTint[1] = gradient.bottomLeft.g / 4096.f;
		m_tintInfo.bottomLeftTint[2] = gradient.bottomLeft.b / 4096.f;
		m_tintInfo.bottomLeftTint[3] = gradient.bottomLeft.a / 4096.f;

		_writeTintInfo();


		_endCommand();
		_beginStateCommand(Command::SetTintGradient, 0);
	}

	//____ clearTintGradient() __________________________________________________________________

	void GlGfxDevice::clearTintGradient()
	{
		if (m_bTintGradient == false)
			return;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return;
		}

		// We don't need to write any new TintInfo here,
		// flatTint variables contain same info and are in the right place already.

		GfxDeviceGen1::clearTintGradient();

		_endCommand();
		_beginStateCommand(Command::ClearTintGradient, 0);
	}


	//____ setBlendMode() __________________________________________________________________

	bool GlGfxDevice::setBlendMode(BlendMode blendMode)
	{
		if (blendMode == m_blendMode)
			return true;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return false;
		}

		GfxDeviceGen1::setBlendMode(blendMode);

		_endCommand();
		_beginStateCommand(Command::SetBlendMode, 1);
		m_commandBuffer[m_commandOfs++] = (int) blendMode;

		return true;
	}

	//____ setBlitSource() __________________________________________________________________

	bool GlGfxDevice::setBlitSource(Surface * pSource)
	{
		if (pSource == m_pBlitSource)
			return true;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return false;
		}

		if (!pSource || pSource->typeInfo() != GlSurface::TYPEINFO)
			return false;

		m_pBlitSource = pSource;
		m_blitSourceSize = CoordF(pSource->pixelSize().w, pSource->pixelSize().h);

        //TODO: Check so that we don't overrun m_surfaceBuffer;

		_endCommand();
		_beginStateCommand(Command::SetBlitSource, 0);
		m_surfaceBuffer[m_surfaceOfs++] = static_cast<GlSurface*>(pSource);

		return true;
	}

	//____ setMorphFactor() ______________________________________________________

	void GlGfxDevice::setMorphFactor(float factor)
	{
		if (factor == m_morphFactor)
			return;

		if (!m_bRendering)
		{
			//TODO: Error handling!
			return;
		}

		limit(factor, 0.f, 1.f);

		m_morphFactor = factor;

		_endCommand();
		_beginStateCommand(Command::SetMorphFactor, 1);
		m_commandBuffer[m_commandOfs++] = (int)(factor*1024);
	}

	//____ setBlurbrush() _____________________________________________________

	void GlGfxDevice::setBlurbrush(Blurbrush * pBrush)
	{
		GfxDeviceGen1::setBlurbrush(pBrush);
		_endCommand();
		_beginStateCommand(Command::SetBlurMatrices, 28);

		m_commandBuffer[m_commandOfs++] = pBrush->size();

		// Copy floats as they are

		memcpy(m_commandBuffer + m_commandOfs, pBrush->red(), sizeof(float) * 9);
		m_commandOfs += 9;
		memcpy(m_commandBuffer + m_commandOfs, pBrush->green(), sizeof(float) * 9);
		m_commandOfs += 9;
		memcpy(m_commandBuffer + m_commandOfs, pBrush->blue(), sizeof(float) * 9);
		m_commandOfs += 9;
	}


	//____ isCanvasReady() ___________________________________________________________

	bool GlGfxDevice::isCanvasReady() const
	{
		return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	//____ beginRender() ___________________________________________________________

	bool GlGfxDevice::beginRender()
	{
		LOG_GLERROR(glGetError());

		if( m_bRendering == true )
			return false;

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			return false;

		// Remember GL states so we can restore in EndRender()

		m_glDepthTest 		= glIsEnabled(GL_DEPTH_TEST);
		m_glScissorTest 	= glIsEnabled(GL_SCISSOR_TEST);
		m_glBlendEnabled  	= glIsEnabled(GL_BLEND);
		m_glSRGBEnabled		= glIsEnabled(GL_FRAMEBUFFER_SRGB);
		glGetIntegerv(GL_BLEND_SRC, &m_glBlendSrc);
		glGetIntegerv(GL_BLEND_DST, &m_glBlendDst);
		glGetIntegerv(GL_VIEWPORT, m_glViewport);
		glGetIntegerv(GL_SCISSOR_BOX, m_glScissorBox);
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &m_glReadFrameBuffer);
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_glDrawFrameBuffer);

		//  Modify states

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_SCISSOR_TEST);

		// If there already is an active device, that needs to be flushed before we
		// take over the role as the active device.

		if (s_pActiveDevice)
			s_pActiveDevice->flush();

		m_pPrevActiveDevice = s_pActiveDevice;
		s_pActiveDevice = this;

		//

		m_bRendering = true;
		m_cmd = Command::None;
		m_pCmdFinalizer = &GlGfxDevice::_dummyFinalizer;
		m_vertexOfs = 0;
		m_extrasOfs = 0;
		m_commandOfs = 0;
		m_surfaceOfs = 0;
		m_segmentsTintTexOfs = 0;
		m_clipWriteOfs = 0;
		m_clipCurrOfs = -1;

		m_pBlitSource = nullptr;
		
		// Set our extras buffer and segments palette textures.

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_BUFFER, m_extrasBufferTex);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_segmentsTintTexId);
		glActiveTexture(GL_TEXTURE0);


		// Prepare for rendering

		glBindVertexArray(m_vertexArrayId);


//		glFinish();  //TODO: Remove.

		//

		LOG_GLERROR(glGetError());
		return true;
	}

	//____ endRender() _____________________________________________________________

	bool GlGfxDevice::endRender()
	{
		LOG_GLERROR(glGetError());
		if( m_bRendering == false )
			return false;

		// Finalize any ongoing operation

		_endCommand();
		_executeBuffer();

		//

 		//glFinish(); //TODO: Remove.

		// Restore render states from before beginRender()

		if( m_glDepthTest )
			glEnable(GL_DEPTH_TEST);

		if( !m_glScissorTest )
			glDisable(GL_SCISSOR_TEST);

		if( m_glBlendEnabled )
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);

		if( m_glSRGBEnabled )
			glEnable(GL_FRAMEBUFFER_SRGB);
		else
			glDisable(GL_FRAMEBUFFER_SRGB);

		glBlendFunc( m_glBlendSrc, m_glBlendDst );
		glViewport(m_glViewport[0], m_glViewport[1], m_glViewport[2], m_glViewport[3]);
		glScissor(m_glScissorBox[0], m_glScissorBox[1], m_glScissorBox[2], m_glScissorBox[3]);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_glReadFrameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_glDrawFrameBuffer);

		glBindVertexArray(0);

		//TODO: Reenable

		//if( m_idleSync != 0 )
		//	glDeleteSync(m_idleSync);

		//m_idleSync = glFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );

		glFlush();

//

		LOG_GLERROR(glGetError());
		m_bRendering = false;

		// Restore previously active device and exit

		s_pActiveDevice = m_pPrevActiveDevice;
		return true;
	}

	//____ isIdle() _____________________________________________________________________

	bool GlGfxDevice::isIdle()
	{
		if( m_bRendering )
			return false;

		//TODO: Reenable
/*
		if (m_idleSync == 0)
			return true;

		GLint isSignaled = 0;
		glGetSynciv(m_idleSync, GL_SYNC_STATUS, 1, NULL, &isSignaled);

		if(isSignaled == GL_UNSIGNALED)
			return false;

		glDeleteSync(m_idleSync);
		m_idleSync = 0;
*/
		return true;
	}

	//____ flush() _______________________________________________________________

	void GlGfxDevice::flush()
	{
		LOG_GLERROR(glGetError());

		// Finalize any ongoing operation

		_endCommand();
		_executeBuffer();

		//

		glFlush();

		LOG_GLERROR(glGetError());
	}



	//____ fill() ____ [standard] __________________________________________________

	void GlGfxDevice::fill(const RectSPX& rect, HiColor col)
	{
		// Skip calls that won't affect destination

		if (col.a == 0 && (m_blendMode == BlendMode::Blend))
			return;

		if (!rect.isOverlapping(m_clipBounds))
			return;

		//

		if (((rect.x | rect.y | rect.w | rect.h) & 0x3F) == 0)
		{
			// No subpixel precision, make it quick and easy

			if (m_vertexOfs > c_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > c_extrasBufferSize - 4)
			{
				_endCommand();
				_executeBuffer();
				_beginDrawCommand(Command::Fill);
			}
			else if (m_cmd != Command::Fill)
			{
				_endCommand();
				_beginDrawCommand(Command::Fill);
			}

			for (int i = 0; i < m_nClipRects; i++)
			{
				RectI patch = roundToPixels(RectSPX::overlap(m_pClipRects[i], rect));
				if (patch.w > 0 && patch.h > 0)
				{
					int	dx1 = patch.x;
					int	dy1 = patch.y;
					int dx2 = patch.x + patch.w;
					int dy2 = patch.y + patch.h;

					m_vertexBufferData[m_vertexOfs].coord.x = dx1;
					m_vertexBufferData[m_vertexOfs].coord.y = dy1;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					m_vertexBufferData[m_vertexOfs].coord.x = dx2;
					m_vertexBufferData[m_vertexOfs].coord.y = dy1;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					m_vertexBufferData[m_vertexOfs].coord.x = dx2;
					m_vertexBufferData[m_vertexOfs].coord.y = dy2;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					m_vertexBufferData[m_vertexOfs].coord.x = dx1;
					m_vertexBufferData[m_vertexOfs].coord.y = dy1;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					m_vertexBufferData[m_vertexOfs].coord.x = dx2;
					m_vertexBufferData[m_vertexOfs].coord.y = dy2;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					m_vertexBufferData[m_vertexOfs].coord.x = dx1;
					m_vertexBufferData[m_vertexOfs].coord.y = dy2;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;
				}
			}

			m_extrasBufferData[m_extrasOfs++] = col.r / 4096.f;
			m_extrasBufferData[m_extrasOfs++] = col.g / 4096.f;
			m_extrasBufferData[m_extrasOfs++] = col.b / 4096.f;
			m_extrasBufferData[m_extrasOfs++] = col.a / 4096.f;
		}
		else
		{
			// We have subpixel precision

			// Create our outer rectangle

			RectI outerRect(rect.x >> 6, rect.y >> 6, ((rect.x + rect.w + 63) >> 6) - (rect.x >> 6), ((rect.y + rect.h + 63) >> 6) - (rect.y >> 6));

			//

			if (m_vertexOfs > c_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > c_extrasBufferSize - 8)
			{
				_endCommand();
				_executeBuffer();
				_beginDrawCommand(Command::FillSubPixel);
			}
			else if (m_cmd != Command::FillSubPixel)
			{
				_endCommand();
				_beginDrawCommand(Command::FillSubPixel);
			}

			for (int i = 0; i < m_nClipRects; i++)
			{
				RectI patch = RectI::overlap(roundToPixels(m_pClipRects[i]), outerRect);
				if (patch.w > 0 && patch.h > 0)
				{
					int	dx1 = patch.x;
					int	dy1 = patch.y;
					int dx2 = patch.x + patch.w;
					int dy2 = patch.y + patch.h;

					m_vertexBufferData[m_vertexOfs].coord.x = dx1;
					m_vertexBufferData[m_vertexOfs].coord.y = dy1;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					m_vertexBufferData[m_vertexOfs].coord.x = dx2;
					m_vertexBufferData[m_vertexOfs].coord.y = dy1;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					m_vertexBufferData[m_vertexOfs].coord.x = dx2;
					m_vertexBufferData[m_vertexOfs].coord.y = dy2;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					m_vertexBufferData[m_vertexOfs].coord.x = dx1;
					m_vertexBufferData[m_vertexOfs].coord.y = dy1;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					m_vertexBufferData[m_vertexOfs].coord.x = dx2;
					m_vertexBufferData[m_vertexOfs].coord.y = dy2;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					m_vertexBufferData[m_vertexOfs].coord.x = dx1;
					m_vertexBufferData[m_vertexOfs].coord.y = dy2;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;
				}
			}

			// Provide color	

			m_extrasBufferData[m_extrasOfs++] = col.r / 4096.f;
			m_extrasBufferData[m_extrasOfs++] = col.g / 4096.f;
			m_extrasBufferData[m_extrasOfs++] = col.b / 4096.f;
			m_extrasBufferData[m_extrasOfs++] = col.a / 4096.f;

			// Provide rectangle center and radius

			SizeF    radius(rect.w / (2.f * 64), rect.h / (2.f * 64));
			CoordF    center((rect.x / 64.f) + radius.w, (rect.y / 64.f) + radius.h);

			m_extrasBufferData[m_extrasOfs++] = center.x;
			m_extrasBufferData[m_extrasOfs++] = center.y;
			m_extrasBufferData[m_extrasOfs++] = radius.w;
			m_extrasBufferData[m_extrasOfs++] = radius.h;
		}
	}

	//____ plotPixels() ______________________________________________________

	void GlGfxDevice::plotPixels(int nPixels, const CoordSPX * pCoords, const HiColor * pColors)
	{
		if (nPixels == 0)
			return;

		if (m_vertexOfs > c_vertexBufferSize - 1 || m_extrasOfs > c_extrasBufferSize - 4)
		{
			_endCommand();
			_executeBuffer();
			_beginDrawCommand(Command::Plot);
		}
		if (m_cmd != Command::Plot)
		{
			_endCommand();
			_beginDrawCommand(Command::Plot);
		}

		float* pConv = GfxBase::defaultToSRGB() ? m_sRGBtoLinearTable : m_linearToLinearTable;

		for (int i = 0; i < m_nClipRects; i++)
		{
			const RectI& clip = m_pClipRects[i];
			for (int pixel = 0; pixel < nPixels; pixel++)
			{
				if (clip.contains(pCoords[pixel]))
				{
					m_vertexBufferData[m_vertexOfs].coord = pCoords[pixel] / 64;
					m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
					m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
					m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
					m_vertexOfs++;

					HiColor color = pColors[pixel];

					m_extrasBufferData[m_extrasOfs++] = color.r / 4096.f;
					m_extrasBufferData[m_extrasOfs++] = color.g / 4096.f;
					m_extrasBufferData[m_extrasOfs++] = color.b / 4096.f;
					m_extrasBufferData[m_extrasOfs++] = color.a / 4096.f;

					if (m_vertexOfs == c_vertexBufferSize || m_extrasOfs == c_extrasBufferSize)
					{
						_endCommand();
						_executeBuffer();
						_beginDrawCommand(Command::Plot);
					}
				}
			}
		}
	}

	//____ drawLine() ____ [from/to] __________________________________________________

	void GlGfxDevice::drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx _thickness)
	{
		float thickness = _thickness / 64.f;

		//TODO: Proper 26:6 support
		begin = roundToPixels(begin);
		end = roundToPixels(end);

		if (m_vertexOfs > c_vertexBufferSize - 6 || m_extrasOfs > c_extrasBufferSize - 8 || m_clipCurrOfs == -1 )
		{
			_endCommand();
			_executeBuffer();
			_beginClippedDrawCommand(Command::LineFromTo);
		}
		else if (m_cmd != Command::LineFromTo)
		{
			_endCommand();
			_beginClippedDrawCommand(Command::LineFromTo);
		}

		int 	length;
		float   width;

		float	slope;
		float	s, w;
		bool	bSteep;

		CoordI	c1, c2, c3, c4;

		if (std::abs(begin.x - end.x) > std::abs(begin.y - end.y))
		{
			// Prepare mainly horizontal line segment

			if (begin.x > end.x)
				swap(begin, end);

			length = end.x - begin.x;
			if (length == 0)
				return;											// TODO: Should stil draw the caps!

			slope = ((float)(end.y - begin.y)) / length;
			width = _scaleThickness(thickness, slope);
			bSteep = false;

			s = m_canvasYstart + m_canvasYmul * ((begin.y + 0.5f) - (begin.x + 0.5f)*slope);
			w =  width / 2 + 0.5f;

			float   y1 = begin.y - width / 2;
			float   y2 = end.y - width / 2;

			c1.x = begin.x;
			c1.y = int(y1) - 1;
			c2.x = end.x;
			c2.y = int(y2) - 1;
			c3.x = end.x;
			c3.y = int(y2 + width) + 2;
			c4.x = begin.x;
			c4.y = int(y1 + width) + 2;
		}
		else
		{
			// Prepare mainly vertical line segment

			if (begin.y > end.y)
				swap(begin, end);

			length = end.y - begin.y;
			if (length == 0)
				return;											// TODO: Should stil draw the caps!

			slope = ((float)(end.x - begin.x)) / length;
			width = _scaleThickness(thickness, slope);
			bSteep = true;

//			s = (begin.x + 0.5f) - (m_canvasYstart + m_canvasYmul * (begin.y + 0.5f))*slope*m_canvasYmul;
			s = (begin.x + 0.5f) - (m_canvasYstart*m_canvasYmul + (begin.y + 0.5f))*slope;
			w = width / 2 + 0.5f;

			float   x1 = begin.x - width / 2;
			float   x2 = end.x - width / 2;

			c1.x = int(x1) - 1;
			c1.y = begin.y;
			c2.x = int(x1 + width) + 2;
			c2.y = begin.y;
			c3.x = int(x2 + width) + 2;
			c3.y = end.y;
			c4.x = int(x2) - 1;
			c4.y = end.y;


		}

		m_vertexBufferData[m_vertexOfs].coord = c1;
		m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
		m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
		m_vertexOfs++;

		m_vertexBufferData[m_vertexOfs].coord = c2;
		m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
		m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
		m_vertexOfs++;

		m_vertexBufferData[m_vertexOfs].coord = c3;
		m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
		m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
		m_vertexOfs++;

		m_vertexBufferData[m_vertexOfs].coord = c1;
		m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
		m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
		m_vertexOfs++;

		m_vertexBufferData[m_vertexOfs].coord = c3;
		m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
		m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
		m_vertexOfs++;

		m_vertexBufferData[m_vertexOfs].coord = c4;
		m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs/4;
		m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
		m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
		m_vertexOfs++;

		m_extrasBufferData[m_extrasOfs++] = color.r / 4096.f;
		m_extrasBufferData[m_extrasOfs++] = color.g / 4096.f;
		m_extrasBufferData[m_extrasOfs++] = color.b / 4096.f;
		m_extrasBufferData[m_extrasOfs++] = color.a / 4096.f;

		m_extrasBufferData[m_extrasOfs++] = s;
		m_extrasBufferData[m_extrasOfs++] = w;
		m_extrasBufferData[m_extrasOfs++] = slope;
		m_extrasBufferData[m_extrasOfs++] = bSteep;
	}

	//____ drawLine() ____ [start/direction] __________________________________________________

	void GlGfxDevice::drawLine(CoordSPX begin, Direction dir, spx length, HiColor color, spx _thickness)
	{
		float thickness = _thickness / 64.f;

		//TODO: Proper 26:6 support
		begin = roundToPixels(begin);
		length = roundToPixels(length);

		// Skip calls that won't affect destination

		if (color.a == 0 && (m_blendMode == BlendMode::Blend))
			return;

		// Create a rectangle from the line

		RectF rect;

		switch (dir)
		{
			case Direction::Up:
				rect.x = begin.x + 0.5f - thickness/2;
				rect.y = float(begin.y - length);
				rect.w = thickness;
				rect.h = float(length);
				break;

			case Direction::Down:
				rect.x = begin.x + 0.5f - thickness/2;
				rect.y = float(begin.y);
				rect.w = thickness;
				rect.h = float(length);
				break;

			case Direction::Left:
				rect.x = float(begin.x - length);
				rect.y = begin.y + 0.5f - thickness/2;
				rect.w = float(length);
				rect.h = thickness;
				break;

			case Direction::Right:
				rect.x = float(begin.x);
				rect.y = begin.y + 0.5f - thickness/2;
				rect.w = float(length);
				rect.h = thickness;
				break;
		}


		// Create our outer rectangle

		RectI outerRect((int)rect.x, (int)rect.y, ((int)(rect.x + rect.w + 0.999f)) - (int)rect.x, ((int)(rect.y + rect.h + 0.999f)) - (int)rect.y);

		// Clip our rectangle

		if (!outerRect.isOverlapping(m_clipBounds/64))
			return;

		//

		if (m_vertexOfs > c_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > c_extrasBufferSize - 8)
		{
			_endCommand();
			_executeBuffer();
			_beginDrawCommand(Command::FillSubPixel);
		}
		else if (m_cmd != Command::FillSubPixel)
		{
			_endCommand();
			_beginDrawCommand(Command::FillSubPixel);
		}


		// Provide the patches

		for (int i = 0; i < m_nClipRects; i++)
		{
			RectI patch = RectI::overlap(m_pClipRects[i]/64, outerRect);
			if (patch.w > 0 && patch.h > 0)
			{
				int	dx1 = patch.x;
				int	dy1 = patch.y;
				int dx2 = patch.x + patch.w;
				int dy2 = patch.y + patch.h;

				m_vertexBufferData[m_vertexOfs].coord.x = dx1;
				m_vertexBufferData[m_vertexOfs].coord.y = dy1;
				m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
				m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
				m_vertexOfs++;

				m_vertexBufferData[m_vertexOfs].coord.x = dx2;
				m_vertexBufferData[m_vertexOfs].coord.y = dy1;
				m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
				m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
				m_vertexOfs++;

				m_vertexBufferData[m_vertexOfs].coord.x = dx2;
				m_vertexBufferData[m_vertexOfs].coord.y = dy2;
				m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
				m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
				m_vertexOfs++;

				m_vertexBufferData[m_vertexOfs].coord.x = dx1;
				m_vertexBufferData[m_vertexOfs].coord.y = dy1;
				m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
				m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
				m_vertexOfs++;

				m_vertexBufferData[m_vertexOfs].coord.x = dx2;
				m_vertexBufferData[m_vertexOfs].coord.y = dy2;
				m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
				m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
				m_vertexOfs++;

				m_vertexBufferData[m_vertexOfs].coord.x = dx1;
				m_vertexBufferData[m_vertexOfs].coord.y = dy2;
				m_vertexBufferData[m_vertexOfs].extrasOfs = m_extrasOfs / 4;
				m_vertexBufferData[m_vertexOfs].canvasInfoOfs = m_canvasInfoOfs / 4;
				m_vertexBufferData[m_vertexOfs].tintInfoOfs = m_tintInfoOfs / 4;
				m_vertexOfs++;
			}
		}

		// Provide color

		m_extrasBufferData[m_extrasOfs++] = color.r / 4096.f;
		m_extrasBufferData[m_extrasOfs++] = color.g / 4096.f;
		m_extrasBufferData[m_extrasOfs++] = color.b / 4096.f;
		m_extrasBufferData[m_extrasOfs++] = color.a / 4096.f;

		// Provide rectangle center and raidus.

		SizeF	radius(rect.w / 2, rect.h / 2);
		CoordF	center(rect.x + radius.w, rect.y + radius.h);

		m_extrasBufferData[m_extrasOfs++] = center.x;
		m_extrasBufferData[m_extrasOfs++] = center.y;
		m_extrasBufferData[m_extrasOfs++] = radius.w;
		m_extrasBufferData[m_extrasOfs++] = radius.h;
	}


	//____ drawEdgemap() __________________________________________________________

	void GlGfxDevice::drawEdgemap(CoordSPX dest, Edgemap* pEdgemap)
	{
		if (pEdgemap->typeInfo() != GlEdgemap::TYPEINFO)
		{
			//TODO: Throw an error.
			return;
		}

		auto pWave = static_cast<GlEdgemap*>(pEdgemap);


		_transformDrawSegments({ dest, pWave->m_size * 64 }, pWave->m_nbRenderSegments, pWave->m_pRenderColors,
			pWave->m_size.w + 1, pWave->m_pSamples, pWave->m_nbSegments - 1, pWave->m_tintMode,
			s_blitFlipTransforms[(int)GfxFlip::None]);
	}

	//____ flipDrawEdgemap() __________________________________________________________

	void GlGfxDevice::flipDrawEdgemap(CoordSPX destPos, Edgemap* pEdgemap, GfxFlip flip)
	{
		if (pEdgemap->typeInfo() != GlEdgemap::TYPEINFO)
		{
			//TODO: Throw an error.
			return;
		}

		auto pWave = static_cast<GlEdgemap*>(pEdgemap);

		const int(&transform)[2][2] = s_blitFlipTransforms[(int)flip];

		RectSPX dest;
		dest.x = destPos.x;
		dest.y = destPos.y;
		dest.w = pWave->m_size.w * 64 * abs(transform[0][0]) + pWave->m_size.h * 64 * abs(transform[1][0]);
		dest.h = pWave->m_size.w * 64 * abs(transform[0][1]) + pWave->m_size.h * 64 * abs(transform[1][1]);

		_transformDrawSegments(dest, pWave->m_nbRenderSegments, pWave->m_pRenderColors,
			pWave->m_size.w + 1, pWave->m_pSamples, pWave->m_nbSegments - 1, pWave->m_tintMode,
			transform);
	}

	//____ _transformBlitSimple() ______________________________________________________

	void GlGfxDevice::_transformBlitSimple(const RectSPX& _dest, CoordSPX src, const int simpleTransform[2][2], OpType type)
	{
		if (m_pBlitSource == nullptr)
			return;

		if (!_dest.isOverlapping(m_clipBounds))
			return;

		if (m_vertexOfs > c_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > c_extrasBufferSize - 8 )
		{
			_endCommand();
			_executeBuffer();
			if( type == OpType::Blur)
				_beginDrawCommandWithSource(Command::Blur);
			else
				_beginDrawCommandWithSource(Command::Blit);
		}
		else if (type == OpType::Blur && m_cmd != Command::Blur)
		{
			_endCommand();
			_beginDrawCommandWithSource(Command::Blur);
		}
		else if (m_cmd != Command::Blit)
		{
			_endCommand();
			_beginDrawCommandWithSource(Command::Blit);
		}

		//TODO: Proper 26:6 support
		RectI dest = roundToPixels(_dest);

		for (int i = 0; i < m_nClipRects; i++)
		{
			RectI patch = RectI::overlap(m_pClipRects[i]/64, dest);
			if (patch.w > 0 && patch.h > 0)
			{
				int		dx1 = patch.x;
				int		dx2 = patch.x + patch.w;
				int		dy1 = patch.y;
				int		dy2 = patch.y + patch.h;

				Vertex * pVertex = m_vertexBufferData + m_vertexOfs;

/*
				float	sx1 = (src.x + (patch.x - dest.x) * simpleTransform[0][0] + (patch.y - dest.y) * simpleTransform[1][0]) / sw;
				float	sy1 = (src.y + (patch.x - dest.x) * simpleTransform[0][1] + (patch.y - dest.y) * simpleTransform[1][1]) / sh;

				float	sx2 = (src.x + (patch.x + patch.w - dest.x) * simpleTransform[0][0] + (patch.y - dest.y) * simpleTransform[1][0]) / sw;
				float	sy2 = (src.y + (patch.x + patch.w - dest.x) * simpleTransform[0][1] + (patch.y - dest.y) * simpleTransform[1][1]) / sh;

				float	sx3 = (src.x + (patch.x + patch.w - dest.x) * simpleTransform[0][0] + (patch.y + patch.h - dest.y) * simpleTransform[1][0]) / sw;
				float	sy3 = (src.y + (patch.x + patch.w - dest.x) * simpleTransform[0][1] + (patch.y + patch.h - dest.y) * simpleTransform[1][1]) / sh;

				float	sx4 = (src.x + (patch.x - dest.x) * simpleTransform[0][0] + (patch.y + patch.h - dest.y) * simpleTransform[1][0]) / sw;
				float	sy4 = (src.y + (patch.x - dest.x) * simpleTransform[0][1] + (patch.y + patch.h - dest.y) * simpleTransform[1][1]) / sh;
*/

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs/4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				m_vertexOfs += 6;
			}
		}

		m_extrasBufferData[m_extrasOfs++] = (GLfloat) roundToPixels(src.x);
		m_extrasBufferData[m_extrasOfs++] = (GLfloat) roundToPixels(src.y);
		m_extrasBufferData[m_extrasOfs++] = (GLfloat) dest.x;
		m_extrasBufferData[m_extrasOfs++] = (GLfloat) dest.y;

		m_extrasBufferData[m_extrasOfs++] = (GLfloat) simpleTransform[0][0];
		m_extrasBufferData[m_extrasOfs++] = (GLfloat) simpleTransform[0][1];
		m_extrasBufferData[m_extrasOfs++] = (GLfloat) simpleTransform[1][0];
		m_extrasBufferData[m_extrasOfs++] = (GLfloat) simpleTransform[1][1];
	}

	//____ _transformBlitComplex() ______________________________________________________

	void GlGfxDevice::_transformBlitComplex(const RectSPX& _dest, BinalCoord src, const binalInt complexTransform[2][2], OpType type)
	{
		if (m_pBlitSource == nullptr)
			return;

		if (!_dest.isOverlapping(m_clipBounds))
			return;

		if (m_vertexOfs > c_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > c_extrasBufferSize - 8)
		{
			_endCommand();
			_executeBuffer();
			if (type == OpType::Blur)
				_beginDrawCommandWithSource(Command::Blur);
			else
				_beginDrawCommandWithSource(Command::Blit);
		}
		else if (type == OpType::Blur && m_cmd != Command::Blur)
		{
			_endCommand();
			_beginDrawCommandWithSource(Command::Blur);
		}
		else if (m_cmd != Command::Blit)
		{
			_endCommand();
			_beginDrawCommandWithSource(Command::Blit);
		}

		//TODO: Proper 26:6 support
		RectI dest = roundToPixels(_dest);

		//

		for (int i = 0; i < m_nClipRects; i++)
		{
			RectI patch = RectI::overlap(m_pClipRects[i]/64, dest);
			if (patch.w > 0 && patch.h > 0)
			{
				Vertex * pVertex = m_vertexBufferData + m_vertexOfs;

				int		dx1 = patch.x;
				int		dx2 = patch.x + patch.w;
				int		dy1 = patch.y;
				int		dy2 = patch.y + patch.h;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = m_blitSourceSize;
				pVertex++;

				m_vertexOfs += 6;
			}
		}
		
		if (m_pBlitSource->sampleMethod() == SampleMethod::Bilinear)
		{
			m_extrasBufferData[m_extrasOfs++] = GLfloat(src.x)/BINAL_MUL + 0.5f;
			m_extrasBufferData[m_extrasOfs++] = GLfloat(src.y)/BINAL_MUL + 0.5f;
			m_extrasBufferData[m_extrasOfs++] = GLfloat(dest.x) + 0.5f;
			m_extrasBufferData[m_extrasOfs++] = GLfloat(dest.y) + 0.5f;
		}
		else
		{
			m_extrasBufferData[m_extrasOfs++] = GLfloat(src.x)/BINAL_MUL;
			m_extrasBufferData[m_extrasOfs++] = GLfloat(src.y)/BINAL_MUL;
			m_extrasBufferData[m_extrasOfs++] = GLfloat(dest.x) +0.5f;
			m_extrasBufferData[m_extrasOfs++] = GLfloat(dest.y) +0.5f;
		}

		m_extrasBufferData[m_extrasOfs++] = GLfloat(complexTransform[0][0])/BINAL_MUL;
		m_extrasBufferData[m_extrasOfs++] = GLfloat(complexTransform[0][1])/BINAL_MUL;
		m_extrasBufferData[m_extrasOfs++] = GLfloat(complexTransform[1][0])/BINAL_MUL;
		m_extrasBufferData[m_extrasOfs++] = GLfloat(complexTransform[1][1])/BINAL_MUL;
	}

	//____ _transformDrawSegments() ______________________________________________________

	void GlGfxDevice::_transformDrawSegments( const RectSPX& _destIn, int nSegments, const HiColor * pSegmentColors, int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode, const int simpleTransform[2][2] )
	{
		if (!_destIn.isOverlapping(m_clipBounds))
			return;

		//TODO: Proper 26:6 support
		RectI _dest = roundToPixels(_destIn);

		//

		int extrasSpaceNeeded = 8 + 4 * (nEdgeStrips - 1)*(nSegments - 1);		// Various data + colors + strips

		assert( extrasSpaceNeeded <= c_extrasBufferSize );               // EXTRAS BUFFER IS SET TOO SMALL!

		if (m_vertexOfs > c_vertexBufferSize - 6 * m_nClipRects || m_extrasOfs > c_extrasBufferSize - extrasSpaceNeeded || m_segmentsTintTexOfs == c_segmentsTintTexMapSize)			// varios data, transform , colors, edgestrips
		{
			m_nSegments = nSegments;
			_endCommand();
			_executeBuffer();
			_beginDrawCommandWithInt(Command::Segments, m_nSegments);

		}
		else if (m_cmd != Command::Segments || m_nSegments != nSegments )
		{
			m_nSegments = nSegments;

			_endCommand();
			_beginDrawCommandWithInt(Command::Segments, m_nSegments);
		}

		// Do transformations

		RectI dest = _dest;

		int uIncX = simpleTransform[0][0];
		int vIncX = simpleTransform[0][1];
		int uIncY = simpleTransform[1][0];
		int vIncY = simpleTransform[1][1];

		// Possibly clip the destination rectangle if we have space for more columns than we have

		int maxCol = (nEdgeStrips - 1);
		if (uIncX != 0)								// Columns are aligned horizontally
		{
			if (dest.w > maxCol)
			{
				if (uIncX < 0)
					dest.x += dest.w - maxCol;
				dest.w = maxCol;
			}
		}
		else										// Columns are aligned vertically
		{
			if (dest.h > maxCol)
			{
				if (uIncY < 0)
					dest.y += dest.h - maxCol;
				dest.h = maxCol;
			}
		}

		// Calc topLeft UV values

		int uTopLeft = 0;
		int vTopLeft = 0;

		if (uIncX + uIncY < 0)
			uTopLeft = maxCol;

		if (vIncX < 0)
			vTopLeft = dest.w;
		else if (vIncY < 0)
			vTopLeft = dest.h;

		// Setup vertices

		for (int i = 0; i < m_nClipRects; i++)
		{
			RectI patch = RectI::overlap(m_pClipRects[i]/64, dest);
			if (patch.w > 0 && patch.h > 0)
			{
				Vertex * pVertex = m_vertexBufferData + m_vertexOfs;

				int		dx1 = patch.x;
				int		dx2 = patch.x + patch.w;
				int		dy1 = patch.y;
				int		dy2 = patch.y + patch.h;

				// Calc UV-coordinates. U is edge offset, V is pixel offset from begin in column.

				float	u1 = (float) (uTopLeft + (patch.x - dest.x) * simpleTransform[0][0] + (patch.y - dest.y) * simpleTransform[1][0]);
				float	v1 = (float) (vTopLeft + (patch.x - dest.x) * simpleTransform[0][1] + (patch.y - dest.y) * simpleTransform[1][1]);

				float	u2 = (float) (uTopLeft + (patch.x + patch.w - dest.x) * simpleTransform[0][0] + (patch.y - dest.y) * simpleTransform[1][0]);
				float	v2 = (float) (vTopLeft + (patch.x + patch.w - dest.x) * simpleTransform[0][1] + (patch.y - dest.y) * simpleTransform[1][1]);

				float	u3 = (float) (uTopLeft + (patch.x + patch.w - dest.x) * simpleTransform[0][0] + (patch.y + patch.h - dest.y) * simpleTransform[1][0]);
				float	v3 = (float) (vTopLeft + (patch.x + patch.w - dest.x) * simpleTransform[0][1] + (patch.y + patch.h - dest.y) * simpleTransform[1][1]);

				float	u4 = (float) (uTopLeft + (patch.x - dest.x) * simpleTransform[0][0] + (patch.y + patch.h - dest.y) * simpleTransform[1][0]);
				float	v4 = (float) (vTopLeft + (patch.x - dest.x) * simpleTransform[0][1] + (patch.y + patch.h - dest.y) * simpleTransform[1][1]);

				CoordF	uv1 = { u1, v1 - 0.5f };
				CoordF	uv2 = { u2, v2 - 0.5f };
				CoordF	uv3 = { u3, v3 - 0.5f };
				CoordF	uv4 = { u4, v4 - 0.5f };


				//

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = uv1;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = uv2;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = uv3;
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy1;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = uv1;
				pVertex++;

				pVertex->coord.x = dx2;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = uv3;
				pVertex++;

				pVertex->coord.x = dx1;
				pVertex->coord.y = dy2;
				pVertex->extrasOfs = m_extrasOfs / 4;
				pVertex->canvasInfoOfs = m_canvasInfoOfs / 4;
				pVertex->tintInfoOfs = m_tintInfoOfs / 4;
				pVertex->uv = uv4;
				pVertex++;

				m_vertexOfs += 6;
			}
		}

		// Setup extras data

		GLfloat * pExtras = m_extrasBufferData + m_extrasOfs;

		// Add various data to extras

		int edgeStripOfs = (m_extrasOfs + 8);	// Offset for edgestrips in buffer.

		pExtras[0] = (GLfloat) nSegments;
		pExtras[1] = (GLfloat) edgeStripOfs/4;
		pExtras[2] = (GLfloat)((_dest.w) * abs(simpleTransform[0][0]) + (_dest.h) * abs(simpleTransform[1][0]));
		pExtras[3] = (GLfloat)((_dest.w) * abs(simpleTransform[0][1]) + (_dest.h) * abs(simpleTransform[1][1]));
		pExtras[4] = GLfloat( 0.25f/c_maxSegments );
		pExtras[5] = GLfloat(m_segmentsTintTexOfs + 0.25f) / c_segmentsTintTexMapSize;
		pExtras[6] = GLfloat(c_maxSegments*2);
		pExtras[7] = GLfloat(c_segmentsTintTexMapSize*2);

		pExtras += 8;												// Alignment for vec4 reads.

		// Add colors to segmentsTintTexMap

		const HiColor* pSegCol = pSegmentColors;

		uint16_t* pMapRow = m_segmentsTintTexMap[m_segmentsTintTexOfs];
		int			mapPitch = c_maxSegments * 4 * 2;

		switch (tintMode)
		{
			case TintMode::None:
			case TintMode::Flat:
			{
				for (int i = 0; i < nSegments; i++)
				{
					uint16_t r = uint16_t(int(pSegCol->r) * 65535 / 4096);
					uint16_t g = uint16_t(int(pSegCol->g) * 65535 / 4096);
					uint16_t b = uint16_t(int(pSegCol->b) * 65535 / 4096);
					uint16_t a = uint16_t(int(pSegCol->a) * 65535 / 4096);

					pMapRow[i * 8 + 0] = b;
					pMapRow[i * 8 + 1] = g;
					pMapRow[i * 8 + 2] = r;
					pMapRow[i * 8 + 3] = a;
					pMapRow[i * 8 + 4] = b;
					pMapRow[i * 8 + 5] = g;
					pMapRow[i * 8 + 6] = r;
					pMapRow[i * 8 + 7] = a;

					pMapRow[mapPitch + i * 8 + 0] = b;
					pMapRow[mapPitch + i * 8 + 1] = g;
					pMapRow[mapPitch + i * 8 + 2] = r;
					pMapRow[mapPitch + i * 8 + 3] = a;
					pMapRow[mapPitch + i * 8 + 4] = b;
					pMapRow[mapPitch + i * 8 + 5] = g;
					pMapRow[mapPitch + i * 8 + 6] = r;
					pMapRow[mapPitch + i * 8 + 7] = a;
					pSegCol++;
				}
				break;
			}

			case TintMode::GradientX:
			{
				for (int i = 0; i < nSegments; i++)
				{
					uint16_t r1 = uint16_t(int(pSegCol->r) * 65535 / 4096);
					uint16_t g1 = uint16_t(int(pSegCol->g) * 65535 / 4096);
					uint16_t b1 = uint16_t(int(pSegCol->b) * 65535 / 4096);
					uint16_t a1 = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					uint16_t r2 = uint16_t(int(pSegCol->r) * 65535 / 4096);
					uint16_t g2 = uint16_t(int(pSegCol->g) * 65535 / 4096);
					uint16_t b2 = uint16_t(int(pSegCol->b) * 65535 / 4096);
					uint16_t a2 = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					pMapRow[i * 8 + 0] = b1;
					pMapRow[i * 8 + 1] = g1;
					pMapRow[i * 8 + 2] = r1;
					pMapRow[i * 8 + 3] = a1;
					pMapRow[i * 8 + 4] = b2;
					pMapRow[i * 8 + 5] = g2;
					pMapRow[i * 8 + 6] = r2;
					pMapRow[i * 8 + 7] = a2;

					pMapRow[mapPitch + i * 8 + 0] = b1;
					pMapRow[mapPitch + i * 8 + 1] = g1;
					pMapRow[mapPitch + i * 8 + 2] = r1;
					pMapRow[mapPitch + i * 8 + 3] = a1;
					pMapRow[mapPitch + i * 8 + 4] = b2;
					pMapRow[mapPitch + i * 8 + 5] = g2;
					pMapRow[mapPitch + i * 8 + 6] = r2;
					pMapRow[mapPitch + i * 8 + 7] = a2;
				}
				break;
			}

			case TintMode::GradientY:
			{
				for (int i = 0; i < nSegments; i++)
				{
					uint16_t r1 = uint16_t(int(pSegCol->r) * 65535 / 4096);
					uint16_t g1 = uint16_t(int(pSegCol->g) * 65535 / 4096);
					uint16_t b1 = uint16_t(int(pSegCol->b) * 65535 / 4096);
					uint16_t a1 = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					uint16_t r2 = uint16_t(int(pSegCol->r) * 65535 / 4096);
					uint16_t g2 = uint16_t(int(pSegCol->g) * 65535 / 4096);
					uint16_t b2 = uint16_t(int(pSegCol->b) * 65535 / 4096);
					uint16_t a2 = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					pMapRow[i * 8 + 0] = b1;
					pMapRow[i * 8 + 1] = g1;
					pMapRow[i * 8 + 2] = r1;
					pMapRow[i * 8 + 3] = a1;
					pMapRow[i * 8 + 4] = b1;
					pMapRow[i * 8 + 5] = g1;
					pMapRow[i * 8 + 6] = r1;
					pMapRow[i * 8 + 7] = a1;

					pMapRow[mapPitch + i * 8 + 0] = b2;
					pMapRow[mapPitch + i * 8 + 1] = g2;
					pMapRow[mapPitch + i * 8 + 2] = r2;
					pMapRow[mapPitch + i * 8 + 3] = a2;
					pMapRow[mapPitch + i * 8 + 4] = b2;
					pMapRow[mapPitch + i * 8 + 5] = g2;
					pMapRow[mapPitch + i * 8 + 6] = r2;
					pMapRow[mapPitch + i * 8 + 7] = a2;
				}
				break;
			}

			case TintMode::GradientXY:
			{
				for (int i = 0; i < nSegments; i++)
				{
					pMapRow[i * 8 + 0] = uint16_t(int(pSegCol->b) * 65535 / 4096);
					pMapRow[i * 8 + 1] = uint16_t(int(pSegCol->g) * 65535 / 4096);
					pMapRow[i * 8 + 2] = uint16_t(int(pSegCol->r) * 65535 / 4096);
					pMapRow[i * 8 + 3] = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					pMapRow[i * 8 + 4] = uint16_t(int(pSegCol->b) * 65535 / 4096);
					pMapRow[i * 8 + 5] = uint16_t(int(pSegCol->g) * 65535 / 4096);
					pMapRow[i * 8 + 6] = uint16_t(int(pSegCol->r) * 65535 / 4096);
					pMapRow[i * 8 + 7] = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					pMapRow[mapPitch + i * 8 + 4] = uint16_t(int(pSegCol->b) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 5] = uint16_t(int(pSegCol->g) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 6] = uint16_t(int(pSegCol->r) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 7] = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;

					pMapRow[mapPitch + i * 8 + 0] = uint16_t(int(pSegCol->b) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 1] = uint16_t(int(pSegCol->g) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 2] = uint16_t(int(pSegCol->r) * 65535 / 4096);
					pMapRow[mapPitch + i * 8 + 3] = uint16_t(int(pSegCol->a) * 65535 / 4096);
					pSegCol++;
				}
				break;
			}
		}

		m_segmentsTintTexOfs++;

		// Add edgestrips to extras

		pExtras = m_extrasBufferData + edgeStripOfs;

		const int * pEdges = pEdgeStrips;

		for (int i = 0; i < nEdgeStrips-1; i++)
		{
			for (int j = 0; j < nSegments - 1; j++)
			{
				int edgeIn = pEdges[j];
				int edgeOut = pEdges[edgeStripPitch+j];

				if (edgeIn > edgeOut)
					std::swap(edgeIn, edgeOut);

				float increment = edgeOut == edgeIn ? 100.f : 64.f / (edgeOut - edgeIn);
				float beginAdder;
				float endAdder;

				if ((edgeOut & 0xFFFFFFC0) <= (unsigned int) edgeIn)
				{
					float firstPixelCoverage = ((64 - (edgeOut & 0x3F)) + (edgeOut - edgeIn) / 2) / 64.f;

					beginAdder = increment * (edgeIn & 0x3F)/64.f + firstPixelCoverage;
					endAdder = beginAdder;
				}
				else
				{
					int height = 64 - (edgeIn & 0x3F);
					int width = (int)(increment * height);
					float firstPixelCoverage = (height * width) / (2 * 4096.f);
					float lastPixelCoverage = 1.f - (edgeOut & 0x3F)*increment*(edgeOut & 0x3F) / (2*4096.f);

					beginAdder = increment * (edgeIn & 0x3F) / 64.f + firstPixelCoverage;
					endAdder = lastPixelCoverage - (1.f - (edgeOut & 0x3F)*increment / 64.f);
// 					endAdder = lastPixelCoverage - ((edgeOut & 0xFFFFFF00)-edgeIn)*increment / 256.f;
				}

				*pExtras++ = edgeIn/64.f;					// Segment begin pixel
				*pExtras++ = increment;						// Segment increment
				*pExtras++ = beginAdder;					// Segment begin adder
				*pExtras++ = endAdder;						// Segment end adder
			}

			pEdges += edgeStripPitch;
		}

		m_extrasOfs += extrasSpaceNeeded;
	}

	//____ _dummyFinalizer() __________________________________________________________

	void GlGfxDevice::_dummyFinalizer()
	{
	}

	//____ _drawCmdFinalizer() __________________________________________________________

	void GlGfxDevice::_drawCmdFinalizer()
	{
		m_commandBuffer[m_commandOfs++] = m_vertexOfs - m_cmdBeginVertexOfs;
	}

	//____ _executeBuffer() ___________________________________________________

	void GlGfxDevice::_executeBuffer()
	{
		LOG_GLERROR(glGetError());

		if (m_commandOfs != 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
			glBufferData(GL_ARRAY_BUFFER, c_vertexBufferSize * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);		// Orphan current buffer if still in use.
			glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertexOfs * sizeof(Vertex), m_vertexBufferData);			// Fill new buffer with as much data as needed.

			glBindBuffer(GL_TEXTURE_BUFFER, m_extrasBufferId);
			glBufferData(GL_TEXTURE_BUFFER, c_extrasBufferSize * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
			glBufferSubData(GL_TEXTURE_BUFFER, 0, m_extrasOfs * sizeof(GLfloat), m_extrasBufferData);

			if (m_segmentsTintTexOfs > 0)
			{
				glActiveTexture(GL_TEXTURE10);
				glBindTexture(GL_TEXTURE_2D, m_segmentsTintTexId);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, c_maxSegments * 2, m_segmentsTintTexOfs * 2, GL_BGRA, GL_UNSIGNED_SHORT, m_segmentsTintTexMap);
				glActiveTexture(GL_TEXTURE0);
			}


			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);
			glEnableVertexAttribArray(4);

			LOG_GLERROR(glGetError());

			int * pCmd = m_commandBuffer;
			int * pCmdEnd = &m_commandBuffer[m_commandOfs];

			int vertexOfs = 0;
			int surfaceOfs = 0;

			// Clear pending flags of active BlitSource and Canvas.

			if (m_pActiveBlitSource)
				m_pActiveBlitSource->m_bPendingReads = false;

			while (pCmd < pCmdEnd)
			{
				Command cmd = (Command) * pCmd++;

				switch (cmd)
				{
					case Command::SetCanvas:
					{
						_setCanvas(m_surfaceBuffer[surfaceOfs], pCmd[0], pCmd[1]);
						m_surfaceBuffer[surfaceOfs++] = nullptr;
						pCmd += 2;
						break;
					}
					case Command::SetBlendMode:
					{
						_setBlendMode((BlendMode)* pCmd++);
						break;
					}
					case Command::SetMorphFactor:
					{
						_setMorphFactor((*pCmd++) / 1024.f);
						break;
					}

					case Command::SetBlurMatrices:
					{
						spx radius = *pCmd++;

						_setBlurMatrices(radius, (float*)(pCmd), (float*)(pCmd + 9), (float*)(pCmd + 18));
						pCmd += 27;
						break;
					}

					case Command::SetTintGradient:
					{
						m_bGradientActive = true;
						break;
					}
					case Command::ClearTintGradient:
					{
						m_bGradientActive = false;
						break;
					}
					case Command::SetBlitSource:
					{
						_setBlitSource(m_surfaceBuffer[surfaceOfs]);
						m_surfaceBuffer[surfaceOfs++] = nullptr;
						break;
					}
					case Command::Blit:
					{
						int nVertices = *pCmd++;
						if (nVertices > 0 && m_pActiveBlitSource)
						{
							GlSurface* pSurf = m_pActiveBlitSource;
							glUseProgram(m_blitProgMatrix[(int)pSurf->m_pixelFormat][(int)pSurf->sampleMethod()][m_bGradientActive][m_bActiveCanvasIsA8]);

							glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);

							if( m_bMipmappedActiveCanvas )
								m_pActiveCanvas->m_bMipmapStale = true;
						}
						vertexOfs += nVertices;
						break;
					}
					case Command::Blur:
					{
						int nVertices = *pCmd++;
						if (nVertices > 0 && m_pActiveBlitSource)
						{
 							auto size = m_pActiveBlitSource->pixelSize();

							float radiusX = m_activeBlurRadius / float(size.w * 64);
							float radiusY = m_activeBlurRadius / float(size.h * 64);

							m_activeBlurInfo.offset[0][0] = -radiusX * 0.7f;
							m_activeBlurInfo.offset[0][1] = -radiusY * 0.7f;
					
							m_activeBlurInfo.offset[1][0] = 0;
							m_activeBlurInfo.offset[1][1] = -radiusY;

							m_activeBlurInfo.offset[2][0] = radiusX * 0.7f;
							m_activeBlurInfo.offset[2][1] = -radiusY * 0.7f;

							m_activeBlurInfo.offset[3][0] = -radiusX;
							m_activeBlurInfo.offset[3][1] = 0;

							m_activeBlurInfo.offset[4][0] = 0;
							m_activeBlurInfo.offset[4][1] = 0;

							m_activeBlurInfo.offset[5][0] = radiusX;
							m_activeBlurInfo.offset[5][1] = 0;

							m_activeBlurInfo.offset[6][0] = -radiusX * 0.7f;
							m_activeBlurInfo.offset[6][1] = radiusY * 0.7f;

							m_activeBlurInfo.offset[7][0] = 0;
							m_activeBlurInfo.offset[7][1] = radiusY;

							m_activeBlurInfo.offset[8][0] = radiusX * 0.7f;
							m_activeBlurInfo.offset[8][1] = radiusY * 0.7f;

							glUseProgram(m_blurProg[m_bGradientActive]);

							glUniform2fv(m_blurUniformLocation[m_bGradientActive][1], 9, (GLfloat*)m_activeBlurInfo.offset);
							glUniform4fv(m_blurUniformLocation[m_bGradientActive][0], 9, (GLfloat*)m_activeBlurInfo.colorMtx);

							glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);

							if (m_bMipmappedActiveCanvas)
								m_pActiveCanvas->m_bMipmapStale = true;
						}
						vertexOfs += nVertices;
						break;
					}
					case Command::Fill:
					{
						int nVertices = *pCmd++;
						if( nVertices > 0 )
						{
							if( m_bGradientActive )
								glUseProgram(m_fillGradientProg[m_bActiveCanvasIsA8]);
							else
								glUseProgram(m_fillProg[m_bActiveCanvasIsA8]);

							glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);
							vertexOfs += nVertices;

							if( m_bMipmappedActiveCanvas )
								m_pActiveCanvas->m_bMipmapStale = true;
						}
						break;
					}
					case Command::FillSubPixel:
					{
						int nVertices = *pCmd++;
						if( nVertices > 0 )
						{
							if (m_bGradientActive)
								glUseProgram(m_aaFillGradientProg[m_bActiveCanvasIsA8]);
							else
								glUseProgram(m_aaFillProg[m_bActiveCanvasIsA8]);

							glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);
							vertexOfs += nVertices;

							if( m_bMipmappedActiveCanvas )
								m_pActiveCanvas->m_bMipmapStale = true;
						}
						break;
					}
					case Command::LineFromTo:
					{
						int clipListOfs = *pCmd++;
						int clipListLen = *pCmd++;
						int nVertices = *pCmd++;
						if( nVertices > 0 )
						{
							glUseProgram(m_lineFromToProg[m_bActiveCanvasIsA8]);

							for (int i = 0; i < clipListLen; i++)
							{
								RectI& clip = m_clipListBuffer[clipListOfs++];
								glScissor(clip.x, clip.y, clip.w, clip.h);
								glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);
							}

							glScissor(0, 0, m_activeCanvasSize.w, m_activeCanvasSize.h);

							vertexOfs += nVertices;

							if( m_bMipmappedActiveCanvas )
								m_pActiveCanvas->m_bMipmapStale = true;
						}
						break;
					}
					case Command::Plot:
					{
						int nVertices = *pCmd++;
						if( nVertices > 0 )
						{
							glUseProgram(m_plotProg[m_bActiveCanvasIsA8]);

							glDrawArrays(GL_POINTS, vertexOfs, nVertices);
							vertexOfs += nVertices;

							if( m_bMipmappedActiveCanvas )
								m_pActiveCanvas->m_bMipmapStale = true;
						}
						break;
					}
					case Command::Segments:
					{
						int nSegments = (*pCmd++);
						if (nSegments > c_maxSegments)
							nSegments = c_maxSegments;
						int nEdges = nSegments-1;
						int nVertices = *pCmd++;
						if( nVertices > 0 )
						{
							glUseProgram(m_segmentsProg[nEdges][m_bGradientActive][m_bActiveCanvasIsA8]);

							glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);
							vertexOfs += nVertices;

							if( m_bMipmappedActiveCanvas )
								m_pActiveCanvas->m_bMipmapStale = true;
						}
						break;
					}
					default:
						assert(false);
				}
			}
		}

		m_cmd = Command::None;
		m_vertexOfs = 0;
		m_extrasOfs = 0;
		m_commandOfs = 0;
		m_surfaceOfs = 0;
		m_segmentsTintTexOfs = 0;
		m_clipWriteOfs = 0;
		m_clipCurrOfs = -1;

		_writeCanvasInfo();
		_writeTintInfo();

		LOG_GLERROR(glGetError());
	}

	//____ _writeCanvasInfo() _________________________________________________

	void	GlGfxDevice::_writeCanvasInfo()
	{
		assert(sizeof(CanvasInfo) % 16 == 0);		// m_extrasOfs needs to grow in steps of 4 floats (read as vec4 by shader).

		int canvasInfoSpace = sizeof(CanvasInfo) / 4;

		if (m_extrasOfs > c_extrasBufferSize - canvasInfoSpace)
		{
			// CanvasInfo is automatically added to the new extrasBuffer by _executeBuffer().

			_endCommand();
			_executeBuffer();
		}
		else
		{
			memcpy(&m_extrasBufferData[m_extrasOfs], &m_canvasInfo, canvasInfoSpace*4);

			m_canvasInfoOfs = m_extrasOfs;
			m_extrasOfs += canvasInfoSpace;
		}
	}

	//____ _writeTintInfo() ___________________________________________________

	void	GlGfxDevice::_writeTintInfo()
	{
		assert(sizeof(GradientTintInfo) % 16 == 0);		// m_extrasOfs needs to grow in steps of 4 floats (read as vec4 by shader).
		assert(sizeof(FlatTintInfo) % 16 == 0);			// m_extrasOfs needs to grow in steps of 4 floats (read as vec4 by shader).

		// We just write a partial GradientTintInfo if we don't use gradients.

		int tintInfoSpace = m_bTintGradient ? sizeof(GradientTintInfo) / 4 : sizeof(FlatTintInfo) / 4;

		if (m_extrasOfs > c_extrasBufferSize - tintInfoSpace)
		{
			// TintInfo is automatically added to the new extrasBuffer by _executeBuffer().

			_endCommand();
			_executeBuffer();
		}
		else
		{
			memcpy(&m_extrasBufferData[m_extrasOfs], &m_tintInfo, tintInfoSpace*4);

			m_tintInfoOfs = m_extrasOfs;
			m_extrasOfs += tintInfoSpace;
		}
	}



	//____ _setCanvas() _______________________________________________________

	void GlGfxDevice::_setCanvas( GlSurface * pCanvas, int width, int height )
	{
		LOG_GLERROR(glGetError());

		bool bWasAlphaOnly = m_bActiveCanvasIsA8;

		if (pCanvas)
		{
			glEnable(GL_FRAMEBUFFER_SRGB);		// Always use SRGB on Canvas that is SRGB.

			glBindFramebuffer(GL_FRAMEBUFFER, pCanvas->m_framebufferId);
//			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pCanvas->getTexture(), 0);

			glDrawBuffer(GL_COLOR_ATTACHMENT0);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				m_pActiveCanvas = nullptr;
                m_activeCanvasSize = { width, height };
				m_bMipmappedActiveCanvas = false;

				//TODO: Signal error that we could not set the specified canvas.

				return;
			}
        }
		else
		{
			if( GfxBase::defaultToSRGB() )
				glEnable(GL_FRAMEBUFFER_SRGB);
			else
				glDisable(GL_FRAMEBUFFER_SRGB);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

        glViewport(0, 0, width, height);
        glScissor(0, 0, width, height);

        
		int canvasYstart	= pCanvas ? 0 : height;
		int canvasYmul		= pCanvas ? 1 : -1;

		// Updating canvas info for our shaders

		m_canvasInfo.canvasDimX = (GLfloat)width;
		m_canvasInfo.canvasDimY = (GLfloat)height;
		m_canvasInfo.canvasYOfs = canvasYstart;
		m_canvasInfo.canvasYMul = canvasYmul;

		_writeCanvasInfo();

		LOG_GLERROR(glGetError());

		m_pActiveCanvas = pCanvas;
        m_activeCanvasSize = { width, height };
		m_bMipmappedActiveCanvas = m_pActiveCanvas ? m_pActiveCanvas->m_bMipmapped : false;

		bool bIsAlphaOnly = m_pActiveCanvas ? m_pActiveCanvas->pixelFormat() == PixelFormat::Alpha_8 : false;

		if (bIsAlphaOnly != bWasAlphaOnly)
			_setBlendMode(m_activeBlendMode);

		m_bActiveCanvasIsA8 = bIsAlphaOnly;
	}

	//____ _setBlendMode() _______________________________________________________

	void GlGfxDevice::_setBlendMode( BlendMode mode )
	{
		LOG_GLERROR(glGetError());

		bool bAlphaOnly = m_pActiveCanvas ? m_pActiveCanvas->pixelFormat() == PixelFormat::Alpha_8 : false;


		switch (mode)
		{
		case BlendMode::Replace:
			glBlendEquation(GL_FUNC_ADD);
			glDisable(GL_BLEND);
			break;

        case BlendMode::Undefined:
		case BlendMode::Blend:
			glBlendEquation(GL_FUNC_ADD);
			glEnable(GL_BLEND);
			if (bAlphaOnly)
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
			else
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;

		case BlendMode::Morph:
			glBlendEquation(GL_FUNC_ADD);
			glEnable(GL_BLEND);
			glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
			break;

		case BlendMode::Add:
			glBlendEquation(GL_FUNC_ADD);
			glEnable(GL_BLEND);

			if( bAlphaOnly )
				glBlendFunc(GL_ONE, GL_ONE);
			else
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);

			break;

		case BlendMode::Subtract:
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
			if (bAlphaOnly)
				glBlendFunc(GL_ONE, GL_ONE);
			else
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
			break;

		case BlendMode::Multiply:
			glBlendEquation(GL_FUNC_ADD);
			glEnable(GL_BLEND);
			if( bAlphaOnly )
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
			else
				glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE);
			break;

		case BlendMode::Invert:
			glBlendEquation(GL_FUNC_ADD);
			glEnable(GL_BLEND);
			if( bAlphaOnly )
				glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
			else
				glBlendFuncSeparate(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE);
			break;

		case BlendMode::Min:
			glEnable(GL_BLEND);
			if (bAlphaOnly)
			{
				glBlendEquation(GL_MIN);
				glBlendFunc(GL_ONE, GL_ONE);
			}
			else
			{
				glBlendEquationSeparate(GL_MIN, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
			}
			break;

		case BlendMode::Max:
			glEnable(GL_BLEND);
			if (bAlphaOnly)
			{
				glBlendEquation(GL_MAX);
				glBlendFunc(GL_ONE, GL_ONE);
			}
			else
			{
				glBlendEquationSeparate(GL_MAX, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE);
			}
			break;

		case BlendMode::Ignore:
			glBlendEquation(GL_FUNC_ADD);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ZERO, GL_ONE);
			break;

		default:
			assert(false);
			break;
		}

		m_activeBlendMode = mode;

		LOG_GLERROR(glGetError());
	}

	//____ _setMorphFactor() __________________________________________________

	void GlGfxDevice::_setMorphFactor(float morphFactor)
	{
		glBlendColor(1.f, 1.f, 1.f, morphFactor);
	}

	//____ _setBlurMatrices() __________________________________________________

	void GlGfxDevice::_setBlurMatrices(spx radius, const float red[9], const float green[9], const float blue[9])
	{
		m_activeBlurRadius = radius;

		for (int i = 0; i < 9; i++)
		{
			m_activeBlurInfo.colorMtx[i][0] = red[i];
			m_activeBlurInfo.colorMtx[i][1] = green[i];
			m_activeBlurInfo.colorMtx[i][2] = blue[i];
			m_activeBlurInfo.colorMtx[i][3] = 0.f;
		}

		m_activeBlurInfo.colorMtx[4][3] = 1.f;
	}


	//____ _setBlitSource() _______________________________________________________

	void GlGfxDevice::_setBlitSource( GlSurface * pSurf )
	{
		LOG_GLERROR(glGetError());

		glActiveTexture(GL_TEXTURE0);

		if( pSurf )
		{
			glBindTexture(GL_TEXTURE_2D, pSurf->getTexture());

			if( pSurf->m_bMipmapStale )
			{
				glGenerateMipmap(GL_TEXTURE_2D);
				pSurf->m_bMipmapStale = false;
			}

			m_pActiveBlitSource = pSurf;
			pSurf->m_bPendingReads = false;			// Clear this as we pass it by...

			if (pSurf->m_pPalette)
			{
				glActiveTexture(GL_TEXTURE2);
				GLuint paletteTex = pSurf->getPaletteTexture();
				glBindTexture(GL_TEXTURE_2D, paletteTex);
				glActiveTexture(GL_TEXTURE0);

				assert(glGetError() == 0);
			}
		}
		else
        {
			glBindTexture(GL_TEXTURE_2D, 0 );
            m_pActiveBlitSource = nullptr;
        }
		LOG_GLERROR(glGetError());
	}

	//____ _loadPrograms() ______________________________________________________________

	void GlGfxDevice::_loadPrograms(int uboBindingPoint)
	{
		int programNb = 0;

		// Create and init Fill shader

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, fillVertexShader, i == 0 ? fillFragmentShader : fillFragmentShader_A8);
			_setDrawUniforms(progId, uboBindingPoint);
			m_fillProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init Fill Gradient shader

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, fillGradientVertexShader, i == 0 ? fillFragmentShader : fillFragmentShader_A8);
			_setDrawUniforms(progId, uboBindingPoint);
			m_fillGradientProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init AA-Fill shader

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, aaFillVertexShader, i == 0 ? aaFillFragmentShader : aaFillFragmentShader_A8);
			_setDrawUniforms(progId, uboBindingPoint);
			m_aaFillProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init AA-Fill gradient shader

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, aaFillGradientVertexShader, i == 0 ? aaFillFragmentShader : aaFillFragmentShader_A8);
			_setDrawUniforms(progId, uboBindingPoint);
			m_aaFillGradientProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init Blur shader

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, i == 0 ? blitVertexShader : blitGradientVertexShader, blurFragmentShader);
			_setBlitUniforms(progId, uboBindingPoint);

			m_blurUniformLocation[i][0] = glGetUniformLocation(progId, "blurInfo.colorMtx");
			m_blurUniformLocation[i][1] = glGetUniformLocation(progId, "blurInfo.offset");

			m_blurProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init Blit shader

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, blitVertexShader, i == 0 ? blitFragmentShader : blitFragmentShader_A8);
			_setBlitUniforms(progId, uboBindingPoint);
			m_blitProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init Gradient Blit shader

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, blitGradientVertexShader, i == 0 ? blitFragmentShader : blitFragmentShader_A8);
			_setBlitUniforms(progId, uboBindingPoint);
			m_blitGradientProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init AlphaBlit shader (shader program for blitting from alpha-only texture)

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, blitVertexShader, i == 0 ? alphaBlitFragmentShader : alphaBlitFragmentShader_A8);
			_setBlitUniforms(progId, uboBindingPoint);
			m_alphaBlitProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init AlphaBlit gradient shader (shader program for blitting from alpha-only texture)

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, blitGradientVertexShader, i == 0 ? alphaBlitFragmentShader : alphaBlitFragmentShader_A8);
			_setBlitUniforms(progId, uboBindingPoint);
			m_alphaBlitGradientProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init Palette Blit shaders

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, paletteBlitNearestVertexShader, i == 0 ? paletteBlitNearestFragmentShader : paletteBlitNearestFragmentShader_A8);
			_setPaletteBlitUniforms(progId, uboBindingPoint);
			m_paletteBlitNearestProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, paletteBlitInterpolateVertexShader, i == 0 ? paletteBlitInterpolateFragmentShader : paletteBlitInterpolateFragmentShader_A8);
			_setPaletteBlitUniforms(progId, uboBindingPoint);
			m_paletteBlitInterpolateProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init Palette Blit gradient shaders

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, paletteBlitNearestGradientVertexShader, i == 0 ? paletteBlitNearestFragmentShader : paletteBlitNearestFragmentShader_A8);
			_setPaletteBlitUniforms(progId, uboBindingPoint);
			m_paletteBlitNearestGradientProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, paletteBlitInterpolateGradientVertexShader, i == 0 ? paletteBlitInterpolateFragmentShader : paletteBlitInterpolateFragmentShader_A8);
			_setPaletteBlitUniforms(progId, uboBindingPoint);
			m_paletteBlitInterpolateGradientProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init Plot shader

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, plotVertexShader, i == 0 ? plotFragmentShader : plotFragmentShader_A8);
			_setDrawUniforms(progId, uboBindingPoint);
			m_plotProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init Line shader

		for (int i = 0; i < 2; i++)
		{
			GLuint progId = _loadOrCompileProgram(programNb++, lineFromToVertexShader, i == 0 ? lineFromToFragmentShader : lineFromToFragmentShader_A8);
			_setDrawUniforms(progId, uboBindingPoint);
			m_lineFromToProg[i] = progId;
			LOG_INIT_GLERROR(glGetError());
		}

		// Create and init Segment shaders

		for (int i = 1; i < c_maxSegments; i++)
		{
			for (int canvType = 0; canvType < 2; canvType++)
			{
				std::string fragShader = canvType == 0 ? segmentsFragmentShader : segmentsFragmentShader_A8;
				auto edgesPos = fragShader.find("$EDGES");
				fragShader.replace(edgesPos, 6, std::to_string(i));

				auto maxsegPos = fragShader.find("$MAXSEG");
				fragShader.replace(maxsegPos, 7, std::to_string(c_maxSegments));

				const char* pVertexShader = segmentsVertexShader;
				for (int j = 0; j < 2; j++)
				{
					GLuint prog = _loadOrCompileProgram(programNb++, pVertexShader, fragShader.c_str());
					m_segmentsProg[i][j][canvType] = prog;

					GLint extrasIdLoc = glGetUniformLocation(prog, "extrasId");
					GLint colorsIdLoc = glGetUniformLocation(prog, "colorsId");
					GLint stripesIdLoc = glGetUniformLocation(prog, "stripesId");
					GLint paletteIdLoc = glGetUniformLocation(prog, "paletteId");

					glUseProgram(prog);
					glUniform1i(extrasIdLoc, 1);		// Needs to be set. Texture unit 1 is used for extras buffer.
					glUniform1i(colorsIdLoc, 1);		// Needs to be set. Texture unit 1 is used for extras buffer, which doubles as the colors buffer.
					glUniform1i(stripesIdLoc, 1);		// Needs to be set. Texture unit 1 is used for segment stripes buffer.
					glUniform1i(paletteIdLoc, 3);		// Needs to be set. Texture unit 3 is used for segment stripes buffer.

					pVertexShader = segmentsVertexShaderGradient;
				}
			}
		}

		LOG_INIT_GLERROR(glGetError());
	}

	//____ _loadOrCompileProgram() __________________________________________________________

	GLuint GlGfxDevice::_loadOrCompileProgram(int programNb, const char* pVertexShaderSource, const char* pFragmentShaderSource)
	{
		GLuint  programID = glCreateProgram();

		if (s_pShaderPrograms)
		{
			char* pBuffer = (char*)s_pShaderPrograms->data();
			ProgramBlobHeader* pHeader = (ProgramBlobHeader*)pBuffer;

			if (pHeader->version == c_versionNb && pHeader->nbPrograms == c_nbPrograms)
			{
				ProgramBlobEntry& prg = pHeader->programs[programNb];
				glProgramBinary(programID, prg.binaryFormat, pBuffer + prg.offset, prg.size);
			}
		}

		GLint linkStatus = 0;
		glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);

		if (linkStatus == false)
		{
			// Since a program failed to load we discard the whole programs blob.

			s_pShaderPrograms = nullptr;

			GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
			GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

			glShaderSource(vertexShaderID, 1, &pVertexShaderSource, NULL);
			glCompileShader(vertexShaderID);

			glShaderSource(fragmentShaderID, 1, &pFragmentShaderSource, NULL);
			glCompileShader(fragmentShaderID);

			glAttachShader(programID, vertexShaderID);
			glAttachShader(programID, fragmentShaderID);
			glLinkProgram(programID);

			// glLinkProgram doesn't use glGetError
			int mess = 0;
			glGetProgramiv(programID, GL_LINK_STATUS, &mess);
			if (mess != GL_TRUE)
			{
				GLchar	vertexShaderLog[2048];
				GLchar	fragmentShaderLog[2048];
				GLchar	programInfoLog[2048];

				GLsizei vertexShaderLogLength;
				GLsizei fragmentShaderLogLength;
				GLsizei programInfoLogLength;

				glGetShaderInfoLog(vertexShaderID, 2048, &vertexShaderLogLength, vertexShaderLog);
				glGetShaderInfoLog(fragmentShaderID, 2048, &fragmentShaderLogLength, fragmentShaderLog);
				glGetProgramInfoLog(programID, 2048, &programInfoLogLength, programInfoLog);

				char	buffer[2048 * 3 + 256];

				snprintf(buffer, 2048*3+256, "Failed compiling OpenGL shader\nVertexShaderLog: %s\nFragmentShaderLog: %s\nProgramInfoLog: %s", vertexShaderLog, fragmentShaderLog, programInfoLog);
				GfxBase::throwError(ErrorLevel::SilentError, ErrorCode::OpenGL, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
			}

			glDetachShader(programID, vertexShaderID);
			glDetachShader(programID, fragmentShaderID);

			glDeleteShader(vertexShaderID);
			glDeleteShader(fragmentShaderID);

			LOG_GLERROR(glGetError());
		}

		return programID;
	}

	//____ _generateProgramBlob() __________________________________________________________

	Blob_p GlGfxDevice::_generateProgramBlob()
	{
		GLuint	programs[c_nbPrograms];

		// Collect the programs

		int prg = 0;

		programs[prg++] = m_fillProg[0];
		programs[prg++] = m_fillProg[1];

		programs[prg++] = m_fillGradientProg[0];
		programs[prg++] = m_fillGradientProg[1];

		programs[prg++] = m_aaFillProg[0];
		programs[prg++] = m_aaFillProg[1];

		programs[prg++] = m_aaFillGradientProg[0];
		programs[prg++] = m_aaFillGradientProg[1];

		programs[prg++] = m_blurProg[0];
		programs[prg++] = m_blurProg[1];

		programs[prg++] = m_blitProg[0];
		programs[prg++] = m_blitProg[1];

		programs[prg++] = m_blitGradientProg[0];
		programs[prg++] = m_blitGradientProg[1];

		programs[prg++] = m_alphaBlitProg[0];
		programs[prg++] = m_alphaBlitProg[1];

		programs[prg++] = m_alphaBlitGradientProg[0];
		programs[prg++] = m_alphaBlitGradientProg[1];

		programs[prg++] = m_paletteBlitNearestProg[0];
		programs[prg++] = m_paletteBlitNearestProg[1];

		programs[prg++] = m_paletteBlitInterpolateProg[0];
		programs[prg++] = m_paletteBlitInterpolateProg[1];

		programs[prg++] = m_paletteBlitNearestGradientProg[0];
		programs[prg++] = m_paletteBlitNearestGradientProg[1];

		programs[prg++] = m_paletteBlitInterpolateGradientProg[0];
		programs[prg++] = m_paletteBlitInterpolateGradientProg[1];

		programs[prg++] = m_plotProg[0];
		programs[prg++] = m_plotProg[1];

		programs[prg++] = m_lineFromToProg[0];
		programs[prg++] = m_lineFromToProg[1];

		for (int i = 1; i < c_maxSegments; i++)
		{
			for (int canvType = 0; canvType < 2; canvType++)
			{
				programs[prg++] = m_segmentsProg[i][0][canvType];
				programs[prg++] = m_segmentsProg[i][1][canvType];
			}
		}

		assert(prg == c_nbPrograms);

		// Store sizes and calculate space needed

		int totalSize = 0;
		int	programSize[c_nbPrograms];

		for (int i = 0; i < c_nbPrograms; i++)
		{
			GLint	size;
			glGetProgramiv(programs[i], GL_PROGRAM_BINARY_LENGTH, &size);
			programSize[i] = size;
			totalSize += size;
		}

		// Create our Blob

		auto pBlob = Blob::create(sizeof(ProgramBlobHeader) + totalSize);

		char* pBuffer = (char*)pBlob->data();

		ProgramBlobHeader* pHeader = (ProgramBlobHeader*)pBuffer;

		pHeader->version = c_versionNb;
		pHeader->nbPrograms = c_nbPrograms;

		// Get our program binaries

		char* wpBinary = pBuffer + sizeof(ProgramBlobHeader);
		int offset = sizeof(ProgramBlobHeader);
		for (int i = 0; i < c_nbPrograms; i++)
		{
			pHeader->programs[i].offset = offset;
			glGetProgramBinary(programs[i], programSize[i], &pHeader->programs[i].size, &pHeader->programs[i].binaryFormat, pBuffer + offset);
			offset += pHeader->programs[i].size;
		}

		assert(wpBinary = pBuffer + sizeof(ProgramBlobHeader) + totalSize);

		return pBlob;
	}

	//____ _initTables() ___________________________________________________________

	void GlGfxDevice::_initTables()
	{
		// Init lineThicknessTable

		for (int i = 0; i < 17; i++)
		{
			double b = i / 16.0;
			m_lineThicknessTable[i] = (float)Util::squareRoot(1.0 + b * b);
		}

		// Init sRGBtoLinearTable

		float max = powf(255, 2.2f);

		for (int i = 0; i < 256; i++)
		{
			m_sRGBtoLinearTable[i] = powf(float(i), 2.2f)/max;
			m_linearToLinearTable[i] = i / 255.f;
		}
	}


	//____ _scaleThickness() ___________________________________________________

	float GlGfxDevice::_scaleThickness(float thickness, float slope)
	{
		slope = std::abs(slope);

		float scale = m_lineThicknessTable[(int)(slope * 16)];

		if (slope < 1.f)
		{
			float scale2 = m_lineThicknessTable[(int)(slope * 16) + 1];
			scale += (scale2 - scale)*((slope * 16) - ((int)(slope * 16)));
		}

		return thickness * scale;
	}

} // namespace wg

