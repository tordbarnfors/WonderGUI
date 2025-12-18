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
#include <wg_slider.h>
#include <wg_gfxdevice.h>
#include <wg_msg.h>
#include <wg_base.h>
#include <wg_msgrouter.h>

namespace wg
{
	using namespace Util;

	const TypeInfo Slider::TYPEINFO = { "Slider", &Widget::TYPEINFO };

	//____ constructor ____________________________________________________________

	Slider::Slider() : m_handleSkin(this)
	{
	}



	//____ Destructor _____________________________________________________________

	Slider::~Slider()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Slider::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _setDefaultSlideLength() _________________________________________

	void Slider::setDefaultSlideLength(pts length)
	{
		if (length < 0)
		{
			//TODO: Error handling!
			return;
		}

		m_defaultSlideLength = length;
		_updateDefaultSize();
	}

	//____ setAxis() _________________________________________________________

	void Slider::setAxis(Axis axis)
	{
		if (axis != m_axis)
		{
			m_axis = axis;
			_requestRender();
			_updateDefaultSize();
		}
	}

	//____ setHandleSkin() ___________________________________________________

	void Slider::setHandleSkin(Skin* pSkin)
	{
		m_handleSkin.set(pSkin);

		_requestRender();
		_updateDefaultSize();
	}


	//____ setSteps() ________________________________________________________

	void Slider::setSteps(int nbSteps)
	{
		if (nbSteps < 0)
		{
			//TODO: Error handling!
			return;
		}
/*
		This causes problems if setSteps() is followed by setValue() since it
		posts a message changing value, which will be processed after setValue().
 
		if (nbSteps != m_nbSteps)
		{
			m_nbSteps = nbSteps;
			if( nbSteps > 0 )
				_setValue(m_value);		// Have the value aligned to the steps.
		}
*/
	}

	//____ setValue() _________________________________________________________

	void Slider::setValue(float value)
	{
		limit(value, 0.f, 1.f);
		_setValue(value,false);
	}

	//____ _defaultSize() __________________________________________________________

	SizeSPX Slider::_defaultSize(int scale) const
	{
		if( scale == m_scale && m_defaultSize.w >= 0 && m_defaultSize.h >= 0)
			return m_defaultSize;
		else
			return _calcDefaultSize(scale);
	}

	//____ _receive() __________________________________________________________

	void Slider::_receive(Msg * _pMsg)
	{
		switch (_pMsg->type())
		{
			case MsgType::MouseEnter:
			case MsgType::MouseMove:
			{
				auto pMsg = static_cast<InputMsg*>(_pMsg);
				CoordSPX pos = _toLocal(pMsg->pointerSpxPos());
				if( _handleGeo(m_size).contains(pos) != m_handleState.isHovered() )
				{
					_setHandleState(m_handleState + State::Hovered);
				}
				break;
			}

			case MsgType::MouseLeave:
				if (m_handleState.isHovered() && !m_handleState.isPressed() )
					_setHandleState(m_handleState - State::Hovered);
				break;

			case MsgType::MousePress:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);

				if (pMsg->button() != MouseButton::Left)
					break;

				CoordSPX pos = _toLocal(pMsg->pointerSpxPos());
				RectSPX  handle = _handleGeo(m_size);
				if (handle.contains(pos))
				{
					_setHandleState(m_handleState + State::Pressed);
					m_valueAtPress = m_value;
				}
				else
				{
					//TODO: Handle click on background.
				}
				break;
			}

			case MsgType::MouseRelease:
			{
				auto pMsg = static_cast<MousePressMsg*>(_pMsg);

				if (pMsg->button() != MouseButton::Left)
					break;

				if (m_handleState.isPressed())
				{
					_setHandleState(m_handleState - State::Pressed);
				}
				else
				{
//					Coord pos = toLocal(pMsg->pointerPos());
//					Rect  handle = _handleGeo(m_size);

					//TODO: Handle click on background.
				}
				break;

			}

			case MsgType::MouseRepeat:
				break;

			case MsgType::MouseDrag:
			{
				if (m_handleState.isPressed())
				{
					auto pMsg = static_cast<MouseDragMsg*>(_pMsg);

					SizeSPX contentSize = m_size - m_skin.contentBorderSize(m_scale);
					SizeSPX handleSize = _handleGeo(m_size).size();
					CoordSPX totalDrag = pMsg->_draggedTotal();

					spx slideLen;
					spx slided;

					if (m_axis == Axis::X)
					{
						slideLen = contentSize.w - handleSize.w;
						slided = totalDrag.x;
					}
					else
					{
						slideLen = contentSize.h - handleSize.h;
						slided = -totalDrag.y;
					}

					float newValue = m_valueAtPress + slided / (float) slideLen;
					limit(newValue, 0.f, 1.f);
					_setValue(newValue);
				}
			}
				
			default:
				break;
		}
	}

	//____ _render() __________________________________________________________

	void Slider::_render(GfxDevice * pDevice, const RectSPX& canvas, const RectSPX& window)
	{
			m_skin.render(pDevice, canvas, m_scale, m_state, m_value);

			m_handleSkin.render(pDevice, _handleGeo(canvas), m_scale, m_handleState, m_value);
	}

	//____ _alphaTest() ________________________________________________________

	bool Slider::_alphaTest(const CoordSPX& ofs)
	{
		bool bMarked = m_skin.markTest(ofs, RectSPX(m_size), m_scale, m_state, m_value);

		if (!bMarked && !m_handleSkin.isEmpty())
			bMarked = m_handleSkin.markTest(ofs, _handleGeo(m_size), m_scale, m_handleState, m_value);

		return bMarked;
	}

	//____ _updateDefaultSize() ________________________________________________

	void Slider::_updateDefaultSize(bool bRequestResize)
	{	
		SizeSPX sz = _calcDefaultSize(m_scale);
		if (sz != m_defaultSize)
		{
			m_defaultSize = sz;
			if( bRequestResize )
				_requestResize();
		}
	}

	//____ _calcDefaultSize() ________________________________________________

	SizeSPX Slider::_calcDefaultSize(int scale) const
	{
		SizeSPX sz = m_handleSkin.isEmpty() ? SizeSPX(4, 4) : m_handleSkin.defaultSize(scale);

		if (m_axis == Axis::X)
			sz.w += align(ptsToSpx(m_defaultSlideLength,scale));
		else
			sz.h += align(ptsToSpx(m_defaultSlideLength,scale));

		sz += m_skin.contentBorderSize(scale);
		sz = SizeSPX::max(sz, m_skin.defaultSize(scale));

		return sz;
	}


	//____ _resize() ____________________________________________________________

	void Slider::_resize(const SizeSPX& size, int scale)
	{
		Widget::_resize(size, scale);
		_updateDefaultSize(false);			// Size is already decided for us, we just need to update m_defaultSize.
	}


	//____ _setValue() __________________________________________________________

	void Slider::_setValue(float value, bool bPostMsg)
	{
		limit(value,0.0f,1.0f);
		
		if (m_nbSteps > 0)
		{
			int val = int(value * m_nbSteps + 0.5f);
			value = val / float(m_nbSteps);
		}

		if (value != m_value)
		{
			
			RectSPX oldGeo = _handleGeo(m_size);
			float	oldValue = m_value;
			m_value = value;
			RectSPX newGeo = _handleGeo(m_size);

			RectSPX changeRect = RectSPX::bounds(oldGeo, newGeo);
			_requestRender(changeRect);

			m_skin.valueChanged(m_value, oldValue);

			if(bPostMsg)
				Base::msgRouter()->post(ValueUpdateMsg::create(this, m_value, false));
		}
	}

	//____ _setHandleState() __________________________________________________

	void Slider::_setHandleState(State state)
	{
		State oldState = m_handleState;
		m_handleState = state;

		m_handleSkin.stateChanged(m_handleState, oldState);
	}


	//____ _handleGeo() _______________________________________________________

	RectSPX Slider::_handleGeo(const RectSPX& widgetGeo) const
	{
		RectSPX contentGeo =  m_skin.contentRect(widgetGeo, m_scale, m_state);

		RectSPX handleGeo;

		SizeSPX handlePrefSize = m_handleSkin.defaultSize(m_scale);

		if (m_axis == Axis::X)
		{
			handleGeo.w = handlePrefSize.w * (contentGeo.h / float(handlePrefSize.h));
			handleGeo.h = contentGeo.h;

			handleGeo.x = contentGeo.x + (contentGeo.w - handleGeo.w) * m_value;
			handleGeo.y = contentGeo.y;
		}
		else
		{
			handleGeo.w = contentGeo.w;
			handleGeo.h = handlePrefSize.h * (contentGeo.w / float(handlePrefSize.w));

			handleGeo.x = contentGeo.x;
			handleGeo.y = contentGeo.y + contentGeo.h - handleGeo.h - (contentGeo.h - handleGeo.h) * m_value;
		}

		return align(handleGeo);
	}

		//____ _skinState() _______________________________________________________

	State Slider::_skinState(const SkinSlot* pSlot) const
	{
		if (pSlot == &m_handleSkin)
			return m_handleState;
		else
			return m_state;
	}

	//____ _skinSize() ___________________________________________________

	SizeSPX Slider::_skinSize(const SkinSlot* pSlot) const
	{
		if (pSlot == &m_handleSkin)
			return _handleGeo(m_size).size();
		else
			return m_size;
	}

	//____ _skinRequestRender() _______________________________________________

	void Slider::_skinRequestRender(const SkinSlot* pSlot, const RectSPX& rect)
	{
		if (pSlot == &m_handleSkin)
			_requestRender(rect + _handleGeo(m_size).pos());
		else
			_requestRender(rect);
	}

	//____ _skinValue() _______________________________________________________

	float Slider::_skinValue(const SkinSlot* pSlot) const
	{
		return m_value;
	}

} // namespace wg
