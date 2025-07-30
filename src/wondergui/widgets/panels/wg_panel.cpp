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

#include <wg_panel.h>

namespace wg
{
	const TypeInfo Panel::TYPEINFO = { "Panel", &Container::TYPEINFO };

	const TypeInfo	PanelSlot::TYPEINFO = { "PanelSlot", &DynamicSlot::TYPEINFO };

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Panel::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setMaskOp() _____________________________________________________________

	void Panel::setMaskOp(MaskOp operation)
	{
		if (operation != m_maskOp)
		{
			m_maskOp = operation;
			_requestRender();
		}
	}

	//____ _maskPatches() __________________________________________________________

	void Panel::_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip)
	{
		RectSPX coverage = m_skin.coverage(geo, m_scale, m_state);

		patches.sub(RectSPX::overlap(coverage, clip));

		if (coverage.contains(_contentRect(geo)))
			return;										// No need to loop through children, skins coverage contains them all.


		switch (m_maskOp)
		{
		case MaskOp::Recurse:
		{
			RectSPX	myClip = RectSPX::overlap(geo, clip);				// Need to limit clip to our geo. Otherwise children outside might mask what they shouldn't (for panels where children can go outside parent).
			SlotWithGeo child;
			_firstSlotWithGeo(child);

			while (child.pSlot)
			{
				child.pSlot->_widget()->_maskPatches(patches, child.geo + geo.pos(), myClip);
				_nextSlotWithGeo(child);
			}
			break;
		}
		case MaskOp::Skip:
			break;
		case MaskOp::Mask:
			patches.sub(RectSPX::overlap(geo, clip));
			break;
		}
	}

	//____ _slotGeo() _______________________________________________________

	RectSPX Panel::_slotGeo(const StaticSlot* pSlot) const
	{
		return ((PanelSlot*)pSlot)->m_geo;
	}

	//____ _childRequestRender() ______________________________________________

	void Panel::_childRequestRender(StaticSlot* pSlot, const RectSPX& rect)
	{
		_requestRender(rect + ((PanelSlot*)pSlot)->m_geo.pos());
	}



} // namespace wg

