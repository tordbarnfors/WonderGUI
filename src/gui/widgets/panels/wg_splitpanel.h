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

#ifndef	WG_SPLITPANEL_DOT_H
#define	WG_SPLITPANEL_DOT_H
#pragma once



#include <wg_panel.h>
#include <wg_slot.h>
#include <wg_slotarray.h>


namespace wg
{

	//____ SplitPanel ___________________________________________________________

	class SplitPanel;
	typedef	StrongPtr<SplitPanel>	SplitPanel_p;
	typedef	WeakPtr<SplitPanel>	SplitPanel_wp;

	class SplitPanel : public Panel
	{

	public:

		//____ Slot _________________________________________________________

		class Slot : public DynamicSlot
		{
			friend class SplitPanel;
			template<class S, int X> friend class SlotArray;

		public:

			//.____ Identification ________________________________________________

			const static TypeInfo	TYPEINFO;

			//.____ Geometry _________________________________________________

			inline Coord	pos() const { return Util::spxToPts(m_geo.pos(),_holder()->_scale()); }
			inline Size		size() const { return Util::spxToPts(m_geo.size(), _holder()->_scale()); }
			inline Rect		geo() const { return Util::spxToPts(m_geo, _holder()->_scale());
			}

			//.____ Operators __________________________________________

			inline void operator=(Widget * pWidget) { setWidget(pWidget); }

		protected:
			Slot() : DynamicSlot(nullptr) {}

			RectSPX	m_geo;
		};

		//.____ Creation __________________________________________

		static SplitPanel_p	create() { return SplitPanel_p(new SplitPanel()); }

		//.____ Components _______________________________________

		SlotArray<Slot, 2>		slots;

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________

		void			setAxis(Axis orientaiton);
		Axis			axis() const { return m_bHorizontal ? Axis::X : Axis::Y; }

		//.____ Appearance _________________________________________________

		void			setHandleSkin(Skin* pSkin);
		Skin_p			handleSkin() const { return m_handleSkin.get(); }

		void			setHandleThickness(pts thickness);
		pts				handleThickness() const { return m_handleThickness;  }

		//.____ Behavior _______________________________________________________

		void			setResizeRatio( float resizeRatioFirstWidget );
		float			resizeRatio() const { return m_resizeRatio; }

		void			setResizeFunction(std::function<pts(Widget * pFirst, Widget * pSecond, pts totalLength, pts handleMovement)> func);
		std::function<pts(Widget * pFirst, Widget * pSecond, pts totalLength, pts handleMovement)> resizeFunction() const { return m_resizeFunc;  }

		//.____ Control ________________________________________________________

		void			setSplit(float fraction);
		float			split() const;

		//.____ Internal _______________________________________________________

		SizeSPX			_defaultSize(int scale = -1) const override;


	protected:
		SplitPanel();
		virtual ~SplitPanel();

		spx			_handleThickness(int scale) const;					// Takes both m_handleThickness and m_pHandleSkin into account.
		void		_updateDefaultSize();
		SizeSPX		_calcDefaultSize(int scale) const;
		bool		_updateGeo(spx handleMovement=0);
		spx			_defaultResizeFunc(Widget * pFirst, Widget * pSecond, spx totalLength, spx handleMovement);
		spx 		_limitFirstSlotLength( spx firstLength, spx totalLength );

		
		// Overloaded from Widget

		void		_resize(const SizeSPX& size, int scale = -1) override;
		void		_setState(State state) override;
		void		_receive(Msg * pMsg) override;

		void		_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;

		void		_collectPatches(PatchesSPX& container, const RectSPX& geo, const RectSPX& clip) override;
		void		_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip, BlendMode blendMode) override;

		bool		_alphaTest(const CoordSPX& ofs) override;

		// Overloaded from Container

		const TypeInfo&	_slotTypeInfo(const StaticSlot * pSlot) const override;

		Widget *	_firstChild() const override;
		Widget *	_lastChild() const override;

		void		_firstSlotWithGeo(SlotWithGeo& package) const override;
		void		_nextSlotWithGeo(SlotWithGeo& package) const override;

		CoordSPX	_childPos( const StaticSlot * pSlot) const override;

		void		_childRequestRender(StaticSlot * pSlot) override;
		void		_childRequestRender(StaticSlot * pSlot, const RectSPX& rect) override;
		void		_childRequestResize(StaticSlot * pSlot) override;

		Widget *	_prevChild(const StaticSlot * pSlot) const override;
		Widget *	_nextChild(const StaticSlot * pSlot) const override;

		void		_releaseChild(StaticSlot * pSlot) override;
		void		_replaceChild(StaticSlot * pSlot, Widget * pNewChild) override;

		//

		State			_skinState(const SkinSlot* pSlot) const override;
		SizeSPX			_skinSize(const SkinSlot* pSlot) const override;

		void			_skinRequestRender(const SkinSlot* pSlot) override;
		void			_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect) override;


		//

		SkinSlot		m_handleSkin;
		bool			m_bHorizontal;
		SizeSPX			m_defaultSize;
		float			m_resizeRatio;

		pts				m_handleThickness;			// Set to 0 to use default from handleSkin.
		RectSPX			m_handleGeo;
		State			m_handleState;
		spx				m_handlePressOfs;

		std::function<pts(Widget * pFirst, Widget * pSecond, pts totalLength, pts handleMovement)>	m_resizeFunc;
	};

}

#endif //WG_SPLITPANEL_DOT_H
