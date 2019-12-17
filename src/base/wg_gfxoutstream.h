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

#ifndef	WG_GFXOUTSTREAM_DOT_H
#define	WG_GFXOUTSTREAM_DOT_H
#pragma once

#include <wg_component.h>
#include <wg_pointers.h>
#include <wg_gfxstream.h>
#include <wg_types.h>
#include <wg_geo.h>
#include <wg_color.h>


namespace wg
{

	class GfxOutStream;
	typedef	StrongComponentPtr<GfxOutStream>	GfxOutStream_p;
	typedef	WeakComponentPtr<GfxOutStream>		GfxOutStream_wp;


	//____ GfxOutStreamHolder ___________________________________________________

	class GfxOutStreamHolder /** @private */
	{
	public:
		virtual Object * _object() = 0;
		virtual const Object * _object() const = 0;

		virtual void	_flushStream() = 0;
		virtual void	_reserveStream(int bytes) = 0;
		virtual void	_closeStream() = 0;
		virtual bool	_reopenStream() = 0;
		virtual bool	_isStreamOpen() = 0;

		virtual void	_pushChar(char c) = 0;
		virtual void	_pushShort(short s) = 0;
		virtual void	_pushInt(int i) = 0;
		virtual void	_pushFloat(float f) = 0;
		virtual void	_pushBytes(int nBytes, char * pBytes) = 0;
	};

	//____ GfxOutStream __________________________________________________________

	class GfxOutStream : public Component, public GfxStream
	{
	public:
		GfxOutStream(GfxOutStreamHolder * pHolder);

		enum Command
		{
			HeaderEnd
		};

		//.____ Control _______________________________________________________

		inline void		flush() { m_pHolder->_flushStream(); }
		inline void		reserve(int bytes) { m_pHolder->_reserveStream(bytes); }
		inline void		close() { m_pHolder->_closeStream(); }
		inline bool		isOpen() { return m_pHolder->_isStreamOpen(); }
		inline bool		reopen() { return m_pHolder->_reopenStream(); }

		GfxOutStream&	operator<< (Header);
		GfxOutStream&	operator<< (int16_t);
		GfxOutStream&	operator<< (uint16_t);
		GfxOutStream&	operator<< (int32_t);
		GfxOutStream&	operator<< (float);

		GfxOutStream&	operator<< (const CoordI&);
		GfxOutStream&	operator<< (const CoordF&);
		GfxOutStream&	operator<< (const SizeI&);
		GfxOutStream&	operator<< (const SizeF&);
		GfxOutStream&	operator<< (const RectI&);
		GfxOutStream&	operator<< (const RectF&);

		GfxOutStream&	operator<< (Color);
		GfxOutStream&	operator<< (Direction);
		GfxOutStream&	operator<< (BlendMode);
		GfxOutStream&	operator<< (Orientation);
		GfxOutStream&	operator<< (PixelFormat);
		GfxOutStream&	operator<< (ScaleMode);
		GfxOutStream&	operator<< (const DataChunk&);

		GfxOutStream&	operator<< (const int[2][2]);
		GfxOutStream&	operator<< (const float[2][2]);



		//.____ Misc __________________________________________________

		inline GfxOutStream_p	ptr() { return GfxOutStream_p(this); }

		short			allocObjectId();
		void			freeObjectId(short id);

	protected:
		Object *				_object() override { return m_pHolder->_object(); }
		const Object *			_object() const  override { return m_pHolder->_object(); }

		short					m_idCounter;

		short *					m_pFreeIdStack;
		int						m_freeIdStackCapacity;
		uint8_t					m_freeIdStackSize;


		GfxOutStreamHolder * 	m_pHolder;
	};


} // namespace wg
#endif //WG_GFXOUTSTREAM_DOT_H
