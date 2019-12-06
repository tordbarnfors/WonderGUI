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
#ifndef	WG_CSPAN_DOT_H
#define WG_CSPAN_DOT_H
#pragma once

#include <wg_geocomponent.h>

namespace wg
{

	//____ CSpan _____________________________________________________________

	class CSpan : public GeoComponent		/** @private */
	{
	public:
		CSpan(GeoComponent::Holder * pHolder );
		virtual ~CSpan() {};

		bool	setMin( int min );
		bool	setMax( int max );
		bool	setRange( int min, int max );

		void	setSpan( int begin, int length );
		void	setBegin( int begin );
		void	setLength( int length );

		void	setRelativeSpan( float begin, float length );
		void	setRelativePos( float pos );
		void	setRelativeBegin( float begin );
		void	setRelativeLength( float length );

		bool	stepForward();
		bool	stepBackward();
		bool	skipForward();
		bool	skipBackward();

		inline float	relativePos() const { return (begin+length/2)/(float)(max-min); }
		inline float	relativeBegin() const { return begin/(float)(max-min); }
		inline float	relativeLength() const { return length/(float)(max-min); };


		const static int	MAX = 0x00FFFFFF;
		const static int	MIN = 0xFF000000;

		// Directly accessible for holder.

		int		min;
		int		max;

		int		begin;
		int		length;


	protected:
		void	_onModified() { _notify( ComponentNotif::SpanModified, 0, nullptr ); }
		int		_stepSize() { return 0; }	//TODO: Implement!!!
		int		_skipSize() { return 0; }	//TODO: Implement!!!
	};




} // namespace wg
#endif //WG_CSPAN_DOT_H
