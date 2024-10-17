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


#include <wg_gradyent.h>

namespace wg
{

	const TypeInfo Gradyent::TYPEINFO = { "Gradyent", &Tintmap::TYPEINFO };

	//____ create() ___________________________________________________________

	Gradyent_p	Gradyent::create(HiColor top, HiColor bottom, HiColor left, HiColor right)
	{
		if (!top.isValid() || !bottom.isValid() || !left.isValid() || !right.isValid() )
		{
			//TODO: Error handling!

			return nullptr;
		}

		return Gradyent_p(new Gradyent(top, bottom, left, right));
	}


	Gradyent_p Gradyent::create( const Gradient& gradient )
	{
		HiColor bottom( (gradient.bottomLeft.r + gradient.bottomRight.r) / 2,
							(gradient.bottomLeft.g + gradient.bottomRight.g) / 2,
							(gradient.bottomLeft.b + gradient.bottomRight.b) / 2,
							(gradient.bottomLeft.a + gradient.bottomRight.a) / 2 );

		HiColor top( (gradient.topLeft.r + gradient.topRight.r) / 2,
			(gradient.topLeft.g + gradient.topRight.g) / 2,
			(gradient.topLeft.b + gradient.topRight.b) / 2,
			(gradient.topLeft.a + gradient.topRight.a) / 2);

		HiColor left( (gradient.topLeft.r + gradient.bottomLeft.r) / 2,
			(gradient.topLeft.g + gradient.bottomLeft.g) / 2,
			(gradient.topLeft.b + gradient.bottomLeft.b) / 2,
			(gradient.topLeft.a + gradient.bottomLeft.a) / 2);

		HiColor right( (gradient.topRight.r + gradient.bottomRight.r) / 2,
			(gradient.topRight.g + gradient.bottomRight.g) / 2,
			(gradient.topRight.b + gradient.bottomRight.b) / 2,
			(gradient.topRight.a + gradient.bottomRight.a) / 2);

		float yDiff = (bottom.r - top.r) * (bottom.r - top.r) +
						(bottom.g - top.g) * (bottom.g - top.g) +
						(bottom.b - top.b) * (bottom.b - top.b) +
						(bottom.a - top.a) * (bottom.a - top.a);

		float xDiff = (left.r - right.r) * (left.r - right.r) +
			(left.g - right.g) * (left.g - right.g) +
			(left.b - right.b) * (left.b - right.b) +
			(left.a - right.a) * (left.a - right.a);


		if (xDiff > yDiff)
			return Gradyent_p( new Gradyent(HiColor::White, HiColor::White, left, right) );
		else
			return Gradyent_p( new Gradyent(top, bottom, HiColor::White, HiColor::White) );
	}


	//____ constructor ________________________________________________________

	Gradyent::Gradyent(HiColor top, HiColor bottom, HiColor left, HiColor right)
	{
		m_top		= top;
		m_bottom	= bottom;
		m_left		= left;
		m_right		= right;

		_setFlags();
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Gradyent::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ exportHorizontalColors() ______________________________________________

	void Gradyent::exportHorizontalColors(spx length, HiColor* pOutput)
	{
		if( !m_bHorizontal )
		{
			_exportDummyColors(length, pOutput);
			return;
		}
		
		int len = length / 64;
		for (int i = 0; i < len; i++)
		{
			HiColor col;

			col.r = m_left.r + ((int(m_right.r) - int(m_left.r)) * i / len);
			col.g = m_left.g + ((int(m_right.g) - int(m_left.g)) * i / len);
			col.b = m_left.b + ((int(m_right.b) - int(m_left.b)) * i / len);
			col.a = m_left.a + ((int(m_right.a) - int(m_left.a)) * i / len);

			pOutput[i] = col;
		}
	}

	//____ exportVerticalColors() ________________________________________________

	void Gradyent::exportVerticalColors(spx length, HiColor* pOutput)
	{
		if( !m_bVertical )
		{
			_exportDummyColors(length, pOutput);
			return;
		}

		int len = length / 64;
		for (int i = 0; i < len; i++)
		{
			HiColor col;

			col.r = m_top.r + ((int(m_bottom.r) - int(m_top.r)) * i / len);
			col.g = m_top.g + ((int(m_bottom.g) - int(m_top.g)) * i / len);
			col.b = m_top.b + ((int(m_bottom.b) - int(m_top.b)) * i / len);
			col.a = m_top.a + ((int(m_bottom.a) - int(m_top.a)) * i / len);

			pOutput[i] = col;
		}
	}

	//____ exportGradient() ______________________________________________________

	Gradient Gradyent::exportGradient()
	{
		Gradient g;
		
		g.topLeft = m_top * m_left;
		g.bottomLeft = m_bottom * m_left;
		g.topRight = m_top * m_right;
		g.bottomRight = m_bottom * m_right;

		return g;
	}


	//____ _setFlags() ________________________________________________________

	void Gradyent::_setFlags()
	{
		m_bOpaque = ((m_top.a + m_bottom.a + m_left.a + m_right.a) == 4096 * 4);
		m_bHorizontal = (m_left != m_right);
		m_bVertical = (m_top != m_bottom);
	}


}



