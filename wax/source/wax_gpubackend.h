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
#ifndef	WAX_GPUBACKEND_DOT_H
#define	WAX_GPUBACKEND_DOT_H
#pragma once

#include <wax_backend.h>
#include <wax_gpushapes.h>
#include <wax_gputools.h>
#include <wax_tinttools.h>

#include <vector>

namespace wax
{
	class GpuEdgemap;
	class Warpmap;

	class GpuBackend;
	typedef	StrongPtr<GpuBackend>	GpuBackend_p;
	typedef	WeakPtr<GpuBackend>		GpuBackend_wp;

	//____ GpuBackend __________________________________________________________
	/**
	 * @brief Base class of backends that draw with the HLSL shaders in wax/extensions/hlsl.
	 *
	 * Turns the command stream into vertices, colors and extras in buffers the shaders read, and
	 * into constants, the same way whatever the graphics API. Subclasses own the buffers and do what
	 * needs the API: render targets, pipelines, binding resources and recording draws, through the
	 * protected virtual methods below, which this class calls between draws, never per vertex.
	 *
	 * Taken from DX12Backend. DX12Backend and VulkanBackend are built on it.
	 *
	 * What the shaders get, see wax/extensions/hlsl:
	 *
	 *   Vertices		Position on the canvas in pixels, offsets into the color and extras
	 *					buffers, and u, v for edgemaps. See Vertex.
	 *   Colors			Linear, premultiplied by nothing, multiplied by the tint color. Tint
	 *					blocks of TintTools::writeGpuTintBlock() are stored here too.
	 *   Extras			Four floats per entry, what each kind of draw needs besides.
	 *   Constants		CanvasConstants, PixelConstants and WarpConstants, see constants.hlsl.
	 *
	 * A session writes at most what its SessionInfo says, see _reserveBuffers(), so the buffers
	 * are made big enough when the session begins and draws are never split for lack of room.
	 */

	class GpuBackend : public Backend
	{
	public:

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		void	beginSession( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo = nullptr ) override;
		void	endSession() override;

		void	setCanvas( Surface * pSurface ) override;
		void	setCanvas( CanvasRef ref ) override;

		void	setObjects(Object* const * pBeg, Object* const * pEnd) override;

		void	setRects(const RectSPX* pBeg, const RectSPX* pEnd) override;
		void	setColors(const HiColor* pBeg, const HiColor* pEnd) override;
		void	setTransforms(const Transform * pBeg, const Transform * pEnd) override;

		void	processCommands( const uint16_t* pBeg, const uint16_t * pEnd, int version = 2) override;

		//.____ Misc _________________________________________________________

		int		maxEdges() const override;

	protected:
		GpuBackend();
		virtual ~GpuBackend();

		const static int	c_maxSegments = 16;				// As many as the segments shader handles.

		// What a pipeline draws. Together with blend mode and canvas format this is what tells one
		// pipeline from another.

		enum class PipelineKind
		{
			Fill,				// Rectangles on whole pixels.
			FillAA,				// Rectangles that aren't, with coverage worked out per pixel.
			Blit,
			Blur,				// Same geometry as a blit, nine taps instead of one.
			PaletteBlit,		// Blit and blur from a palette based source,
			PaletteBlur,		// which has to be looked up before it is filtered.
			Line,
			Segments,			// Edgemaps.
			Shape,				// Shapes, coverage worked out per pixel, see GpuShapes.
			WarpEdgemap,		// Warps, coverage worked out per pixel, see warp.hlsl.
			WarpBlit,
			WarpPaletteBlit,

			Size
		};

		static bool			_hasPartialCoverage( PipelineKind kind );	// Covers pixels partly, so Replace blends.
		static bool			_isWarp( PipelineKind kind );				// No vertex input, reads the Warpmap.

		// What the shaders read, see wax/extensions/hlsl.

		struct Vertex
		{
			float		x, y;			// Canvas pixels, origin top left. Subpixel positions allowed.
			uint32_t	colorOfs;		// Offset into the color buffer.
			uint32_t	extrasOfs;		// Offset into the extras buffer. Not used by plain fills.

			// Only edgemaps use these. They cost every other primitive eight bytes a vertex, the
			// alternative being a second vertex format and a second buffer to keep in step with it.

			float		u, v;			// Column, and distance down the column.
		};

		struct Color
		{
			float	r, g, b, a;
		};

		struct Extras
		{
			float	x, y, z, w;
		};

		// The constants, in the order and layout of constants.hlsl.

		struct CanvasConstants
		{
			float		canvasScale[2];		// 2/canvasWidth, 2/canvasHeight, in pixels.
			float		textureSize[2];		// Size of the blit source, in pixels.
			uint32_t	flags;				// Blit source, see _bindBlitSamplerAndFlags().
			uint32_t	blurOfs;			// Blur brush in the extras buffer.
			uint32_t	edgemapEdges;		// Edges to draw of the edgemap being drawn.
			uint32_t	edgemapPitch;		// Edges per column of the edgemap being drawn.
		};

		struct PixelConstants
		{
			int32_t		tintOfs;			// Tint block in the color buffer, -1 for none.
			uint32_t	outputMode;			// How the pixel shaders hand what they draw to the blender, see output.hlsl.
			uint32_t	canvasKind;			// See CanvasKind.
			float		morphFactor;		// 1 unless the blend mode is Morph.
		};

		struct WarpConstants
		{
			float		destX, destY;		// Position of the Warpmap on the canvas, in pixels.
			uint32_t	colorOfs;			// Tint color in the color buffer.
			uint32_t	extrasOfs;			// The warp in the extras buffer, see warp.hlsl.
			int32_t		refU, refV;			// u and v of the first vertex, in spx.
			uint32_t	pad[2];				// HLSL rounds the cbuffer up to whole registers.
		};

		enum class ConstantBlock
		{
			Canvas,
			Pixel,
			Warp
		};

		// What the pixel shaders need to know about the canvas, see output.hlsl.

		enum CanvasKind : uint32_t
		{
			WithAlpha = 0,
			WithoutAlpha = 1,
			AlphaOnly = 2
		};

		static CanvasKind	_canvasKind( const Surface * pSurface );

		//.____ For subclasses to implement ______________________________

		// True while draws can be recorded: between beginRender() and endRender() of a backend with
		// a working device. Commands are read but not drawn otherwise.

		virtual bool		_isRecording() const = 0;

		// Whether an object is a surface, edgemap or Warpmap of the subclass. Edgemaps are
		// GpuEdgemaps.

		virtual bool		_isNativeSurface( const Object * pObject ) const = 0;
		virtual bool		_isNativeEdgemap( const Object * pObject ) const = 0;
		virtual bool		_isNativeWarpmap( const Object * pObject ) const = 0;

		// Called by beginSession() and endSession() while recording, before and after everything
		// else. pCanvas is the canvas of the session, null for the default canvas.

		virtual void		_beginSessionOnDevice( Surface * pCanvas ) = 0;
		virtual void		_endSessionOnDevice() = 0;

		// Makes room in the buffers of the frame for what a session may write, and points
		// m_buffers at it. nObjects is how many objects the object tables of the session hold at
		// most, which is also the most blit sources it can have, 0 if not known. Returns false if
		// some buffer couldn't be made big enough, in which case m_buffers points at what there
		// is, or is null.

		virtual bool		_reserveFrameBuffers( int nVertices, int nColors, int nExtras, int nObjects ) = 0;

		// Points the recording at a canvas, null for the default one. Fills in its size in pixels
		// and its kind, returns false if it can't be rendered into.

		virtual bool		_bindCanvas( Surface * pCanvas, SizeI& size, CanvasKind& kind ) = 0;

		// Binds the pipeline of the given kind for m_activeBlendMode on the canvas. Returns false if
		// there is none, which drops the draw.

		virtual bool		_setPipeline( PipelineKind kind ) = 0;

		// Sets the constants of a block, from m_canvasConstants, m_pixelConstants or m_warpConstants.

		virtual void		_setConstants( ConstantBlock block ) = 0;

		// Draws vertices of the vertex buffer as triangles.

		virtual void		_draw( int firstVertex, int nVertices ) = 0;

		// Clips the draws that follow to a rectangle of the canvas, in pixels, or no longer.

		virtual void		_setScissor( const RectI& rect ) = 0;
		virtual void		_resetScissor() = 0;

		// Makes sure the texture of a surface about to become blit source holds what has been
		// written to its pixels.

		virtual void		_prepareBlitSource( Surface * pSurface ) = 0;

		// Binds the texture of the blit source, and its palette if palette based. Draws any stale
		// mip levels first.

		virtual bool		_bindBlitTexture( Surface * pSurface ) = 0;

		// Binds the sampler for the blit source, clamping, clipping (transparent outside) or tiling,
		// nearest or bilinear as the surface says.

		virtual void		_bindBlitSampler( Surface * pSurface, bool bClip ) = 0;

		// Binds the buffer of an edgemap, where the palette of a blit source also goes.

		virtual bool		_bindEdgemap( GpuEdgemap * pEdgemap ) = 0;

		// Draws the triangles of a Warpmap once per patch, with the patch as scissor, and puts the
		// scissor back. m_warpConstants are set.

		virtual void		_drawWarpmap( Warpmap * pWarpmap, const RectI * pPatches, int nPatches ) = 0;

		//.____ Internal ______________________________________________________

		void				_resetStates();			// To start values, for a new session or canvas.
		void				_setCanvas( Surface * pCanvas );

		void				_reserveBuffers( const SessionInfo * pInfo );
		void				_checkSessionUsage();
		bool				_blameSessionInfo() const { return !m_bSessionBuffersShort && _isRecording(); }

		Object *			_object( uint32_t index ) const;		// From the object table, null if the index is out of range.

		void				_applyStateChange( const GpuTools::StateChangeParams& state );
		void				_setBlitSource( Surface * pSurface, uint32_t objectIndex = c_noObject );
		bool				_bindBlitSource();
		void				_bindBlitSamplerAndFlags();

		int					_addColor( HiColor color );			// Returns offset into color buffer, -1 if full.
		void				_setTint( const TintTools::DecodedTint& tint );
		void				_clearTint();
		void				_updatePixelInfo();					// Blend mode, morph factor or canvas has changed.

		int					_addExtras( const Extras& extras );						// Returns offset, -1 if full.
		int					_addExtras( const Extras& first, const Extras& second );
		int					_addBlurExtras();										// Returns offset to 18 entries, -1 if full.

		void				_drawFillRects( const RectSPX * pRects, int nRects, HiColor color );
		void				_drawFillRun( PipelineKind kind, int firstVertex, int nRects );
		void				_drawBlitRects( const uint16_t*& pCmd, const RectSPX*& pRects, int nRects, int version, PipelineKind kind, bool bClip );
		void				_drawLines( const uint16_t*& pCmd, const RectSPX*& pRects, const HiColor*& pColors, int nClipRects, int nLines );
		void				_drawCappedLines( const RectSPX * pClipRects, int nClipRects );
		void				_drawScissored( const RectSPX * pClipRects, int nClipRects, int firstVertex, int nVertices );
		void				_drawEdgemap( const uint16_t*& pCmd, const RectSPX*& pRects, int nRects, bool bTile );
		void				_drawShape( ShapeKind kind, const uint16_t * pPayload, const RectSPX * pRects, int nRects, const HiColor * pColors );
		void				_drawWarp( Command cmd, const WarpParams& params, const RectSPX * pRects );

		void				_reportBufferFull( const char * pBuffer, const char * pWhat, bool& bReported );

		static BlendMode	_normalizeBlendMode( BlendMode blendMode );
		static bool			_isOfType( const TypeInfo& type, const TypeInfo& base );

		static const uint32_t	c_noObject = 0xFFFFFFFF;

		// Without a SessionInfo we have nothing to go on, so we make sure there is at least this
		// much room.

		const static int	c_defaultVertices = 32*1024;		// ~5400 rects.
		const static int	c_defaultColors = 8*1024;
		const static int	c_defaultExtras = 16*1024;

		// Where the session writes. The subclass points them into its buffers for the frame,
		// see _reserveFrameBuffers(). Beg is where the buffer starts, which offsets are counted
		// from, Ptr the next entry to write and End the end of the buffer. All null when there
		// is nothing to write to.

		struct FrameBuffers
		{
			Vertex *	pVertexBeg = nullptr;
			Vertex *	pVertexPtr = nullptr;
			Vertex *	pVertexEnd = nullptr;

			Color *		pColorBeg = nullptr;
			Color *		pColorPtr = nullptr;
			Color *		pColorEnd = nullptr;

			Extras *	pExtrasBeg = nullptr;
			Extras *	pExtrasPtr = nullptr;
			Extras *	pExtrasEnd = nullptr;
		};

		FrameBuffers		m_buffers;

		// Command streams.

		Object* const*		m_pObjectsBeg = nullptr;		// Object table, commands refer to objects by index.
		Object* const*		m_pObjectsEnd = nullptr;

		const RectSPX*		m_pRectsBeg = nullptr;
		const RectSPX*		m_pRectsEnd = nullptr;
		const RectSPX*		m_pRectsPtr = nullptr;

		const HiColor*		m_pColorsBeg = nullptr;
		const HiColor*		m_pColorsEnd = nullptr;
		const HiColor*		m_pColorsPtr = nullptr;

		const Transform*	m_pTransformsBeg = nullptr;
		const Transform*	m_pTransformsEnd = nullptr;

		// Session.

		bool				m_bInSession = false;			// Between beginSession() and endSession().

		Surface *			m_pActiveCanvas = nullptr;		// Canvas being rendered into, null for the default one.
		bool				m_bCanvasBound = false;			// _bindCanvas() succeeded for it.
		SizeI				m_activeCanvasSize;
		CanvasKind			m_activeCanvasKind = CanvasKind::WithAlpha;

		// What _reserveBuffers() was asked to make room for this session, and where the session
		// started writing, so that _checkSessionUsage() can tell when SessionInfo said too little.
		// m_bSessionBuffersShort is set when a buffer couldn't be grown, which makes the drops that
		// follow ours rather than SessionInfo's.

		int					m_sessionVertices = 0;
		int					m_sessionColors = 0;
		int					m_sessionExtras = 0;
		Vertex *			m_pSessionVertexBeg = nullptr;
		Color *				m_pSessionColorBeg = nullptr;
		Extras *			m_pSessionExtrasBeg = nullptr;
		bool				m_bSessionInfoGiven = false;
		bool				m_bSessionBuffersShort = false;

		// State set by StateChange commands.

		HiColor				m_tintColor = HiColor::White;	// Multiplied into every color _addColor() writes.
		BlendMode			m_activeBlendMode = BlendMode::Blend;
		float				m_morphFactor = 0.5f;			// Only used by BlendMode::Morph.

		int					m_blurRadius = 0;				// In spx, turned into texture coordinates when drawn.
		float				m_blurColorMtx[9][4] = {};

		Surface *			m_pBlitSource = nullptr;
		uint32_t			m_blitSourceObject = c_noObject;	// Its index in the object table.
		SizeI				m_blitSourceSize;
		bool				m_bBlitSourceAlphaOnly = false;
		bool				m_bBlitSourceIndexed = false;		// Palette based, see PipelineKind::PaletteBlit.
		bool				m_bBlitClip = false;				// Coming draw is a ClipBlit: nothing outside the source is read.
		bool				m_bBoundBlitClip = false;			// What the bound sampler and flags were set up for.
		bool				m_bBlitSourceBound = false;			// Cleared when source, session or bindings change.

		// Constants as last set, see _setConstants().

		CanvasConstants		m_canvasConstants = {};
		PixelConstants		m_pixelConstants = { -1, 0, 0, 1.f };
		WarpConstants		m_warpConstants = {};

		std::vector<GpuShapes::Segment>		m_shapeSegments;		// Segments of the shape being drawn.
		std::vector<GpuTools::CappedLine>	m_cappedLines;			// Lines with caps of the line command being drawn.
	};

} // namespace wax
#endif //WAX_GPUBACKEND_DOT_H
