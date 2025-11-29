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
#include <wg_transitions.h>

namespace wg
{
	const TypeInfo Transition::TYPEINFO = { "Transition", &Object::TYPEINFO };
	const TypeInfo ColorTransition::TYPEINFO = { "ColorTransition", &Transition::TYPEINFO };
	const TypeInfo ValueTransition::TYPEINFO = { "ValueTransition", &Transition::TYPEINFO };
	const TypeInfo CoordTransition::TYPEINFO = { "CoordTransition", &Transition::TYPEINFO };


	//____ typeInfo() ____________________________________________________________

	const TypeInfo& Transition::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _normalize() _______________________________________________________

	int	Transition::_normalize(TransitionCurve type, int duration, int timestamp)
	{
		// 8.24 binal format.

		// Methods adapted from Creak's answer on StackOverflow.
		// https://stackoverflow.com/questions/13462001/ease-in-and-ease-out-animation-formula


		int64_t normTS = int((int64_t(timestamp) << 24) / duration);

		switch (type)
		{
			default:
			case TransitionCurve::Linear:
				return (int)normTS;

			case TransitionCurve::EaseIn:
				return int((normTS * normTS) >> 24);

			case TransitionCurve::EaseOut:
				return int((normTS * ((2 << 24) - normTS)) >> 24);

			case TransitionCurve::EaseInOut:
			{
				int halfpoint = 1 << 23;

				if (normTS <= halfpoint)
					return int((2 * (normTS * normTS)) >> 24);

				normTS -= halfpoint;
				return int((2 * normTS * ((1 << 24) - normTS)) >> 24) + halfpoint;
			}

			case TransitionCurve::Bezier:
				return int((((normTS * normTS) >> 24) * ((3 << 24) - (((2 << 24) * normTS)>>24))) >> 24);

			case TransitionCurve::Parametric:
			{
				int64_t sqt = (normTS * normTS) >> 24;

				return int((sqt << 24) / (2 * (sqt - normTS) + (1 << 24)));
			}
		}

	}

	//____ ColorTransition::create() __________________________________________

	ColorTransition_p ColorTransition::create(const Blueprint& blueprint)
	{
		return ColorTransition_p(new ColorTransition(blueprint));
	}

	ColorTransition_p ColorTransition::create(int duration, TransitionCurve curve)
	{
		return ColorTransition_p(new ColorTransition(WGBP(ColorTransition, _.duration = duration, _.curve = curve)));
	}


	//____ ColorTransition::constructor ________________________________________

	ColorTransition::ColorTransition(const Blueprint& bp) :

		m_duration(bp.duration),
		m_midPoint(bp.midPointColor)
	{
		

		if (m_midPoint.isValid() )
		{
			int begin = m_duration / 2 + bp.midPointOffset - bp.midPointDelay/2;
			int end = begin + bp.midPointDelay;
			
			limit(begin, 1, m_duration - 1);
			limit(end, begin, m_duration - 1);

			m_midPointBegin = begin;
			m_midPointEnd = end;

			const static TransitionCurve curves[4] = { TransitionCurve::Linear, TransitionCurve::EaseIn, TransitionCurve::EaseOut, TransitionCurve::EaseInOut };

			int easeOne = 0;

			if (bp.curve == TransitionCurve::EaseIn || bp.curve == TransitionCurve::EaseInOut)
				easeOne = 1;

			if (bp.midPointCurve == TransitionCurve::EaseInOut || bp.midPointCurve == TransitionCurve::EaseIn)
				easeOne += 2;

			m_curve = curves[easeOne];

			int easeTwo = 0;

			if (bp.curve == TransitionCurve::EaseOut || bp.curve == TransitionCurve::EaseInOut)
				easeTwo = 2;

			if (bp.midPointCurve == TransitionCurve::EaseInOut || bp.midPointCurve == TransitionCurve::EaseOut)
				easeTwo += 2;

			m_curve2 = curves[easeTwo];
 		}
		else
		{
			m_curve = bp.curve;
			m_curve2 = TransitionCurve::Linear;
			m_midPointBegin = -1;
			m_midPointEnd = -1;
		}

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& ColorTransition::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ ColorTransition::snapshot() ________________________________________

	HiColor	ColorTransition::snapshot(int timestamp, HiColor startColor, HiColor endColor)
	{
		limit(timestamp, 0, m_duration);

		if (m_midPointBegin > 0)
		{
			if (timestamp < m_midPointBegin)
			{
				int progress = _normalize(m_curve, m_midPointBegin, timestamp);
				return HiColor::mix(startColor, m_midPoint, progress >> (24-12));
			}
			else if (timestamp >= m_midPointEnd)
			{
				int progress = _normalize(m_curve2, m_duration - m_midPointEnd, timestamp - m_midPointEnd);
				return HiColor::mix(m_midPoint, endColor, progress >> (24-12));
			}
			else
				return m_midPoint;
		}
		else
		{
			int progress = _normalize(m_curve, m_duration, timestamp);
			return HiColor::mix(startColor, endColor, progress >> (24-12));
		}
	}

	void ColorTransition::snapshot(int timestamp, int nColors, const HiColor* pStartColors, const HiColor* pEndColors, HiColor* pOutput)
	{
		limit(timestamp, 0, m_duration);

		if (m_midPointBegin > 0)
		{
			if (timestamp < m_midPointBegin)
			{
				int progress = _normalize(m_curve, m_midPointBegin, timestamp) >> (24 - 12);

				for (int i = 0; i < nColors; i++)
					pOutput[i] = HiColor::mix(pStartColors[i], m_midPoint, progress);
			}
			else if (timestamp >= m_midPointEnd)
			{
				int progress = _normalize(m_curve2, m_duration - m_midPointEnd, timestamp - m_midPointEnd) >> (24 - 12);

				for (int i = 0; i < nColors; i++)
					pOutput[i] = HiColor::mix(m_midPoint, pEndColors[i], progress);
			}
			else
			{
				for (int i = 0; i < nColors; i++)
					pOutput[i] = m_midPoint;
			}
		}
		else
		{
			int progress = _normalize(m_curve, m_duration, timestamp) >> (24 - 12);

			for (int i = 0; i < nColors; i++)
				pOutput[i] = HiColor::mix(pStartColors[i], pEndColors[i], progress);
		}
	}


	//____ CoordTransition::create() __________________________________________

	CoordTransition_p CoordTransition::create(int duration, TransitionCurve curve)
	{
		return CoordTransition_p(new CoordTransition( duration, curve, curve));
	}

	CoordTransition_p CoordTransition::create(int duration, TransitionCurve curveX, TransitionCurve curveY )
	{
		return CoordTransition_p(new CoordTransition(duration, curveX, curveY));
	}

	CoordTransition_p CoordTransition::create(const Blueprint& blueprint)
	{
		return CoordTransition_p(new CoordTransition(blueprint));
	}

	//____ CoordTransition::constructor ________________________________________

	CoordTransition::CoordTransition(int duration, TransitionCurve curveX, TransitionCurve curveY) :
		m_duration(duration),
		m_curveX(curveX),
		m_curveY(curveY)
	{
	}

	CoordTransition::CoordTransition(const Blueprint& bp) :
		m_duration(bp.duration),
		m_curveX(bp.curveX),
		m_curveY(bp.curveY)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}


	//____ typeInfo() ____________________________________________________________

	const TypeInfo& CoordTransition::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ CoordTransition::snapshot() ________________________________________

	CoordI CoordTransition::snapshot(int timestamp, CoordI startValue, CoordI endValue)
	{
		limit(timestamp, 0, m_duration);

		int64_t progressX = _normalize(m_curveX, m_duration, timestamp);
		int64_t progressY = _normalize(m_curveY, m_duration, timestamp);

		return { startValue.x + int(((endValue.x - startValue.x) * progressX) >> 24),
				 startValue.y + int(((endValue.y - startValue.y) * progressY) >> 24) };
	}

	CoordF CoordTransition::snapshot(int timestamp, CoordF startValue, CoordF endValue)
	{
		limit(timestamp, 0, m_duration);

		float progressX = float(_normalize(m_curveX, m_duration, timestamp)) / (65536.f * 256.f);
		float progressY = float(_normalize(m_curveY, m_duration, timestamp)) / (65536.f * 256.f);

		return { startValue.x + (endValue.x - startValue.x) * progressX,
				 startValue.y + (endValue.y - startValue.y) * progressY };
	}

	void CoordTransition::snapshot(int timestamp, int nCoords, const CoordI* pStartCoords, const CoordI* pEndCoords, CoordI* pOutput)
	{
		limit(timestamp, 0, m_duration);

		int64_t progressX = _normalize(m_curveX, m_duration, timestamp);
		int64_t progressY = _normalize(m_curveY, m_duration, timestamp);

		for (int i = 0; i < nCoords; i++)
		{
			CoordI start = pStartCoords[i];
			CoordI end = pEndCoords[i];

			pOutput[i] = {  start.x + int(((end.x - start.x) * progressX) >> 24),
							start.y + int(((end.y - start.y) * progressY) >> 24) };
		}
	}

	void CoordTransition::snapshot(int timestamp, int nCoords, const CoordF* pStartCoords, const CoordF* pEndCoords, CoordF* pOutput)
	{
		limit(timestamp, 0, m_duration);

		float progressX = float(_normalize(m_curveX, m_duration, timestamp)) / (65536.f * 256.f);
		float progressY = float(_normalize(m_curveY, m_duration, timestamp)) / (65536.f * 256.f);

		for (int i = 0; i < nCoords; i++)
		{
			CoordF start	= pStartCoords[i];
			CoordF end		= pEndCoords[i];

			pOutput[i] = { start.x + (end.x - start.x) * progressX,
							start.y + (end.y - start.y) * progressY };
		}
	}


	//____ ValueTransition::create() __________________________________________

	ValueTransition_p ValueTransition::create(int duration, TransitionCurve curve)
	{
		return ValueTransition_p(new ValueTransition(duration, curve));
	}

	//____ ValueTransition::constructor ________________________________________

	ValueTransition::ValueTransition(int duration, TransitionCurve curve) :
		m_duration(duration),
		m_curve(curve)
	{
	}

	ValueTransition::ValueTransition(const Blueprint& bp) :
		m_duration(bp.duration),
		m_curve(bp.curve)
	{
		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& ValueTransition::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ ValueTransition::snapshot() ________________________________________

	int ValueTransition::snapshot(int timestamp, int startValue, int endValue)
	{
		limit(timestamp, 0, m_duration);

		int64_t progress = _normalize(m_curve, m_duration, timestamp);
		
		return startValue + int(((endValue - startValue) * progress) >> 24);
	}

	float ValueTransition::snapshot(int timestamp, float startValue, float endValue)
	{
		limit(timestamp, 0, m_duration);

		float progress = float(_normalize(m_curve, m_duration, timestamp)) / (65536.f*256.f);

		return startValue + (endValue - startValue) * progress;
	}

	void ValueTransition::snapshot(int timestamp, int nValues, const int* pStartValues, const int* pEndValues, int* pOutput)
	{
		limit(timestamp, 0, m_duration);

		int64_t progress = _normalize(m_curve, m_duration, timestamp);

		for (int i = 0; i < nValues; i++)
		{
			int start = pStartValues[i];
			int end = pEndValues[i];

			pOutput[i] = start + int(((end - start) * progress) >> 24);
		}
	}

	void ValueTransition::snapshot(int timestamp, int nValues, const float* pStartValues, const float* pEndValues, float* pOutput)
	{
		limit(timestamp, 0, m_duration);

		float progress = float(_normalize(m_curve, m_duration, timestamp)) / (65536.f * 256.f);

		for (int i = 0; i < nValues; i++)
		{
			float start = pStartValues[i];
			float end = pEndValues[i];

			pOutput[i] = start + (end - start) * progress;
		}
	}
}
