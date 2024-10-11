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

#include <wg_glbackend.h>
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
const TypeInfo GlBackend::TYPEINFO = { "GlBackend", &GfxBackend::TYPEINFO };

Blob_p			GlBackend::s_pShaderPrograms = nullptr;


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

GlBackend_p GlBackend::create( int uboBindingPoint)
{
	GlBackend_p p(new GlBackend( uboBindingPoint ));

	if( !p->m_bFullyInitialized )
		return GlBackend_p(nullptr);

	return p;
}

//____ setCanvas() _________________________________________________________

void GlBackend::setCanvas(CanvasRef ref)
{
	if (ref == CanvasRef::Default)
	{
		m_pCommandQueue[m_commandQueueSize++] = (int)CommandGL::SetCanvas;
		m_objects.push_back(nullptr);

		m_pCanvas = nullptr;
	}
	else
	{
		//TODO: Error handling!
	}
}

void GlBackend::setCanvas(Surface* pSurface)
{
	m_pCommandQueue[m_commandQueueSize++] = (int) CommandGL::SetCanvas;
	m_objects.push_back(pSurface);

	m_pCanvas = pSurface;
}


void GlBackend::_setCanvas(Surface* pSurface)
{
	GlSurface* pCanvas = static_cast<GlSurface*>(pSurface);

	LOG_GLERROR(glGetError());

	bool bWasAlphaOnly = m_bActiveCanvasIsA8;

	SizeI size;

	if (pCanvas)
	{
		size = pCanvas->pixelSize();

		glEnable(GL_FRAMEBUFFER_SRGB);		// Always use SRGB on Canvas that is SRGB.

		glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferId);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pCanvas->getTexture(), 0);

		GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			m_pActiveCanvas = nullptr;
			m_activeCanvasSize = { size.w, size.h };
			m_bMipmappedActiveCanvas = false;

			//TODO: Signal error that we could not set the specified canvas.

			return;
		}
	}
	else
	{
		size = m_defaultCanvas.size / 64;

		if (GfxBase::defaultToSRGB())
			glEnable(GL_FRAMEBUFFER_SRGB);
		else
			glDisable(GL_FRAMEBUFFER_SRGB);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glViewport(0, 0, size.w, size.h);
	glScissor(0, 0, size.w, size.h);


	// Updating canvas info for our shaders

	m_canvasUBO.width = size.w;
	m_canvasUBO.height = size.h;
	m_canvasUBO.yOfs = pCanvas ? 0 : size.h;
	m_canvasUBO.yMul = pCanvas ? 1 : -1;

	glBindBuffer(GL_UNIFORM_BUFFER, m_canvasUBOId);
	int b = true; // bools in GLSL are represented as 4 bytes, so we store it in an integer
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CanvasUBO), &m_canvasUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	LOG_GLERROR(glGetError());

	m_pActiveCanvas = pCanvas;
	m_activeCanvasSize = { size.w, size.h };
	m_bMipmappedActiveCanvas = m_pActiveCanvas ? m_pActiveCanvas->m_bMipmapped : false;

	m_bActiveCanvasIsA8 = m_pActiveCanvas ? m_pActiveCanvas->pixelFormat() == PixelFormat::Alpha_8 : false;

//	if (m_bActiveCanvasIsA8 != bWasAlphaOnly)
//		_setBlendMode(m_activeBlendMode);

	// Reset states

	m_activeBlendMode = BlendMode::Blend;
	_setBlendMode(m_activeBlendMode);


	m_activeMorphFactor = 0.5f;
	m_bTintmapIsActive = false;
	m_tintColorOfs = -1;

/*
	m_colTrans.mode = TintMode::None;
	m_colTrans.flatTintColor = HiColor::White;
	m_colTrans.bTintOpaque = true;
	m_colTrans.tintRect.clear();
	m_colTrans.pTintAxisX = nullptr;
	m_colTrans.pTintAxisY = nullptr;
	m_colTrans.morphFactor = 2048;
	m_colTrans.fixedBlendColor = HiColor::White;

*/

}

//____ setObjects() ________________________________________________________

void GlBackend::setObjects(Object** pBeg, Object** pEnd)
{
	m_pObjectsBeg = pBeg;
	m_pObjectsEnd = pEnd;
}

//____ setCoords() _________________________________________________________

void GlBackend::setCoords(spx* pBeg, spx* pEnd)
{
	m_pCoordsBeg = pBeg;
	m_pCoordsEnd = pEnd;
	m_pCoordsPtr = pBeg;
}

//____ setColors() _________________________________________________________

void GlBackend::setColors(HiColor* pBeg, HiColor* pEnd)
{
	m_pColorsBeg = pBeg;
	m_pColorsEnd = pEnd;
	m_pColorsPtr = pBeg;
}

//____ setTransforms() _____________________________________________________

void GlBackend::setTransforms(Transform* pBeg, Transform* pEnd)
{
	m_pTransformsBeg = pBeg;
	m_pTransformsEnd = pEnd;

}

//____ processCommands() _________________________________________________

void GlBackend::processCommands(int32_t* pBeg, int32_t* pEnd)
{
	spx* pCoords = m_pCoordsPtr;
	HiColor* pColors = m_pColorsPtr;

	VertexGL *	pVertexGL	= m_pVertexBuffer + m_nVertices;
	ColorGL*	pColorGL	= m_pColorBuffer + m_nColors;
	GLfloat*	pExtrasGL	= m_pExtrasBuffer + m_extrasBufferSize;
	int*		pCommandGL	= m_pCommandQueue + m_commandQueueSize;

	auto p = pBeg;
	while (p < pEnd)
	{
		auto cmd = Command(*p++);
		switch (cmd)
		{
		case Command::None:
			break;

		case Command::StateChange:
		{
			int32_t statesChanged = *p++;

			* pCommandGL++ = (int) CommandGL::StateChange;
			* pCommandGL++ = statesChanged;

			if (statesChanged & uint8_t(StateChange::BlitSource))
			{
				int32_t objectOfs = *p++;

				auto pSource = static_cast<GlSurface*>(m_pObjectsBeg[objectOfs]);

				// Save size and sampleMethod for blit-command processing

				m_blitSourceSize = CoordF(pSource->pixelSize().w, pSource->pixelSize().h);
				m_blitSourceSampleMethod = pSource->sampleMethod();

				// Save pointer for later processing, needs to be applied between draw calls.

				m_objects.push_back(pSource);
			}

			if (statesChanged & uint8_t(StateChange::BlendMode))
			{
				// Save for later processing, needs to be applied between draw calls.

				* pCommandGL++ = *p++;
			}

			if (statesChanged & uint8_t(StateChange::TintColor))
			{
				// Process right away, tint color ofs is stored in vertex data

				HiColor tintColor = *pColors++;

				if (tintColor == HiColor::White || tintColor == HiColor::Undefined)
				{
					m_tintColorOfs = -1;
				}
				else
				{
					m_tintColorOfs = pColorGL - m_pColorBuffer;

					pColorGL->r = tintColor.r / 4096.f;
					pColorGL->g = tintColor.g / 4096.f;
					pColorGL->b = tintColor.b / 4096.f;
					pColorGL->a = tintColor.a / 4096.f;
					pColorGL++;
	 			}
				m_bTintmap = false;
			}

			if (statesChanged & uint8_t(StateChange::TintMap))
			{
				int32_t objectOfs = *p++;
				int32_t	x = *p++;
				int32_t	y = *p++;
				int32_t	w = *p++;
				int32_t	h = *p++;


//				m_colTrans.tintRect = RectI(x, y, w, h);

				auto pTintmap = static_cast<Tintmap*>(m_pObjectsBeg[objectOfs]);

				bool bHorizontal = false, bVertical = false;

				m_bTintmap = true;
				m_tintmapRect = RectI(x, y, w, h) / 64;

				if (pTintmap->isHorizontal())
				{
					int nColors = w / 64;

					m_tintmapBeginX = pColorGL - m_pColorBuffer;
					m_tintmapEndX = pColorGL - m_pColorBuffer + nColors;


					HiColor * pTmp = (HiColor*) GfxBase::memStackAlloc(sizeof(HiColor) * nColors);

					pTintmap->exportHorizontalColors(w, pTmp);
					for (int i = 0; i < nColors; i++)
					{
						pColorGL->r = pTmp->r / 4096.f;
						pColorGL->g = pTmp->g / 4096.f;
						pColorGL->b = pTmp->b / 4096.f;
						pColorGL->a = pTmp->a / 4096.f;
						pColorGL++;
						pTmp++;
					}

					GfxBase::memStackFree(sizeof(HiColor) * nColors);
					bHorizontal = true;
				}
				else
				{
					// Default to use white

					m_tintmapBeginX = 0;
					m_tintmapEndX = 0;
				}


				if (pTintmap->isVertical())
				{
					int nColors = h / 64;

					m_tintmapBeginY = pColorGL - m_pColorBuffer;
					m_tintmapEndY = pColorGL - m_pColorBuffer + nColors;

					HiColor* pTmp = (HiColor*)GfxBase::memStackAlloc(sizeof(HiColor) * nColors);

					pTintmap->exportVerticalColors(h, pTmp);
					for (int i = 0; i < nColors; i++)
					{
						pColorGL->r = pTmp->r / 4096.f;
						pColorGL->g = pTmp->g / 4096.f;
						pColorGL->b = pTmp->b / 4096.f;
						pColorGL->a = pTmp->a / 4096.f;
						pColorGL++;
						pTmp++;
					}

					GfxBase::memStackFree(sizeof(HiColor) * nColors);
					bVertical = true;
				}
				else
				{
					// Default to use white

					m_tintmapBeginY= 0;
					m_tintmapEndY = 0;
				}

				m_tintColorOfs = -1;
			}

			if (statesChanged & uint8_t(StateChange::MorphFactor))
			{
				// Save for later processing, needs to be applied between draw calls.

				*pCommandGL++ = *p++;
			}

			if (statesChanged & uint8_t(StateChange::FixedBlendColor))
			{
				// Just retrieve the value but do nothing. We use BlendMode::Blend 
				// in place of BlendMode::BlendFixedColor.

				HiColor fixedBlendColor = *pColors++;
			}

			if (statesChanged & uint8_t(StateChange::Blur))
			{
				int32_t objectOfs = *p++;

				auto pBlurbrush = static_cast<Blurbrush*>(m_pObjectsBeg[objectOfs]);
				m_objects.push_back(pBlurbrush);
			}

			break;
		}
		case Command::Fill:
		{
			int32_t nRects = *p++;

			HiColor& col = *pColors++;

			bool	bStraightFill = true;
			int		nRectsWritten = 0;
			int		extrasOfs = 0;

			// Setup Tintmap



			// Add rects to vertex buffer

			for (int i = 0; i < nRects; i++)
			{
	
				int	dx1 = * pCoords++;
				int	dy1 = * pCoords++;
				int dx2 = dx1 + *pCoords++;
				int dy2 = dy1 + *pCoords++;


				if (((dx1 | dy1 | dx2 | dy2) & 63) == 0)
				{
					// Straight fill

					if (!bStraightFill)
					{
						if( nRectsWritten > 0 )
						{
							*pCommandGL++ = (int)CommandGL::SubpixelFill;
							*pCommandGL++ = nRectsWritten * 6;
							nRectsWritten = 0;
							extrasOfs = 0;
						}
						bStraightFill = true;
					}
				}
				else
				{
					// Subpixel fill

					if(bStraightFill)
					{
						if (nRectsWritten > 0)
						{
							*pCommandGL++ = (int)CommandGL::StraightFill;
							*pCommandGL++ = nRectsWritten * 6;
							nRectsWritten = 0;
						}
						bStraightFill = false;
					}

					extrasOfs = (pExtrasGL - m_pExtrasBuffer)/4;

					// Provide rectangle center and radius as extras

					RectSPX rect(dx1, dy1, dx2 - dx1, dy2 - dy1);

					SizeF    radius(rect.w / (2.f * 64), rect.h / (2.f * 64));
					CoordF    center((rect.x / 64.f) + radius.w, (rect.y / 64.f) + radius.h);

					* pExtrasGL++ = center.x;
					* pExtrasGL++ = center.y;
					* pExtrasGL++ = radius.w;
					* pExtrasGL++ = radius.h;

					dx2 += 63;
					dy2 += 63;
				}

				dx1 >>= 6;
				dy1 >>= 6;
				dx2 >>= 6;
				dy2 >>= 6;


				float tintmapBeginX, tintmapBeginY, tintmapEndX, tintmapEndY;

				if (m_bTintmap)
				{
					if (m_tintmapBeginX == 0)
					{
						tintmapBeginX = 0.5f;
						tintmapEndX = 0.5f;
					}
					else
					{
						tintmapBeginX = m_tintmapBeginX + (dx1 - m_tintmapRect.x) + 0.5f;
						tintmapEndX = tintmapBeginX + (dx2 - dx1) + 0.5f;
					}

					if (m_tintmapBeginY == 0)
					{
						tintmapBeginY = 0.5f;
						tintmapEndY = 0.5f;
					}
					else
					{
						tintmapBeginY = m_tintmapBeginY + (dy1 - m_tintmapRect.y) + 0.5f;
						tintmapEndY = tintmapBeginY + (dy2 - dy1) + 0.5f;
					}

				}
				else
				{
					tintmapBeginX = 0.5f;
					tintmapBeginY = 0.5f; 
					tintmapEndX = 0.5f;
					tintmapEndY = 0.5f;
				}

				pVertexGL->coord.x = dx1;
				pVertexGL->coord.y = dy1;
				pVertexGL->colorsOfs = pColorGL - m_pColorBuffer;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapBeginX,tintmapBeginY };
				pVertexGL++;

				pVertexGL->coord.x = dx2;
				pVertexGL->coord.y = dy1;
				pVertexGL->colorsOfs = pColorGL - m_pColorBuffer;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapEndX,tintmapBeginY };
				pVertexGL++;

				pVertexGL->coord.x = dx2;
				pVertexGL->coord.y = dy2;
				pVertexGL->colorsOfs = pColorGL - m_pColorBuffer;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapEndX,tintmapEndY };
				pVertexGL++;

				pVertexGL->coord.x = dx1;
				pVertexGL->coord.y = dy1;
				pVertexGL->colorsOfs = pColorGL - m_pColorBuffer;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapBeginX,tintmapBeginY };
				pVertexGL++;

				pVertexGL->coord.x = dx2;
				pVertexGL->coord.y = dy2;
				pVertexGL->colorsOfs = pColorGL - m_pColorBuffer;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapEndX,tintmapEndY };
				pVertexGL++;

				pVertexGL->coord.x = dx1;
				pVertexGL->coord.y = dy2;
				pVertexGL->colorsOfs = pColorGL - m_pColorBuffer;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapBeginX,tintmapEndY };
				pVertexGL++;

				nRectsWritten++;
			}

			// Add colors to buffer

			if (m_tintColorOfs >= 0)
			{
				ColorGL& tint = m_pColorBuffer[m_tintColorOfs];

				pColorGL->r = (col.r / 4096.f) * tint.r;
				pColorGL->g = (col.g / 4096.f) * tint.g;
				pColorGL->b = (col.b / 4096.f) * tint.b;
				pColorGL->a = (col.a / 4096.f) * tint.a;

			}
			else
			{
				pColorGL->r = col.r / 4096.f;
				pColorGL->g = col.g / 4096.f;
				pColorGL->b = col.b / 4096.f;
				pColorGL->a = col.a / 4096.f;
			}

			pColorGL++;

			// Store command

			* pCommandGL++ = (int) bStraightFill ? CommandGL::StraightFill : CommandGL::SubpixelFill;
			* pCommandGL++ = nRectsWritten * 6;

			break;
		}

		case Command::Plot:
		{
			int nCoords = *p++;

			for (int pixel = 0; pixel < nCoords; pixel++)
			{
				pVertexGL->coord.x = * pCoords++ / 64;
				pVertexGL->coord.y = * pCoords++ / 64;
				pVertexGL->colorsOfs = pColorGL - m_pColorBuffer;
				pVertexGL++;

				HiColor col = * pColors++;

				pColorGL->r = col.r / 4096.f;
				pColorGL->g = col.g / 4096.f;
				pColorGL->b = col.b / 4096.f;
				pColorGL->a = col.a / 4096.f;

				pColorGL++;

			}

			// Store command

			*pCommandGL++ = CommandGL::Plot;
			*pCommandGL++ = nCoords;
			break;
		}

		case Command::Line:
		{
			float thickness = *p++ / 64.f;
			int32_t nClipRects = *p++;
			int32_t nLines = *p++;

			HiColor col = *pColors++;

			const RectSPX* pClipRects = reinterpret_cast<const RectSPX*>(pCoords);

			pCoords += nClipRects * 4;

			// Calculate and store vertices

			int nLinesWritten = 0;

			for (int i = 0; i < nLines; i++)
			{
				CoordSPX begin = { *pCoords++, *pCoords++ };
				CoordSPX end = { *pCoords++, *pCoords++ };

				begin = roundToPixels(begin);
				end = roundToPixels(end);

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
						continue;											// TODO: Should stil draw the caps!

					slope = ((float)(end.y - begin.y)) / length;
					width = _scaleThickness(thickness, slope);
					bSteep = false;

					if( m_pCanvas )
						s = ((begin.y + 0.5f) - (begin.x + 0.5f) * slope);
					else
						s = m_defaultCanvas.size.h / 64 - ((begin.y + 0.5f) - (begin.x + 0.5f) * slope);

					w = width / 2 + 0.5f;

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
						continue;											// TODO: Should stil draw the caps!

					slope = ((float)(end.x - begin.x)) / length;
					width = _scaleThickness(thickness, slope);
					bSteep = true;

					s = (begin.x + 0.5f) - (m_canvasUBO.yOfs * m_canvasUBO.yMul + (begin.y + 0.5f)) * slope;
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

				int extrasOfs = (pExtrasGL - m_pExtrasBuffer) / 4;
				int colorsOfs = (pColorGL - m_pColorBuffer);

				pVertexGL->coord = c1;
				pVertexGL->colorsOfs = colorsOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL++;

				pVertexGL->coord = c2;
				pVertexGL->colorsOfs = colorsOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL++;

				pVertexGL->coord = c3;
				pVertexGL->colorsOfs = colorsOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL++;

				pVertexGL->coord = c1;
				pVertexGL->colorsOfs = colorsOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL++;

				pVertexGL->coord = c3;
				pVertexGL->colorsOfs = colorsOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL++;

				pVertexGL->coord = c4;
				pVertexGL->colorsOfs = colorsOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL++;

				*pExtrasGL++ = s;
				*pExtrasGL++ = w;
				*pExtrasGL++ = slope;
				*pExtrasGL++ = bSteep;

				nLinesWritten++;
			}

			// Store command, with clip rects

			if (nLinesWritten > 0)
			{
				*pCommandGL++ = CommandGL::Lines;
				*pCommandGL++ = nClipRects;
				*pCommandGL++ = nLinesWritten * 6;

				for (int i = 0; i < nClipRects; i++)
				{
					RectSPX r = *pClipRects++;
					*pCommandGL++ = r.x >> 6;
					*pCommandGL++ = r.y >> 6;
					*pCommandGL++ = r.w >> 6;
					*pCommandGL++ = r.h >> 6;
				}

				pColorGL->r = col.r / 4096.f;
				pColorGL->g = col.g / 4096.f;
				pColorGL->b = col.b / 4096.f;
				pColorGL->a = col.a / 4096.f;
				pColorGL++;
			}

			break;
		}

		case Command::DrawEdgemap:
		{
			int32_t objectOfs = *p++;
			auto pEdgemap = static_cast<GlEdgemap*>(m_pObjectsBeg[objectOfs]);

			int32_t	destX = *p++;
			int32_t	destY = *p++;

			int32_t	flip = *p++;

			int32_t nRects = *p++;
			RectSPX* pRects = reinterpret_cast<RectSPX*>(pCoords);

			pCoords += nRects * 4;

			//

			auto& mtx = s_blitFlipTransforms[flip];

			RectSPX _destIn = {
				destX,
				destY,
				pEdgemap->m_size.w * 64 * int(abs(mtx.xx)) + pEdgemap->m_size.h * 64 * int(abs(mtx.yx)),
				pEdgemap->m_size.w * 64 * int(abs(mtx.xy)) + pEdgemap->m_size.h * 64 * int(abs(mtx.yy)),
			};
			/*

	void GlGfxDevice::_transformDrawSegments( const RectSPX& _destIn, int nSegments, const HiColor * pSegmentColors, 
	int nEdgeStrips, const int * pEdgeStrips, int edgeStripPitch, TintMode tintMode, const int simpleTransform[2][2] )


			_transformDrawSegments(dest, pWave->m_nbRenderSegments, pWave->m_pRenderColors,
				pWave->m_size.w + 1, pWave->m_pSamples, pWave->m_nbSegments - 1, pWave->m_tintMode,
				transform);
			*/

			int nSegments = pEdgemap->m_nbRenderSegments;
			const HiColor * pSegmentColors = pEdgemap->m_pRenderColors;
			int nEdgeStrips = pEdgemap->m_size.w + 1;
			const int* pEdgeStrips = pEdgemap->m_pSamples;
			int edgeStripPitch = pEdgemap->m_nbSegments - 1;
			TintMode tintMode = pEdgemap->m_tintMode;



			//TODO: Proper 26:6 support
			RectI _dest = roundToPixels(_destIn);

			//

			int extrasSpaceNeeded = 8 + 4 * (nEdgeStrips - 1) * (nSegments - 1);		// Various data + colors + strips

	
			// Do transformations

			RectI dest = _dest;

			int uIncX = mtx.xx;
			int vIncX = mtx.xy;
			int uIncY = mtx.yx;
			int vIncY = mtx.yy;

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

			for (int i = 0; i < nRects; i++)
			{
				RectI patch = (*pRects++) / 64;
				
				int		dx1 = patch.x;
				int		dx2 = patch.x + patch.w;
				int		dy1 = patch.y;
				int		dy2 = patch.y + patch.h;

				// Calc UV-coordinates. U is edge offset, V is pixel offset from begin in column.

				float	u1 = (float)(uTopLeft + (patch.x - dest.x) * mtx.xx + (patch.y - dest.y) * mtx.yx);
				float	v1 = (float)(vTopLeft + (patch.x - dest.x) * mtx.xy + (patch.y - dest.y) * mtx.yy);

				float	u2 = (float)(uTopLeft + (patch.x + patch.w - dest.x) * mtx.xx + (patch.y - dest.y) * mtx.yx);
				float	v2 = (float)(vTopLeft + (patch.x + patch.w - dest.x) * mtx.xy + (patch.y - dest.y) * mtx.yy);

				float	u3 = (float)(uTopLeft + (patch.x + patch.w - dest.x) * mtx.xx + (patch.y + patch.h - dest.y) * mtx.yx);
				float	v3 = (float)(vTopLeft + (patch.x + patch.w - dest.x) * mtx.xy + (patch.y + patch.h - dest.y) * mtx.yy);

				float	u4 = (float)(uTopLeft + (patch.x - dest.x) * mtx.xx + (patch.y + patch.h - dest.y) * mtx.yx);
				float	v4 = (float)(vTopLeft + (patch.x - dest.x) * mtx.xy + (patch.y + patch.h - dest.y) * mtx.yy);

				CoordF	uv1 = { u1, v1 - 0.5f };
				CoordF	uv2 = { u2, v2 - 0.5f };
				CoordF	uv3 = { u3, v3 - 0.5f };
				CoordF	uv4 = { u4, v4 - 0.5f };

				int extrasOfs = (pExtrasGL - m_pExtrasBuffer) / 4;
				int colorsOfs = (pColorGL - m_pColorBuffer);

				//

				pVertexGL->coord.x = dx1;
				pVertexGL->coord.y = dy1;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->uv = uv1;
				pVertexGL++;

				pVertexGL->coord.x = dx2;
				pVertexGL->coord.y = dy1;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->uv = uv2;
				pVertexGL++;

				pVertexGL->coord.x = dx2;
				pVertexGL->coord.y = dy2;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->uv = uv3;
				pVertexGL++;

				pVertexGL->coord.x = dx1;
				pVertexGL->coord.y = dy1;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->uv = uv1;
				pVertexGL++;

				pVertexGL->coord.x = dx2;
				pVertexGL->coord.y = dy2;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->uv = uv3;
				pVertexGL++;

				pVertexGL->coord.x = dx1;
				pVertexGL->coord.y = dy2;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->uv = uv4;
				pVertexGL++;
			}

			// Setup extras data


			// Add various data to extras

			int edgeStripOfs = (pExtrasGL - m_pExtrasBuffer + 8);	// Offset for edgestrips in buffer.

			pExtrasGL[0] = (GLfloat)nSegments;
			pExtrasGL[1] = (GLfloat)edgeStripOfs / 4;
			pExtrasGL[2] = (GLfloat)((_dest.w) * abs(mtx.xx) + (_dest.h) * abs(mtx.yx));
			pExtrasGL[3] = (GLfloat)((_dest.w) * abs(mtx.xy) + (_dest.h) * abs(mtx.yy));
			pExtrasGL[4] = GLfloat(0.25f / c_maxSegments);
			pExtrasGL[5] = GLfloat(m_segmentsTintTexOfs + 0.25f) / c_segmentsTintTexMapSize;
			pExtrasGL[6] = GLfloat(c_maxSegments * 2);
			pExtrasGL[7] = GLfloat(c_segmentsTintTexMapSize * 2);

			pExtrasGL += 8;												// Alignment for vec4 reads.

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

			const int* pEdges = pEdgeStrips;

			for (int i = 0; i < nEdgeStrips - 1; i++)
			{
				for (int j = 0; j < nSegments - 1; j++)
				{
					int edgeIn = pEdges[j];
					int edgeOut = pEdges[edgeStripPitch + j];

					if (edgeIn > edgeOut)
						std::swap(edgeIn, edgeOut);

					float increment = edgeOut == edgeIn ? 100.f : 64.f / (edgeOut - edgeIn);
					float beginAdder;
					float endAdder;

					if ((edgeOut & 0xFFFFFFC0) <= (unsigned int)edgeIn)
					{
						float firstPixelCoverage = ((64 - (edgeOut & 0x3F)) + (edgeOut - edgeIn) / 2) / 64.f;

						beginAdder = increment * (edgeIn & 0x3F) / 64.f + firstPixelCoverage;
						endAdder = beginAdder;
					}
					else
					{
						int height = 64 - (edgeIn & 0x3F);
						int width = (int)(increment * height);
						float firstPixelCoverage = (height * width) / (2 * 4096.f);
						float lastPixelCoverage = 1.f - (edgeOut & 0x3F) * increment * (edgeOut & 0x3F) / (2 * 4096.f);

						beginAdder = increment * (edgeIn & 0x3F) / 64.f + firstPixelCoverage;
						endAdder = lastPixelCoverage - (1.f - (edgeOut & 0x3F) * increment / 64.f);
						// 					endAdder = lastPixelCoverage - ((edgeOut & 0xFFFFFF00)-edgeIn)*increment / 256.f;
					}

					*pExtrasGL++ = edgeIn / 64.f;					// Segment begin pixel
					*pExtrasGL++ = increment;						// Segment increment
					*pExtrasGL++ = beginAdder;					// Segment begin adder
					*pExtrasGL++ = endAdder;						// Segment end adder
				}

				pEdges += edgeStripPitch;
			}

			break;
		}
/*
		case Command::Blur:
		{
			int32_t nRects = *p++;
			int32_t transform = *p++;

			int srcX = *p++;
			int srcY = *p++;
			spx dstX = *p++;
			spx dstY = *p++;

			pCoords += nRects * 4;

			break;
		}
*/

		case Command::Blit:
		case Command::ClipBlit:
		case Command::Tile:
		case Command::Blur:
		{
			int32_t nRects = *p++;
			int32_t transform = *p++;

			int srcX = *p++;
			int srcY = *p++;
			spx dstX = *p++;
			spx dstY = *p++;

			//

			int tintColorOfs = m_tintColorOfs >= 0 ? m_tintColorOfs : 0;
			int extrasOfs = (pExtrasGL - m_pExtrasBuffer) / 4;

			for (int i = 0; i < nRects; i++)
			{
				int	dx1 = (*pCoords++) >> 6;
				int	dy1 = (*pCoords++) >> 6;
				int dx2 = dx1 + ((*pCoords++) >> 6);
				int dy2 = dy1 + ((*pCoords++) >> 6);

				float tintmapBeginX, tintmapBeginY, tintmapEndX, tintmapEndY;

				if (m_bTintmap)
				{
					if (m_tintmapBeginX == 0)
					{
						tintmapBeginX = 0.5f;
						tintmapEndX = 0.5f;
					}
					else
					{
						tintmapBeginX = m_tintmapBeginX + (dx1 - m_tintmapRect.x) + 0.5f;
						tintmapEndX = tintmapBeginX + (dx2 - dx1) + 0.5f;
					}

					if (m_tintmapBeginY == 0)
					{
						tintmapBeginY = 0.5f;
						tintmapEndY = 0.5f;
					}
					else
					{
						tintmapBeginY = m_tintmapBeginY + (dy1 - m_tintmapRect.y) + 0.5f;
						tintmapEndY = tintmapBeginY + (dy2 - dy1) + 0.5f;
					}
				}
				else
				{
					tintmapBeginX = 0.5f;
					tintmapBeginY = 0.5f;
					tintmapEndX = 0.5f;
					tintmapEndY = 0.5f;
				}


				pVertexGL->coord.x = dx1;
				pVertexGL->coord.y = dy1;
				pVertexGL->uv = m_blitSourceSize;
				pVertexGL->colorsOfs = tintColorOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapBeginX,tintmapBeginY };
				pVertexGL++;

				pVertexGL->coord.x = dx2;
				pVertexGL->coord.y = dy1;
				pVertexGL->uv = m_blitSourceSize;
				pVertexGL->colorsOfs = tintColorOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapEndX,tintmapBeginY };
				pVertexGL++;

				pVertexGL->coord.x = dx2;
				pVertexGL->coord.y = dy2;
				pVertexGL->uv = m_blitSourceSize;
				pVertexGL->colorsOfs = tintColorOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapEndX,tintmapEndY };
				pVertexGL++;

				pVertexGL->coord.x = dx1;
				pVertexGL->coord.y = dy1;
				pVertexGL->uv = m_blitSourceSize;
				pVertexGL->colorsOfs = tintColorOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapBeginX,tintmapBeginY };
				pVertexGL++;

				pVertexGL->coord.x = dx2;
				pVertexGL->coord.y = dy2;
				pVertexGL->uv = m_blitSourceSize;
				pVertexGL->colorsOfs = tintColorOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapEndX,tintmapEndY };
				pVertexGL++;

				pVertexGL->coord.x = dx1;
				pVertexGL->coord.y = dy2;
				pVertexGL->uv = m_blitSourceSize;
				pVertexGL->colorsOfs = tintColorOfs;
				pVertexGL->extrasOfs = extrasOfs;
				pVertexGL->tintmapOfs = { tintmapBeginX,tintmapEndY };
				pVertexGL++;

			}

			if (m_blitSourceSampleMethod == SampleMethod::Bilinear)
			{
				* pExtrasGL++ = srcX / 1024.f + 0.5f;
				* pExtrasGL++ = srcY / 1024.f + 0.5f;
				* pExtrasGL++ = GLfloat(dstX >> 6) + 0.5f;
				* pExtrasGL++ = GLfloat(dstY >> 6) + 0.5f;
			}
			else
			{
				*pExtrasGL++ = srcX / 1024.f;
				*pExtrasGL++ = srcY / 1024.f;
				*pExtrasGL++ = GLfloat(dstX >> 6) + 0.5f;
				*pExtrasGL++ = GLfloat(dstY >> 6) + 0.5f;
			}

			auto& mtx = transform < GfxFlip_size ? s_blitFlipTransforms[transform] : m_pTransformsBeg[transform - GfxFlip_size];


			*pExtrasGL++ = mtx.xx;
			*pExtrasGL++ = mtx.xy;
			*pExtrasGL++ = mtx.yx;
			*pExtrasGL++ = mtx.yy;

			// Store command

			CommandGL commandGL = cmd == Command::Blur ? CommandGL::Blur : CommandGL::Blit;

			*pCommandGL++ = (int) commandGL;
			*pCommandGL++ = nRects * 6;

			break;
		}

		default:
			break;
		} 
	} 

	// Save progress.

	m_pCoordsPtr = pCoords;
	m_pColorsPtr = pColors;

	m_nVertices			= pVertexGL - m_pVertexBuffer;
	m_nColors			= pColorGL	- m_pColorBuffer;
	m_extrasBufferSize	= pExtrasGL - m_pExtrasBuffer;
	m_commandQueueSize	= pCommandGL- m_pCommandQueue;
}


//____ _setUniforms() __________________________________________________

void GlBackend::_setUniforms(GLuint progId, int uboBindingPoint)
{
	LOG_GLERROR(glGetError());

	glUseProgram(progId);

	LOG_GLERROR(glGetError());

	GLint texIdLoc = glGetUniformLocation(progId, "texId");

	if (texIdLoc != -1)
	{
		glUniform1i(texIdLoc, 0);			// Needs to be set. Texture unit 0 is used for textures.
	}

	GLint colorBufferIdLoc = glGetUniformLocation(progId, "colorBufferId");

	LOG_GLERROR(glGetError());

	if (colorBufferIdLoc != -1)
	{
		glUniform1i(colorBufferIdLoc, 1);		// Needs to be set. Texture unit 1 is used for color buffer.
	}

	GLint tintmapBufferIdLoc = glGetUniformLocation(progId, "tintmapBufferId");

	LOG_GLERROR(glGetError());

	if (tintmapBufferIdLoc != -1)
	{
		glUniform1i(tintmapBufferIdLoc, 1);		// Needs to be set. Texture unit 1 is used for color buffer.
	}


	LOG_GLERROR(glGetError());

	GLint extrasBufferIdLoc = glGetUniformLocation(progId, "extrasBufferId");

	LOG_GLERROR(glGetError());

	if (extrasBufferIdLoc != -1)
	{
		glUniform1i(extrasBufferIdLoc, 2);		// Needs to be set. Texture unit 2 is used for extras.
	}

	LOG_GLERROR(glGetError());

	GLint paletteIdLoc = glGetUniformLocation(progId, "paletteId");

	if (paletteIdLoc != -1)
	{
		glUniform1i(paletteIdLoc, 3);			// Needs to be set. Texture unit 3 is used for palette.
	}

	GLuint canvasIndex = glGetUniformBlockIndex(progId, "Canvas");

	if( canvasIndex != GL_INVALID_INDEX)
		glUniformBlockBinding(progId, canvasIndex, 0);

	LOG_GLERROR(glGetError());
}

//____ constructor _____________________________________________________________________

GlBackend::GlBackend( int uboBindingPoint )
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
			m_blitProgMatrix[i][0][1][j] = m_blitTintmapProg[j];
			m_blitProgMatrix[i][1][1][j] = m_blitTintmapProg[j];
		}
	}


	m_blitProgMatrix[(int)PixelFormat::Alpha_8][0][0][0] = m_alphaBlitProg[0];
	m_blitProgMatrix[(int)PixelFormat::Alpha_8][1][0][0] = m_alphaBlitProg[0];

	m_blitProgMatrix[(int)PixelFormat::Alpha_8][0][1][0] = m_alphaBlitTintmapProg[0];
	m_blitProgMatrix[(int)PixelFormat::Alpha_8][1][1][0] = m_alphaBlitTintmapProg[0];

	m_blitProgMatrix[(int)PixelFormat::Index_8_linear][0][0][0] = m_paletteBlitNearestProg[0];
	m_blitProgMatrix[(int)PixelFormat::Index_8_linear][1][0][0] = m_paletteBlitInterpolateProg[0];

	m_blitProgMatrix[(int)PixelFormat::Index_8_linear][0][1][0] = m_paletteBlitNearestTintmapProg[0];
	m_blitProgMatrix[(int)PixelFormat::Index_8_linear][1][1][0] = m_paletteBlitInterpolateTintmapProg[0];

	m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][0][0][0] = m_paletteBlitNearestProg[0];
	m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][1][0][0] = m_paletteBlitInterpolateProg[0];

	m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][0][1][0] = m_paletteBlitNearestTintmapProg[0];
	m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][1][1][0] = m_paletteBlitInterpolateTintmapProg[0];


	m_blitProgMatrix[(int)PixelFormat::Alpha_8][0][0][1] = m_alphaBlitProg[1];
	m_blitProgMatrix[(int)PixelFormat::Alpha_8][1][0][1] = m_alphaBlitProg[1];

	m_blitProgMatrix[(int)PixelFormat::Alpha_8][0][1][1] = m_alphaBlitTintmapProg[1];
	m_blitProgMatrix[(int)PixelFormat::Alpha_8][1][1][1] = m_alphaBlitTintmapProg[1];

	m_blitProgMatrix[(int)PixelFormat::Index_8_linear][0][0][1] = m_paletteBlitNearestProg[1];
	m_blitProgMatrix[(int)PixelFormat::Index_8_linear][1][0][1] = m_paletteBlitInterpolateProg[1];

	m_blitProgMatrix[(int)PixelFormat::Index_8_linear][0][1][1] = m_paletteBlitNearestTintmapProg[1];
	m_blitProgMatrix[(int)PixelFormat::Index_8_linear][1][1][1] = m_paletteBlitInterpolateTintmapProg[1];

	m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][0][0][1] = m_paletteBlitNearestProg[1];
	m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][1][0][1] = m_paletteBlitInterpolateProg[1];

	m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][0][1][1] = m_paletteBlitNearestTintmapProg[1];
	m_blitProgMatrix[(int)PixelFormat::Index_8_sRGB][1][1][1] = m_paletteBlitInterpolateTintmapProg[1];

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

	glGenFramebuffers(1, &m_framebufferId);

	glGenBuffers(1, &m_vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);

	// Create and initialize our VertexArray

	glGenVertexArrays(1, &m_vertexArrayId);
	glBindVertexArray(m_vertexArrayId);

	glVertexAttribIPointer(		// Attribute: Coord
		0,						// attribute number, must match the layout in the shader.
		2,						// size
		GL_INT,					// type
		sizeof(VertexGL),		// stride
		(void*)0				// array buffer offset
	);

	glVertexAttribPointer(		// Attribute: uv
		1,						// attribute number, must match the layout in the shader.
		2,						// size
		GL_FLOAT,				// type
		GL_TRUE,				// normalized?
		sizeof(VertexGL),		// stride
		(void*)(sizeof(CoordI))  // array buffer offset
	);

	glVertexAttribIPointer(		// Attribute: colorOfs
		2,						// attribute number, must match the layout in the shader.
		1,						// size
		GL_INT,					// type
		sizeof(VertexGL),			// stride
		(void*)(sizeof(CoordI) + sizeof(CoordF))  // array buffer offset
	);

	glVertexAttribIPointer(		// Attribute: extrasOfs
		3,						// attribute number, must match the layout in the shader.
		1,						// size
		GL_INT,					// type
		sizeof(VertexGL),			// stride
		(void*)(sizeof(CoordI) + sizeof(CoordF) + sizeof(int))  // array buffer offset
	);

	glVertexAttribPointer(		// Attribute: tintmapOfs
		4,						// attribute number, must match the layout in the shader.
		2,						// size
		GL_FLOAT,				// type
		GL_TRUE,				// normalized?
		sizeof(VertexGL),			// stride
		(void*)(sizeof(CoordI) + sizeof(CoordF) + sizeof(int)*2)  // array buffer offset
	);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	LOG_INIT_GLERROR(glGetError());

	// Create a buffer for our CanvasUBO

	glGenBuffers(1, &m_canvasUBOId);
	glBindBuffer(GL_UNIFORM_BUFFER, m_canvasUBOId);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CanvasUBO), NULL, GL_STATIC_DRAW); 
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_canvasUBOId);

	LOG_INIT_GLERROR(glGetError());

	// Create a TextureBufferObject for providing extra data to our shaders

	glGenBuffers(1, &m_extrasBufferId);
	glBindBuffer(GL_TEXTURE_BUFFER, m_extrasBufferId);

	LOG_INIT_GLERROR(glGetError());

	glGenTextures(1, &m_extrasBufferTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, m_extrasBufferTex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_extrasBufferId);
	glActiveTexture(GL_TEXTURE0);

	LOG_INIT_GLERROR(glGetError());

	glGenBuffers(1, &m_colorBufferId);
	glBindBuffer(GL_TEXTURE_BUFFER, m_colorBufferId);

	LOG_INIT_GLERROR(glGetError());

	glGenTextures(1, &m_colorBufferTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, m_colorBufferTex);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_colorBufferId);
	glActiveTexture(GL_TEXTURE0);



	LOG_INIT_GLERROR(glGetError());
}

//____ Destructor ______________________________________________________________

GlBackend::~GlBackend()
{
	LOG_GLERROR(glGetError());

	glDeleteProgram(m_fillProg[0]);
	glDeleteProgram(m_fillProg[1]);
	glDeleteProgram(m_fillTintmapProg[0]);
	glDeleteProgram(m_fillTintmapProg[1]);
	glDeleteProgram(m_aaFillProg[0]);
	glDeleteProgram(m_aaFillProg[1]);
	glDeleteProgram(m_aaFillTintmapProg[0]);
	glDeleteProgram(m_aaFillTintmapProg[1]);

	glDeleteProgram(m_blurProg[0]);
	glDeleteProgram(m_blurProg[1]);

	glDeleteProgram(m_blitProg[0]);
	glDeleteProgram(m_blitProg[1]);
	glDeleteProgram(m_blitTintmapProg[0]);
	glDeleteProgram(m_blitTintmapProg[1]);
	glDeleteProgram(m_alphaBlitProg[0]);
	glDeleteProgram(m_alphaBlitProg[1]);
	glDeleteProgram(m_alphaBlitTintmapProg[0]);
	glDeleteProgram(m_alphaBlitTintmapProg[1]);
	glDeleteProgram(m_paletteBlitNearestProg[0]);
	glDeleteProgram(m_paletteBlitNearestProg[1]);
	glDeleteProgram(m_paletteBlitNearestTintmapProg[0]);
	glDeleteProgram(m_paletteBlitNearestTintmapProg[1]);
	glDeleteProgram(m_paletteBlitInterpolateProg[0]);
	glDeleteProgram(m_paletteBlitInterpolateProg[1]);
	glDeleteProgram(m_paletteBlitInterpolateTintmapProg[0]);
	glDeleteProgram(m_paletteBlitInterpolateTintmapProg[1]);

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

	glDeleteFramebuffers(1, &m_framebufferId);
	glDeleteTextures(1, &m_extrasBufferTex);
//	glDeleteTextures(1, &m_segmentsTintTexId);

	glDeleteBuffers(1, &m_vertexBufferId);
	glDeleteBuffers(1, &m_extrasBufferId);

	glDeleteVertexArrays(1, &m_vertexArrayId);

	if( m_idleSync != 0 )
		glDeleteSync(m_idleSync);

	LOG_GLERROR(glGetError());
}

//____ typeInfo() _________________________________________________________

const TypeInfo& GlBackend::typeInfo(void) const
{
	return TYPEINFO;
}

//____ surfaceType() _______________________________________________________

const TypeInfo& GlBackend::surfaceType( void ) const
{
	return GlSurface::TYPEINFO;
}

//____ maxEdges() _____________________________________________

int GlBackend::maxEdges() const
{
	return c_maxSegments - 1;
}

//____ surfaceFactory() ______________________________________________________

SurfaceFactory_p GlBackend::surfaceFactory()
{
	if( !m_pSurfaceFactory )
		m_pSurfaceFactory = GlSurfaceFactory::create();

	return m_pSurfaceFactory;
}

//____ edgemapFactory() ______________________________________________________

EdgemapFactory_p GlBackend::edgemapFactory()
{
	if (!m_pEdgemapFactory)
		m_pEdgemapFactory = GlEdgemapFactory::create();

	return m_pEdgemapFactory;
}


//____ setDefaultCanvas() ___________________________________________

bool GlBackend::setDefaultCanvas(SizeSPX size, int scale)
{
	m_defaultCanvas.ref = CanvasRef::Default;		// Starts as Undefined until this method is called.
	m_defaultCanvas.size = size;
	m_defaultCanvas.scale = scale;
	return true;
}

//____ canvas() ___________________________________________________________

const CanvasInfo * GlBackend::canvasInfo(CanvasRef ref) const
{
	if (ref == CanvasRef::Default)
		return &m_defaultCanvas;
	else
	{
		//TODO: Error handling!
		return &m_dummyCanvas;
	}
}

//____ beginRender() ___________________________________________________________

void GlBackend::beginRender()
{
	LOG_GLERROR(glGetError());

//		if( m_bRendering == true )
//			return false;

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		//TODO: Error handling!
		return;
	}


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


	//

//		m_bRendering = true;
		
	// Set our extras buffer and segments palette textures.
/*
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, m_extrasBufferTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, m_segmentsTintTexId);
	glActiveTexture(GL_TEXTURE0);
*/

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, m_colorBufferTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, m_extrasBufferTex);

	glActiveTexture(GL_TEXTURE0);


	// Prepare for rendering

	glBindVertexArray(m_vertexArrayId);


//		glFinish();  //TODO: Remove.

	//


	LOG_GLERROR(glGetError());
}

//____ endRender() _____________________________________________________________

void GlBackend::endRender()
{
	LOG_GLERROR(glGetError());
//		if( m_bRendering == false )
//			return false;

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
//		m_bRendering = false;
}

//____ _beginSession() _______________________________________________________

void GlBackend::beginSession(const SessionInfo* pSession)
{
	// Reserve buffer for coordinates

	int nCoords = pSession->nPoints + pSession->nRects * 6 + pSession->nLineCoords/2 * 6;

	m_pVertexBuffer = new VertexGL[nCoords];
	m_nVertices = 0;

	// Reserve buffer for colors

	m_pColorBuffer = new ColorGL[pSession->nColors+1+pSession->nTintmapColors];

	m_pColorBuffer[0].r = 1.f;
	m_pColorBuffer[0].g = 1.f;
	m_pColorBuffer[0].b = 1.f;
	m_pColorBuffer[0].a = 1.f;
	//	= { 1.f,1.f,1.f,1.f };	// Always present white color used as default tint for blits.
	m_nColors = 1;

	// Reserve buffer for extras

	int nExtrasFloats = 
		pSession->nBlit * 8
		+ pSession->nBlur * 8
		+ pSession->nLineCoords/2 * 4
		+ pSession->nRects * 4;			// This is for possible subpixel fills. We have no way of knowing exactly how much is needed.

	m_pExtrasBuffer = new GLfloat[nExtrasFloats];
	m_extrasBufferSize = 0;

	// Reserve buffer for commands

	m_pCommandQueue = new int[
		pSession->nStateChanges * 16	//TODO: Check exactly size needed
		+ pSession->nFill * 2 
		+ pSession->nBlit * 2
		+ pSession->nBlur * 2
		+ pSession->nPlots * 2
		+ pSession->nLines * 3 + pSession->nLineClipRects * 4
		+ pSession->nCanvases];	

	m_commandQueueSize = 0;
}

//____ _endSession() _______________________________________________________

void GlBackend::endSession()
{
	LOG_GLERROR(glGetError());


	// Send vertices to GPU

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, m_nVertices * sizeof(VertexGL), m_pVertexBuffer, GL_DYNAMIC_DRAW);		// Orphan current buffer if still in use and create new.
//	glBindBuffer(GL_ARRAY_BUFFER, 0);

	LOG_GLERROR(glGetError());

	// Send colors to GPU

	glBindBuffer(GL_TEXTURE_BUFFER, m_colorBufferId);
	glBufferData(GL_TEXTURE_BUFFER, m_nColors * sizeof(ColorGL), m_pColorBuffer, GL_DYNAMIC_DRAW);
//	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	LOG_GLERROR(glGetError());

	// Send extras to GPU

	glBindBuffer(GL_TEXTURE_BUFFER, m_extrasBufferId);
	glBufferData(GL_TEXTURE_BUFFER, m_extrasBufferSize * sizeof(GLfloat), m_pExtrasBuffer, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	LOG_GLERROR(glGetError());


	// Process command queue

	glEnableVertexAttribArray(0);	// Vertices
	glEnableVertexAttribArray(1);	// UV
	glEnableVertexAttribArray(2);	// Colors
	glEnableVertexAttribArray(3);	// Extras
	glEnableVertexAttribArray(4);	// Tintmap ofs

	LOG_GLERROR(glGetError());

	int* pCmd = m_pCommandQueue;
	int* pEnd = m_pCommandQueue + m_commandQueueSize;

	int vertexOfs = 0;
	int objectOfs = 0;

	while (pCmd < pEnd)
	{
		CommandGL cmd = (CommandGL)*pCmd++;

		switch (cmd)
		{
			case CommandGL::SetCanvas:
			{
				_setCanvas( static_cast<Surface*>(m_objects[objectOfs++]) );
				break;
			}

			case CommandGL::StateChange:
			{
				int32_t statesChanged = *pCmd++;

				if (statesChanged & uint8_t(StateChange::BlitSource))
				{
					_setBlitSource(static_cast<GlSurface*>(m_objects[objectOfs++]));
				}

				if (statesChanged & uint8_t(StateChange::BlendMode))
				{
					m_activeBlendMode = (BlendMode)*pCmd++;
				}

				if (statesChanged & uint8_t(StateChange::TintColor))
				{
					m_bTintmapIsActive = false;
				}

				if (statesChanged & uint8_t(StateChange::TintMap))
				{
					m_bTintmapIsActive = true;
				}

				if (statesChanged & uint8_t(StateChange::MorphFactor))
				{
					m_activeMorphFactor = *pCmd++ / 4096.f;
				}

				if (statesChanged & uint8_t(StateChange::FixedBlendColor))
				{
				}

				if (statesChanged & uint8_t(StateChange::Blur))
				{
					auto pBlurbrush = static_cast<Blurbrush*>(m_objects[objectOfs++]);

					m_pActiveBlurbrush = pBlurbrush;
				}

				// We postpone setting blend mode after having retrieved active morphFactor and fixedBlendColor

				if (statesChanged & (uint8_t(StateChange::BlendMode) | uint8_t(StateChange::MorphFactor)) )
				{
					_setBlendMode(m_activeBlendMode);
				}


				break;

			}

			case CommandGL::Plot:
			{
				int nVertices = *pCmd++;
				glUseProgram(m_plotProg[m_bActiveCanvasIsA8]);

				glDrawArrays(GL_POINTS, vertexOfs, nVertices);
				vertexOfs += nVertices;
				break;
			}

			case CommandGL::StraightFill:
			{
				int nVertices = *pCmd++;

					if (m_bTintmapIsActive)
						glUseProgram(m_fillTintmapProg[m_bActiveCanvasIsA8]);
					else
						glUseProgram(m_fillProg[m_bActiveCanvasIsA8]);

				glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);
				vertexOfs += nVertices;
				break;
			}

			case CommandGL::SubpixelFill:
			{
				int nVertices = *pCmd++;

				if (m_bTintmapIsActive)
					glUseProgram(m_aaFillTintmapProg[m_bActiveCanvasIsA8]);
				else
					glUseProgram(m_aaFillProg[m_bActiveCanvasIsA8]);

				glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);
				vertexOfs += nVertices;
				break;
			}

			case CommandGL::Lines:
			{
				int clipListLen = *pCmd++;
				int nVertices = *pCmd++;

				glUseProgram(m_lineFromToProg[m_bActiveCanvasIsA8]);

				for (int i = 0; i < clipListLen; i++)
				{
					int x = *pCmd++;
					int y = *pCmd++;
					int w = *pCmd++;
					int h = *pCmd++;

					glScissor(x, y, w, h);
					glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);
				}

				glScissor(0, 0, m_activeCanvasSize.w, m_activeCanvasSize.h);

				vertexOfs += nVertices;
				break;
			}

			case CommandGL::Blit:
			{
				int nVertices = *pCmd++;

				GlSurface* pSurf = m_pActiveBlitSource;
				glUseProgram(m_blitProgMatrix[(int)pSurf->m_pixelFormat][(int)pSurf->sampleMethod()][m_bTintmapIsActive][m_bActiveCanvasIsA8]);

				glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);
				vertexOfs += nVertices;
				break;
			}

			case CommandGL::Blur:
			{
				int nVertices = *pCmd++;

				if( m_pActiveBlurbrush)
				{
					spx radius = m_pActiveBlurbrush->size();

					for (int i = 0; i < 9; i++)
					{
						m_activeBlurInfo.colorMtx[i][0] = m_pActiveBlurbrush->red()[i];
						m_activeBlurInfo.colorMtx[i][1] = m_pActiveBlurbrush->green()[i];
						m_activeBlurInfo.colorMtx[i][2] = m_pActiveBlurbrush->blue()[i];
						m_activeBlurInfo.colorMtx[i][3] = 0.f;
					}

					m_activeBlurInfo.colorMtx[4][3] = 1.f;

					auto size = m_pActiveBlitSource->pixelSize();

					float radiusX = radius / float(size.w * 64);
					float radiusY = radius / float(size.h * 64);

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

					glUseProgram(m_blurProg[m_bTintmapIsActive]);

					glUniform2fv(m_blurUniformLocation[m_bTintmapIsActive][1], 9, (GLfloat*)m_activeBlurInfo.offset);
					glUniform4fv(m_blurUniformLocation[m_bTintmapIsActive][0], 9, (GLfloat*)m_activeBlurInfo.colorMtx);

					glDrawArrays(GL_TRIANGLES, vertexOfs, nVertices);
				}

				vertexOfs += nVertices;
				break;
			}

			default:
			{
				assert(false);
			}

		}
	}

	glFlush();

	LOG_GLERROR(glGetError());


	// Release temporary buffers

	delete[] m_pVertexBuffer;
	m_pVertexBuffer = nullptr;

	delete[] m_pColorBuffer;
	m_pColorBuffer = nullptr;

	delete[] m_pExtrasBuffer;
	m_pExtrasBuffer = nullptr;

	delete[] m_pCommandQueue;
	m_pCommandQueue = nullptr;

	m_objects.clear();
	m_pCanvas = nullptr;
}


//____ _setBlendMode() _______________________________________________________

void GlBackend::_setBlendMode(BlendMode mode)
{
	LOG_GLERROR(glGetError());

	bool bAlphaOnly = m_bActiveCanvasIsA8;

	switch (mode)
	{
	case BlendMode::Replace:
		glBlendEquation(GL_FUNC_ADD);
		glDisable(GL_BLEND);
		break;

	case BlendMode::Undefined:
	case BlendMode::BlendFixedColor:		// Defaults to Blend for BlendFixedColor which can not be implemented easily.
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
		glBlendColor(1.f, 1.f, 1.f, m_activeMorphFactor);
		break;

	case BlendMode::Add:
		glBlendEquation(GL_FUNC_ADD);
		glEnable(GL_BLEND);

		if (bAlphaOnly)
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
		if (bAlphaOnly)
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
		else
			glBlendFuncSeparate(GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE);
		break;

	case BlendMode::Invert:
		glBlendEquation(GL_FUNC_ADD);
		glEnable(GL_BLEND);
		if (bAlphaOnly)
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

	LOG_GLERROR(glGetError());
}

//____ _setBlitSource() _______________________________________________________

void GlBackend::_setBlitSource(GlSurface* pSurf)
{
	LOG_GLERROR(glGetError());

	glActiveTexture(GL_TEXTURE0);

	if (pSurf)
	{
		glBindTexture(GL_TEXTURE_2D, pSurf->getTexture());

		m_pActiveBlitSource = pSurf;

		if (pSurf->m_pPalette)
		{
			glActiveTexture(GL_TEXTURE3);
			GLuint paletteTex = pSurf->getPaletteTexture();
			glBindTexture(GL_TEXTURE_2D, paletteTex);
			glActiveTexture(GL_TEXTURE0);

			assert(glGetError() == 0);
		}
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		m_pActiveBlitSource = nullptr;
	}
	LOG_GLERROR(glGetError());
}

//____ _loadPrograms() ______________________________________________________________

void GlBackend::_loadPrograms(int uboBindingPoint)
{
	int programNb = 0;

	// Create and init Fill shader

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, fillVertexShader, i == 0 ? fillFragmentShader : fillFragmentShader_A8);
		_setUniforms(progId, uboBindingPoint);
		m_fillProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init Fill Gradient shader

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, fillTintmapVertexShader, i == 0 ? fillFragmentShaderTintmap : fillFragmentShaderTintmap_A8);
		_setUniforms(progId, uboBindingPoint);
		m_fillTintmapProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init AA-Fill shader

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, aaFillVertexShader, i == 0 ? aaFillFragmentShader : aaFillFragmentShader_A8);
		_setUniforms(progId, uboBindingPoint);
		m_aaFillProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init AA-Fill gradient shader

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, aaFillTintmapVertexShader, i == 0 ? aaFillFragmentShaderTintmap : aaFillFragmentShaderTintmap_A8);
		_setUniforms(progId, uboBindingPoint);
		m_aaFillTintmapProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init Blur shader

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, i == 0 ? blitVertexShader : blitTintmapVertexShader, i == 0 ? blurFragmentShader : blurFragmentShaderTintmap );
		_setUniforms(progId, uboBindingPoint);

		m_blurUniformLocation[i][0] = glGetUniformLocation(progId, "blurInfo.colorMtx");
		m_blurUniformLocation[i][1] = glGetUniformLocation(progId, "blurInfo.offset");

		m_blurProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}


	// Create and init Blit shader

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, blitVertexShader, i == 0 ? blitFragmentShader : blitFragmentShader_A8);
		_setUniforms(progId, uboBindingPoint);
		m_blitProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init tintmap Blit shader

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, blitTintmapVertexShader, i == 0 ? blitFragmentShaderTintmap : blitFragmentShaderTintmap_A8);
		_setUniforms(progId, uboBindingPoint);
		m_blitTintmapProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init AlphaBlit shader (shader program for blitting from alpha-only texture)

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, blitVertexShader, i == 0 ? alphaBlitFragmentShader : alphaBlitFragmentShader_A8);
		_setUniforms(progId, uboBindingPoint);
		m_alphaBlitProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init AlphaBlit tintmap shader (shader program for blitting from alpha-only texture)

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, blitTintmapVertexShader, i == 0 ? alphaBlitFragmentShaderTintmap: alphaBlitFragmentShaderTintmap_A8);
		_setUniforms(progId, uboBindingPoint);
		m_alphaBlitTintmapProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init Palette Blit shaders

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, paletteBlitNearestVertexShader, i == 0 ? paletteBlitNearestFragmentShader : paletteBlitNearestFragmentShader_A8);
		_setUniforms(progId, uboBindingPoint);
		m_paletteBlitNearestProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, paletteBlitInterpolateVertexShader, i == 0 ? paletteBlitInterpolateFragmentShader : paletteBlitInterpolateFragmentShader_A8);
		_setUniforms(progId, uboBindingPoint);
		m_paletteBlitInterpolateProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init Palette Blit tintmap shaders

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, paletteBlitNearestTintmapVertexShader, i == 0 ? paletteBlitNearestFragmentShaderTintmap : paletteBlitNearestFragmentShaderTintmap_A8);
		_setUniforms(progId, uboBindingPoint);
		m_paletteBlitNearestTintmapProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, paletteBlitInterpolateTintmapVertexShader, i == 0 ? paletteBlitInterpolateFragmentShaderTintmap : paletteBlitInterpolateFragmentShaderTintmap_A8);
		_setUniforms(progId, uboBindingPoint);
		m_paletteBlitInterpolateTintmapProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init Plot shader

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, plotVertexShader, i == 0 ? plotFragmentShader : plotFragmentShader_A8);
		_setUniforms(progId, uboBindingPoint);
		m_plotProg[i] = progId;
		LOG_INIT_GLERROR(glGetError());
	}

	// Create and init Line shader

	for (int i = 0; i < 2; i++)
	{
		GLuint progId = _loadOrCompileProgram(programNb++, lineFromToVertexShader, i == 0 ? lineFromToFragmentShader : lineFromToFragmentShader_A8);
		_setUniforms(progId, uboBindingPoint);
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

GLuint GlBackend::_loadOrCompileProgram(int programNb, const char* pVertexShaderSource, const char* pFragmentShaderSource)
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

Blob_p GlBackend::_generateProgramBlob()
{
	GLuint	programs[c_nbPrograms];

	// Collect the programs

	int prg = 0;

	programs[prg++] = m_fillProg[0];
	programs[prg++] = m_fillProg[1];

	programs[prg++] = m_fillTintmapProg[0];
	programs[prg++] = m_fillTintmapProg[1];

	programs[prg++] = m_aaFillProg[0];
	programs[prg++] = m_aaFillProg[1];

	programs[prg++] = m_aaFillTintmapProg[0];
	programs[prg++] = m_aaFillTintmapProg[1];

	programs[prg++] = m_blurProg[0];
	programs[prg++] = m_blurProg[1];

	programs[prg++] = m_blitProg[0];
	programs[prg++] = m_blitProg[1];

	programs[prg++] = m_blitTintmapProg[0];
	programs[prg++] = m_blitTintmapProg[1];

	programs[prg++] = m_alphaBlitProg[0];
	programs[prg++] = m_alphaBlitProg[1];

	programs[prg++] = m_alphaBlitTintmapProg[0];
	programs[prg++] = m_alphaBlitTintmapProg[1];

	programs[prg++] = m_paletteBlitNearestProg[0];
	programs[prg++] = m_paletteBlitNearestProg[1];

	programs[prg++] = m_paletteBlitInterpolateProg[0];
	programs[prg++] = m_paletteBlitInterpolateProg[1];

	programs[prg++] = m_paletteBlitNearestTintmapProg[0];
	programs[prg++] = m_paletteBlitNearestTintmapProg[1];

	programs[prg++] = m_paletteBlitInterpolateTintmapProg[0];
	programs[prg++] = m_paletteBlitInterpolateTintmapProg[1];

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

void GlBackend::_initTables()
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

float GlBackend::_scaleThickness(float thickness, float slope)
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

