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

#include <wg_labelcapsule.h>
#include <wg_gfxdevice.h>

namespace wg
{

	const TypeInfo	LabelCapsule::TYPEINFO = { "LabelCapsule", &Capsule::TYPEINFO };

	//____ create() ___________________________________________________________

	LabelCapsule_p LabelCapsule::create()
	{
		return LabelCapsule_p(new LabelCapsule());
	}

	LabelCapsule_p LabelCapsule::create(const Blueprint& blueprint)
	{
		return LabelCapsule_p(new LabelCapsule(blueprint));
	}


	//____ typeInfo() _________________________________________________________

	const TypeInfo& LabelCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setLabelPlacement() ________________________________________________

	void LabelCapsule::setLabelPlacement(Placement placement)
	{
		if (placement != m_labelPlacement)
		{
			m_labelPlacement = placement;
			_requestRender();
		}
	}

	//____ setLabelSkin() _____________________________________________________

	void LabelCapsule::setLabelSkin(Skin* pSkin)
	{
		if( pSkin != m_labelSkin.get() )
		{ 
			m_labelSkin.set(pSkin);
			_requestRender();
			_requestResize();
		}
	}


	//____ _matchingWidth() ___________________________________________________

	spx LabelCapsule::_matchingWidth(spx height, int scale) const
	{
		spx mainWidth = Capsule::_matchingWidth(height, scale);
		spx labelWidth = label._defaultSize(scale).w + m_labelSkin.contentBorderSize(scale).w;

		return std::max(mainWidth, labelWidth);
	}

	//____ _matchingHeight() __________________________________________________

	spx LabelCapsule::_matchingHeight(spx width, int scale) const
	{
		spx mainHeight = Capsule::_matchingHeight(width,scale);
		spx labelHeight = label._defaultSize(scale).h + m_labelSkin.contentBorderSize(scale).h;

		return std::max(mainHeight, labelHeight);
	}

	//____ _defaultSize() _____________________________________________________

	SizeSPX LabelCapsule::_defaultSize(int scale) const
	{
		SizeSPX mainSize = Capsule::_defaultSize(scale);
		SizeSPX labelSize = label._defaultSize(scale) + m_labelSkin.contentBorderSize(scale);

		return SizeSPX::max(mainSize, labelSize);
	}

	//____ _render() __________________________________________________________

	void LabelCapsule::_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window)
	{
		Capsule::_render(pDevice, _canvas, _window);

		RectSPX labelGeo = _labelGeo(_canvas, m_scale);

		m_labelSkin.render(pDevice, labelGeo, m_scale, m_state);

		RectSPX labelContentGeo = m_labelSkin.contentRect(labelGeo, m_scale, m_state);
		label._render(pDevice, labelContentGeo);
	}

	//____ _alphaTest() _______________________________________________________

	bool LabelCapsule::_alphaTest(const CoordSPX& ofs)
	{
		if (m_skin.markTest(ofs, RectSPX(m_size), m_scale, m_state))
			return true;

		return m_labelSkin.markTest(ofs, _labelGeo(m_size,m_scale), m_scale, m_state);
	}

	//____ _resize() __________________________________________________________

	void LabelCapsule::_resize(const SizeSPX& size, int scale)
	{
		Capsule::_resize(size, scale);
		label._setSize(_labelGeo(size,scale).size(), scale);
	}

	//____ _setState() ________________________________________________________

	void LabelCapsule::_setState(State state)
	{
		Capsule::_setState(state);
		label._setState(state);
	}

	//____ _componentPos() ____________________________________________________

	CoordSPX LabelCapsule::_componentPos(const Component* pComponent) const
	{
		return _labelGeo(m_size, m_scale) - m_labelSkin.contentBorder(m_scale, m_state);
	}

	//____ _componentSize() ___________________________________________________

	SizeSPX LabelCapsule::_componentSize(const Component* pComponent) const
	{
		return _labelGeo(m_size, m_scale) - m_labelSkin.contentBorder(m_scale, m_state);
	}

	//____ _componentGeo() ____________________________________________________

	RectSPX LabelCapsule::_componentGeo(const Component* pComponent) const
	{
		return _labelGeo(m_size, m_scale) - m_labelSkin.contentBorder(m_scale,m_state);
	}

	//____ _skinSize() ________________________________________________________

	SizeSPX LabelCapsule::_skinSize(const SkinSlot* pSlot) const
	{
		if (pSlot == &m_labelSkin)
			return _labelGeo(m_size, m_scale).size();
		else
			return m_size;
	}

	//____ _skinRequestRender() _______________________________________________

	void LabelCapsule::_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect)
	{
		if (pSlot == &m_labelSkin)
			_requestRender(rect + _labelGeo(m_size, m_scale).pos());
		else
			_requestRender(rect);
	}


	//____ _labelGeo() ________________________________________________________

	RectSPX LabelCapsule::_labelGeo(RectSPX widgetRect, int scale) const
	{
		SizeSPX sz = label._defaultSize(m_scale) + m_labelSkin.contentBorderSize(m_scale);;
		SizeSPX labelSize = SizeSPX::min(widgetRect.size(), sz);

		return Util::placementToRect(m_labelPlacement, widgetRect, labelSize);
	}



} // namespace wg
