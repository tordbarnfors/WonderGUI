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
#ifndef	WAX_COLOR_DOT_H
#define	WAX_COLOR_DOT_H
#pragma once

#include	<wax_types.h>

namespace wax
{

	//____ Class Color8 _______________________________________________________________________

	/**
		@brief Color with RGBA components in the range of 0-255.

		Specifies a color in ARGB-format, where the alpha component controls transparency.
		Range for the individual color components is 0-255.

		Setting alpha to 0 gives full transparency while 255 gives full opacity.

		The different components can be accessed individually through the r, g, b and a members,
		or as a group through the argb member.

		Wax uses Color for all API calls and internal operations, so Color8 is only provided
		for developer convenience since colors often are expressed in this range.

		Color channels are sRGB, the alpha channel is linear. Color8 and Color only differ in
		resolution, so converting between them doesn't change the color.

		The predefined colors of the standard web palette are in Color, e.g. Color::Cyan.
		Converting one to Color8 gives the 8-bit value of the palette.

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



	};


	//____ Color ____________________________________________________________

	/**
		@brief Color with RGBA components in range from 0 to 4096.

		Specifies a color in ARGB - format, where the alpha component controls transparency.
		Range for the individual color components is 0 - 4096.
		Setting alpha to 0 gives full transparency while 4096 gives full opacity.

		Color channels are sRGB, just like Color8, but with higher resolution. The alpha channel
		is linear. Mixing and interpolation of Colors (Color::mix(), transitions, Tint gradients)
		therefore happens in sRGB space, while blending of pixels when rendering happens in linear space.

		The different components can be accessed individually through the r, g, b, and a members.

		For convenience, the color class also provides static, predefined colors matching the standard web palette.
		They are constant-initialized, so they can be used before Base::init(), unlike a Color converted from
		a Color8.

		This is the color format used internally by Wax.
	*/


	class Color
	{
		friend class Base;
	public:

		//.____ Creation __________________________________________

		inline	constexpr Color() : argb(0) {};
				Color(Color8 lowColor);
		explicit inline	constexpr Color(int r, int g, int b, int a = 4096);
		explicit inline	constexpr Color(float r, float g, float b, float a = 1.f);

		//.____ Misc ________________________________________________

		static Color	mix(Color color1, Color color2, int balance);			// Balance: 0 -> 4096
		static Color	invert(Color color, int grade);						// Grade: 0 -> 4096
		static Color	min(Color color1, Color color2);
		static Color	max(Color color1, Color color2);
		static Color	blend(Color baseColor, Color blendColor, BlendMode operation);

		inline bool		isOpaqueWhite() const { return argb == 0x1000100010001000; }
		inline bool		isOpaqueBlack() const { return argb == 0x1000000000000000; }
		inline bool		isOpaque() const { return a == 4096; }
		inline bool		isFullyTransparent() const { return a == 0; }
		inline bool		isUndefined() const { return a < 0; }
		inline bool		isValid() const { return (argb & 0xE000E000E000E000) == 0 && a <= 4096 && r <= 4096 && g <= 4096 && b <= 4096; }

		inline void		clamp();
		inline Color	withAlpha(int alpha);

		//.____ Operators ___________________________________________

		operator Color8() const;

		inline bool operator==(const Color& k) const;
		inline bool operator!=(const Color& k) const;

		Color	operator+(const Color& k) const;
		Color	operator-(const Color& k) const;

		Color	operator*(float f) const;
		Color	operator*(const Color& k) const;

		Color& operator+=(const Color& k);
		Color& operator-=(const Color& k);

		Color& operator*=(float f);
		Color& operator*=(const Color& k);



		//.____ Internal ____________________________________________

		inline Color		toLinear() const;		// RGB from sRGB to linear, for blending. Alpha is kept.
		inline Color		toSRGB() const;			// RGB from linear to sRGB. Alpha is kept.
		inline void			toLinearFloat(float* pOut) const;	// Linear RGBA as four floats (0.0-1.0), for GPU backends.

		// Conversion between 8-bit and 12-bit (0-4096) channels. Names tell the color space of the
		// 8-bit side, while the 12-bit side always is linear. unpackLinearTab and packLinearTab
		// therefore just changes resolution and are also used for alpha and for Color8 <-> Color.

		static int16_t		unpackSRGBTab[256];
		static int16_t		unpackLinearTab[256];

		static uint8_t		packSRGBTab[4097];
		static uint8_t		packLinearTab[4097];

		// Conversion of 12-bit (0-4096) channels between sRGB and linear.

		static int16_t		sRGBToLinearTab[4097];
		static int16_t		linearToSRGBTab[4097];

		static float		sRGBToLinearFloatTab[4097];		// 12-bit sRGB to linear 0.0-1.0, without the loss of precision of sRGBToLinearTab.

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

		const static Color	Undefined;				// All channels -1
		const static Color	ShowError;				// Pink, for what is missing
		const static Color	Transparent;			// 0x00000000
		const static Color	TransparentBlack;		// 0x00000000
		const static Color	TransparentWhite;		// 0x00ffffff

		// The standard web palette

		const static Color	AliceBlue;				// 0xf0f8ff
		const static Color	AntiqueWhite;			// 0xfaebd7
		const static Color	Aqua;					// 0x00ffff
		const static Color	Aquamarine;				// 0x7fffd4
		const static Color	Azure;					// 0xf0ffff
		const static Color	Beige;					// 0xf5f5dc
		const static Color	Bisque;					// 0xffe4c4
		const static Color	Black;					// 0x000000
		const static Color	BlanchedAlmond;			// 0xffebcd
		const static Color	Blue;					// 0x0000ff
		const static Color	BlueViolet;				// 0x8a2be2
		const static Color	Brown;					// 0xa52a2a
		const static Color	Burlywood;				// 0xdeb887
		const static Color	CadetBlue;				// 0x5f9ea0
		const static Color	Chartreuse;				// 0x7fff00
		const static Color	Chocolate;				// 0xd2691e
		const static Color	Coral;					// 0xff7f50
		const static Color	CornflowerBlue;			// 0x6495ed
		const static Color	Cornsilk;				// 0xfff8dc
		const static Color	Crimson;				// 0xdc143c
		const static Color	Cyan;					// 0x00ffff
		const static Color	DarkBlue;				// 0x00008b
		const static Color	DarkCyan;				// 0x008b8b
		const static Color	DarkGoldenrod;			// 0xb8860b
		const static Color	DarkGray;				// 0xa9a9a9
		const static Color	DarkGrey;				// 0xa9a9a9
		const static Color	DarkGreen;				// 0x006400
		const static Color	DarkKhaki;				// 0xbdb76b
		const static Color	DarkMagenta;			// 0x8b008b
		const static Color	DarkOliveGreen;			// 0x556b2f
		const static Color	DarkOrange;				// 0xff8c00
		const static Color	DarkOrchid;				// 0x9932cc
		const static Color	DarkRed;				// 0x8b0000
		const static Color	DarkSalmon;				// 0xe9967a
		const static Color	DarkSeaGreen;			// 0x8fbc8f
		const static Color	DarkSlateBlue;			// 0x483d8b
		const static Color	DarkSlateGray;			// 0x2f4f4f
		const static Color	DarkSlateGrey;			// 0x2f4f4f
		const static Color	DarkTurquoise;			// 0x00ced1
		const static Color	DarkViolet;				// 0x9400d3
		const static Color	DeepPink;				// 0xff1493
		const static Color	DeepSkyBlue;			// 0x00bfff
		const static Color	DimGray;				// 0x696969
		const static Color	DimGrey;				// 0x696969
		const static Color	DodgerBlue;				// 0x1e90ff
		const static Color	FireBrick;				// 0xb22222
		const static Color	FloralWhite;			// 0xfffaf0
		const static Color	ForestGreen;			// 0x228b22
		const static Color	Fuchsia;				// 0xff00ff
		const static Color	Gainsboro;				// 0xdcdcdc
		const static Color	GhostWhite;				// 0xf8f8ff
		const static Color	Gold;					// 0xffd700
		const static Color	Goldenrod;				// 0xdaa520
		const static Color	Gray;					// 0x808080
		const static Color	Grey;					// 0x808080
		const static Color	Green;					// 0x008000
		const static Color	GreenYellow;			// 0xadff2f
		const static Color	Honeydew;				// 0xf0fff0
		const static Color	HotPink;				// 0xff69b4
		const static Color	IndianRed;				// 0xcd5c5c
		const static Color	Indigo;					// 0x4b0082
		const static Color	Ivory;					// 0xfffff0
		const static Color	Khaki;					// 0xf0e68c
		const static Color	Lavender;				// 0xe6e6fa
		const static Color	LavenderBlush;			// 0xfff0f5
		const static Color	LawnGreen;				// 0x7cfc00
		const static Color	LemonChiffon;			// 0xfffacd
		const static Color	LightBlue;				// 0xadd8e6
		const static Color	LightCoral;				// 0xf08080
		const static Color	LightCyan;				// 0xe0ffff
		const static Color	LightGoldenrodYellow;	// 0xfafad2
		const static Color	LightGray;				// 0xd3d3d3
		const static Color	LightGrey;				// 0xd3d3d3
		const static Color	LightGreen;				// 0x90ee90
		const static Color	LightPink;				// 0xffb6c1
		const static Color	LightSalmon;			// 0xffa07a
		const static Color	LightSeaGreen;			// 0x20b2aa
		const static Color	LightSkyBlue;			// 0x87cefa
		const static Color	LightSlateGray;			// 0x778899
		const static Color	LightSlateGrey;			// 0x778899
		const static Color	LightSteelBlue;			// 0xb0c4de
		const static Color	LightYellow;			// 0xffffe0
		const static Color	Lime;					// 0x00ff00
		const static Color	LimeGreen;				// 0x32cd32
		const static Color	Linen;					// 0xfaf0e6
		const static Color	Magenta;				// 0xff00ff
		const static Color	Maroon;					// 0x800000
		const static Color	MediumAquamarine;		// 0x66cdaa
		const static Color	MediumBlue;				// 0x0000cd
		const static Color	MediumOrchid;			// 0xba55d3
		const static Color	MediumPurple;			// 0x9370d8
		const static Color	MediumSeaGreen;			// 0x3cb371
		const static Color	MediumSlateBlue;		// 0x7b68ee
		const static Color	MediumSpringGreen;		// 0x00fa9a
		const static Color	MediumTurquoise;		// 0x48d1cc
		const static Color	MediumVioletRed;		// 0xc71585
		const static Color	MidnightBlue;			// 0x191970
		const static Color	MintCream;				// 0xf5fffa
		const static Color	MistyRose;				// 0xffe4e1
		const static Color	Moccasin;				// 0xffe4b5
		const static Color	NavajoWhite;			// 0xffdead
		const static Color	Navy;					// 0x000080
		const static Color	OldLace;				// 0xfdf5e6
		const static Color	Olive;					// 0x808000
		const static Color	OliveDrab;				// 0x6b8e23
		const static Color	Orange;					// 0xffa500
		const static Color	OrangeRed;				// 0xff4500
		const static Color	Orchid;					// 0xda70d6
		const static Color	PaleGoldenrod;			// 0xeee8aa
		const static Color	PaleGreen;				// 0x98fb98
		const static Color	PaleTurquoise;			// 0xafeeee
		const static Color	PaleVioletRed;			// 0xd87093
		const static Color	PapayaWhip;				// 0xffefd5
		const static Color	PeachPuff;				// 0xffdab9
		const static Color	Peru;					// 0xcd853f
		const static Color	Pink;					// 0xffc0cb
		const static Color	Plum;					// 0xdda0dd
		const static Color	PowderBlue;				// 0xb0e0e6
		const static Color	Purple;					// 0x800080
		const static Color	Red;					// 0xff0000
		const static Color	RosyBrown;				// 0xbc8f8f
		const static Color	RoyalBlue;				// 0x4169e1
		const static Color	SaddleBrown;			// 0x8b4513
		const static Color	Salmon;					// 0xfa8072
		const static Color	SandyBrown;				// 0xf4a460
		const static Color	SeaGreen;				// 0x2e8b57
		const static Color	Seashell;				// 0xfff5ee
		const static Color	Sienna;					// 0xa0522d
		const static Color	Silver;					// 0xc0c0c0
		const static Color	SkyBlue;				// 0x87ceeb
		const static Color	SlateBlue;				// 0x6a5acd
		const static Color	SlateGray;				// 0x708090
		const static Color	SlateGrey;				// 0x708090
		const static Color	Snow;					// 0xfffafa
		const static Color	SpringGreen;			// 0x00ff7f
		const static Color	SteelBlue;				// 0x4682b4
		const static Color	Tan;					// 0xd2b48c
		const static Color	Teal;					// 0x008080
		const static Color	Thistle;				// 0xd8bfd8
		const static Color	Tomato;					// 0xff6347
		const static Color	Turquoise;				// 0x40e0d0
		const static Color	Violet;					// 0xee82ee
		const static Color	Wheat;					// 0xf5deb3
		const static Color	White;					// 0xffffff
		const static Color	WhiteSmoke;				// 0xf5f5f5
		const static Color	Yellow;					// 0xffff00
		const static Color	YellowGreen;			// 0x9acd32

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
	void Color::clamp()
	{
		limit(r, 0, 4096);
		limit(g, 0, 4096);
		limit(b, 0, 4096);
		limit(a, 0, 4096);
	}

	//-------------------------------------------------------------------
	inline Color Color::withAlpha(int alpha)
	{
		return Color{ r,g,b,alpha };
	}

	//-------------------------------------------------------------------
	inline Color Color::toLinear() const
	{
		auto clamp = [](int v) { return v < 0 ? 0 : v > 4096 ? 4096 : v; };

		Color out;
		out.r = sRGBToLinearTab[clamp(r)];
		out.g = sRGBToLinearTab[clamp(g)];
		out.b = sRGBToLinearTab[clamp(b)];
		out.a = a;
		return out;
	}

	//-------------------------------------------------------------------
	inline void Color::toLinearFloat(float* pOut) const
	{
		auto clamp = [](int v) { return v < 0 ? 0 : v > 4096 ? 4096 : v; };

		pOut[0] = sRGBToLinearFloatTab[clamp(r)];
		pOut[1] = sRGBToLinearFloatTab[clamp(g)];
		pOut[2] = sRGBToLinearFloatTab[clamp(b)];
		pOut[3] = a / 4096.f;
	}

	//-------------------------------------------------------------------
	inline Color Color::toSRGB() const
	{
		auto clamp = [](int v) { return v < 0 ? 0 : v > 4096 ? 4096 : v; };

		Color out;
		out.r = linearToSRGBTab[clamp(r)];
		out.g = linearToSRGBTab[clamp(g)];
		out.b = linearToSRGBTab[clamp(b)];
		out.a = a;
		return out;
	}

	//-------------------------------------------------------------------
	inline bool Color::operator==(const Color& k) const
	{
		if (argb == k.argb)
			return true;

		return false;
	}

	//-------------------------------------------------------------------
	inline bool Color::operator!=(const Color& k) const
	{
		if (argb != k.argb)
			return true;

		return false;
	}



	//____ Color::Color() _________________________________________________________________

	inline constexpr Color::Color(int r, int g, int b, int a)
	: b(b), g(g), r(r), a(a)
	{
	}

	inline constexpr Color::Color(float r, float g, float b, float a)
	: b(int16_t(b * 4096)), g(int16_t(g * 4096)), r(int16_t(r * 4096)), a(int16_t(a * 4096))
	{
	}


} // namespace wax
#endif // WAX_COLOR_DOT_H
