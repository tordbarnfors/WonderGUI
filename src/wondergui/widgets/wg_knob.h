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
#ifndef WG_KNOB_DOT_H
#define WG_KNOB_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_skin.h>

namespace wg
{

	class Knob;
	typedef	StrongPtr<Knob>	Knob_p;
	typedef	WeakPtr<Knob>		Knob_wp;

	//____ Knob ____________________________________________________________
	/**
	 * @brief A simple knob widget.
	 *
	 *
	 **/


	class Knob : public Widget
	{
	public:

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			bool			disabled	= false;
			Axis			dragAxis = Axis::Y;
			pts				dragRange = 200;
			bool			dropTarget	= false;
			Finalizer_p		finalizer	= nullptr;
			int				id			= 0;
			bool			lockMouse	= true;
			MarkPolicy		markPolicy	= MarkPolicy::AlphaTest;
			bool			pickable	= false;
			uint8_t			pickCategory= 0;
			bool			pickHandle = false;
			PointerStyle	pointer		= PointerStyle::Undefined;
			bool			selectable	= false;
			Skin_p			skin;
			int				steps = 0;
			bool			stickyFocus = false;
			bool			tabLock		= false;
			String			tooltip;
			float			value = 0.f;
			float			wheelStepSize = 0.01f;
		};

		//.____ Creation __________________________________________

		static Knob_p	create() { return Knob_p(new Knob()); }
		static Knob_p	create( const Blueprint& blueprint ) { return Knob_p(new Knob(blueprint)); }

		//.____ Identification __________________________________________

		const TypeInfo&	typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control _______________________________________________________

		void			setSteps(int nbSteps );
		inline int		steps() const { return m_nbSteps; }

		void			setDragAxis(Axis axis);
		Axis			dragAxis() const { return m_dragAxis; }

		void			setDragRange(pts length);
		pts				dragRange() const { return m_dragRange; }

		void			setWheelStepSize(float stepSize);
		float			wheelStepSize() const { return m_wheelStepSize; }

		void			setLockMouse(bool bLock);
		bool			isLockingMouse() const { return m_bLockMouse;  }

		//.____ Content _______________________________________________________

		void			setValue(float value);
		inline float	value() const { return m_value;	}

		//.____ Internal ______________________________________________________

		bool		_alphaTest(const CoordSPX& ofs) override;

		
	protected:
		Knob() {};
		
		template<class BP> Knob(const BP& bp) : Widget(bp)
		{
			m_nbSteps = bp.steps;
			m_dragAxis = bp.dragAxis;
			m_dragRange = bp.dragRange;
			m_value		= bp.value;
			m_wheelStepSize = bp.wheelStepSize;
			m_bLockMouse = bp.lockMouse;
		}
		
		virtual ~Knob() {};

		void		_receive(Msg* pMsg) override;
		void 		_render(GfxDevice * pDevice, const RectSPX& canvas, const RectSPX& window) override;
		void		_setValue(float value, bool bPostMsg = true);

		float		_skinValue(const SkinSlot* pSlot) const override;


	private:
		pts			m_dragRange = 200;
		pts			m_dragged = 0;
		float		m_value = 0.f;
		float		m_valueAtPress = 0.f;
		float		m_wheelStepSize = 0.01f;
		int			m_nbSteps = 0;
		Axis		m_dragAxis = Axis::Y;
		bool		m_bLockMouse = true;
	};



} // namespace wg
#endif //WG_KNOB_DOT_H
