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

#include <wg_color.h>

typedef wg::Color		WgColor;


#if false

//____ Class WgColor _______________________________________________________________________

/**

	Specifies a color in ARGB-format, where the alpha component controls transparency.
	Range for the individual color components is 0-255.

	Setting alpha to 0 gives full transparency while 255 gives full opacity.

	The different components can be accessed individually through the r, g, b, and a members,
	or as a group through the argb member.

	For convenience, the color class also provides static, predefined colors matching the standard web-palette.

**/

class WgColor
{
public:
	inline WgColor() {};
	inline WgColor( const WgColor& col );
	inline WgColor( Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255 );
	inline WgColor( Uint32 argb );
	inline WgColor( Uint32 rgb, Uint8 a );

	inline bool operator==( const WgColor& k ) const;
	inline bool operator!=( const WgColor& k ) const;

	inline WgColor operator+( const WgColor& k ) const;
	inline WgColor operator-( const WgColor& k ) const;

	inline WgColor operator*( float f ) const;
	inline WgColor operator*( const WgColor& k ) const;

	static WgColor Blend( const WgColor& start, const WgColor& dest, float grade );


	union
	{
		struct
		{
			Uint8 b;
			Uint8 g;
			Uint8 r;
			Uint8 a;
		};
		Uint32 argb;
	};



	const static WgColor	transparent;		// 0x00FFFFFF

	const static WgColor 	aliceblue;			// 0xF0F8FF
	const static WgColor 	antiquewhite;		// 0xFAEBD7
	const static WgColor 	aqua;				// 0x00ffff
	const static WgColor 	aquamarine;			// 0x7fffd4
	const static WgColor 	azure;				// 0xf0ffff
	const static WgColor 	beige;				// 0xf5f5dc
	const static WgColor 	bisque;  			// 0xffe4c4
	const static WgColor 	black;	  			// 0x000000
	const static WgColor 	blanchedalmond; 	// 0xffebcD
	const static WgColor 	blue; 	  			// 0x0000ff
	const static WgColor 	blueviolet; 		// 0x8a2be2
	const static WgColor 	brown;   			// 0xa52a2a
	const static WgColor 	burlywood;  		// 0xdeb887
	const static WgColor 	cadetblue;  		// 0x5f9ea0
	const static WgColor 	chartreuse; 		// 0x7fff00
	const static WgColor 	chocolate;  		// 0xd2691e
	const static WgColor 	coral;   			// 0xff7f50
	const static WgColor 	cornflowerblue; 	// 0x6495eD
	const static WgColor 	cornsilk; 			// 0xfff8dc
	const static WgColor 	crimson;  			// 0xdc143c
	const static WgColor 	cyan;  				// 0x00ffff
	const static WgColor 	darkblue;  			// 0x00008b
	const static WgColor 	darkcyan;  			// 0x008b8b
	const static WgColor 	darkgoldenrod;  	// 0xb8860b
	const static WgColor 	darkgray;  			// 0xa9a9a9
	const static WgColor 	darkgrey;  			// 0xa9a9a9
	const static WgColor 	darkgreen;  		// 0x006400
	const static WgColor 	darkkhaki;  		// 0xbdb76b
	const static WgColor 	darkmagenta;  		// 0x8b008b
	const static WgColor 	darkolivegreen; 	// 0x556b2F
	const static WgColor 	darkorange;  		// 0xff8c00
	const static WgColor 	darkorchid;  		// 0x9932cc
	const static WgColor 	darkred;  			// 0x8b0000
	const static WgColor 	darksalmon;  		// 0xe9967a
	const static WgColor 	darkseagreen;  		// 0x8fbc8f
	const static WgColor 	darkslateblue;  	// 0x483d8b
	const static WgColor 	darkslategray;  	// 0x2f4f4f
	const static WgColor 	darkslategrey;  	// 0x2f4f4f
	const static WgColor 	darkturquoise;  	// 0x00ced1
	const static WgColor 	darkviolet;  		// 0x9400d3
	const static WgColor 	deeppink;  			// 0xff1493
	const static WgColor 	deepskyblue; 		// 0x00bfff
	const static WgColor 	dimgray;  			// 0x696969
	const static WgColor 	dimgrey;  			// 0x696969
	const static WgColor 	dodgerblue;  		// 0x1e90ff
	const static WgColor 	firebrick;  		// 0xb22222
	const static WgColor 	floralwhite; 		// 0xfffaf0
	const static WgColor 	forestgreen; 		// 0x228b22
	const static WgColor 	fuchsia;  			// 0xff00ff
	const static WgColor 	gainsboro;  		// 0xdcdcdc
	const static WgColor 	ghostwhite;  		// 0xf8f8ff
	const static WgColor 	gold;  				// 0xffd700
	const static WgColor 	goldenrod;  		// 0xdaa520
	const static WgColor 	gray;  				// 0x808080
	const static WgColor 	grey;  				// 0x808080
	const static WgColor 	green;  			// 0x008000
	const static WgColor 	greenyellow; 		// 0xadff2f
	const static WgColor 	honeydew;  			// 0xf0fff0
	const static WgColor 	hotpink;  			// 0xff69b4
	const static WgColor 	indianred;   		// 0xcd5c5c
	const static WgColor 	indigo;   			// 0x4b0082
	const static WgColor 	ivory;  			// 0xfffff0
	const static WgColor 	khaki;  			// 0xf0e68c
	const static WgColor 	lavender;  			// 0xe6e6fa
	const static WgColor 	lavenderblush;  	// 0xfff0f5
	const static WgColor 	lawngreen;  		// 0x7cfc00
	const static WgColor 	lemonchiffon;		// 0xfffacd
	const static WgColor 	lightblue;  		// 0xadd8e6
	const static WgColor 	lightcoral;  		// 0xf08080
	const static WgColor 	lightcyan;  		// 0xe0ffff
	const static WgColor 	lightgoldenrodyellow;	// 0xfAFAD2
	const static WgColor 	lightgray;  		// 0xd3d3d3
	const static WgColor 	lightgrey;  		// 0xd3d3d3
	const static WgColor 	lightgreen;  		// 0x90ee90
	const static WgColor 	lightpink;  		// 0xffb6c1
	const static WgColor 	lightsalmon;  		// 0xffa07a
	const static WgColor 	lightseagreen;  	// 0x20b2aa
	const static WgColor 	lightskyblue;  		// 0x87cefa
	const static WgColor 	lightslategray;  	// 0x778899
	const static WgColor 	lightslategrey;  	// 0x778899
	const static WgColor 	lightsteelblue;  	// 0xb0c4dE
	const static WgColor 	lightyellow;  		// 0xffffe0
	const static WgColor 	lime;  				// 0x00ff00
	const static WgColor 	limegreen;  		// 0x32cd32
	const static WgColor 	linen;  			// 0xfaf0e6
	const static WgColor 	magenta;  			// 0xff00ff
	const static WgColor 	maroon;  			// 0x800000
	const static WgColor 	mediumaquamarine;  	// 0x66cdaA
	const static WgColor 	mediumblue;  		// 0x0000cd
	const static WgColor 	mediumorchid;		// 0xba55d3
	const static WgColor 	mediumpurple;	 	// 0x9370d8
	const static WgColor 	mediumseagreen;  	// 0x3cb371
	const static WgColor 	mediumslateblue;  	// 0x7b68eE
	const static WgColor 	mediumspringgreen;  // 0x00fa9A
	const static WgColor 	mediumturquoise;  	// 0x48d1cC
	const static WgColor 	mediumvioletred;  	// 0xc71585
	const static WgColor 	midnightblue;  		// 0x191970
	const static WgColor 	mintcream;  		// 0xf5fffa
	const static WgColor 	mistyrose;  		// 0xffe4e1
	const static WgColor 	moccasin;  			// 0xffe4b5
	const static WgColor 	navajowhite;  		// 0xffdead
	const static WgColor 	navy;  				// 0x000080
	const static WgColor 	oldlace;  			// 0xfdf5e6
	const static WgColor 	olive;  			// 0x808000
	const static WgColor 	olivedrab;  		// 0x6b8e23
	const static WgColor 	orange;  			// 0xffa500
	const static WgColor 	orangered;  		// 0xff4500
	const static WgColor 	orchid;  			// 0xda70d6
	const static WgColor 	palegoldenrod;  	// 0xeee8aa
	const static WgColor 	palegreen;  		// 0x98fb98
	const static WgColor 	paleturquoise;  	// 0xafeeee
	const static WgColor 	palevioletred;  	// 0xd87093
	const static WgColor 	papayawhip;  		// 0xffefd5
	const static WgColor 	peachpuff;  		// 0xffdab9
	const static WgColor 	peru;  				// 0xcd853f
	const static WgColor 	pink;  				// 0xffc0cb
	const static WgColor 	plum;  				// 0xdda0dd
	const static WgColor 	powderblue;  		// 0xb0e0e6
	const static WgColor 	purple;  			// 0x800080
	const static WgColor 	red;  				// 0xff0000
	const static WgColor 	rosybrown;  		// 0xbc8f8f
	const static WgColor 	royalblue;  		// 0x4169e1
	const static WgColor 	saddlebrown; 		// 0x8b4513
	const static WgColor 	salmon;  			// 0xfa8072
	const static WgColor 	sandybrown;  		// 0xf4a460
	const static WgColor 	seagreen;  			// 0x2e8b57
	const static WgColor 	seashell;  			// 0xfff5ee
	const static WgColor 	sienna;  			// 0xa0522d
	const static WgColor 	silver;  			// 0xc0c0c0
	const static WgColor 	skyblue;  			// 0x87ceeb
	const static WgColor 	slateblue;  		// 0x6a5acd
	const static WgColor 	slategray;  		// 0x708090
	const static WgColor 	slategrey;  		// 0x708090
	const static WgColor 	snow;  				// 0xfffafa
	const static WgColor 	springgreen; 		// 0x00ff7f
	const static WgColor 	steelblue;  		// 0x4682b4
	const static WgColor 	tan;  				// 0xd2b48c
	const static WgColor 	teal;  				// 0x008080
	const static WgColor 	thistle;  			// 0xd8bfd8
	const static WgColor 	tomato;  			// 0xff6347
	const static WgColor 	turquoise;  		// 0x40e0d0
	const static WgColor 	violet;  			// 0xee82ee
	const static WgColor 	wheat;  			// 0xf5deb3
	const static WgColor 	white;  			// 0xffffff
	const static WgColor 	whitesmoke;  		// 0xf5f5f5
	const static WgColor 	yellow;  			// 0xffff00
	const static WgColor 	yellowgreen; 		// 0x9acd32
};


//-------------------------------------------------------------------
inline bool WgColor::operator==( const WgColor& k ) const
{
	if( argb == k.argb )
		return true;

	return false;
}

//-------------------------------------------------------------------
inline bool WgColor::operator!=( const WgColor& k ) const
{
	if( argb != k.argb )
		return true;

	return false;
}

//-------------------------------------------------------------------
inline WgColor WgColor::operator+( const WgColor& k ) const
{
	WgColor kNewColor;
	kNewColor.r = r + k.r;
	kNewColor.g = g + k.g;
	kNewColor.b = b + k.b;
	kNewColor.a = a + k.a;
	return kNewColor;
}

//-------------------------------------------------------------------
inline WgColor WgColor::operator-( const WgColor& k ) const
{
	WgColor kNewColor;
	kNewColor.r = r - k.r;
	kNewColor.g = g - k.g;
	kNewColor.b = b - k.b;
	kNewColor.a = a - k.a;
	return kNewColor;
}

//-------------------------------------------------------------------
inline WgColor WgColor::operator*( float f ) const
{
	WgColor kNewColor;
	kNewColor.r = (Uint8)( (float)r * f );
	kNewColor.g = (Uint8)( (float)g * f );
	kNewColor.b = (Uint8)( (float)b * f );
	kNewColor.a = (Uint8)( (float)a * f );
	return kNewColor;
}

//-------------------------------------------------------------------
inline WgColor WgColor::operator*( const WgColor& k ) const
{
	WgColor kNewColor;
	kNewColor.r = (Uint8)(((int)r * (int)k.r )/255);
	kNewColor.g = (Uint8)(((int)g * (int)k.g )/255);
	kNewColor.b = (Uint8)(((int)b * (int)k.b )/255);
	kNewColor.a = (Uint8)(((int)a * (int)k.a )/255);
	return kNewColor;
}



//____ WgColor::WgColor() _________________________________________________________________

inline WgColor::WgColor( const WgColor& _col )
{
	argb = _col.argb;
}

inline WgColor::WgColor( Uint32 _argb )
{
	argb = _argb;
}

inline WgColor::WgColor( Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a )
{
	a = _a;
	r = _r;
	g = _g;
	b = _b;
}

inline WgColor::WgColor( Uint32 rgb, Uint8 _a )
{
	argb = rgb;
	a = _a;
}

#endif

#endif // WG2_COLOR_DOT_H
