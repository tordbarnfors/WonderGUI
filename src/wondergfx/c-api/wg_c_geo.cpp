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


#include <wg_geo.h>
#include <wg_c_geo.h>

#include <type_traits>

static_assert( std::is_same<wg_pts,wg::pts>::value, "wg_pts and wg::pts must be of same type!" );
static_assert( std::is_same<wg_spx,wg::spx>::value, "wg_spx and wg::spx must be of same type!" );

static_assert( sizeof(wg_coordI) == sizeof(wg::CoordI), "wg_coordI differs in size from wg:CoordI!" );
static_assert( std::is_trivially_copyable<wg::CoordI>::value, "wg::CoordI is not trivially copyable" );

static_assert( sizeof(wg_coordF) == sizeof(wg::CoordF), "wg_coordF differs in size from wg:CoordF!" );
static_assert( std::is_trivially_copyable<wg::CoordF>::value, "wg::CoordF is not trivially copyable" );

static_assert( sizeof(wg_coord) == sizeof(wg::Coord), "wg_coord differs in size from wg:Coord!" );
static_assert( std::is_trivially_copyable<wg::Coord>::value, "wg::Coord is not trivially copyable" );

static_assert( sizeof(wg_coordSPX) == sizeof(wg::CoordSPX), "wg_coordSPX differs in size from wg:CoordSPX!" );
static_assert( std::is_trivially_copyable<wg::CoordSPX>::value, "wg::CoordSPX is not trivially copyable" );


static_assert( sizeof(wg_sizeI) == sizeof(wg::SizeI), "wg_sizeI differs in size from wg:SizeI!" );
static_assert( std::is_trivially_copyable<wg::SizeI>::value, "wg::SizeI is not trivially copyable" );

static_assert( sizeof(wg_sizeF) == sizeof(wg::SizeF), "wg_sizeF differs in size from wg:SizeF!" );
static_assert( std::is_trivially_copyable<wg::SizeF>::value, "wg::SizeF is not trivially copyable" );

static_assert( sizeof(wg_size) == sizeof(wg::Size), "wg_size differs in size from wg:Size!" );
static_assert( std::is_trivially_copyable<wg::Size>::value, "wg::Size is not trivially copyable" );

static_assert( sizeof(wg_borderI) == sizeof(wg::BorderI), "wg_borderI differs in size from wg:BorderI!" );
static_assert( std::is_trivially_copyable<wg::BorderI>::value, "wg::BorderI is not trivially copyable" );

static_assert( sizeof(wg_borderF) == sizeof(wg::BorderF), "wg_borderF differs in size from wg:BorderF!" );
static_assert( std::is_trivially_copyable<wg::BorderF>::value, "wg::BorderF is not trivially copyable" );

static_assert( sizeof(wg_border) == sizeof(wg::Border), "wg_border differs in size from wg:Border!" );
static_assert( std::is_trivially_copyable<wg::Border>::value, "wg::Border is not trivially copyable" );

static_assert( sizeof(wg_rectI) == sizeof(wg::RectI), "wg_rectI differs in size from wg:RectI!" );
static_assert( std::is_trivially_copyable<wg::RectI>::value, "wg::RectI is not trivially copyable" );

static_assert( sizeof(wg_rectF) == sizeof(wg::RectF), "wg_rectF differs in size from wg:RectF!" );
static_assert( std::is_trivially_copyable<wg::RectF>::value, "wg::RectF is not trivially copyable" );

static_assert( sizeof(wg_rect) == sizeof(wg::Rect), "wg_rect differs in size from wg:Rect!" );
static_assert( std::is_trivially_copyable<wg::Rect>::value, "wg::Rect is not trivially copyable" );

