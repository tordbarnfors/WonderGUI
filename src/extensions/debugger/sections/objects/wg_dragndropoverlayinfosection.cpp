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
#include "wg_dragndropoverlayinfosection.h"


namespace wg
{

	const TypeInfo DragNDropOverlayInfoSection::TYPEINFO = { "DragNDropOverlayInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	DragNDropOverlayInfoSection::DragNDropOverlayInfoSection(const DebugTheme& theme, IDebugContext* pContext, DragNDropOverlay * pInspected)
		: TypedInfoSection<DragNDropOverlay>( theme, pContext, DragNDropOverlay::TYPEINFO.className, pInspected )
	{
		//TODO: Drag slot, picked widget, drag state and dataset are only reachable through protected members.

		this->slot = _createRows({
			boolRow( "Drag in progress: ", [](DragNDropOverlay* o) { return o->isDragInProgress(); } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& DragNDropOverlayInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
