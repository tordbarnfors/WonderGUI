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
#ifndef WG_RANGESLIDER_DOT_H
#define WG_RANGESLIDER_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_skin.h>

namespace wg
{

	class RangeSlider;
	typedef	StrongPtr<RangeSlider>	RangeSlider_p;
	typedef	WeakPtr<RangeSlider>		RangeSlider_wp;

	//____ RangeSlider ____________________________________________________________
	/**
	 * @brief A simple slider widget.
	 *
	 *
	 **/


	class RangeSlider : public Widget
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Axis			axis = Axis::X;
			Object_p		baggage;
			Skin_p			beginHandle;
			bool			disabled = false;
			bool			dropTarget = false;
			Skin_p			endHandle;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			pts				defaultSlideLength = 64;
			float			rangeBegin = 0.f;
			float			rangeEnd = 1.f;
            float		    minRange = 0.0f;
			bool			selectable = false;
			Skin_p			skin;
			int				steps = 0;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
		};

		//.____ Creation __________________________________________

		static RangeSlider_p	create() { return RangeSlider_p(new RangeSlider()); }
		static RangeSlider_p	create(const Blueprint& bp) { return RangeSlider_p(new RangeSlider(bp)); }

		//.____ Identification __________________________________________

		const TypeInfo&	typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________

		void			setDefaultSlideLength(pts length);
		inline pts		defaultSlideLength() const { return m_defaultSlideLength; }

		//.____ Appearance _____________________________________________

		void			setAxis(Axis axis);
		Axis			axis() const { return m_axis; }

		void			setBeginHandleSkin(Skin* pSkin);
		inline Skin_p	beginHandleSkin() const { return m_beginHandleSkin.get();  }

		void			setEndHandleSkin(Skin* pSkin);
		inline Skin_p	endHandleSkin() const { return m_endHandleSkin.get(); }

		//.____ Control _______________________________________________________

		void			setSteps(int nbSteps );
		inline int		steps() const { return m_nbSteps; }

        void            setRangeDragable(bool isDragable) { m_rangeDragable = isDragable; }
        bool            isRangeDragable() const { return m_rangeDragable; }

		//.____ Content _______________________________________________________

		void			setRange(float begin, float end);

		inline float	rangeBegin() const { return m_rangeBegin; }
		inline float	rangeEnd() const { return m_rangeEnd; }

		//.____ Internal _______________________________________________________

		SizeSPX			_defaultSize(int scale) const override;


	protected:
		RangeSlider();

		template<class BP> RangeSlider(const BP& bp) : m_beginHandleSkin(this), m_endHandleSkin(this), Widget(bp)
		{
			m_axis = bp.axis;
			m_beginHandleSkin.set(bp.beginHandle);
			m_endHandleSkin.set(bp.endHandle);

			m_defaultSlideLength = bp.defaultSlideLength;
			m_nbSteps = bp.steps;
            m_minRange = bp.minRange;

			_updateDefaultSize();
			_setRange(bp.rangeBegin,bp.rangeEnd);
		}

		virtual ~RangeSlider();

		void 		_receive(Msg* pMsg) override;
		void		_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		bool		_alphaTest(const CoordSPX& ofs) override;

		void		_updateDefaultSize();
		SizeSPX		_calcDefaultSize(int scale) const;

		void		_setRange(float begin, float end, bool bPostMsg = true);
        void        _handlesPressed(CoordSPX pos, bool& beginHandlePressed, bool& endHandlePressed);
		void		_setHandleState(State state, bool isBeginHandle);
		RectSPX		_handleGeo(const RectSPX& widgetGeo, bool isbBeginHandle) const;

		State		_skinState(const SkinSlot* pSlot) const override;
		SizeSPX		_skinSize(const SkinSlot * pSlot) const override;

		void		_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect) override;

		float		_skinValue(const SkinSlot* pSlot) const override;
		float		_skinValue2(const SkinSlot* pSlot) const override;


	private:
		SkinSlot	m_beginHandleSkin;
		SkinSlot	m_endHandleSkin;

		SizeSPX		m_defaultSize;
		pts			m_defaultSlideLength = 64;
		float		m_rangeBegin = 0.f;
		float		m_rangeEnd = 1.f;
		float		m_minRange = 0.f;
		float		m_valueAtPress = 0.f;
        float       m_endValueAtPress = 0.f;
		int			m_nbSteps = 0;
		Axis		m_axis = Axis::X;
		State		m_beginHandleState;
		State		m_endHandleState;
        bool        m_rangeDragable = false;
        bool        m_dragMiddle = false;
	};



} // namespace wg
#endif //WG_RANGESLIDER_DOT_H
