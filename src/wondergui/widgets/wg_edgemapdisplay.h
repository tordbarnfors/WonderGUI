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
#ifndef WG_EDGEMAPDISPLAY_DOT_H
#define WG_EDGEMAPDISPLAY_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_gfxdevice.h>

namespace wg
{

	class EdgemapDisplay;
	typedef	StrongPtr<EdgemapDisplay>	EdgemapDisplay_p;
	typedef	WeakPtr<EdgemapDisplay>		EdgemapDisplay_wp;





	//____ EdgemapDisplay ____________________________________________________________
	/**
	 **/

	class EdgemapDisplay : public Widget
	{

	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			Edgemap_p		edgemap;
			Placement		edgemapPlacement = Placement::Center;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Creation ______________________________________________________

		static EdgemapDisplay_p	create() { return EdgemapDisplay_p(new EdgemapDisplay()); }
		static EdgemapDisplay_p	create(const Blueprint& blueprint ) { return EdgemapDisplay_p(new EdgemapDisplay(blueprint)); }

		//.____ Identification ________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance ____________________________________________________

		void		setEdgemap(Edgemap * pEdgemap);
		Edgemap_p	edgemap() const { return m_pEdgemap; }

		void		setEdgemapPlacement(Placement placement);
		Placement	edgemapPlacement() const { return m_placement;  }

		//		void		setGrid(int x, int y, HiColor color, float thickness);


		//.____ Internal ____________________________________________

		SizeSPX	_defaultSize(int scale) const override;
		spx		_matchingHeight(spx width, int scale) const override;
		spx		_matchingWidth(spx height, int scale) const override;

	protected:
		EdgemapDisplay() {};
		template<class BP> EdgemapDisplay(const BP& bp) : Widget(bp)
		{
			m_placement = bp.edgemapPlacement;

			if (bp.edgemap)
				setEdgemap(bp.edgemap);
		}
		
		virtual ~EdgemapDisplay();

		RectSPX			_edgemapDisplayRect(int scale) const;

		void			_resize(const SizeSPX& size, int scale) override;
		void			_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		bool			_alphaTest(const CoordSPX& ofs) override;

		Edgemap_p		m_pEdgemap;
		Placement		m_placement = Placement::Center;
	};



} // namespace wg
#endif //WG_EDGEMAPDISPLAY_DOT_H
