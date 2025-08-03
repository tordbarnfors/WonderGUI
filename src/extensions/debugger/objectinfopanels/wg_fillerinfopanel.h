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

#ifndef	WG_FILLERINFOPANEL_DOT_H
#define WG_FILLERINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_filler.h>

namespace wg
{
	class FillerInfoPanel;
	typedef	StrongPtr<FillerInfoPanel>	FillerInfoPanel_p;
	typedef	WeakPtr<FillerInfoPanel>	FillerInfoPanel_wp;



	class FillerInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static FillerInfoPanel_p		create( const Blueprint& blueprint, DebugPanel::Holder* pHolder, Filler * pFiller) { return FillerInfoPanel_p(new FillerInfoPanel(blueprint, pHolder, pFiller) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		FillerInfoPanel(const Blueprint& blueprint, DebugPanel::Holder* pHolder, Filler * pFiller );
		~FillerInfoPanel() {}
	};

} // namespace wg
#endif //WG_FILLERINFOPANEL_DOT_H

