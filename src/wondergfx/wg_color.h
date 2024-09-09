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

#ifndef	WG_COLOR_DOT_H
#define	WG_COLOR_DOT_H
#pragma once

#include	<wg_gfxtypes.h>

namespace wg
{

	//____ Class Color8 _______________________________________________________________________

	/**
		@brief Color with RGBA components in the range of 0-255.

		Specifies a color in ARGB-format, where the alpha component controls transparency.
		Range for the individual color components is 0-255.

		Setting alpha to 0 gives full transparency while 255 gives full opacity.

		The different components can be accessed individually through the r, g, b and a members,
		or as a group through the argb member.

		WonderGUI uses HiColor for all API calls and internal operations, so Color8 is only provided
		for developer convenience since colors often are expressed in this range.

		The gammaCorrection-flag or the active Context decides how WonderGUI understands the color ranges
		in a Color8 object. If gammaCorrection is set, the range is believed to be in standard non-linear
		sRGB-format, otherwise in linear color format. The alpha channel is always linear.

		For convenience, the color class also provides static, predefined colors matching the standard web-palette.

	**/

	class Color8
	{
	public:

		//.____ Creation __________________________________________

		inline constexpr Color8() : argb(0) {};
		inline constexpr Color8( const Color8& col );
		inline constexpr Color8( uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255 );
		inline constexpr Color8( uint32_t argb );
		inline constexpr Color8( uint32_t rgb, uint8_t a );

		//.____ Operators ___________________________________________

		inline bool operator==(const Color8& k) const;
		inline bool operator!=(const Color8& k) const;

		Color8	operator+(const Color8& k) const;
		Color8	operator-(const Color8& k) const;

		Color8	operator*(float f) const;
		Color8	operator*(const Color8& k) const;

		//.____ Properties __________________________________________

		union
		{
			struct
			{
				uint8_t b;
				uint8_t g;
				uint8_t r;
				uint8_t a;
			};
			uint32_t argb;
		};

		//.____ Definitions _________________________________________

		const static Color8		Transparent;		// 0x000000
		const static Color8 	AliceBlue;			// 0xF0F8FF
		const static Color8 	AntiqueWhite;		// 0xFAEBD7
		const static Color8 	Aqua;				// 0x00ffff
		const static Color8 	Aquamarine;			// 0x7fffd4
		const static Color8 	Azure;				// 0xf0ffff
		const static Color8 	Beige;				// 0xf5f5dc
		const static Color8 	Bisque;  			// 0xffe4c4
		const static Color8 	Black;	  			// 0x000000
		const static Color8 	BlanchedAlmond; 	// 0xffebcD
		const static Color8 	Blue; 	  			// 0x0000ff
		const static Color8 	BlueViolet; 		// 0x8a2be2
		const static Color8 	Brown;   			// 0xa52a2a
		const static Color8 	Burlywood;  		// 0xdeb887
		const static Color8 	CadetBlue;  		// 0x5f9ea0
		const static Color8 	Chartreuse; 		// 0x7fff00
		const static Color8 	Chocolate;  		// 0xd2691e
		const static Color8 	Coral;   			// 0xff7f50
		const static Color8 	CornflowerBlue; 	// 0x6495eD
		const static Color8 	Cornsilk; 			// 0xfff8dc
		const static Color8 	Crimson;  			// 0xdc143c
		const static Color8 	Cyan;  				// 0x00ffff
		const static Color8 	DarkBlue;  			// 0x00008b
		const static Color8 	DarkCyan;  			// 0x008b8b
		const static Color8 	DarkGoldenrod;  	// 0xb8860b
		const static Color8 	DarkGray;  			// 0xa9a9a9
		const static Color8 	DarkGrey;  			// 0xa9a9a9
		const static Color8 	DarkGreen;  		// 0x006400
		const static Color8 	DarkKhaki;  		// 0xbdb76b
		const static Color8 	DarkMagenta;  		// 0x8b008b
		const static Color8 	DarkOliveGreen; 	// 0x556b2F
		const static Color8 	DarkOrange;  		// 0xff8c00
		const static Color8 	DarkOrchid;  		// 0x9932cc
		const static Color8 	DarkRed;  			// 0x8b0000
		const static Color8 	DarkSalmon;  		// 0xe9967a
		const static Color8 	DarkSeaGreen;  		// 0x8fbc8f
		const static Color8 	DarkSlateBlue;  	// 0x483d8b
		const static Color8 	DarkSlateGray;  	// 0x2f4f4f
		const static Color8 	DarkSlateGrey;  	// 0x2f4f4f
		const static Color8 	DarkTurquoise;  	// 0x00ced1
		const static Color8 	DarkViolet;  		// 0x9400d3
		const static Color8 	DeepPink;  			// 0xff1493
		const static Color8 	DeepSkyBlue; 		// 0x00bfff
		const static Color8 	DimGray;  			// 0x696969
		const static Color8 	DimGrey;  			// 0x696969
		const static Color8 	DodgerBlue;  		// 0x1e90ff
		const static Color8 	FireBrick;  		// 0xb22222
		const static Color8 	FloralWhite; 		// 0xfffaf0
		const static Color8 	ForestGreen; 		// 0x228b22
		const static Color8 	Fuchsia;  			// 0xff00ff
		const static Color8 	Gainsboro;  		// 0xdcdcdc
		const static Color8 	GhostWhite;  		// 0xf8f8ff
		const static Color8 	Gold;  				// 0xffd700
		const static Color8 	Goldenrod;  		// 0xdaa520
		const static Color8 	Gray;  				// 0x808080
		const static Color8 	Grey;  				// 0x808080
		const static Color8 	Green;  			// 0x008000
		const static Color8 	GreenYellow; 		// 0xadff2f
		const static Color8 	Honeydew;  			// 0xf0fff0
		const static Color8 	HotPink;  			// 0xff69b4
		const static Color8 	IndianRed;   		// 0xcd5c5c
		const static Color8 	Indigo;   			// 0x4b0082
		const static Color8 	Ivory;  			// 0xfffff0
		const static Color8 	Khaki;  			// 0xf0e68c
		const static Color8 	Lavender;  			// 0xe6e6fa
		const static Color8 	LavenderBlush;  	// 0xfff0f5
		const static Color8 	LawnGreen;  		// 0x7cfc00
		const static Color8 	LemonCiffon;		// 0xfffacd
		const static Color8 	LightBlue;  		// 0xadd8e6
		const static Color8 	LightCoral;  		// 0xf08080
		const static Color8 	LightCyan;  		// 0xe0ffff
		const static Color8 	LightGoldenrodYellow;	// 0xfAFAD2
		const static Color8 	LightGray;  		// 0xd3d3d3
		const static Color8 	LightGrey;  		// 0xd3d3d3
		const static Color8 	LightGreen;  		// 0x90ee90
		const static Color8 	LightPink;  		// 0xffb6c1
		const static Color8 	LightSalmon;  		// 0xffa07a
		const static Color8 	LightSeaGreen;  	// 0x20b2aa
		const static Color8 	LightSkyBlue;  		// 0x87cefa
		const static Color8 	LightSlateGray;  	// 0x778899
		const static Color8 	LightSlateGrey;  	// 0x778899
		const static Color8 	LightSteelBlue;  	// 0xb0c4dE
		const static Color8 	LightYellow;  		// 0xffffe0
		const static Color8 	Lime;  				// 0x00ff00
		const static Color8 	LimeGreen;  		// 0x32cd32
		const static Color8 	Linen;  			// 0xfaf0e6
		const static Color8 	Magenta;  			// 0xff00ff
		const static Color8 	Maroon;  			// 0x800000
		const static Color8 	MediumAquamarine;  	// 0x66cdaA
		const static Color8 	MediumBlue;  		// 0x0000cd
		const static Color8 	MediumOrchid;		// 0xba55d3
		const static Color8 	MediumPurple;	 	// 0x9370d8
		const static Color8 	MediumSeaGreen;  	// 0x3cb371
		const static Color8 	MediumSlateBlue;  	// 0x7b68eE
		const static Color8 	MediumSpringGreen;  // 0x00fa9A
		const static Color8 	MediumTurquoise;  	// 0x48d1cC
		const static Color8 	MediumVioletRed;  	// 0xc71585
		const static Color8 	MidnightBlue;  		// 0x191970
		const static Color8 	MintCream;  		// 0xf5fffa
		const static Color8 	MistyRose;  		// 0xffe4e1
		const static Color8 	Moccasin;  			// 0xffe4b5
		const static Color8 	NavajoWhite;  		// 0xffdead
		const static Color8 	Navy;  				// 0x000080
		const static Color8 	OldLace;  			// 0xfdf5e6
		const static Color8 	Olive;  			// 0x808000
		const static Color8 	OliveDrab;  		// 0x6b8e23
		const static Color8 	Orange;  			// 0xffa500
		const static Color8 	OrangeRed;  		// 0xff4500
		const static Color8 	Orchid;  			// 0xda70d6
		const static Color8 	PaleGoldenrod;  	// 0xeee8aa
		const static Color8 	PaleGreen;  		// 0x98fb98
		const static Color8 	PaleTurquoise;  	// 0xafeeee
		const static Color8 	PaleVioletRed;  	// 0xd87093
		const static Color8 	PapayaWhip;  		// 0xffefd5
		const static Color8 	PeachPuff;  		// 0xffdab9
		const static Color8 	Peru;  				// 0xcd853f
		const static Color8 	Pink;  				// 0xffc0cb
		const static Color8 	Plum;  				// 0xdda0dd
		const static Color8 	PowderBlue;  		// 0xb0e0e6
		const static Color8 	Purple;  			// 0x800080
		const static Color8 	Red;  				// 0xff0000
		const static Color8 	RosyBrown;  		// 0xbc8f8f
		const static Color8 	RoyalBlue;  		// 0x4169e1
		const static Color8 	SaddleBrown; 		// 0x8b4513
		const static Color8 	Salmon;  			// 0xfa8072
		const static Color8 	SandyBrown;  		// 0xf4a460
		const static Color8 	SeaGreen;  			// 0x2e8b57
		const static Color8 	Seashell;  			// 0xfff5ee
		const static Color8 	Sienna;  			// 0xa0522d
		const static Color8 	Silver;  			// 0xc0c0c0
		const static Color8 	SkyBlue;  			// 0x87ceeb
		const static Color8 	SlateBlue;  		// 0x6a5acd
		const static Color8 	SlateGray;  		// 0x708090
		const static Color8 	SlateGrey;  		// 0x708090
		const static Color8 	Snow;  				// 0xfffafa
		const static Color8 	SpringGreen; 		// 0x00ff7f
		const static Color8 	SteelBlue;  		// 0x4682b4
		const static Color8 	Tan;  				// 0xd2b48c
		const static Color8 	Teal;  				// 0x008080
		const static Color8 	Thistle;  			// 0xd8bfd8
		const static Color8 	Tomato;  			// 0xff6347
		const static Color8 	Turquoise;  		// 0x40e0d0
		const static Color8 	Violet;  			// 0xee82ee
		const static Color8 	Wheat;  			// 0xf5deb3
		const static Color8 	White;  			// 0xffffff
		const static Color8 	WhiteSmoke;  		// 0xf5f5f5
		const static Color8 	Yellow;  			// 0xffff00
		const static Color8 	YellowGreen; 		// 0x9acd32


	};

	typedef Color8	Color;


	//____ HiColor ____________________________________________________________

	/**
		@brief Color with RGBA components in linear range from 0 to 4096.

		Specifies a color in ARGB - format, where the alpha component controls transparency.
		Range for the individual color components is 0 - 4096.
		Setting alpha to 0 gives full transparency while 4096 gives full opacity.

		Color range is linear, meaning that 2048 is half the maximum color intensity and
		that mathematical operations on colors gives expected results.

		The different components can be accessed individually through the r, g, b, and a members.

		For convenience, the color class also provides static, predefined colors matching the standard web - palette.

		This is the color format used internally by WonderGUI.
	*/


	class HiColor
	{
		friend class GfxBase;
	public:

		//.____ Creation __________________________________________

		inline	constexpr HiColor() : argb(0) {};
				HiColor(Color8 lowColor);
		explicit inline	constexpr HiColor(int r, int g, int b, int a = 4096);
		explicit inline	constexpr HiColor(float r, float g, float b, float a = 1.f);

		//.____ Misc ________________________________________________

		static HiColor	mix(HiColor color1, HiColor color2, int balance);			// Balance: 0 -> 4096
		static HiColor	invert(HiColor color, int grade);						// Grade: 0 -> 4096
		static HiColor	min(HiColor color1, HiColor color2);
		static HiColor	max(HiColor color1, HiColor color2);
		static HiColor	blend(HiColor baseColor, HiColor blendColor, BlendMode operation);

		inline bool		isOpaqueWhite() const { return argb == 0x1000100010001000; }
		inline bool		isOpaqueBlack() const { return argb == 0x1000000000000000; }
		inline bool		isOpaque() const { return a == 4096; }
		inline bool		isFullyTransparent() const { return a == 0; }
		inline bool		isUndefined() const { return a < 0; }
		inline bool		isValid() const { return (argb & 0xE000E000E000E000) == 0 && a <= 4096 && r <= 4096 && g <= 4096 && b <= 4096; }

		inline void		clamp();

		//.____ Operators ___________________________________________

		operator Color8() const;

		inline bool operator==(const HiColor& k) const;
		inline bool operator!=(const HiColor& k) const;

		HiColor	operator+(const HiColor& k) const;
		HiColor	operator-(const HiColor& k) const;

		HiColor	operator*(float f) const;
		HiColor	operator*(const HiColor& k) const;

		HiColor& operator+=(const HiColor& k);
		HiColor& operator-=(const HiColor& k);

		HiColor& operator*=(float f);
		HiColor& operator*=(const HiColor& k);



		//.____ Internal ____________________________________________

		static int16_t		unpackSRGBTab[256];
		static int16_t		unpackLinearTab[256];

		static uint8_t		packSRGBTab[4097];
		static uint8_t		packLinearTab[4097];

		//.____ Properties __________________________________________

		union
		{
			struct
			{
				int16_t b;
				int16_t g;
				int16_t r;
				int16_t a;
			};
			int64_t argb;
		};


		//.____ Definitions ___________________________________________________

		const static HiColor	Undefined;			// -1
		const static HiColor	Transparent;		// 0x000000
		const static HiColor	TransparentBlack;	// 0x000000
		const static HiColor	TransparentWhite;	// 0x000000
		const static HiColor 	Black;  			// 0xffffff
		const static HiColor 	White;  			// 0xffffff

	private:
		static void	_initTables();
	};

	//-------------------------------------------------------------------
	inline bool Color8::operator==( const Color8& k ) const
	{
		if( argb == k.argb )
			return true;

		return false;
	}

	//-------------------------------------------------------------------
	inline bool Color8::operator!=( const Color8& k ) const
	{
		if( argb != k.argb )
			return true;

		return false;
	}



	//____ Color8::Color8() _________________________________________________________________

	inline constexpr Color8::Color8( const Color8& _col )
	: argb(_col.argb)
	{
	}

	inline constexpr Color8::Color8( uint32_t _argb )
	: argb(_argb)
	{
	}

	inline constexpr Color8::Color8( uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a )
	: b(_b), g(_g), r(_r), a(_a)
	{
	}

	inline constexpr Color8::Color8( uint32_t rgb, uint8_t _a )
	: argb( rgb | (uint32_t(_a) << 24))
	{
	}

	//-------------------------------------------------------------------
	void HiColor::clamp()
	{
		limit(r, 0, 4096);
		limit(g, 0, 4096);
		limit(b, 0, 4096);
		limit(a, 0, 4096);
	}

	//-------------------------------------------------------------------
	inline bool HiColor::operator==(const HiColor& k) const
	{
		if (argb == k.argb)
			return true;

		return false;
	}

	//-------------------------------------------------------------------
	inline bool HiColor::operator!=(const HiColor& k) const
	{
		if (argb != k.argb)
			return true;

		return false;
	}



	//____ HiColor::HiColor() _________________________________________________________________

	inline constexpr HiColor::HiColor(int r, int g, int b, int a)
	: b(b), g(g), r(r), a(a)
	{
	}

	inline constexpr HiColor::HiColor(float r, float g, float b, float a)
	: b(int16_t(b * 4096)), g(int16_t(g * 4096)), r(int16_t(r * 4096)), a(int16_t(a * 4096))
	{
	}


} // namespace wg
#endif // WG_COLOR_DOT_H
