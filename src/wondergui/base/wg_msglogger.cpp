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

#include <cstdio>

#include <wg_msglogger.h>
#include <wg_widget.h>

namespace wg
{

	using std::string;
	using std::snprintf;

	const TypeInfo MsgLogger::TYPEINFO = { "MsgLogger", &Receiver::TYPEINFO };

	//____ constructor ____________________________________________________________

	MsgLogger::MsgLogger( std::ostream& stream ) : m_pStream(&stream)
	{
		logAllMsgs();
	}

	MsgLogger::MsgLogger( const std::function<void(const char*)>& func ) : m_func(func)
	{
		logAllMsgs();
	}

	//____ Destructor _____________________________________________________________

	MsgLogger::~MsgLogger()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& MsgLogger::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ IgnoreMsg ____________________________________________________________

	void MsgLogger::ignoreMsg( MsgType type )
	{
		m_msgFilter[(int)type] = false;
	}

	//____ LogMsg _______________________________________________________________

	void MsgLogger::logMsg( MsgType type )
	{
		m_msgFilter[(int)type] = true;
	}

	//____ IgnorePointerMsgs ____________________________________________________________

	void MsgLogger::ignorePointerMsgs()
	{
		m_msgFilter[(int)MsgType::MouseEnter] = false;
		m_msgFilter[(int)MsgType::MouseMove] = false;
		m_msgFilter[(int)MsgType::MouseLeave] = false;
	}

	//____ LogPointerMsgs _______________________________________________________________

	void MsgLogger::logPointerMsgs()
	{
		m_msgFilter[(int)MsgType::MouseEnter] = true;
		m_msgFilter[(int)MsgType::MouseMove] = true;
		m_msgFilter[(int)MsgType::MouseLeave] = true;
	}


	//____ IgnoreMouseButtonMsgs ____________________________________________________________

	void MsgLogger::ignoreMouseButtonMsgs()
	{
		m_msgFilter[(int)MsgType::MousePress] = false;
		m_msgFilter[(int)MsgType::MouseRepeat] = false;
		m_msgFilter[(int)MsgType::MouseDrag] = false;
		m_msgFilter[(int)MsgType::MouseRelease] = false;
		m_msgFilter[(int)MsgType::MouseClick] = false;
		m_msgFilter[(int)MsgType::MouseDoubleClick] = false;
	}

	//____ LogMouseButtonMsgs _______________________________________________________________

	void MsgLogger::logMouseButtonMsgs()
	{
		m_msgFilter[(int)MsgType::MousePress] = true;
		m_msgFilter[(int)MsgType::MouseRepeat] = true;
		m_msgFilter[(int)MsgType::MouseDrag] = true;
		m_msgFilter[(int)MsgType::MouseRelease] = true;
		m_msgFilter[(int)MsgType::MouseClick] = true;
		m_msgFilter[(int)MsgType::MouseDoubleClick] = true;
	}

	//____ IgnoreKeyboardMsgs ____________________________________________________________

	void MsgLogger::ignoreKeyboardMsgs()
	{
		m_msgFilter[(int)MsgType::KeyPress] = false;
		m_msgFilter[(int)MsgType::KeyRepeat] = false;
		m_msgFilter[(int)MsgType::KeyRelease] = false;
		m_msgFilter[(int)MsgType::TextInput] = false;
	}

	//____ LogKeyboardMsgs _______________________________________________________________

	void MsgLogger::logKeyboardMsgs()
	{
		m_msgFilter[(int)MsgType::KeyPress] = true;
		m_msgFilter[(int)MsgType::KeyRepeat] = true;
		m_msgFilter[(int)MsgType::KeyRelease] = true;
		m_msgFilter[(int)MsgType::TextInput] = true;
	}

	//____ logMouseMsgs() _______________________________________________________

	void MsgLogger::logMouseMsgs()
	{
		logPointerMsgs();
		logMouseButtonMsgs();
		logMsg( MsgType::WheelRoll );
	}

	//____ ignoreMouseMsgs() ____________________________________________________

	void MsgLogger::ignoreMouseMsgs()
	{
		ignorePointerMsgs();
		ignoreMouseButtonMsgs();
		ignoreMsg( MsgType::WheelRoll );
	}


	//____ logInputMsgs() _______________________________________________________

	void MsgLogger::logInputMsgs()
	{
		logMouseMsgs();
		logKeyboardMsgs();
	}

	//____ ignoreInputMsgs() ____________________________________________________

	void MsgLogger::ignoreInputMsgs()
	{
		ignoreMouseMsgs();
		ignoreKeyboardMsgs();
	}




	//____ IgnoreAllMsgs ________________________________________________________

	void MsgLogger::ignoreAllMsgs()
	{
		for( int i = 0 ; i < (int) MsgType_size ; i++ )
			m_msgFilter[i] = false;
	}

	//____ LogAllMsgs ___________________________________________________________
	void MsgLogger::logAllMsgs()
	{
		for( int i = 0 ; i < (int) MsgType_size ; i++ )
			m_msgFilter[i] = true;
	}



	//____ receive() ______________________________________________________________

	void MsgLogger::receive( Msg * _pMsg )
	{
		if( m_msgFilter[(int)_pMsg->type()] == false )
			return;

		string	source;
		string	copyTo;

		const static int c_paramLen = 256;
		char	params[c_paramLen]; params[0] = 0;			// Msg specific parameters


		switch( _pMsg->type() )
		{
			case MsgType::Dummy:
				break;
			case MsgType::PointerChange:
				snprintf( params, c_paramLen, " style=%s", _formatPointerStyle( static_cast<PointerChangeMsg*>(_pMsg)).c_str() );
				break;
			case MsgType::MouseEnter:
				break;
			case MsgType::MouseMove:
				break;
			case MsgType::MouseLeave:
				break;
			case MsgType::MousePress:
				snprintf( params, c_paramLen, " button=%s", _formatMouseButton( static_cast<MousePressMsg*>(_pMsg)->button()).c_str() );
				break;
			case MsgType::MouseRepeat:
				snprintf( params, c_paramLen, " button=%s", _formatMouseButton( static_cast<MouseRepeatMsg*>(_pMsg)->button()).c_str() );
				break;
			case MsgType::MouseDrag:
			{
				auto pMsg = static_cast<MouseDragMsg*>(_pMsg);

				CoordF	now		= pMsg->currPos();
				CoordF 	prev	= pMsg->prevPos();
				CoordF	start	= pMsg->startPos();

				CoordF	dragNow	= pMsg->draggedNow();
				CoordF	dragTotal=pMsg->draggedTotal();

				snprintf( params, c_paramLen, " button=%s position(start=%.2f,%.2f prev=%.2f,%.2f now=%.2f,%.2f) dragged(now=%.2f,%.2f total=%.2f,%.2f)",
						_formatMouseButton(pMsg->button()).c_str(), start.x, start.y, prev.x, prev.y, now.x, now.y, dragNow.x, dragNow.y, dragTotal.x, dragTotal.y );
				break;
			}
			case MsgType::MouseRelease:
			{
				auto pMsg = static_cast<MouseReleaseMsg*>(_pMsg);

				const static char outside[] = "outside";
				const static char inside[] = "inside";
//				const char * pPress = outside;
				const char * pRelease = outside;

				if( pMsg->releaseInside() )
					pRelease = inside;

				snprintf( params, c_paramLen, " button=%s release=%s", _formatMouseButton(pMsg->button()).c_str(), pRelease );
				break;
			}
			case MsgType::MouseClick:
			{
				auto pMsg = static_cast<MouseClickMsg*>(_pMsg);

				snprintf( params, c_paramLen, " button=%s duration=%d ordinal=%d", _formatMouseButton(pMsg->button()).c_str(), pMsg->duration(), pMsg->ordinal() );
				break;
			}
			case MsgType::MouseDoubleClick:
				snprintf( params, c_paramLen, " button=%s", _formatMouseButton(static_cast<MouseDoubleClickMsg*>(_pMsg)->button()).c_str() );
				break;

			case MsgType::KeyPress:
			{
				auto pMsg = static_cast<KeyPressMsg*>(_pMsg);
				snprintf( params, c_paramLen, " wg_keycode=%d native_keycode=%d", (int) pMsg->translatedKeyCode(), pMsg->nativeKeyCode() );
				break;
			}
			case MsgType::KeyRepeat:
			{
				auto pMsg = static_cast<KeyRepeatMsg*>(_pMsg);
				snprintf( params, c_paramLen, " wg_keycode=%d native_keycode=%d", (int) pMsg->translatedKeyCode(), pMsg->nativeKeyCode() );
				break;
			}
			case MsgType::KeyRelease:
			{
				auto pMsg = static_cast<KeyReleaseMsg*>(_pMsg);
				snprintf( params, c_paramLen, " wg_keycode=%d native_keycode=%d", (int) pMsg->translatedKeyCode(), pMsg->nativeKeyCode() );
				break;
			}
			case MsgType::TextInput:
			{
//				snprintf(params, c_paramLen, " content=%s", TextInputMsg_p>(_pMsg)->text().chars());
				break;
			}
			case MsgType::EditCommand:
				snprintf( params, c_paramLen, " cmd=%s", _formatEditCommand(static_cast<EditCommandMsg*>(_pMsg)->command()).c_str() );
				break;

			case MsgType::WheelRoll:
			{
				auto pMsg = static_cast<WheelRollMsg*>(_pMsg);
				snprintf(params, c_paramLen, "wheel=%d distance=%.2f invertScroll=%s", pMsg->wheel(), (float) pMsg->distance(), pMsg->invertScroll()?"true":"false");
				break;
			}

			case MsgType::Select:
				break;

			case MsgType::Toggle:
				snprintf( params, c_paramLen, " checked=%d", (int) (static_cast<ToggleMsg*>(_pMsg)->isChecked()) );
				break;

			case MsgType::ValueUpdate:
			{
				auto pMsg = static_cast<ValueUpdateMsg*>(_pMsg);
				snprintf( params, c_paramLen, " value=%f", pMsg->value() );
				break;
			}

			case MsgType::RangeUpdate:
			{
				auto pMsg = static_cast<RangeUpdateMsg*>(_pMsg);
				snprintf( params, c_paramLen, " offset=%f length=%f fracOfs=%f fracLen=%f final=%s", float(pMsg->offset()), float(pMsg->length()), pMsg->fracOffset(), pMsg->fracLength(), pMsg->isFinal()?"true":"false" );
				break;
			}

			case MsgType::TextEdit:
			{
				auto pMsg = static_cast<TextEditMsg*>(_pMsg);
				snprintf( params, c_paramLen, " offset=%d deleted=%d inserted=%d", pMsg->offset(), pMsg->deleted(), pMsg->inserted() );
				break;
			}

			case MsgType::Selected:
			{
				auto pMsg = static_cast<SelectedMsg*>(_pMsg);

				snprintf(params, c_paramLen, " nbSelected=%d selected=", pMsg->nbSelected());

				int nbToPrint = std::min(pMsg->nbSelected(), 3);

				const Widget_p* pWidgets = pMsg->selected();
				for (int i = 0; i < nbToPrint; i++)
				{
					int ofs = strlen(params);
					snprintf(params + ofs, c_paramLen - ofs, " %p", pWidgets[i].rawPtr() );
				}

				if( nbToPrint < pMsg->nbSelected() )
				{
					int ofs = strlen(params);
					snprintf(params + ofs, c_paramLen - ofs, " ..." );
				}

				break;
			}

			case MsgType::Unselected:
			{
				auto pMsg = static_cast<UnselectedMsg*>(_pMsg);

				snprintf(params, c_paramLen, " nbUnselected=%d unselected=", pMsg->nbUnselected());

				int nbToPrint = std::min(pMsg->nbUnselected(), 3);

				const Widget_p* pWidgets = pMsg->unselected();
				for (int i = 0; i < nbToPrint; i++)
				{
					int ofs = strlen(params);
					snprintf(params + ofs, c_paramLen - ofs, " %p", pWidgets[i].rawPtr());
				}

				if (nbToPrint < pMsg->nbUnselected())
				{
					int ofs = strlen(params);
					snprintf(params + ofs, c_paramLen - ofs, " ...");
				}				break;
			}

			case MsgType::ItemToggle:
			{
				ItemToggleMsg_p pMsg = static_cast<ItemToggleMsg*>(_pMsg);
				snprintf( params, c_paramLen, " set=%s id=%d object=%p", pMsg->isSet()?"true":"false", pMsg->itemId(), pMsg->itemObject().rawPtr() );
				break;
			}
			case MsgType::ItemMousePress:
			{
				ItemMousePressMsg_p pMsg = static_cast<ItemMousePressMsg*>(_pMsg);
				snprintf( params, c_paramLen, " id=%d mouseButton=%s", pMsg->itemId(), _formatMouseButton(pMsg->button()).c_str() );
				break;
			}

			case MsgType::ItemsSelect:
			{
				ItemsSelectMsg_p pMsg = static_cast<ItemsSelectMsg*>(_pMsg);
				snprintf( params, c_paramLen, " nbItems=%d", pMsg->nbItems() );
				break;
			}

			case MsgType::ItemsUnselect:
			{
				ItemsUnselectMsg_p pMsg = static_cast<ItemsUnselectMsg*>(_pMsg);
				snprintf( params, c_paramLen, " nbItems=%d", pMsg->nbItems() );
				break;
			}

			case MsgType::ModalMoveOutside:
				break;
			case MsgType::ModalBlockedPress:
				snprintf( params, c_paramLen, " button=%s", _formatMouseButton(static_cast<ModalBlockedPressMsg*>(_pMsg)->button()).c_str() );
				break;
			case MsgType::ModalBlockedRelease:
				snprintf( params, c_paramLen, " button=%s", _formatMouseButton(static_cast<ModalBlockedReleaseMsg*>(_pMsg)->button()).c_str() );
				break;

			default:
				break;
		};

		source = _formatSource( _pMsg );
		copyTo = _formatCopyTo( _pMsg );

		string	timestamp;
		string	modkeys;
		string	pointerPos;

		if( _pMsg->isInstanceOf( InputMsg::TYPEINFO ) )
		{
			InputMsg_p p = static_cast<InputMsg*>(_pMsg);
			timestamp = _formatTimestamp( p->timestamp() );
			modkeys = _formatModkeys( p );
			pointerPos = _formatPointerPos( p );
		}

		if( m_func )
		{
			const int 	c_len = c_paramLen + 1024;
			char		output[c_len];
			
			snprintf( output, c_len, " - %s - %s%s%s%s%s\n", _pMsg->typeInfo().className, source.c_str(), copyTo.c_str(), pointerPos.c_str(), modkeys.c_str(), params );
			m_func(output);
		}
		else
		{
			*m_pStream << " - " << _pMsg->typeInfo().className << " - " << source << copyTo << pointerPos << modkeys << params;
			*m_pStream << std::endl;
		}
	}

	//____ _formatTimestamp() ______________________________________________________

	string MsgLogger::_formatTimestamp( int64_t ms ) const
	{
		char	temp[20];

		int milli	= (int) (ms % 1000);
		int sec		= (int) ((ms/1000) % 60);
		int min		= (int) ((ms/60000) % 60);
		int hour	= (int) (ms/3600000);

		snprintf( temp, 20, "%d:%02d:%02d.%03d", hour, min, sec, milli );
		return string( temp );
	}

	//____ _formatSource() __________________________________________________________

	string MsgLogger::_formatSource( Msg * _pMsg ) const
	{
		std::string	out;

		if( _pMsg->sourceRawPtr() )
		{
			char	temp[64];
			Object * pObject = _pMsg->sourceRawPtr();

			static const char def_type[] = "deleted";
			const char * pType = def_type;

			if( pObject )
				pType = pObject->typeInfo().className;

			snprintf( temp, 64, " source=%p (%s)", pObject, pType );
			out = temp;
		}

		return out;
	}

	//____ _formatCopyTo() __________________________________________________________

	string MsgLogger::_formatCopyTo( Msg * _pMsg ) const
	{
		std::string	out;

		if( _pMsg->hasCopyTo() )
		{
			char	temp[64];
			Receiver * pCopyTo = _pMsg->getCopyTo().rawPtr();

			const char * pType = pCopyTo->typeInfo().className;

			snprintf( temp, 64, " copyTo=%p (%s)", pCopyTo, pType );
			out = temp;
		}

		return out;
	}


	//____ _formatModkeys() __________________________________________________________

	string MsgLogger::_formatModkeys( InputMsg * _pMsg ) const
	{
		ModKeys keys = _pMsg->modKeys();

		string	out;

		if( keys != ModKeys::None )
		{
			char	temp[64];
			snprintf( temp, 64, " modkeys=%X", keys );
			out = temp;
		}

		return out;
	}

	//____ _formatPointerPos() _____________________________________________________

	string MsgLogger::_formatPointerPos( InputMsg * _pMsg ) const
	{
		Coord globalPos = _pMsg->pointerPos();


		char	temp[64];

		snprintf( temp, 64, " pointer=%.2f,%.2f", (float) globalPos.x, (float) globalPos.y );

		return string(temp);
	}

	//____ _formatPointerStyle() _____________________________________________________

	string MsgLogger::_formatPointerStyle( PointerChangeMsg * _pMsg ) const
	{
		switch( _pMsg->style() )
		{
			case PointerStyle::Arrow:
				return "PointerStyle::Arrow";
			case PointerStyle::Hourglass:
				return "PointerStyle::Hourglass";
			case PointerStyle::Hand:
				return "PointerStyle::Hand";
			case PointerStyle::Crosshair:
				return "PointerStyle::Crosshair";
			case PointerStyle::Help:
				return "PointerStyle::Help";
			case PointerStyle::Ibeam:
				return "PointerStyle::Ibeam";
			case PointerStyle::Stop:
				return "PointerStyle::Stop";
			case PointerStyle::UpArrow:
				return "PointerStyle::UpArrow";
			case PointerStyle::ResizeAll:
				return "PointerStyle::ResizeAll";
			case PointerStyle::ResizeNeSw:
				return "PointerStyle::ResizeNeSw";
			case PointerStyle::ResizeNwSe:
				return "PointerStyle::ResizeNwSe";
			case PointerStyle::ResizeNS:
				return "PointerStyle::ResizeNS";
			case PointerStyle::ResizeWE:
				return "PointerStyle::ResizeWE";
			default:
			{
				char	temp[64];
				snprintf( temp, 64, "%d (unkown enum)", (int) _pMsg->style() );
				return string(temp);
			}
		}
	}

	//____ _formatMouseButton() ____________________________________________________

	std::string MsgLogger::_formatMouseButton( MouseButton button ) const
	{
		switch( button )
		{
			case MouseButton::Left:
				return "LEFT";
			case MouseButton::Middle:
				return "MIDDLE";
			case MouseButton::Right:
				return "RIGHT";
			case MouseButton::X1:
				return "X1";
			case MouseButton::X2:
				return "X2";
			default:
			{
				char	temp[64];
				snprintf( temp, 64, "%d (unkown enum)", (int) button );
				return string(temp);
			}
		}
	}

	//____ _formatEditCommand() ____________________________________________________

	std::string MsgLogger::_formatEditCommand( EditCmd command ) const
	{
		switch( command )
		{
			case EditCmd::None:
				return "None";
			case EditCmd::Break:
				return "Break";
			case EditCmd::Escape:
				return "Escape";
			case EditCmd::Undo:
				return "Undo";
			case EditCmd::Redo:
				return "Redo";
			case EditCmd::Cut:
				return "Cut";
			case EditCmd::Copy:
				return "Copy";
			case EditCmd::Paste:
				return "Paste";
			case EditCmd::SelectAll:
				return "SelectAll";
			default:
			{
				char	temp[64];
				snprintf( temp, 64, "%d (unkown enum)", (int) command );
				return string(temp);
			}
		}
	}


	//____ _onRouteAdded() _________________________________________________________

	void  MsgLogger::_onRouteAdded()
	{
		_incRefCount();
	}

	//____ _onRouteRemoved() _______________________________________________________

	void  MsgLogger::_onRouteRemoved()
	{
		_decRefCount();
	}

} // namespace wg
