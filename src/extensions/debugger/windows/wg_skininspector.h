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

#ifndef	WG_SKINISPECTOR_DOT_H
#define WG_SKINISPECTOR_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugwindow.h>
#include <wg_skindisplay.h>
#include <wg_selectbox.h>


namespace wg
{
	class SkinInspector;
	typedef	StrongPtr<SkinInspector>	SkinInspector_p;
	typedef	WeakPtr<SkinInspector>	SkinInspector_wp;



	class SkinInspector : public DebugWindow
	{
	public:

		//.____ Creation __________________________________________

		static SkinInspector_p		create( const Blueprint& blueprint, IDebugger * pHolder, Skin * pSkin) { return SkinInspector_p(new SkinInspector(blueprint, pHolder, pSkin) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		SkinInspector(const Blueprint& blueprint, IDebugger * pHolder, Skin * pSkin );
		~SkinInspector() {}

		void	_refreshState();

		Skin_p	m_pSkin;			// Keep our skin alive as long as we are, so refresh won't fail.

		SkinDisplay_p	m_pSkinDisplay;
		SelectBox_p		m_pStateSelector;

	};

} // namespace wg
#endif //WG_SKINISPECTOR_DOT_H


