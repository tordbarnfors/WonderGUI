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
#ifndef TRANSITIONS_DOT_H
#define TRANSITIONS_DOT_H
#pragma once

#include <wg_gfxtypes.h>
#include <wg_color.h>
#include <wg_geo.h>
#include <wg_object.h>

namespace wg
{

	//____ Transition _________________________________________________________

	class Transition;
	typedef	StrongPtr<Transition>	Transition_p;
	typedef	WeakPtr<Transition>		Transition_wp;

	class Transition : public Object
	{
	public:

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		Transition() {}

		int		_normalize(TransitionCurve curve, int durationMicrosec , int timestampMicrosec);

	};

	//____ ColorTransition ____________________________________________________

	class ColorTransition;
	typedef	StrongPtr<ColorTransition>	ColorTransition_p;
	typedef	WeakPtr<ColorTransition>	ColorTransition_wp;

	class ColorTransition : public Transition
	{
	public:

		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			TransitionCurve		curve = TransitionCurve::Linear;
			int					duration = 250*1000;					// Microseconds!
			Finalizer_p			finalizer = nullptr;
			HiColor				midPointColor = HiColor::Undefined;
			int					midPointDelay = 0;
			TransitionCurve		midPointCurve = TransitionCurve::Linear;
			int					midPointOffset = 0;
		};

		//.____ Creation ______________________________________________________

		static ColorTransition_p	create(int duration, TransitionCurve curve = TransitionCurve::Linear);
		static ColorTransition_p	create(const Blueprint& blueprint);

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc ___________________________________________________________

		int		duration() const { return m_duration; }

		HiColor	snapshot(int timestampMicroSec, HiColor startColor, HiColor endColor );

		void	snapshot(int timestampMicroSec, int nColors, const HiColor* pStartColors, const HiColor* pEndColors, HiColor* pOutput);


	protected:
		ColorTransition( const Blueprint& blueprint );

		TransitionCurve	m_curve;		// Curve for transition before midpoint or for whole curve if no midpoint.
		TransitionCurve	m_curve2;		// Curve for transition after midpoint.
		int				m_duration;		// Microseconds
		int				m_midPointBegin;
		int				m_midPointEnd;

		HiColor			m_midPoint;
	};

	//____ CoordTransition ____________________________________________________

	class CoordTransition;
	typedef	StrongPtr<CoordTransition>	CoordTransition_p;
	typedef	WeakPtr<CoordTransition>	CoordTransition_wp;

	class CoordTransition : public Transition
	{
	public:

		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			TransitionCurve		curveX = TransitionCurve::Linear;
			TransitionCurve		curveY = TransitionCurve::Linear;
			int					duration = 250 * 1000;					// Microseconds!
			Finalizer_p			finalizer = nullptr;
		};

		//.____ Creation ______________________________________________________

		static CoordTransition_p	create(int duration, TransitionCurve curve = TransitionCurve::Linear);
		static CoordTransition_p	create(int duration, TransitionCurve curveX, TransitionCurve curveY );
		static CoordTransition_p	create(const Blueprint& blueprint);

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc ___________________________________________________________

		int		duration() const { return m_duration; }

		CoordI	snapshot(int timestampMicroSec, CoordI startPos, CoordI endPos);
		CoordF	snapshot(int timestampMicroSec, CoordF startPos, CoordF endPos);

		void	snapshot(int timestampMicroSec, int nCoords, const CoordI* pStartValues, const CoordI* pEndValues, CoordI* pOutput);
		void	snapshot(int timestampMicroSec, int nCoords, const CoordF* pStartValues, const CoordF* pEndValues, CoordF* pOutput);

	protected:
		CoordTransition(const Blueprint& blueprint);
		CoordTransition(int duration, TransitionCurve curveX, TransitionCurve curveY);

		TransitionCurve	m_curveX;
		TransitionCurve	m_curveY;
		int				m_duration;		// Microseconds
	};



	//____ ValueTransition ____________________________________________________

	class ValueTransition;
	typedef	StrongPtr<ValueTransition>	ValueTransition_p;
	typedef	WeakPtr<ValueTransition>	ValueTransition_wp;

	class ValueTransition : public Transition
	{
	public:

		//.____ Blueprint _____________________________________________________

		struct Blueprint
		{
			TransitionCurve		curve = TransitionCurve::Linear;
			int					duration = 250 * 1000;					// Microseconds!
			Finalizer_p			finalizer = nullptr;
		};


		//.____ Creation ______________________________________________________

		static ValueTransition_p	create( int duration, TransitionCurve curve = TransitionCurve::Linear);
		static ValueTransition_p	create( const Blueprint& blueprint) { return ValueTransition_p(new ValueTransition(blueprint)); };

		//.____ Identification _________________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Misc ___________________________________________________________

		int		duration() const { return m_duration; }

		float	snapshot(int timestampMicroSec, float startValue, float endValue);
		int		snapshot(int timestampMicroSec, int startValue, int endValue);

		void	snapshot(int timestampMicroSec, int nValues, const int* pStartValues, const int* pEndValues, int* pOutput);
		void	snapshot(int timestampMicroSec, int nValues, const float* pStartValues, const float* pEndValues, float* pOutput);

	protected:
		ValueTransition(int duration, TransitionCurve curve);
		ValueTransition(const Blueprint& blueprint);

		TransitionCurve	m_curve;
		int				m_duration;		// Microseconds
	};
}

#endif //TRANSITIONS_DOT_H
