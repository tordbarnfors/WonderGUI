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
#include <wax_geo.h>
#include <wax_c_geo.h>

#include <type_traits>

static_assert( std::is_same<wax_pts,wax::pts>::value, "wax_pts and wax::pts must be of same type!" );
static_assert( std::is_same<wax_spx,wax::spx>::value, "wax_spx and wax::spx must be of same type!" );

static_assert( sizeof(wax_coordI) == sizeof(wax::CoordI), "wax_coordI differs in size from wg:CoordI!" );
static_assert( std::is_trivially_copyable<wax::CoordI>::value, "wax::CoordI is not trivially copyable" );

static_assert( sizeof(wax_coordF) == sizeof(wax::CoordF), "wax_coordF differs in size from wg:CoordF!" );
static_assert( std::is_trivially_copyable<wax::CoordF>::value, "wax::CoordF is not trivially copyable" );

static_assert( sizeof(wax_coord) == sizeof(wax::Coord), "wax_coord differs in size from wg:Coord!" );
static_assert( std::is_trivially_copyable<wax::Coord>::value, "wax::Coord is not trivially copyable" );

static_assert( sizeof(wax_coordSPX) == sizeof(wax::CoordSPX), "wax_coordSPX differs in size from wg:CoordSPX!" );
static_assert( std::is_trivially_copyable<wax::CoordSPX>::value, "wax::CoordSPX is not trivially copyable" );


static_assert( sizeof(wax_sizeI) == sizeof(wax::SizeI), "wax_sizeI differs in size from wg:SizeI!" );
static_assert( std::is_trivially_copyable<wax::SizeI>::value, "wax::SizeI is not trivially copyable" );

static_assert( sizeof(wax_sizeF) == sizeof(wax::SizeF), "wax_sizeF differs in size from wg:SizeF!" );
static_assert( std::is_trivially_copyable<wax::SizeF>::value, "wax::SizeF is not trivially copyable" );

static_assert( sizeof(wax_size) == sizeof(wax::Size), "wax_size differs in size from wg:Size!" );
static_assert( std::is_trivially_copyable<wax::Size>::value, "wax::Size is not trivially copyable" );

static_assert( sizeof(wax_borderI) == sizeof(wax::BorderI), "wax_borderI differs in size from wg:BorderI!" );
static_assert( std::is_trivially_copyable<wax::BorderI>::value, "wax::BorderI is not trivially copyable" );

static_assert( sizeof(wax_borderF) == sizeof(wax::BorderF), "wax_borderF differs in size from wg:BorderF!" );
static_assert( std::is_trivially_copyable<wax::BorderF>::value, "wax::BorderF is not trivially copyable" );

static_assert( sizeof(wax_border) == sizeof(wax::Border), "wax_border differs in size from wg:Border!" );
static_assert( std::is_trivially_copyable<wax::Border>::value, "wax::Border is not trivially copyable" );

static_assert( sizeof(wax_rectI) == sizeof(wax::RectI), "wax_rectI differs in size from wg:RectI!" );
static_assert( std::is_trivially_copyable<wax::RectI>::value, "wax::RectI is not trivially copyable" );

static_assert( sizeof(wax_rectF) == sizeof(wax::RectF), "wax_rectF differs in size from wg:RectF!" );
static_assert( std::is_trivially_copyable<wax::RectF>::value, "wax::RectF is not trivially copyable" );

static_assert( sizeof(wax_rect) == sizeof(wax::Rect), "wax_rect differs in size from wg:Rect!" );
static_assert( std::is_trivially_copyable<wax::Rect>::value, "wax::Rect is not trivially copyable" );

