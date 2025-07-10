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

#include <wg_paddingcapsule.h>

namespace wg
{
	using namespace Util;

	const TypeInfo PaddingCapsule::TYPEINFO = { "PaddingCapsule", &Capsule::TYPEINFO };

	//____ create() ______________________________________________________________

	PaddingCapsule_p PaddingCapsule::create() 
	{ 
		return PaddingCapsule_p(new PaddingCapsule()); 
 	}

	PaddingCapsule_p PaddingCapsule::create(const Blueprint& blueprint) 
	{ 
		return PaddingCapsule_p(new PaddingCapsule(blueprint)); 
	}

	//____ constructor ____________________________________________________________

	PaddingCapsule::PaddingCapsule()
	{
	}

	//____ destructor _____________________________________________________________

	PaddingCapsule::~PaddingCapsule()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PaddingCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _matchingHeight() __________________________________________________

	spx PaddingCapsule::_matchingHeight(spx width, int scale) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale) + align(ptsToSpx(m_padding, scale));

		if (slot._widget())
			return slot._widget()->_matchingHeight(width - padding.w, scale) + padding.h;
		else
			return padding.h;
	}

	//____ _matchingWidth() ___________________________________________________

	spx PaddingCapsule::_matchingWidth(spx height, int scale) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale) + align(ptsToSpx(m_padding, scale));

		if (slot._widget())
			return slot._widget()->_matchingWidth(height - padding.h, scale) + padding.w;
		else
			return padding.w;
	}

	//____ _defaultSize() _____________________________________________________

	SizeSPX PaddingCapsule::_defaultSize(int scale) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale) + align(ptsToSpx(m_padding, scale));

		if (slot._widget())
			return slot._widget()->_defaultSize(scale) + padding;
		else
			return padding;
	}

	//____ _minSize() _________________________________________________________

	SizeSPX PaddingCapsule::_minSize(int scale) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale) + align(ptsToSpx(m_padding, scale));

		if (slot._widget())
			return slot._widget()->_minSize(scale) + padding;
		else
			return padding;
	}

	//____ _maxSize() _________________________________________________________

	SizeSPX PaddingCapsule::_maxSize(int scale) const
	{
		SizeSPX padding = m_skin.contentBorderSize(scale) + align(ptsToSpx(m_padding, scale));

		if (slot._widget())
			return slot._widget()->_maxSize(scale) + padding;
		else
			return Widget::_maxSize(scale);
	}

	//____ _maskPatches() _____________________________________________________

	void PaddingCapsule::_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip)
	{
		RectSPX coverage = m_skin.coverage(geo, m_scale, m_state);

		patches.sub(RectSPX::overlap(coverage, clip));

		if (coverage.contains(_contentRect(geo)))
			return;										// No need to loop through children, skins coverage contains them all.

		if (slot._widget())
		{
			SizeSPX padding = m_skin.contentBorderSize(m_scale) + align(ptsToSpx(m_padding, m_scale));

			slot._widget()->_maskPatches(patches, geo - padding, clip);
		}
	}

	//____ _resize() __________________________________________________________

	void PaddingCapsule::_resize(const SizeSPX& size, int scale)
	{
		Container::_resize(size, scale);

		if (slot._widget())
		{
			SizeSPX sz = size - m_skin.contentBorderSize(scale) - align(ptsToSpx(m_padding, scale));
			// Make sure size isn't negative
			if (sz.w < 0)
				sz.w = 0;
			if (sz.h < 0)
				sz.h = 0;
			slot._widget()->_resize(sz, scale);
		}
	}

	//____ _slotGeo() _________________________________________________________

	RectSPX PaddingCapsule::_slotGeo(const StaticSlot* pSlot) const
	{
		return m_skin.contentRect(m_size, m_scale, m_state) - align(ptsToSpx(m_padding, m_scale));
	}

	//____ _firstSlotWithGeo() ________________________________________________

	void PaddingCapsule::_firstSlotWithGeo(SlotWithGeo& package) const
	{
		if (slot.isEmpty())
		{
			package.pSlot = nullptr;
		}
		else
		{
			package.pSlot = &slot;
			package.geo = m_skin.contentRect(RectSPX(0, 0, m_size), m_scale, m_state) - align(ptsToSpx(m_padding, m_scale));
		}
	}

} // namespace wg
