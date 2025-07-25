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

#ifndef	WG_ICON_DOT_H
#define	WG_ICON_DOT_H
#pragma once

#include <wg_types.h>
#include <wg_component.h>
#include <wg_geo.h>
#include <wg_skin.h>
#include <wg_util.h>

namespace wg
{

	//____ Icon _____________________________________________________________

	class Icon : public Component, private SkinSlot::Holder
	{
	public:
		Icon( Widget * pWidget );
		virtual ~Icon() {};


		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Skin_p			skin;
			Placement		placement = Placement::West;
			pts				spacing;
		};

		//.____ Content _____________________________________________

		bool			set(Skin * pIconGfx, Placement placement = Placement::West, pts spacing = 0,
							bool _bOverlap = false);
		void			clear();

		inline bool		isEmpty() const { return m_skin.isEmpty(); }

		//.____ Appearance _____________________________________________

		inline void			setPlacement(Placement placement);
		inline void			setSpacing(pts spacing);
		inline void			setSkin(Skin* pSkin);
		inline Placement	placement() const { return m_placement; }
		inline pts			spacing() const { return m_spacing; }
		inline Skin_p		skin() const { return m_skin.get(); }

		//.____ Internal ______________________________________________________

		void			_initFromBlueprint(const Blueprint& blueprint);

	protected:

//		spx				_spacing(int scale) const { return Util::align(Util::ptsToSpx(m_spacing,scale)); }

		RectSPX			_getIconRect( const RectSPX& contentRect, int scale ) const;
		RectSPX			_getIconRect( const RectSPX& contentRect, const SizeSPX& iconSize, int scale ) const;
		RectSPX			_getTextRect( const RectSPX& contentRect, const RectSPX& iconRect, int scale ) const;
		SizeSPX			_defaultSize(int scale) const;
		SizeSPX			_defaultSize(int scale, SizeSPX& textSize) const;

		SizeSPX			_textPaddingSize(int scale) const;

		float	_skinValue(const SkinSlot* pSlot) const override;
		float	_skinValue2(const SkinSlot* pSlot) const override;
		State	_skinState(const SkinSlot* pSlot) const override;

		SizeSPX	_skinSize(const SkinSlot* pSlot) const override;

		void	_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect) override;

		int		_scale() const { return Component::_scale(); }


	private:
		Placement		m_placement;
		pts				m_spacing;
		SkinSlot		m_skin;
	};


} // namespace wg
#endif	// WG_ICON_DOT_H
