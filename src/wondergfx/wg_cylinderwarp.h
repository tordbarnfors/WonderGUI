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
#ifndef	WG_CYLINDERWARP_DOT_H
#define	WG_CYLINDERWARP_DOT_H
#pragma once

#include <wg_warpgenerator.h>

namespace wg
{
	class CylinderWarp;
	typedef	StrongPtr<CylinderWarp>	CylinderWarp_p;
	typedef	WeakPtr<CylinderWarp>	CylinderWarp_wp;

	//____ CylinderWarp ________________________________________________________
	/**
	 * @brief Makes a Warpmap that wraps the source around a cylinder seen from the side, such as the
	 * reel of a slot machine.
	 *
	 * The cylinder fills the Warpmap. With axis X it lies down and turns up and down, with axis Y it
	 * stands up and turns sideways. Along the axis the source is stretched to the length of the
	 * cylinder. Across it, span degrees of the cylinder are seen, centered on the side facing the
	 * viewer, and the source is squeezed more towards the rims. The view is straight from the
	 * side, without perspective.
	 *
	 * Around the cylinder, the side of the source across the axis (its height for axis X, its width
	 * for axis Y) covers sweep degrees, starting at startAngle. Angles are measured from the side
	 * facing the viewer, increasing downwards for axis X and to the right for axis Y. That axis of
	 * the source is periodic, with a period of its length * 360 / |sweep|, so a sweep of 360 wraps
	 * the source once around the cylinder, seamlessly.
	 *
	 * Turn the cylinder by drawing with rotationOffset() rather than generating again.
	 *
	 * Positions change linearly along the axis, so cells span the length of the cylinder unless
	 * set. Across it, the cell size is calculated from precision at the rims, where the cylinder
	 * curves away the most, so a span close to 180 degrees takes small cells. Shading, such as the
	 * cylinder getting darker towards the rims, is done with a tint when drawing.
	 */

	class CylinderWarp : public WarpGenerator
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			Axis				axis = Axis::X;					// Axis of the cylinder. X lies down, like a slot machine reel.
			SizeI				cellSize;						// Pixels between nodes of the Warpmap. 0 = calculated, see above.
			Finalizer_p			finalizer = nullptr;
			float				precision = 0.25f;				// Max position error in pixels when cellSize is calculated.
			SizeI				size;							// Mandatory. Size of the Warpmap in pixels, which the cylinder fills.
			SizeSPX				sourceSize;						// Mandatory. Size of the source (edgemap or source rectangle) in spx.
			float				span = 120.f;					// Degrees of the cylinder seen, centered on the side facing the viewer. Above 0, below 180.
			float				startAngle = 0.f;				// Degrees. Where the start of the source across the axis is, 0 = facing the viewer.
			float				sweep = 360.f;					// Degrees around the cylinder covered by the source. Negative goes the other way.
		};

		//.____ Creation __________________________________________

		static CylinderWarp_p	create( const Blueprint& blueprint, WarpmapFactory * pFactory = nullptr );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________________

		bool		setAxis( Axis axis );
		Axis		axis() const { return m_axis; }

		bool		setSpan( float span );
		float		span() const { return m_span; }

		bool		setAngles( float startAngle, float sweep );
		float		startAngle() const { return m_startAngle; }
		float		sweep() const { return m_sweep; }

		//.____ Drawing _____________________________________________________

		// Source offset that turns the cylinder by degrees, downwards (axis X) or to the right
		// (axis Y) if positive, as if startAngle was that much larger. Pass it when drawing to turn
		// the cylinder without generating it again. Goes by the axis, sweep and source size of the
		// generated Warpmap.

		CoordSPX	rotationOffset( float degrees ) const;

	protected:

		CylinderWarp( const Blueprint& bp, WarpmapFactory * pFactory );
		virtual ~CylinderWarp();

		static bool	_isValidSpan( float span ) { return span > 0.f && span < 180.f; }

		SizeI		_calcCellSize() const override;
		void		_generateNodes( CoordSPX * pNodes, SizeI nodes, SizeI cellSize ) const override;
		spx			_uPeriod() const override;
		spx			_vPeriod() const override;
		void		_generated() override { m_generatedAxis = m_axis; m_offsetFactor = _offsetFactor(m_sweep / 360.0, _lengthAcross(m_axis, m_sourceSize)); }

		static spx	_lengthAcross( Axis axis, SizeSPX source ) { return axis == Axis::X ? source.h : source.w; }
		void		_rim( int32_t& sin, int32_t& cos ) const;

		spx			_periodAcross() const;

		Axis		m_axis;
		float		m_span;
		float		m_startAngle;
		float		m_sweep;

		Axis		m_generatedAxis;							// For rotationOffset(), by the generated Warpmap.
		Factor		m_offsetFactor;
	};

} // namespace wg
#endif // WG_CYLINDERWARP_DOT_H
