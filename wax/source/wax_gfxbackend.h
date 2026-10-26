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
#ifndef	WAX_GFXBACKEND_DOT_H
#define WAX_GFXBACKEND_DOT_H
#pragma once

#include <climits>
#include <bitset>

#include <wax_gfxdevice.h>

#include <wax_pointers.h>
#include <wax_types.h>
#include <wax_color.h>
#include <wax_edgemap.h>


#include <wax_geo.h>
#include <wax_surface.h>
#include <wax_surfacefactory.h>
#include <wax_edgemapfactory.h>
#include <wax_canvaslayers.h>

#include <vector>
#include <cstring>

namespace wax
{


	class GfxBackend;
	typedef	StrongPtr<GfxBackend>	GfxBackend_p;
	typedef	WeakPtr<GfxBackend>		GfxBackend_wp;


	//____ GfxBackend __________________________________________________________

	class GfxBackend : public Object
	{
	public:

		struct SessionInfo
		{
			// Commands

			int		nSetCanvas;			// Number of setCanvas calls in session.
			int		nStateChanges;		// Number of times state will change through session.
			int		nLines;				// Number of line commands.
			int		nFill;
			int		nBlit;				// Includes Blit, ClipBlit och Tile.
			int		nBlur;
			int		nEdgemapDraws;		// Includes DrawEdgemap and TileEdgemap.
			int		nWarpDraws;			// WarpBlit and WarpDrawEdgemap.
			int		nLineCoords;		// Total number of coordinates for line commands (=total lines in line commands *2)

			// Total content of coord buffers

			int		nLineClipRects;		// Total number of clip rectangles for line commands.
			int		nRects;				// Total number of rects for fill, blit, blur and edgemapDraw commands.

			// Total size of other buffers

			int		nColors;			// Total number of colors to be used through all commands.
			int		nTransforms;		// Total number of transforms for session.
			int		nObjects;			// Total number of entries in the object tables of the session's layers.

			// Shapes

			int		nShapeDraws;		// Number of DrawShape commands.
			int		nShapeSegments;		// Total number of segments (fill, outline, slices...) of all shapes.
			int		nShapeTints;		// Total number of Tints encoded in DrawShape commands.
		};

		enum class Command : uint16_t
		{
			None,
			StateChange,
			Fill,
			Line,
			Blit,
			ClipBlit,					// Might be blitting from outside source surface.
			Tile,
			Blur,
			DrawEdgemap,
			TileEdgemap,				// Same parameters as DrawEdgemap, but edgemap is repeated over the rects.
			DrawShape,					// See below.
			WarpBlit,					// See below.
			WarpDrawEdgemap				// See below.
		};

		/*
			Objects - blit sources, edgemaps and warpmaps - are referred to by their index in
			the object table that setObjects() was last given. GfxDeviceGen2 gives each layer
			a table of its own, which lists the objects of the layer once each. Not always
			exactly once though: a backend must not count on there being no duplicates, only
			on an index of the table always meaning the same object. An entry can be null,
			for no blit source.

			An index is 32 bits, written as two 16-bit words in native byte order and always
			32-bit aligned, see readObjectIndex(). They are found in:

			  StateChange		First in the payload, when BlitSource is among the states changed.
			  DrawEdgemap		After the flip and padding word, see below.
			  TileEdgemap		Same as DrawEdgemap.
			  WarpBlit			See below.
			  WarpDrawEdgemap	See below.

			DrawEdgemap and TileEdgemap. Layout, in 16-bit words:

			  uint16	Command::DrawEdgemap or Command::TileEdgemap
			  uint16	nRects			Clip rectangles in rect buffer.
			  uint16	flip			GfxFlip.
			  uint16	padding
			  uint32	edgemap			Index of the edgemap in the object table.
			  int32		origo x, y		spx.
		*/

		/*
			Line draws lines in one color each. Layout, in 16-bit words:

			  uint16	Command::Line
			  uint16	nClipRects		Clip rectangles in rect buffer, shared by all lines.
			  uint16	nLines			Lines that follow, each with a color in color buffer.
			  uint16	padding
			  per line:
			    int32	begin x, y		spx.
			    int32	end x, y		spx.
			    uint16	thickness		spx.
			    uint16	caps			See decodeLineCaps(). 0 for LineCap::None at both ends.

			Lines with caps 0 are drawn the legacy way, see LineCap. Backends that don't support
			caps draw all lines that way.
		*/

		/*
			WarpBlit and WarpDrawEdgemap draw the blit source or an edgemap through a Warpmap, see
			Warpmap for how. Layout, in 16-bit words:

			  uint16	Command::WarpBlit or Command::WarpDrawEdgemap
			  uint16	nRects			Clip rectangles in rect buffer, within the rectangle of the Warpmap.
			  uint32	warpmap			Index of the Warpmap in the object table.
			  uint32	edgemap			WarpDrawEdgemap only. Index of the edgemap in the object table.
			  int32		dest x, y		Top-left corner of the Warpmap, spx. Rounded to nearest pixel.
			  int32		offset x, y		Source offset, spx.
			  int32		source x, y, w, h	WarpBlit only. Source rectangle of the blit source, spx.
		*/

		/*
			DrawShape draws an analytic shape, made of segments (fill, outline...) that are
			drawn together. Layout, in 16-bit words:

			  uint16	Command::DrawShape
			  uint16	ShapeKind
			  uint16	nRects			Clip rectangles in rect buffer.
			  uint16	nColors			Colors in color buffer.
			  uint16	nWords			Words of payload following this header.
			  uint16	padding
			  ...		payload

			The command starts at a 32-bit boundary (padded with Command::None if needed), so
			32-bit values in the payload are aligned. Backends that don't support a kind of
			shape can skip it using nWords, nRects and nColors. Payloads can be decoded with
			decodeRoundedBox() and decodeRing().

			ShapeKind::RoundedBox payload:

			  spx		rect x, y, w, h
			  spx		radius top-left, top-right, bottom-right, bottom-left
			  spx		outline top, right, bottom, left
			  spx		softness		Reserved for soft edges (shadows, glows), always 0 for now.
			  uint16	flags			See RoundedBoxFlags.
			  uint16	padding
			  ...		fill tint		Encoded as by TintTools::encodeTint(), placed in rect. Only if HasFillTint.
			  ...		outline tint	Encoded as by TintTools::encodeTint(), placed in rect. Only if HasOutlineTint.

			  Colors: fill color if HasFill, outline color if HasOutline, then the colors of
			  the encoded tints.

			  Radii are clamped as by GfxUtil::clampCornerRadii().

			ShapeKind::Ring payload:

			  spx		rect x, y, w, h
			  spx		thickness		Width of ring, outline included. 0 or at least the radius for no hole.
			  spx		outline
			  spx		softness		Reserved, always 0 for now.
			  uint16	flags			See RingFlags.
			  uint16	nSlices
			  uint32	start			Clockwise from 12 o'clock, c_shapeFullTurn is a full turn.
			  per slice:
			    uint32	size			c_shapeFullTurn is a full turn.
			    uint16	flags			See RingSliceFlags.
			    uint16	padding
			  ...		tints			Encoded as by TintTools::encodeTint(), placed in rect: outline, hub, back
										and slices, each only if flagged as tinted.

			  Colors: outline, hub and back color if they are drawn, color of each slice that
			  is drawn, then the colors of the encoded tints.

			  Segments are numbered back (0), outline (1), hub (2) and slices (3...).
		*/

		enum class ShapeKind : uint16_t
		{
			RoundedBox = 1,
			Ring = 2
		};

		enum class RingFlags : uint16_t
		{
			Square			= 1,
			HasOutline		= 1 << 1,
			HasHub			= 1 << 2,
			HasBack			= 1 << 3,
			HasOutlineTint	= 1 << 4,
			HasHubTint		= 1 << 5,
			HasBackTint		= 1 << 6
		};

		enum class RingSliceFlags : uint16_t
		{
			Drawn			= 1,
			Tinted			= 1 << 1
		};

		enum class RoundedBoxFlags : uint16_t
		{
			HasFill			= 1,
			HasOutline		= 1 << 1,
			HasFillTint		= 1 << 2,
			HasOutlineTint	= 1 << 3
		};

		static constexpr int		c_drawShapeHeaderWords = 6;
		static constexpr int		c_roundedBoxPayloadWords = 28;		// RoundedBox payload before its tints.
		static constexpr int		c_ringPayloadWords = 18;			// Ring payload before its slices...
		static constexpr int		c_ringSliceWords = 4;				// ...and the words of each.
		static constexpr uint32_t	c_shapeFullTurn = 0x80000000;		// Angles and slice sizes of Ring.

		// Decoded DrawShape payloads, see decodeRoundedBox() and decodeRing().

		struct RoundedBoxPayload
		{
			RectSPX			rect;
			spx				radius[4];				// Top-left, top-right, bottom-right, bottom-left.
			BorderSPX		outline;
			spx				softness;
			uint16_t		flags;					// See RoundedBoxFlags.
			const uint16_t*	pTints;					// Encoded tints.
		};

		struct RingPayload
		{
			RectSPX			rect;
			spx				thickness;
			spx				outline;
			spx				softness;
			uint16_t		flags;					// See RingFlags.
			int				nSlices;
			uint32_t		start;
			const uint16_t*	pSlices;				// See sliceSize() and sliceFlags().
			const uint16_t*	pTints;					// Encoded tints.

			uint32_t		sliceSize(int slice) const;
			uint16_t		sliceFlags(int slice) const;	// See RingSliceFlags.
		};

		static RoundedBoxPayload	decodeRoundedBox(const uint16_t* pPayload);
		static RingPayload			decodeRing(const uint16_t* pPayload);

		// Decoded WarpBlit and WarpDrawEdgemap commands, see decodeWarp().

		struct WarpParams
		{
			int				nRects;
			uint32_t		warpmap;				// Index in object table.
			uint32_t		edgemap;				// Index in object table, WarpDrawEdgemap only.
			CoordSPX		dest;
			CoordSPX		sourceOffset;
			RectSPX			source;					// WarpBlit only.
		};

		// Decodes a WarpBlit or WarpDrawEdgemap command. p points at the word after the command,
		// the returned pointer at the word after its parameters.

		static const uint16_t*		decodeWarp(Command cmd, const uint16_t* p, WarpParams& params);

		// Reads an index into the object table and steps p past it.

		static inline uint32_t		readObjectIndex(const uint16_t*& p);

		// The caps word of a line. Bits 0-2 hold the LineCapShape of the begin and bits 8-10 that
		// of the end. Bits 3-7 and 11-15 hold the spear lengths of the begin and end in steps of
		// 25 percent, minus one (0 is 25%, 31 is 800%), and are 0 for other shapes.

		static uint16_t				encodeLineCaps(LineCaps caps);
		static LineCaps				decodeLineCaps(uint16_t caps);

		enum class StateChange : uint8_t
		{
			BlitSource		= 1,
			TintColor		= 1 << 1,		// Flat tint color. Multiplied with Tint if both are set.
			Tint			= 1 << 2,		// Tint (gradient), encoded as described in TintTools.
			BlendMode		= 1 << 3,
			MorphFactor		= 1 << 4,
			FixedBlendColor	= 1 << 5,
			Blur			= 1 << 6,
		};
		
		
		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering ________________________________________________

		virtual void	beginRender() = 0;
		virtual void	endRender() = 0;
		
		virtual void	beginSession( CanvasRef canvasRef, Surface * pCanvas, int nUpdateRects, const RectSPX * pUpdateRects, const SessionInfo * pInfo = nullptr ) = 0;
		virtual void	endSession() = 0;

		virtual void	setCanvas( Surface * pSurface ) = 0;
		virtual void	setCanvas( CanvasRef ref ) = 0;

		virtual void	setObjects(Object* const * pBeg, Object* const * pEnd) = 0;

		virtual void	setRects(const RectSPX* pBeg, const RectSPX* pEnd) = 0;
		virtual void	setColors(const HiColor* pBeg, const HiColor* pEnd) = 0;
		virtual void	setTransforms(const Transform * pBeg, const Transform * pEnd) = 0;

		virtual void	processCommands( const uint16_t* pBeg, const uint16_t * pEnd, int version = 2) = 0;


		//.____ Misc _________________________________________________________

		virtual const CanvasInfo *	canvasInfo(CanvasRef ref) const = 0;

		virtual SurfaceFactory_p	surfaceFactory() = 0;
		virtual EdgemapFactory_p	edgemapFactory() = 0;
		virtual WarpmapFactory_p	warpmapFactory() = 0;

		virtual int		maxEdges() const = 0;

		virtual bool	canBeBlitSource(const TypeInfo& type) const = 0;
		virtual bool	canBeCanvas(const TypeInfo& type) const = 0;

		virtual void	waitForCompletion() = 0;

		static const int	NbStandardTransforms = GfxFlip_size + 3;

	protected:
		GfxBackend() {};
		virtual ~GfxBackend() {};

		static const Transform			s_standardTransforms[NbStandardTransforms];
		static const int				s_defaultBlur[9];
		static const spx				s_defaultBlurRadius;

	};

	//____ readObjectIndex() _____________________________________________________

	inline uint32_t GfxBackend::readObjectIndex(const uint16_t*& p)
	{
		uint32_t index;
		std::memcpy(&index, p, 4);
		p += 2;
		return index;
	}

} // namespace wax
#endif	// WAX_GFXBACKEND_DOT_H

