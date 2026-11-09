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
#ifndef	WAX_GPUTOOLS_DOT_H
#define	WAX_GPUTOOLS_DOT_H
#pragma once

#include <wax_backend.h>
#include <wax_tinttools.h>

namespace wax
{
	/**
	 * @brief Work that the GPU backends do the same way, kept here so that they do.
	 *
	 * Only what is worked out on the CPU and doesn't depend on the graphics API: decoding the
	 * command stream and the geometry and constants that shaders get. How the results reach the
	 * GPU (buffers, uniforms, root constants...) stays with each backend. See also GpuShapes,
	 * for shapes and lines with caps.
	 */

	namespace GpuTools
	{
		//____ State changes __________________________________________________

		/**
		 * The payload of a Backend::Command::StateChange, decoded. Only the members of the states
		 * in `changed` are set.
		 */

		struct StateChangeParams
		{
			uint8_t						changed;			// Backend::StateChange bits.

			uint32_t					blitSource;			// Index in the object table.
			HiColor						tintColor;
			TintTools::DecodedTint		tint;				// nLayers is 0 for no tint.
			BlendMode					blendMode;
			uint16_t					morphFactor;		// 4096 is 1.0.
			HiColor						fixedBlendColor;
			spx							blurRadius;
			const uint16_t*				pBlurWeights;		// 27 weights: nine taps of red, then green, then blue. See blurColorMatrix().
		};

		/**
		 * Decodes a StateChange. p points at the word after the command, pColors at the next color
		 * of the color buffer and is stepped past the colors of the payload. Returns a pointer to
		 * the word after the payload, which is 32-bit aligned as the next command needs.
		 */

		const uint16_t*	decodeStateChange(const uint16_t* p, const HiColor*& pColors, StateChangeParams& params);

		inline bool		hasChanged(const StateChangeParams& params, Backend::StateChange state) { return (params.changed & uint8_t(state)) != 0; }

		inline float	morphFactor(uint16_t value) { return value / 4096.f; }


		//____ Blur ___________________________________________________________

		/**
		 * Weights of the nine taps of a blur per color channel, as rows of red, green, blue and
		 * alpha. Weights are 15 bit fixed point and can go above 1.0. Alpha is taken from the
		 * middle tap only. Weights are read as unsigned 16-bit values, also from a backend that has
		 * copied them into wider integers.
		 */

		template<typename Weight>
		void	blurColorMatrix(const Weight* pWeights, float matrix[9][4])
		{
			for (int i = 0; i < 9; i++)
			{
				matrix[i][0] = pWeights[i] / 32768.f;
				matrix[i][1] = pWeights[9 + i] / 32768.f;
				matrix[i][2] = pWeights[18 + i] / 32768.f;
				matrix[i][3] = 0.f;
			}

			matrix[4][3] = 1.f;
		}

		/**
		 * Offsets of the nine taps of a blur in texture coordinates of a source of the given size,
		 * left to right and top to bottom. Taps are whole pixels, rounded the way the software
		 * backends do it, so that they land on texel centers and read the same texels whatever the
		 * sample method. The corner taps are at 0.7 of the radius.
		 */

		void	blurOffsets(spx radius, SizeI sourceSize, float offsets[9][2]);


		//____ Lines __________________________________________________________

		/**
		 * Width of a line of the given thickness that keeps it even for a slope from 0 to 1 (the
		 * slope along its main axis), looked up in a table and interpolated.
		 */

		float	scaleLineThickness(float thickness, float slope);

		/**
		 * A line without caps (the legacy way, see LineCap), drawn as a quad that holds every pixel
		 * whose center is closer than w to the center of the line, rounded outwards. The pixel
		 * shader works out the coverage of each pixel from offset, w and slope.
		 *
		 * The line runs along x unless steep, then along y. Positions are canvas pixels, top-left
		 * origin. offset is where the center of the line crosses the other axis at 0 of the main
		 * axis. Backends that flip y work out their own from begin, which is the end the line is
		 * stepped from, after rounding to whole pixels.
		 */

		struct LineQuad
		{
			CoordI		corners[4];			// In order around the quad.
			CoordI		begin;
			float		offset;
			float		w;					// Half the width plus half a pixel.
			float		slope;
			bool		bSteep;
		};

		/**
		 * Returns false for lines of no length, which aren't drawn.
		 */

		bool	lineQuad(CoordSPX begin, CoordSPX end, spx thickness, LineQuad& quad);

		/**
		 * A line with caps, collected from a Line command to be drawn as a shape once the command
		 * has been read, see GpuShapes::writeLine().
		 */

		struct CappedLine
		{
			CoordSPX	begin;
			CoordSPX	end;
			spx			thickness;
			uint16_t	caps;
			HiColor		color;
		};
	}
}

#endif //WAX_GPUTOOLS_DOT_H
