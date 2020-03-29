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
#ifndef WG_SKIN_DOT_H
#define WG_SKIN_DOT_H
#pragma once

#include <wg_pointers.h>
#include <wg_geo.h>
#include <wg_types.h>
#include <wg_util.h>

namespace wg
{

	class Skin;
	class GfxDevice;

	typedef	StrongPtr<Skin>	Skin_p;


	class Skin : public Object
	{
	public:
		virtual ~Skin() {};

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		virtual Size	minSize() const = 0;
		virtual Size	preferredSize() const = 0;

		virtual Size	sizeForContent(const Size& contentSize) const = 0;
		virtual Border	contentPadding() const = 0;
		virtual Size	contentPaddingSize() const = 0;
		virtual Coord	contentOfs(State state) const = 0;
		virtual Rect	contentRect(const Rect& canvas, State state) const = 0;

		//.____ Misc ____________________________________________________

		virtual bool	isOpaque() const = 0;
		virtual bool	isOpaque( State state ) const = 0;
		virtual bool	isOpaque( const Rect& rect, const Size& canvasSize, State state ) const = 0;

		virtual bool	isStateIdentical( State state, State comparedTo ) const = 0;

		virtual bool	markTest(const Coord& ofs, const Rect& canvas, State state, int opacityTreshold) const = 0;
		virtual void 	render(GfxDevice * pDevice, const Rect& canvas, State state) const = 0;

		//.____ Internal ______________________________________________________

		inline bool		_markTest(const CoordI& ofs, const RectI& canvas, State state, int opacityTreshold) const
		{
			return markTest(reinterpret_cast<const Coord&>(ofs), reinterpret_cast<const Rect&>(canvas), state, opacityTreshold);
		}

		inline bool		_isOpaque(const RectI& rect, const SizeI& canvasSize, State state) const
		{
			return isOpaque(reinterpret_cast<const Rect&>(rect), reinterpret_cast<const Size&>(canvasSize), state);
		}

		inline SizeI	_minSize() const
		{
			return minSize().qpix();
		}

		inline SizeI	_preferredSize() const
		{
			return preferredSize().qpix();
		}

		inline SizeI	_sizeForContent(const SizeI contentSize) const
		{
			return sizeForContent(Util::qpixToMU(contentSize)).qpix();
		}

		inline BorderI	_contentPadding() const
		{
			return contentPadding().qpix();
		}

		inline SizeI	_contentPaddingSize() const
		{
			return contentPaddingSize().qpix();
		}

		inline CoordI	_contentOfs(State state) const
		{
			return contentOfs(state).qpix();
		}

		inline RectI	_contentRect(const RectI& canvas, State state) const
		{
			return contentRect(Util::qpixToMU(canvas), state).qpix();
		}

		inline void 	_render(GfxDevice * pDevice, const RectI& canvas, State state) const
		{
			render(pDevice, Util::qpixToMU(canvas), state);
		}

	protected:
		Skin() {};
	};




} // namespace wg
#endif //WG_SKIN_DOT_H
