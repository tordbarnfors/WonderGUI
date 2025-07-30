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

#ifndef	WG_CONTAINERINFOPANEL_DOT_H
#define WG_CONTAINERINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>

namespace wg
{
	class ContainerInfoPanel;
	typedef	StrongPtr<ContainerInfoPanel>	ContainerInfoPanel_p;
	typedef	WeakPtr<ContainerInfoPanel>	ContainerInfoPanel_wp;



	class ContainerInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static ContainerInfoPanel_p		create( const Blueprint& blueprint, Container * pContainer) { return ContainerInfoPanel_p(new ContainerInfoPanel(blueprint, pContainer) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		ContainerInfoPanel(const Blueprint& blueprint, Container * pContainer );
		~ContainerInfoPanel() {}
	};

} // namespace wg
#endif //WG_CONTAINERINFOPANEL_DOT_H

