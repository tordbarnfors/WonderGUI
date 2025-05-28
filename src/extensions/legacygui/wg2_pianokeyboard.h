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
#ifndef WG2_PIANOKEYBOARD_DOT_H
#define WG2_PIANOKEYBOARD_DOT_H
#pragma once


#include <wg2_widget.h>
#include <wg_skin.h>

#include <bitset>


//____ PianoKeyboard __________________________________________________________
/**
	* @brief	Widget displaying a piano keyboard.
	*
	* PianoKeyboard is a widget that displays a piano keyboard.
	*
	**/

class WgPianoKeyboard : public WgWidget
{
public:
	WgPianoKeyboard();
	virtual ~WgPianoKeyboard();

	//.____ Identification __________________________________________

	const char * Type(void) const override;
	static const char * GetClass();
	virtual WgWidget* NewOfMyType() const override { return new WgPianoKeyboard(); };

	//.____ Appearance ____________________________________________________

	void		setLayout(int nbWhiteKeys, const std::bitset<7>& blackKeyPositions );
    void        setGeoTweak( int keyIdx, const WgRect& geoTweak );

	void		setSurfaces(const wg::Surface_p& pOddWhiteKeys, const wg::Surface_p& pEvenWhiteKeys, const wg::Surface_p& pBlackKeys, std::initializer_list<wg::State> states);

	//.____ Control _______________________________________________________

	void		pressKey(int keyIdx);
	void		releaseKey(int keyIdx);

	bool		isKeyPressed(int keyIdx) const;

	void		setPressedKeys(std::initializer_list<int> pressedKeys);
	void		releaseAllKeys();

	void		setFlipKeyOnPress(bool bFlip);
	bool		flipKeyOnPress() const { return m_bFlipOnPress; }

	void		selectKey(int keyIdx);
	void		unselectKey(int keyIdx);

	bool		isKeySelected(int keyIdx) const;

	void		setSelectedKeys(std::initializer_list<int> selectedKeys);
	void		unselectAllKeys();

	int			userPressedKey() const { return m_bIsHoveredPressed ? m_hoveredKey : -1; }

	wg::SizeI		PreferredPixelSize() const override;
	wg::SizeI		MinPixelSize() const override;
	wg::SizeI		MaxPixelSize() const override;


protected:

	void		_onCloneContent(const WgWidget * _pOrg) override;
	void		_onRender(wg::GfxDevice * pDevice, const wg::RectI& _canvas, const wg::RectI& _window) override;
	void		_onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler) override;
	bool		_onAlphaTest(const wg::CoordI& ofs) override;
//		void		_setState(State state) override;

	void		_requestRenderKey(int keyIdx);
	int			_markTestKey(const wg::CoordI& pos);
	wg::RectI	_keyRect(int keyIdx);
	void		_setHoveredKey(int keyIdx, uint64_t timestamp);
	void		_setAPIPressed(int keyIdx, bool bPressed);
	void		_setHoverPress(bool bPressed, uint64_t timestamp);

private:

	static const int c_maxKeys = 12 * 6;

	int				m_nbWhiteKeys = 0;
	std::bitset<7>	m_blackKeyPositions;
	int				m_nbKeys = 0;
	wg::SizeI		m_keyboardSourceSize;			// In pixels
	wg::SizeI		m_preferredKeyboardSize;		// In points
	int				m_blackKeySourceHeight = 0;		// In pixels

	wg::Surface_p		m_pOddWhiteKeys;
	wg::Surface_p		m_pEvenWhiteKeys;
	wg::Surface_p		m_pBlackKeys;

	int				m_stateOfsY[wg::State::NbStates];

	struct KeyInfo
	{
		wg::State	state;
		int			keyPos;				// 0+, the key position from left to right, counting only white keys.
		bool		isBlack;			// key is the black key following the white key of keyPos.
		bool		isSetToPressed;		// Set when key has been pressed through API-call and should remain pressed after mouse release.
        WgRect      geoTweak;           // Relative tweak in position and size to the automatically generated geometry.
	};

	KeyInfo			m_keyInfo[c_maxKeys];

	int				m_hoveredKey		= -1;
	bool			m_bIsHoveredPressed = false;
	bool			m_bFlipOnPress		= false;


};


#endif //WG2_PIANOKEYBOARD_DOT_H
