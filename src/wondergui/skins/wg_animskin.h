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
#ifndef WG_ANIMSKIN_DOT_H
#define WG_ANIMSKIN_DOT_H
#pragma once

#include <wg_skin.h>
#include <wg_object.h>

#include <utility>
#include <vector>

namespace wg
{

	class AnimSkin;
	typedef	StrongPtr<AnimSkin>	AnimSkin_p;
	typedef	WeakPtr<AnimSkin>		AnimSkin_wp;

	class AnimSkin : public Skin
	{
	public:
		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
		AnimSkin();

		template<class BP>
		AnimSkin(const BP& bp) : Skin(bp) {}

		~AnimSkin();

	};


} // namespace wg
#endif //WG_ANIMSKIN_DOT_H
