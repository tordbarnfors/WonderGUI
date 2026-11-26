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
#include <cstdio>
#include <cstring>
#include <algorithm>

#include <wax_glbackend.h>
#include <wax_glbitmap.h>
#include <wax_glbitmapfactory.h>
#include <wax_gledgemap.h>
#include <wax_gledgemapfactory.h>
#include <wax_glwarpmap.h>
#include <wax_glwarpmapfactory.h>
#include <wax_base.h>

namespace wax
{
const TypeInfo GlBackend::TYPEINFO = { "GlBackend", &GpuBackend::TYPEINFO };

GlBackend::ProgramBinary	GlBackend::s_programBinaries[c_nbPrograms];

#define LOG_GLERROR(check) { GLenum err = check; if(err != 0) onGlError(err, this, &TYPEINFO, __func__, __FILE__, __LINE__ ); }

#define LOG_INIT_GLERROR(check) { GLenum err = check; if(err != 0) { onGlError(err, this, &TYPEINFO, __func__, __FILE__, __LINE__ ); m_bFullyInitialized = false; } }


//____ onGlError() _______________________________________________________________

void GlBackend::onGlError(GLenum errorCode, const Object * pObject, const TypeInfo* pClassType, const char * func, const char * file, int line)
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

		default:
			pErrorName = "UNKNOWN GL ERROR! (should not happen)";
			break;
	}

	snprintf( buffer, 1024, "OpenGL error 0x%x: %s", errorCode, pErrorName );
	Base::reportError(ErrorLevel::SilentError, ErrorCode::OpenGL, buffer, pObject, pClassType, func, file, line);
}

//____ create() _______________________________________________________________

GlBackend_p GlBackend::create( int uboBindingPoint)
{
	GlBackend_p p(new GlBackend( uboBindingPoint ));

	if( !p->m_bFullyInitialized )
		return GlBackend_p(nullptr);

	return p;
}

//____ constructor _____________________________________________________________________

GlBackend::GlBackend( int uboBindingPoint )
{
	m_constants = ShaderConstants();

	// Get version number, set flags

	GLint major = 0, minor = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	if((major == 4 && minor >= 1) || major > 4)
	{
		GLint formats = 0;
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
		m_bProgramBinariesSupported = (formats >= 1);
	}

	m_bFullyInitialized = true;

	// The programs are compiled when first drawn with, see _program(), so this is where we find
	// out if they can be: the shaders are GLSL 3.3.

	if( major < 3 || (major == 3 && minor < 3) )
	{
		Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "GlBackend needs OpenGL 3.3 or later.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		m_bFullyInitialized = false;
	}

	LOG_INIT_GLERROR(glGetError());

	// Framebuffer for rendering into bitmaps, vertex buffer and its vertex array. The vertex array
	// has the layout of GpuBackend::Vertex, which the vertex shaders read as the inputs of
	// ../hlsl/README.md, in the order they are declared.

	glGenFramebuffers(1, &m_framebufferId);

	glGenBuffers(1, &m_vertexBufferId);
	glGenVertexArrays(1, &m_vertexArrayId);

	glBindVertexArray(m_vertexArrayId);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, x));
	glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, colorOfs));
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*) offsetof(Vertex, extrasOfs));

	for( int i = 0 ; i < 3 ; i++ )
		glEnableVertexAttribArray(i);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	LOG_INIT_GLERROR(glGetError());

	// Colors and extras, read by the shaders as buffer textures.

	glGenBuffers(1, &m_colorBufferId);
	glGenTextures(1, &m_colorBufferTex);
	glGenBuffers(1, &m_extrasBufferId);
	glGenTextures(1, &m_extrasBufferTex);

	glBindBuffer(GL_TEXTURE_BUFFER, m_colorBufferId);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(Color8), nullptr, GL_STREAM_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, m_extrasBufferId);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(Extras), nullptr, GL_STREAM_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	glBindTexture(GL_TEXTURE_BUFFER, m_colorBufferTex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_colorBufferId);
	glBindTexture(GL_TEXTURE_BUFFER, m_extrasBufferTex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_extrasBufferId);
	glBindTexture(GL_TEXTURE_BUFFER, 0);

	LOG_INIT_GLERROR(glGetError());

	// Samplers, as those of DX12Backend and VulkanBackend: tiling sources repeat, ClipBlit reads
	// transparent black outside the source and everything else, including Blur, reads the edge
	// pixels. A mipmap filter on a texture without mipmaps leaves it incomplete, so the mipmapped
	// ones are only used for mipmapped bitmaps.

	glGenSamplers(16, m_samplers);

	for( int i = 0 ; i < 16 ; i++ )
	{
		bool bMipmapped = (i & 8) != 0;
		bool bClip = (i & 4) != 0;
		bool bTiling = (i & 2) != 0;
		bool bBilinear = (i & 1) != 0;

		GLint wrap = bTiling ? GL_REPEAT : bClip ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE;
		GLint minFilter = bBilinear ? (bMipmapped ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR) : GL_NEAREST;

		glSamplerParameteri(m_samplers[i], GL_TEXTURE_WRAP_S, wrap);
		glSamplerParameteri(m_samplers[i], GL_TEXTURE_WRAP_T, wrap);
		glSamplerParameteri(m_samplers[i], GL_TEXTURE_MIN_FILTER, minFilter);
		glSamplerParameteri(m_samplers[i], GL_TEXTURE_MAG_FILTER, bBilinear ? GL_LINEAR : GL_NEAREST);

		const GLfloat transparent[4] = { 0.f, 0.f, 0.f, 0.f };
		glSamplerParameterfv(m_samplers[i], GL_TEXTURE_BORDER_COLOR, transparent);
	}

	LOG_INIT_GLERROR(glGetError());
}

//____ Destructor ______________________________________________________________

GlBackend::~GlBackend()
{
	LOG_GLERROR(glGetError());

	for( auto& programs : m_programs )
	{
		for( GLuint program : programs )
		{
			if( program != 0 )
				glDeleteProgram(program);
		}
	}

	glDeleteSamplers(16, m_samplers);

	glDeleteFramebuffers(1, &m_framebufferId);
	glDeleteVertexArrays(1, &m_vertexArrayId);

	glDeleteTextures(1, &m_colorBufferTex);
	glDeleteTextures(1, &m_extrasBufferTex);

	glDeleteBuffers(1, &m_vertexBufferId);
	glDeleteBuffers(1, &m_colorBufferId);
	glDeleteBuffers(1, &m_extrasBufferId);

	LOG_GLERROR(glGetError());
}

//____ typeInfo() _________________________________________________________

const TypeInfo& GlBackend::typeInfo(void) const
{
	return TYPEINFO;
}

//____ canBeBlitSource() _____________________________________________

bool GlBackend::canBeBlitSource(const TypeInfo& type) const
{
	return (type == GlBitmap::TYPEINFO);
}

//____ canBeCanvas() _____________________________________________

bool GlBackend::canBeCanvas(const TypeInfo& type) const
{
	return (type == GlBitmap::TYPEINFO);
}

//____ waitForCompletion() ____________________________________________________

void GlBackend::waitForCompletion()
{
	glFinish();
}

//____ bitmapFactory() ______________________________________________________

BitmapFactory_p GlBackend::bitmapFactory()
{
	if( !m_pBitmapFactory )
		m_pBitmapFactory = GlBitmapFactory::create();

	return m_pBitmapFactory;
}

//____ edgemapFactory() ______________________________________________________

EdgemapFactory_p GlBackend::edgemapFactory()
{
	if (!m_pEdgemapFactory)
		m_pEdgemapFactory = GlEdgemapFactory::create();

	return m_pEdgemapFactory;
}

//____ warpmapFactory() ___________________________________________________

WarpmapFactory_p GlBackend::warpmapFactory()
{
	if (!m_pWarpmapFactory)
		m_pWarpmapFactory = GlWarpmapFactory::create();

	return m_pWarpmapFactory;
}

//____ setDefaultCanvas() ___________________________________________

bool GlBackend::setDefaultCanvas(SizeSPX size, int scale, ColorSpace colorSpace)
{
	m_defaultCanvas.ref = CanvasRef::Default;		// Starts as Undefined until this method is called.
	m_defaultCanvas.size = size;
	m_defaultCanvas.scale = scale;
	m_defaultCanvas.colorSpace = colorSpace == ColorSpace::Undefined ? ColorSpace::sRGB : colorSpace;
	return true;
}

//____ canvasInfo() ___________________________________________________________

const CanvasInfo * GlBackend::canvasInfo(CanvasRef ref) const
{
	if (ref == CanvasRef::Default)
		return &m_defaultCanvas;
	else
	{
		Base::reportError(ErrorLevel::Error, ErrorCode::InvalidParam, "Only Default canvas is supported.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return &m_dummyCanvas;
	}
}

//____ beginRender() ___________________________________________________________

void GlBackend::beginRender()
{
	LOG_GLERROR(glGetError());

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Framebuffer is not complete.", this, &TYPEINFO, __func__, __FILE__, __LINE__);
		return;
	}

	// Remember GL states so we can restore in EndRender()

	m_glDepthTest 		= glIsEnabled(GL_DEPTH_TEST);
	m_glScissorTest 	= glIsEnabled(GL_SCISSOR_TEST);
	m_glBlendEnabled  	= glIsEnabled(GL_BLEND);
	m_glSRGBEnabled		= glIsEnabled(GL_FRAMEBUFFER_SRGB);
	m_glCullFace		= glIsEnabled(GL_CULL_FACE);
	glGetIntegerv(GL_BLEND_SRC, &m_glBlendSrc);
	glGetIntegerv(GL_BLEND_DST, &m_glBlendDst);
	glGetIntegerv(GL_VIEWPORT, m_glViewport);
	glGetIntegerv(GL_SCISSOR_BOX, m_glScissorBox);
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &m_glReadFrameBuffer);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_glDrawFrameBuffer);

	//  Modify states. Bitmaps are drawn upside down to the window, so triangles turn the other
	//  way there, and must not be culled.

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_SCISSOR_TEST);

	// The default canvas is the framebuffer bound for drawing now, see setDefaultCanvas(). It
	// might not have alpha.

	GLenum attachment = GL_COLOR_ATTACHMENT0;

	if (m_glDrawFrameBuffer == 0)
	{
		GLint drawBuffer = GL_BACK;
		glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);

		attachment = (drawBuffer == GL_FRONT || drawBuffer == GL_FRONT_LEFT) ? GL_FRONT_LEFT : GL_BACK_LEFT;
	}

	GLint alphaBits = 8;
	glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &alphaBits);
	if (glGetError() != GL_NO_ERROR)
		alphaBits = 8;

	m_bDefaultCanvasHasAlpha = (alphaBits > 0);

	m_bRendering = true;

	LOG_GLERROR(glGetError());
}

//____ endRender() _____________________________________________________________

void GlBackend::endRender()
{
	LOG_GLERROR(glGetError());

	if( !m_bRendering )
		return;

	// Restore render states from before beginRender()

	if( m_glDepthTest )
		glEnable(GL_DEPTH_TEST);

	if( m_glCullFace )
		glEnable(GL_CULL_FACE);

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

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc( m_glBlendSrc, m_glBlendDst );
	glViewport(m_glViewport[0], m_glViewport[1], m_glViewport[2], m_glViewport[3]);
	glScissor(m_glScissorBox[0], m_glScissorBox[1], m_glScissorBox[2], m_glScissorBox[3]);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_glReadFrameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_glDrawFrameBuffer);

	glFlush();

	m_bRendering = false;

	LOG_GLERROR(glGetError());
}

//____ _isRecording() ________________________________________________________

bool GlBackend::_isRecording() const
{
	return m_bRendering;
}

//____ _isNativeBitmap() ____________________________________________________

bool GlBackend::_isNativeBitmap( const Object * pObject ) const
{
	return pObject && _isOfType(pObject->typeInfo(), GlBitmap::TYPEINFO);
}

//____ _isNativeEdgemap() ____________________________________________________

bool GlBackend::_isNativeEdgemap( const Object * pObject ) const
{
	return pObject && _isOfType(pObject->typeInfo(), GlEdgemap::TYPEINFO);
}

//____ _isNativeWarpmap() ____________________________________________________

bool GlBackend::_isNativeWarpmap( const Object * pObject ) const
{
	return pObject && _isOfType(pObject->typeInfo(), GlWarpmap::TYPEINFO);
}

//____ _beginSessionOnDevice() _______________________________________________

void GlBackend::_beginSessionOnDevice( Bitmap * pCanvas )
{
	m_ops.clear();
	m_patches.clear();
	m_sessionObjects.clear();
	m_constantSnapshots.clear();

	m_bConstantsChanged = true;
	m_bCanvasBound = false;
	m_targetSize = SizeI();
}

//____ _endSessionOnDevice() _________________________________________________

void GlBackend::_endSessionOnDevice()
{
	_execute();

	m_ops.clear();
	m_patches.clear();
	m_sessionObjects.clear();
	m_constantSnapshots.clear();

	m_buffers = FrameBuffers();
	m_bCanvasBound = false;
}

//____ _reserveFrameBuffers() ________________________________________________
//
// The buffers are ours, in memory, uploaded when the session ends. They only grow, so a frame
// settles on a size and isn't cleared for every session.

bool GlBackend::_reserveFrameBuffers( int nVertices, int nColors, int nExtras, int nObjects )
{
	auto reserve = [](auto& buffer, auto*& pBeg, auto*& pPtr, auto*& pEnd, int needed)
	{
		if( needed < 1 )
			needed = 1;

		if( int(buffer.size()) < needed )
			buffer.resize(needed);

		pBeg = buffer.data();
		pPtr = pBeg;
		pEnd = pBeg + buffer.size();
	};

	reserve(m_vertices, m_buffers.pVertexBeg, m_buffers.pVertexPtr, m_buffers.pVertexEnd, nVertices);
	reserve(m_colors, m_buffers.pColorBeg, m_buffers.pColorPtr, m_buffers.pColorEnd, nColors);
	reserve(m_extras, m_buffers.pExtrasBeg, m_buffers.pExtrasPtr, m_buffers.pExtrasEnd, nExtras);
	return true;
}

//____ _record() _____________________________________________________________

void GlBackend::_record( Op op, int a, int b, Object * pObject, const RectI& rect )
{
	m_ops.push_back( { op, a, b, pObject, rect } );
}

//____ _holdForSession() _____________________________________________________
//
// Keeps an object the recording points at until it has been played back.

void GlBackend::_holdForSession( Object * pObject )
{
	if( pObject && (m_sessionObjects.empty() || m_sessionObjects.back().rawPtr() != pObject) )
		m_sessionObjects.push_back(pObject);
}

//____ _bindCanvas() _________________________________________________________

bool GlBackend::_bindCanvas( Bitmap * pCanvas, SizeI& size, CanvasKind& kind )
{
	if( !m_bRendering )
		return false;

	auto pBitmap = static_cast<GlBitmap*>(pCanvas);

	if( pBitmap && (!pBitmap->m_bCanvas || pBitmap->getTexture() == 0) )
	{
		Base::reportError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Bitmap can not be used as canvas, rendering to default canvas instead.",
			this, &TYPEINFO, __func__, __FILE__, __LINE__);
		pBitmap = nullptr;
	}

	// How y is flipped, see compile_shaders.py. Bitmaps are drawn upside down to GL, so their
	// first row is at the top as on the other backends. In the window gl_FragCoord is upside down.

	if( pBitmap )
	{
		size = pBitmap->pixelSize();
		kind = _canvasKind(pBitmap);

		m_constants.glCanvas[0] = -1.f;
		m_constants.glCanvas[1] = 0.f;
		m_constants.glCanvas[2] = 1.f;
	}
	else
	{
		size = { m_defaultCanvas.size.w / 64, m_defaultCanvas.size.h / 64 };
		kind = m_bDefaultCanvasHasAlpha ? CanvasKind::WithAlpha : CanvasKind::WithoutAlpha;

		m_constants.glCanvas[0] = 1.f;
		m_constants.glCanvas[1] = float(size.h);
		m_constants.glCanvas[2] = -1.f;
	}

	m_constants.glCanvas[3] = 0.f;
	m_bConstantsChanged = true;

	_holdForSession(pBitmap);
	_record(Op::BindCanvas, 0, 0, pBitmap);

	m_bCanvasBound = true;
	m_targetSize = size;
	return true;
}

//____ _bindPipeline() _______________________________________________________
//
// A pipeline is a program, of which there is one per kind and one more for those that blend with
// a second color, and the blending, which is set when played back, see _executeSetPipeline().

bool GlBackend::_bindPipeline( PipelineKind kind, BlendMode blendMode )
{
	if( !m_bCanvasBound )
		return false;

	BlendState blend = _blendState(kind, blendMode, m_activeCanvasKind);

	if( _program(kind, blend.bDualSource) == 0 )
		return false;

	_record(Op::SetPipeline, int(kind), int(blendMode));
	return true;
}

//____ _setConstants() _______________________________________________________
//
// The shaders have the three blocks as one, see ShaderConstants, a copy of which goes with the
// next draw.

void GlBackend::_setConstants( ConstantBlock block )
{
	static_assert(sizeof(CanvasConstants) == 32 && sizeof(PixelConstants) == 16 && sizeof(WarpConstants) == 32, "Constant blocks must match constants.hlsl");
	static_assert(offsetof(ShaderConstants, pixel) == 32 && offsetof(ShaderConstants, warp) == 48 && offsetof(ShaderConstants, glCanvas) == 80
				  && sizeof(ShaderConstants) == 6 * 16, "Constant blocks must be where constants.hlsl and compile_shaders.py have them");

	switch( block )
	{
		case ConstantBlock::Canvas:
			m_constants.canvas = m_canvasConstants;
			break;

		case ConstantBlock::Pixel:
			m_constants.pixel = m_pixelConstants;
			break;

		case ConstantBlock::Warp:
			m_constants.warp = m_warpConstants;
			break;
	}

	m_bConstantsChanged = true;
}

//____ _pushConstants() ______________________________________________________

void GlBackend::_pushConstants()
{
	if( !m_bConstantsChanged )
		return;

	m_constantSnapshots.push_back(m_constants);
	_record(Op::SetConstants, int(m_constantSnapshots.size()) - 1);
	m_bConstantsChanged = false;
}

//____ _draw() _______________________________________________________________

void GlBackend::_draw( int firstVertex, int nVertices )
{
	if( !m_bCanvasBound || nVertices <= 0 )
		return;

	_pushConstants();
	_record(Op::Draw, firstVertex, nVertices);
}

//____ _setScissor() _________________________________________________________

void GlBackend::_setScissor( const RectI& rect )
{
	if( m_bCanvasBound )
		_record(Op::SetScissor, 0, 0, nullptr, rect);
}

//____ _resetScissor() _______________________________________________________

void GlBackend::_resetScissor()
{
	_setScissor( RectI(0, 0, m_targetSize.w, m_targetSize.h) );
}

//____ _prepareBlitSource() __________________________________________________
//
// GlBitmap copies what is written to its pixels to the texture right away.

void GlBackend::_prepareBlitSource( Bitmap * pBitmap )
{
}

//____ _bindBlitTexture() ____________________________________________________

bool GlBackend::_bindBlitTexture( Bitmap * pBitmap )
{
	auto pSource = static_cast<GlBitmap*>(pBitmap);

	if( !m_bCanvasBound || pSource->getTexture() == 0 )
		return false;

	if( pSource->pixelDescription()->type == PixelType::Index && pSource->getPaletteTexture() == 0 )
		return false;

	_holdForSession(pSource);
	_record(Op::BindBlitTexture, 0, 0, pSource);
	return true;
}

//____ _bindBlitSampler() ____________________________________________________
//
// Palette based sources are read with texelFetch(), but an integer texture must have a nearest
// filter to be complete.

void GlBackend::_bindBlitSampler( Bitmap * pBitmap, bool bClip )
{
	if( !m_bCanvasBound )
		return;

	bool bIndexed = pBitmap->pixelDescription()->type == PixelType::Index;
	bool bBilinear = !bIndexed && pBitmap->sampleMethod() == SampleMethod::Bilinear;
	bool bMipmapped = bBilinear && pBitmap->isMipmapped();

	int index = (bMipmapped ? 8 : 0) + (bClip ? 4 : 0) + (pBitmap->isTiling() ? 2 : 0) + (bBilinear ? 1 : 0);

	_record(Op::BindSampler, index);
}

//____ _bindEdgemap() ________________________________________________________

bool GlBackend::_bindEdgemap( GpuEdgemap * pEdgemap )
{
	auto pGlEdgemap = static_cast<GlEdgemap*>(pEdgemap);

	if( !m_bCanvasBound || pGlEdgemap->m_content.empty() )
		return false;

	_holdForSession(pGlEdgemap);
	_record(Op::BindEdgemap, 0, 0, pGlEdgemap);
	return true;
}

//____ _drawWarpmap() ________________________________________________________

void GlBackend::_drawWarpmap( Warpmap * pWarpmap, const RectI * pPatches, int nPatches )
{
	if( !m_bCanvasBound || pWarpmap->triangleCount() == 0 || nPatches <= 0 )
		return;

	_pushConstants();
	_holdForSession(pWarpmap);

	int first = int(m_patches.size());
	m_patches.insert(m_patches.end(), pPatches, pPatches + nPatches);

	_record(Op::DrawWarpmap, first, nPatches, pWarpmap);
	_resetScissor();
}

//____ _execute() ____________________________________________________________
//
// Uploads what the session has written and plays back the recording. Called by endSession().

void GlBackend::_execute()
{
	if( m_ops.empty() )
		return;

	LOG_GLERROR(glGetError());

	glBindVertexArray(m_vertexArrayId);

	// Uploads. Each replaces the storage of its buffer, so draws of the previous session still in
	// flight keep what they read.

	size_t nVertices = m_buffers.pVertexBeg ? size_t(m_buffers.pVertexPtr - m_buffers.pVertexBeg) : 0;
	size_t nColors = m_buffers.pColorBeg ? size_t(m_buffers.pColorPtr - m_buffers.pColorBeg) : 0;
	size_t nExtras = m_buffers.pExtrasBeg ? size_t(m_buffers.pExtrasPtr - m_buffers.pExtrasBeg) : 0;

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(std::max(nVertices, size_t(1)) * sizeof(Vertex)), m_vertices.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_TEXTURE_BUFFER, m_colorBufferId);
	glBufferData(GL_TEXTURE_BUFFER, GLsizeiptr(std::max(nColors, size_t(1)) * sizeof(Color8)), m_colors.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, m_extrasBufferId);
	glBufferData(GL_TEXTURE_BUFFER, GLsizeiptr(std::max(nExtras, size_t(1)) * sizeof(Extras)), m_extras.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	glActiveTexture(GL_TEXTURE0 + ColorsUnit);
	glBindTexture(GL_TEXTURE_BUFFER, m_colorBufferTex);
	glActiveTexture(GL_TEXTURE0 + ExtrasUnit);
	glBindTexture(GL_TEXTURE_BUFFER, m_extrasBufferTex);
	glActiveTexture(GL_TEXTURE0 + BlitSourceUnit);

	LOG_GLERROR(glGetError());

	m_pExecCanvas = nullptr;
	m_bExecCanvasOK = false;
	m_execProgram = 0;
	m_execProgramIndex = -1;
	m_execConstants = -1;
	m_execBlendKey = -1;

	for( int& constants : m_programConstants )
		constants = -1;

	for( const Op_t& op : m_ops )
	{
		switch( op.op )
		{
			case Op::BindCanvas:
				_executeBindCanvas( static_cast<GlBitmap*>(op.pObject) );
				break;

			case Op::SetPipeline:
				_executeSetPipeline( PipelineKind(op.a), BlendMode(op.b) );
				break;

			case Op::SetConstants:
				m_execConstants = op.a;
				break;

			case Op::Draw:
				if( m_bExecCanvasOK && m_execProgram != 0 )
				{
					_executeConstants();
					glDrawArrays(GL_TRIANGLES, op.a, op.b);
				}
				break;

			case Op::SetScissor:
				_executeScissor(op.rect);
				break;

			case Op::BindBlitTexture:
			{
				auto pSource = static_cast<GlBitmap*>(op.pObject);

				glBindTexture(GL_TEXTURE_2D, pSource->getTexture());

				if( pSource->isMipmapStale() )
				{
					glGenerateMipmap(GL_TEXTURE_2D);
					pSource->clearMipmapStale();
				}

				// A palette based source has its palette where edgemaps go, which is free while we blit.

				if( pSource->getPaletteTexture() != 0 )
				{
					glActiveTexture(GL_TEXTURE0 + EdgemapOrPaletteUnit);
					glBindTexture(GL_TEXTURE_BUFFER, pSource->getPaletteTexture());
					glActiveTexture(GL_TEXTURE0 + BlitSourceUnit);
				}
				break;
			}

			case Op::BindSampler:
				glBindSampler(BlitSourceUnit, m_samplers[op.a]);
				break;

			case Op::BindEdgemap:
			{
				auto pEdgemap = static_cast<GlEdgemap*>(op.pObject);

				pEdgemap->_upload();

				glActiveTexture(GL_TEXTURE0 + EdgemapOrPaletteUnit);
				glBindTexture(GL_TEXTURE_BUFFER, pEdgemap->m_textureId);
				glActiveTexture(GL_TEXTURE0 + BlitSourceUnit);
				break;
			}

			case Op::DrawWarpmap:
			{
				if( !m_bExecCanvasOK || m_execProgram == 0 )
					break;

				auto pWarpmap = static_cast<GlWarpmap*>(op.pObject);

				_executeConstants();
				glBindVertexArray(pWarpmap->m_vertexArrayId);

				for( int i = 0 ; i < op.b ; i++ )
				{
					_executeScissor(m_patches[op.a + i]);
					glDrawElements(GL_TRIANGLES, pWarpmap->triangleCount() * 3, GL_UNSIGNED_SHORT, nullptr);
				}

				glBindVertexArray(m_vertexArrayId);
				break;
			}
		}
	}

	// Leave GL as the application may expect it, see also endRender().

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindSampler(BlitSourceUnit, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	glFlush();

	m_pExecCanvas = nullptr;

	LOG_GLERROR(glGetError());
}

//____ _executeBindCanvas() __________________________________________________

void GlBackend::_executeBindCanvas( GlBitmap * pCanvas )
{
	SizeI size;

	m_bExecCanvasOK = true;

	if (pCanvas)
	{
		size = pCanvas->pixelSize();

		// An sRGB canvas has an sRGB texture, which GL blends in linear and writes as sRGB.

		if (pCanvas->colorSpace() == ColorSpace::sRGB)
			glEnable(GL_FRAMEBUFFER_SRGB);
		else
			glDisable(GL_FRAMEBUFFER_SRGB);

		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pCanvas->getTexture(), 0);

		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			Base::reportError(ErrorLevel::Error, ErrorCode::RenderFailure, "Bitmap could not be bound as canvas, nothing is drawn into it.",
				this, &TYPEINFO, __func__, __FILE__, __LINE__);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			m_bExecCanvasOK = false;
		}

		// What we draw isn't in its alpha map or mip levels.

		pCanvas->notifyRendered();

		m_execTargetKind = _canvasKind(pCanvas);
	}
	else
	{
		size = { m_defaultCanvas.size.w / 64, m_defaultCanvas.size.h / 64 };

		// The framebuffer needs to be sRGB-capable for this to have any effect.

		if (m_defaultCanvas.colorSpace == ColorSpace::sRGB)
			glEnable(GL_FRAMEBUFFER_SRGB);
		else
			glDisable(GL_FRAMEBUFFER_SRGB);

		// The one that was bound for drawing when beginRender() was called.

		glBindFramebuffer(GL_FRAMEBUFFER, GLuint(m_glDrawFrameBuffer));

		m_execTargetKind = m_bDefaultCanvasHasAlpha ? CanvasKind::WithAlpha : CanvasKind::WithoutAlpha;
	}

	m_pExecCanvas = pCanvas;
	m_bExecDefaultCanvas = (pCanvas == nullptr);
	m_execTargetSize = size;
	m_execBlendKey = -1;			// The canvas decides how we blend.

	glViewport(0, 0, size.w, size.h);
	glScissor(0, 0, size.w, size.h);
}

//____ _executeSetPipeline() _________________________________________________

//
// Blending is that of GpuBackend::_blendState(), in GL terms. An alpha only canvas is GL_R8, as in
// VulkanBackend.

void GlBackend::_executeSetPipeline( PipelineKind kind, BlendMode blendMode )
{
	static const GLenum factors[] = {										// In the order of BlendFactor.
		GL_ZERO, GL_ONE,
		GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
		GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR, GL_SRC1_ALPHA, GL_ONE_MINUS_SRC1_ALPHA };

	static const GLenum ops[] = {											// In the order of BlendOp.
		GL_FUNC_ADD, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX };

	static_assert(sizeof(factors) / sizeof(factors[0]) == size_t(BlendFactor::Size) && sizeof(ops) / sizeof(ops[0]) == size_t(BlendOp::Size),
				  "A GL value for each blend factor and operation");

	bool bPartial = _hasPartialCoverage(kind);

	BlendState blend = _blendState(kind, blendMode, m_execTargetKind);

	GLuint program = m_programs[int(kind)][blend.bDualSource ? 1 : 0];

	if( program != m_execProgram )
	{
		glUseProgram(program);
		m_execProgram = program;
		m_execProgramIndex = int(kind) * 2 + (blend.bDualSource ? 1 : 0);
	}

	int key = (int(blendMode) << 8) | (int(m_execTargetKind) << 4) | (bPartial ? 1 : 0);

	if( key == m_execBlendKey )
		return;

	m_execBlendKey = key;

	if( blend.bEnable )
	{
		glEnable(GL_BLEND);
		glBlendEquationSeparate(ops[int(blend.colorOp)], ops[int(blend.alphaOp)]);
		glBlendFuncSeparate(factors[int(blend.srcColor)], factors[int(blend.dstColor)], factors[int(blend.srcAlpha)], factors[int(blend.dstAlpha)]);
	}
	else
		glDisable(GL_BLEND);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, blend.bWriteAlpha ? GL_TRUE : GL_FALSE);
}

//____ _executeConstants() ___________________________________________________
//
// Uniforms belong to the program, so each program is given the constants of a draw when it hasn't
// got them yet.

void GlBackend::_executeConstants()
{
	if( m_execConstants < 0 || m_execProgramIndex < 0 || m_programConstants[m_execProgramIndex] == m_execConstants )
		return;

	glUniform4uiv(m_constantsLocations[m_execProgramIndex], 6, (const GLuint *) &m_constantSnapshots[m_execConstants]);
	m_programConstants[m_execProgramIndex] = m_execConstants;
}

//____ _executeScissor() _____________________________________________________
//
// GL counts rows from the bottom, which is where bitmaps have their first row, as they are drawn
// upside down to GL. The window has its first row at the top.

void GlBackend::_executeScissor( const RectI& rect )
{
	int x1 = std::max(rect.x, 0);
	int y1 = std::max(rect.y, 0);
	int x2 = std::min(rect.x + rect.w, m_execTargetSize.w);
	int y2 = std::min(rect.y + rect.h, m_execTargetSize.h);

	int w = std::max(x2 - x1, 0);
	int h = std::max(y2 - y1, 0);

	if( m_bExecDefaultCanvas )
		y1 = m_execTargetSize.h - (y1 + h);

	glScissor(x1, y1, w, h);
}

//____ _shaderSource() _______________________________________________________

const char * GlBackend::_shaderSource( const char * pName )
{
	for( int i = 0 ; i < s_nbShaderSources ; i++ )
	{
		if( std::strcmp(s_shaderSources[i].pName, pName) == 0 )
			return s_shaderSources[i].pSource;
	}

	return nullptr;
}

//____ _program() ____________________________________________________________
//
// The program of a kind of pipeline, or the one of it that blends with a second color. Created
// the first time it is asked for, as an application typically draws with a few of them only and
// compiling them all takes a while. 0 if it can't be created, which is only tried once.

GLuint GlBackend::_program( PipelineKind kind, bool bDualSource )
{
	int index = int(kind) * 2 + (bDualSource ? 1 : 0);
	GLuint& program = m_programs[int(kind)][bDualSource ? 1 : 0];

	if( program == 0 && !m_bProgramFailed[index] )
	{
		program = _createProgram(index);
		m_bProgramFailed[index] = (program == 0);

		LOG_GLERROR(glGetError());
	}

	return program;
}

//____ _createProgram() ______________________________________________________
//
// Loads the program from its binary if we have one, see shaderBlob(), and compiles it otherwise.

GLuint GlBackend::_createProgram( int index )
{
	ProgramBinary& binary = s_programBinaries[index];

	GLuint program = glCreateProgram();
	GLint linkStatus = GL_FALSE;

	if( m_bProgramBinariesSupported && !binary.data.empty() )
	{
		glProgramBinary(program, binary.format, binary.data.data(), GLsizei(binary.data.size()));
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

		if( linkStatus != GL_TRUE )
		{
			// The driver doesn't take it, as when it has been updated since the binary was made,
			// which then goes for the others too. Not an error of ours.

			for( auto& stale : s_programBinaries )
				stale = ProgramBinary();

			while( glGetError() != GL_NO_ERROR ) {}
		}
	}

	if( linkStatus != GL_TRUE )
	{
		const char * pVertexShaderName = _vertexShaderName(PipelineKind(index / 2));
		const char * pPixelShaderName = _pixelShaderName(PipelineKind(index / 2), (index % 2) != 0);

		const char * pVertexShaderSource = _shaderSource(pVertexShaderName);
		const char * pPixelShaderSource = _shaderSource(pPixelShaderName);

		if( !pVertexShaderSource || !pPixelShaderSource )
		{
			char buffer[128];
			snprintf(buffer, sizeof(buffer), "Shader %s or %s is missing, run compile_shaders.py.", pVertexShaderName, pPixelShaderName);
			Base::reportError(ErrorLevel::Error, ErrorCode::Internal, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);

			glDeleteProgram(program);
			return 0;
		}

		GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		glShaderSource(vertexShaderID, 1, &pVertexShaderSource, NULL);
		glCompileShader(vertexShaderID);

		glShaderSource(fragmentShaderID, 1, &pPixelShaderSource, NULL);
		glCompileShader(fragmentShaderID);

		glAttachShader(program, vertexShaderID);
		glAttachShader(program, fragmentShaderID);
		glLinkProgram(program);

		// glLinkProgram doesn't use glGetError

		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

		if( linkStatus != GL_TRUE )
		{
			GLchar	vertexShaderLog[2048] = "";
			GLchar	fragmentShaderLog[2048] = "";
			GLchar	programInfoLog[2048] = "";

			glGetShaderInfoLog(vertexShaderID, 2048, nullptr, vertexShaderLog);
			glGetShaderInfoLog(fragmentShaderID, 2048, nullptr, fragmentShaderLog);
			glGetProgramInfoLog(program, 2048, nullptr, programInfoLog);

			char	buffer[2048 * 3 + 256];

			snprintf(buffer, sizeof(buffer), "Failed compiling OpenGL shaders %s and %s\nVertexShaderLog: %s\nFragmentShaderLog: %s\nProgramInfoLog: %s",
					 pVertexShaderName, pPixelShaderName, vertexShaderLog, fragmentShaderLog, programInfoLog);
			Base::reportError(ErrorLevel::Error, ErrorCode::OpenGL, buffer, this, &TYPEINFO, __func__, __FILE__, __LINE__);
		}

		glDetachShader(program, vertexShaderID);
		glDetachShader(program, fragmentShaderID);

		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);

		if( linkStatus != GL_TRUE )
		{
			glDeleteProgram(program);
			return 0;
		}

		// Its binary, for the GlBackends to come and for shaderBlob().

		if( m_bProgramBinariesSupported )
		{
			GLint size = 0;
			glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &size);

			if( size > 0 )
			{
				GLsizei written = 0;

				binary.data.resize(size);
				glGetProgramBinary(program, size, &written, &binary.format, binary.data.data());
				binary.data.resize(written);
			}
		}
	}

	_setUniforms(program);
	m_constantsLocations[index] = glGetUniformLocation(program, "waxConstants");

	return program;
}

//____ _setUniforms() __________________________________________________
//
// Points the samplers at the texture units we bind to, see compile_shaders.py. GL 3.3 can't have
// it in the shaders.

void GlBackend::_setUniforms(GLuint progId)
{
	glUseProgram(progId);

	const struct { const char * pName; int unit; } samplers[] = {
		{ "waxBlitSource", BlitSourceUnit },
		{ "waxBlitSource1", BlitSourceUnit },
		{ "waxBlitSource2", BlitSourceUnit },
		{ "waxBuffer0", ColorsUnit },
		{ "waxBuffer1", ExtrasUnit },
		{ "waxBuffer3", EdgemapOrPaletteUnit } };

	for( auto& sampler : samplers )
	{
		GLint location = glGetUniformLocation(progId, sampler.pName);
		if( location != -1 )
			glUniform1i(location, sampler.unit);
	}

	glUseProgram(0);

	LOG_GLERROR(glGetError());
}

//____ setShaderBlob() _______________________________________________________
//
// Takes the binaries out of a blob shaderBlob() made, replacing those we have. A blob made for
// other shaders than ours, or cut short, is ignored.

void GlBackend::setShaderBlob( Blob * pBlob )
{
	for( auto& binary : s_programBinaries )
		binary = ProgramBinary();

	if( !pBlob || pBlob->size() < int(sizeof(ProgramBlobHeader)) )
		return;

	const char * pBuffer = (const char*) pBlob->data();
	const ProgramBlobHeader * pHeader = (const ProgramBlobHeader*) pBuffer;

	if( pHeader->version != c_versionNb || pHeader->nbPrograms != c_nbPrograms )
		return;

	for( int i = 0 ; i < c_nbPrograms ; i++ )
	{
		const ProgramBlobEntry& entry = pHeader->programs[i];

		if( entry.size > 0 && entry.offset >= int(sizeof(ProgramBlobHeader)) && entry.offset <= pBlob->size() - entry.size )
		{
			s_programBinaries[i].format = entry.binaryFormat;
			s_programBinaries[i].data.assign(pBuffer + entry.offset, pBuffer + entry.offset + entry.size);
		}
	}
}

//____ shaderBlob() __________________________________________________________
//
// A program not compiled yet has an entry of size 0.

Blob_p GlBackend::shaderBlob()
{
	int totalSize = 0;

	for( auto& binary : s_programBinaries )
		totalSize += int(binary.data.size());

	if( totalSize == 0 )
		return nullptr;

	auto pBlob = Blob::create(int(sizeof(ProgramBlobHeader)) + totalSize);

	char * pBuffer = (char*) pBlob->data();
	ProgramBlobHeader * pHeader = (ProgramBlobHeader*) pBuffer;

	std::memset(pHeader, 0, sizeof(ProgramBlobHeader));

	pHeader->version = c_versionNb;
	pHeader->nbPrograms = c_nbPrograms;

	int offset = int(sizeof(ProgramBlobHeader));

	for( int i = 0 ; i < c_nbPrograms ; i++ )
	{
		const ProgramBinary& binary = s_programBinaries[i];

		pHeader->programs[i].offset = offset;
		pHeader->programs[i].size = GLsizei(binary.data.size());
		pHeader->programs[i].binaryFormat = binary.format;

		if( !binary.data.empty() )
			std::memcpy(pBuffer + offset, binary.data.data(), binary.data.size());

		offset += int(binary.data.size());
	}

	return pBlob;
}

} // namespace wax
