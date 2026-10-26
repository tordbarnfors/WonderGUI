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
#include <wg_blurbrush.h>

namespace wg
{

	const TypeInfo Blurbrush::TYPEINFO = { "Blurbrush", &Object::TYPEINFO };

	const float Blurbrush::s_defaultBlue[9] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
	const float Blurbrush::s_defaultGreen[9] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };
	const float Blurbrush::s_defaultRed[9] = { 0.1f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f };

	//____ create() ____________________________________________________________

	Blurbrush_p	Blurbrush::create(const Blueprint& blueprint)
	{
		return Blurbrush_p(new Blurbrush(blueprint));
	}

	//____ constructor _________________________________________________________

	Blurbrush::Blurbrush(const Blueprint& bp)
	{
		m_radius = bp.size;
		for (int i = 0; i < 9; i++)
		{
			m_blueMtx[i] = bp.blue[i];
			m_greenMtx[i] = bp.green[i];
			m_redMtx[i] = bp.red[i];
		}
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Blurbrush::typeInfo(void) const
	{
		return TYPEINFO;
	}

}



