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
#include "wg_animplayerinfosection.h"


namespace wg
{

	const TypeInfo AnimPlayerInfoSection::TYPEINFO = { "AnimPlayerInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	AnimPlayerInfoSection::AnimPlayerInfoSection(const DebugTheme& theme, IDebugContext* pContext, AnimPlayer * pInspected)
		: TypedInfoSection<AnimPlayer>( theme, pContext, AnimPlayer::TYPEINFO.className, pInspected )
	{
		this->slot = _createRows({
			textRow   ( "Play mode: ",            [](AnimPlayer* a) { return toString(a->playMode()); } ),
			boolRow   ( "Playing: ",              [](AnimPlayer* a) { return a->isPlaying(); } ),
			intRow    ( "Play position (ms): ",   [](AnimPlayer* a) { return a->playPos(); } ),
			intRow    ( "Cycle duration (ms): ",  [](AnimPlayer* a) { return a->cycleDuration(); } ),
			decimalRow( "Speed: ",                [](AnimPlayer* a) { return a->speed(); } ),
			intRow    ( "Mark alpha: ",           [](AnimPlayer* a) { return a->markAlpha(); } ),
			intRow    ( "Number of frames: ",     [](AnimPlayer* a) { return a->frames.size(); } ),
			intRow    ( "Frames duration (ms): ", [](AnimPlayer* a) { return a->frames.duration(); } ),
			objectRow ( "Frames surface: ",       [](AnimPlayer* a) -> Object* { return a->frames.surface().rawPtr(); } ),
			ptsRow    ( "Frame width (pts): ",    [](AnimPlayer* a) { return a->frames.frameSize().w; } ),
			ptsRow    ( "Frame height (pts): ",   [](AnimPlayer* a) { return a->frames.frameSize().h; } )
		});
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& AnimPlayerInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

} // namespace wg
