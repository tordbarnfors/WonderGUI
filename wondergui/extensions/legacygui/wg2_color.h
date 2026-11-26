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
#ifndef	WG2_COLOR_DOT_H
#define	WG2_COLOR_DOT_H

#ifndef	WG2_TYPES_DOT_H
#	include	<wg2_types.h>
#endif

#include <wax_color.h>

//____ WgColor ________________________________________________________________

/**
	Color8 with the named colors of the standard web palette, which Color8 itself no longer
	has (they are in wg::Color), so that code written for WgColor::Black and the like still
	compiles. Converts to and from Color8 freely.
**/

class WgColor : public wg::Color8
{
public:
	using wg::Color8::Color8;

	inline constexpr WgColor() {}
	inline constexpr WgColor( const wg::Color8& color ) : wg::Color8(color) {}
	inline WgColor( const wg::Color& color ) : wg::Color8(color) {}

	const static WgColor	Transparent;			// 0x000000
	const static WgColor	AliceBlue;				// 0xf0f8ff
	const static WgColor	AntiqueWhite;			// 0xfaebd7
	const static WgColor	Aqua;					// 0x00ffff
	const static WgColor	Aquamarine;				// 0x7fffd4
	const static WgColor	Azure;					// 0xf0ffff
	const static WgColor	Beige;					// 0xf5f5dc
	const static WgColor	Bisque;					// 0xffe4c4
	const static WgColor	Black;					// 0x000000
	const static WgColor	BlanchedAlmond;			// 0xffebcd
	const static WgColor	Blue;					// 0x0000ff
	const static WgColor	BlueViolet;				// 0x8a2be2
	const static WgColor	Brown;					// 0xa52a2a
	const static WgColor	Burlywood;				// 0xdeb887
	const static WgColor	CadetBlue;				// 0x5f9ea0
	const static WgColor	Chartreuse;				// 0x7fff00
	const static WgColor	Chocolate;				// 0xd2691e
	const static WgColor	Coral;					// 0xff7f50
	const static WgColor	CornflowerBlue;			// 0x6495ed
	const static WgColor	Cornsilk;				// 0xfff8dc
	const static WgColor	Crimson;				// 0xdc143c
	const static WgColor	Cyan;					// 0x00ffff
	const static WgColor	DarkBlue;				// 0x00008b
	const static WgColor	DarkCyan;				// 0x008b8b
	const static WgColor	DarkGoldenrod;			// 0xb8860b
	const static WgColor	DarkGray;				// 0xa9a9a9
	const static WgColor	DarkGrey;				// 0xa9a9a9
	const static WgColor	DarkGreen;				// 0x006400
	const static WgColor	DarkKhaki;				// 0xbdb76b
	const static WgColor	DarkMagenta;			// 0x8b008b
	const static WgColor	DarkOliveGreen;			// 0x556b2f
	const static WgColor	DarkOrange;				// 0xff8c00
	const static WgColor	DarkOrchid;				// 0x9932cc
	const static WgColor	DarkRed;				// 0x8b0000
	const static WgColor	DarkSalmon;				// 0xe9967a
	const static WgColor	DarkSeaGreen;			// 0x8fbc8f
	const static WgColor	DarkSlateBlue;			// 0x483d8b
	const static WgColor	DarkSlateGray;			// 0x2f4f4f
	const static WgColor	DarkSlateGrey;			// 0x2f4f4f
	const static WgColor	DarkTurquoise;			// 0x00ced1
	const static WgColor	DarkViolet;				// 0x9400d3
	const static WgColor	DeepPink;				// 0xff1493
	const static WgColor	DeepSkyBlue;			// 0x00bfff
	const static WgColor	DimGray;				// 0x696969
	const static WgColor	DimGrey;				// 0x696969
	const static WgColor	DodgerBlue;				// 0x1e90ff
	const static WgColor	FireBrick;				// 0xb22222
	const static WgColor	FloralWhite;			// 0xfffaf0
	const static WgColor	ForestGreen;			// 0x228b22
	const static WgColor	Fuchsia;				// 0xff00ff
	const static WgColor	Gainsboro;				// 0xdcdcdc
	const static WgColor	GhostWhite;				// 0xf8f8ff
	const static WgColor	Gold;					// 0xffd700
	const static WgColor	Goldenrod;				// 0xdaa520
	const static WgColor	Gray;					// 0x808080
	const static WgColor	Grey;					// 0x808080
	const static WgColor	Green;					// 0x008000
	const static WgColor	GreenYellow;			// 0xadff2f
	const static WgColor	Honeydew;				// 0xf0fff0
	const static WgColor	HotPink;				// 0xff69b4
	const static WgColor	IndianRed;				// 0xcd5c5c
	const static WgColor	Indigo;					// 0x4b0082
	const static WgColor	Ivory;					// 0xfffff0
	const static WgColor	Khaki;					// 0xf0e68c
	const static WgColor	Lavender;				// 0xe6e6fa
	const static WgColor	LavenderBlush;			// 0xfff0f5
	const static WgColor	LawnGreen;				// 0x7cfc00
	const static WgColor	LemonCiffon;			// 0xfffacd
	const static WgColor	LightBlue;				// 0xadd8e6
	const static WgColor	LightCoral;				// 0xf08080
	const static WgColor	LightCyan;				// 0xe0ffff
	const static WgColor	LightGoldenrodYellow;	// 0xfafad2
	const static WgColor	LightGray;				// 0xd3d3d3
	const static WgColor	LightGrey;				// 0xd3d3d3
	const static WgColor	LightGreen;				// 0x90ee90
	const static WgColor	LightPink;				// 0xffb6c1
	const static WgColor	LightSalmon;			// 0xffa07a
	const static WgColor	LightSeaGreen;			// 0x20b2aa
	const static WgColor	LightSkyBlue;			// 0x87cefa
	const static WgColor	LightSlateGray;			// 0x778899
	const static WgColor	LightSlateGrey;			// 0x778899
	const static WgColor	LightSteelBlue;			// 0xb0c4de
	const static WgColor	LightYellow;			// 0xffffe0
	const static WgColor	Lime;					// 0x00ff00
	const static WgColor	LimeGreen;				// 0x32cd32
	const static WgColor	Linen;					// 0xfaf0e6
	const static WgColor	Magenta;				// 0xff00ff
	const static WgColor	Maroon;					// 0x800000
	const static WgColor	MediumAquamarine;		// 0x66cdaa
	const static WgColor	MediumBlue;				// 0x0000cd
	const static WgColor	MediumOrchid;			// 0xba55d3
	const static WgColor	MediumPurple;			// 0x9370d8
	const static WgColor	MediumSeaGreen;			// 0x3cb371
	const static WgColor	MediumSlateBlue;		// 0x7b68ee
	const static WgColor	MediumSpringGreen;		// 0x00fa9a
	const static WgColor	MediumTurquoise;		// 0x48d1cc
	const static WgColor	MediumVioletRed;		// 0xc71585
	const static WgColor	MidnightBlue;			// 0x191970
	const static WgColor	MintCream;				// 0xf5fffa
	const static WgColor	MistyRose;				// 0xffe4e1
	const static WgColor	Moccasin;				// 0xffe4b5
	const static WgColor	NavajoWhite;			// 0xffdead
	const static WgColor	Navy;					// 0x000080
	const static WgColor	OldLace;				// 0xfdf5e6
	const static WgColor	Olive;					// 0x808000
	const static WgColor	OliveDrab;				// 0x6b8e23
	const static WgColor	Orange;					// 0xffa500
	const static WgColor	OrangeRed;				// 0xff4500
	const static WgColor	Orchid;					// 0xda70d6
	const static WgColor	PaleGoldenrod;			// 0xeee8aa
	const static WgColor	PaleGreen;				// 0x98fb98
	const static WgColor	PaleTurquoise;			// 0xafeeee
	const static WgColor	PaleVioletRed;			// 0xd87093
	const static WgColor	PapayaWhip;				// 0xffefd5
	const static WgColor	PeachPuff;				// 0xffdab9
	const static WgColor	Peru;					// 0xcd853f
	const static WgColor	Pink;					// 0xffc0cb
	const static WgColor	Plum;					// 0xdda0dd
	const static WgColor	PowderBlue;				// 0xb0e0e6
	const static WgColor	Purple;					// 0x800080
	const static WgColor	Red;					// 0xff0000
	const static WgColor	RosyBrown;				// 0xbc8f8f
	const static WgColor	RoyalBlue;				// 0x4169e1
	const static WgColor	SaddleBrown;			// 0x8b4513
	const static WgColor	Salmon;					// 0xfa8072
	const static WgColor	SandyBrown;				// 0xf4a460
	const static WgColor	SeaGreen;				// 0x2e8b57
	const static WgColor	Seashell;				// 0xfff5ee
	const static WgColor	Sienna;					// 0xa0522d
	const static WgColor	Silver;					// 0xc0c0c0
	const static WgColor	SkyBlue;				// 0x87ceeb
	const static WgColor	SlateBlue;				// 0x6a5acd
	const static WgColor	SlateGray;				// 0x708090
	const static WgColor	SlateGrey;				// 0x708090
	const static WgColor	Snow;					// 0xfffafa
	const static WgColor	SpringGreen;			// 0x00ff7f
	const static WgColor	SteelBlue;				// 0x4682b4
	const static WgColor	Tan;					// 0xd2b48c
	const static WgColor	Teal;					// 0x008080
	const static WgColor	Thistle;				// 0xd8bfd8
	const static WgColor	Tomato;					// 0xff6347
	const static WgColor	Turquoise;				// 0x40e0d0
	const static WgColor	Violet;					// 0xee82ee
	const static WgColor	Wheat;					// 0xf5deb3
	const static WgColor	White;					// 0xffffff
	const static WgColor	WhiteSmoke;				// 0xf5f5f5
	const static WgColor	Yellow;					// 0xffff00
	const static WgColor	YellowGreen;			// 0x9acd32
};

#endif // WG2_COLOR_DOT_H
