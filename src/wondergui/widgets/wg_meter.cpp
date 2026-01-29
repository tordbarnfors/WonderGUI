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
#include <wg_meter.h>
#include <wg_gfxdevice.h>
#include <wg_msg.h>
#include <wg_base.h>
#include <wg_msgrouter.h>

namespace wg
{
	using namespace Util;

	const TypeInfo Meter::TYPEINFO = { "Meter", &Widget::TYPEINFO };


	//____ typeInfo() _________________________________________________________

	const TypeInfo& Meter::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setValue() _________________________________________________________

	void Meter::setValue(float value)
	{
		limit(value, 0, 1.f);

		if (value != m_value)
		{
			m_skin.valueChanged(value, m_value);
			m_value = value;
		}
	}

	//____ _render() __________________________________________________________

	void Meter::_render(GfxDevice * pDevice, const RectSPX& canvas, const RectSPX& window)
	{
		if (!m_skin.isEmpty())
		{
			m_skin.render(pDevice, canvas, m_scale, m_state, m_value );
		}
	}

	//____ _skinValue() _______________________________________________________

	float Meter::_skinValue(const SkinSlot* pSlot) const
	{
		return m_value;
	}


} // namespace wg
