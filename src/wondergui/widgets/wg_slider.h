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
#ifndef WG_SLIDER_DOT_H
#define WG_SLIDER_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_skin.h>

namespace wg
{

	class Slider;
	typedef	StrongPtr<Slider>	Slider_p;
	typedef	WeakPtr<Slider>		Slider_wp;

	//____ Slider ____________________________________________________________
	/**
	 * @brief A simple slider widget.
	 *
	 *
	 **/


	class Slider : public Widget
	{
	public:
		
		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Axis			axis = Axis::X;
			Object_p		baggage;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			Skin_p			handle;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			bool			pickHandle = false;
			uint8_t			pickCategory = 0;
			PointerStyle	pointer = PointerStyle::Undefined;
			pts				defaultSlideLength = 64;
			bool			selectable = false;
			Skin_p			skin;
			int				steps = 0;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
			float			value = 0.f;
		};

		//.____ Creation __________________________________________

		static Slider_p	create() { return Slider_p(new Slider()); }
		static Slider_p	create(const Blueprint& bp ) { return Slider_p(new Slider(bp)); }

		//.____ Identification __________________________________________

		const TypeInfo&	typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ____________________________________________


		void			setDefaultSlideLength(pts length);
		inline pts		defaultSlideLength() const { return m_defaultSlideLength; }

		//.____ Appearance _____________________________________________

		void			setAxis(Axis axis);
		Axis			axis() const { return m_axis; }

		void			setHandleSkin(Skin* pSkin);
		inline Skin_p	handleSkin() const { return m_handleSkin.get(); }

		//.____ Control _______________________________________________________

		void			setSteps(int nbSteps );
		inline int		steps() const { return m_nbSteps; }

		//.____ Content _______________________________________________________

		void			setValue(float value);
		inline float	value() const { return m_value;	}

		//.____ Internal ______________________________________________________

		SizeSPX			_defaultSize(int scale) const override;


	protected:
		Slider();
		
		template<class BP> Slider(const BP& bp) : m_handleSkin(this), Widget(bp)
		{
			m_axis = bp.axis;
			m_handleSkin.set(bp.handle);

			m_defaultSlideLength = bp.defaultSlideLength;
			m_nbSteps = bp.steps;

			_updateDefaultSize();
			_setValue(bp.value, false);
		}

		virtual ~Slider();
		void 			_receive(Msg* pMsg) override;

		void			_render(GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		bool			_alphaTest(const CoordSPX& ofs) override;
        void        	_resize(const SizeSPX& size, int scale) override;

		SizeSPX			_calcDefaultSize(int scale) const;
		void			_updateDefaultSize(bool bRequestResize = true);

		virtual void	_setValue(float value, bool bPostMsg = true);
		void			_setHandleState(State state);
		virtual RectSPX	_handleGeo(const RectSPX& widgetGeo) const;

		float			_skinValue(const SkinSlot* pSlot) const override;

		State			_skinState(const SkinSlot* pSlot) const override;
		SizeSPX			_skinSize(const SkinSlot* pSlot) const override;
		void			_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect) override;

		
		SkinSlot	m_handleSkin;
		SizeSPX		m_defaultSize;
		pts			m_defaultSlideLength = 64;
		float		m_value = 0.f;
		float		m_valueAtPress = 0.f;
		int			m_nbSteps = 0;
		Axis		m_axis = Axis::X;
		State		m_handleState;
	};



} // namespace wg
#endif //WG_SLIDER_DOT_H
