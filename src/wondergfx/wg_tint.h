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
#ifndef	WG_TINT_DOT_H
#define	WG_TINT_DOT_H
#pragma once

#include <wg_object.h>
#include <wg_gfxtypes.h>
#include <wg_geo.h>
#include <wg_color.h>

#include <vector>
#include <initializer_list>

namespace wg
{

	//____ ColorStop __________________________________________________________

	struct ColorStop
	{
		float		pos = 0.f;					// 0.0 -> 1.0 along the gradient.
		HiColor		color = HiColor::White;
	};


	class Tint;
	typedef	StrongPtr<Tint>	Tint_p;
	typedef	WeakPtr<Tint>	Tint_wp;

	//____ Tint _______________________________________________________________
	/**
	 * @brief Immutable description of a color gradient used for tinting.
	 *
	 * A Tint gives each pixel a position along a gradient, determined by its
	 * shape (linear or radial), and turns that position into a color through
	 * a list of color stops. Everything drawn inside the Tint is multiplied
	 * by that color.
	 *
	 * Geometry is specified relative to the rectangle the Tint is placed in,
	 * (0,0) being the top-left corner and (1,1) the bottom-right, so a Tint
	 * stretches with whatever it is used for.
	 *
	 * Two stops with the same position give a hard edge. A single stop gives
	 * a flat color.
	 *
	 * A Tint can also be a mix of up to four simple Tints with individual
	 * weights, which is what Tint::blend() returns when two Tints can't be
	 * morphed into each other. A mix is accepted everywhere a Tint is.
	 */

	class Tint : public Object
	{
	public:

		static constexpr int	c_maxStops = 16;
		static constexpr int	c_maxMixComponents = 4;

		//.____ Blueprint _________________________________________________________

		struct Blueprint
		{
			CoordF					begin = { 0.f, 0.f };		// Linear: where the gradient starts (position 0.0), relative to rect.
			CoordF					center = { 0.5f, 0.5f };	// Radial: center (position 0.0), relative to rect.
			ColorSpace				colorSpace = ColorSpace::Linear;	// Color space stops are interpolated in.
			CoordF					end = { 0.f, 1.f };			// Linear: where the gradient ends (position 1.0), relative to rect.
			Finalizer_p				finalizer = nullptr;
			SizeF					radius = { 0.5f, 0.5f };	// Radial: distance to position 1.0, see radiusMode.
			TintRadius				radiusMode = TintRadius::Fit;
			TintShape				shape = TintShape::Linear;
			TintSpread				spread = TintSpread::Pad;
			std::vector<ColorStop>	stops;						// 1 to c_maxStops stops, positions in ascending order.
		};

		//.____ Creation __________________________________________________________

		static Tint_p	create(const Blueprint& blueprint);
		static Tint_p	create(HiColor color);
		static Tint_p	create(HiColor from, HiColor to, CoordF begin = { 0.f, 0.f }, CoordF end = { 0.f, 1.f }, ColorSpace colorSpace = ColorSpace::Linear);
		static Tint_p	create(std::initializer_list<ColorStop> stops, CoordF begin = { 0.f, 0.f }, CoordF end = { 0.f, 1.f }, ColorSpace colorSpace = ColorSpace::Linear);

		static Tint_p	blend(Tint* pFrom, Tint* pTo, float progress);
		static Tint_p	createMix(int nComponents, Tint* const * pComponents, const float* pWeights);	// Components must be simple Tints, weights are normalized.

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content _________________________________________________

		inline bool			isMix() const { return m_nMixComponents > 0; }
		inline bool			isOpaque() const { return m_bOpaque; }
		inline bool			isFlat() const { return m_bFlat; }

		inline TintShape	shape() const { return m_shape; }
		inline TintSpread	spread() const { return m_spread; }
		inline ColorSpace	colorSpace() const { return m_colorSpace; }

		inline CoordF		begin() const { return m_begin; }
		inline CoordF		end() const { return m_end; }

		inline CoordF		center() const { return m_center; }
		inline SizeF		radius() const { return m_radius; }
		inline TintRadius	radiusMode() const { return m_radiusMode; }

		inline int				nbStops() const { return m_nStops; }
		inline const ColorStop*	stops() const { return m_stops; }

		inline int			nbMixComponents() const { return m_nMixComponents; }
		inline Tint*		mixComponent(int index) const { return m_mixComponents[index]; }
		inline float		mixWeight(int index) const { return m_mixWeights[index]; }

		//.____ Misc ______________________________________________________

		float		position(CoordF pos, const RectF& rect) const;	// Position along gradient before spread is applied. Simple Tints only.
		HiColor		colorAtPosition(float position) const;			// Spread is applied. Simple Tints only.

		HiColor		colorAt(CoordSPX pos, const RectSPX& rect) const;	// Color of pixel whose top-left corner is at pos.
		int			alpha(CoordSPX pos, const RectSPX& rect) const;	// 0 -> 4096.

		bool		isMorphableTo(const Tint* pOther) const;

	protected:
		Tint();
		Tint(const Blueprint& bp);
		virtual ~Tint() {}

		static bool		_morphStops(const Tint* pFrom, const Tint* pTo, float progress, ColorStop* pOutput, int& nOutput);
		static HiColor	_mixColors(HiColor from, HiColor to, float fraction, ColorSpace colorSpace);
		HiColor			_colorAtStops(float position, bool bLeftLimit) const;

		void			_updateFlags();

		TintShape		m_shape = TintShape::Linear;
		TintSpread		m_spread = TintSpread::Pad;
		ColorSpace		m_colorSpace = ColorSpace::Linear;
		TintRadius		m_radiusMode = TintRadius::Fit;

		CoordF			m_begin = { 0.f, 0.f };
		CoordF			m_end = { 0.f, 1.f };
		CoordF			m_center = { 0.5f, 0.5f };
		SizeF			m_radius = { 0.5f, 0.5f };

		int				m_nStops = 0;
		ColorStop		m_stops[c_maxStops];

		int				m_nMixComponents = 0;
		Tint_p			m_mixComponents[c_maxMixComponents];
		float			m_mixWeights[c_maxMixComponents] = { 0.f, 0.f, 0.f, 0.f };

		bool			m_bOpaque = true;
		bool			m_bFlat = false;
	};

}



#endif //WG_TINT_DOT_H
