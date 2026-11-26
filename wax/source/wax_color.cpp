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
#include <wax_color.h>
#include <wax_util.h>
#include <wax_base.h>

#include <algorithm>
#include <math.h>

namespace wax
{


	const Color Color::Undefined(-1, -1, -1, -1);
	const Color Color::ShowError(4096, 0, 2048, 4096);
	const Color Color::Transparent(0, 0, 0, 0);
	const Color Color::TransparentBlack(0, 0, 0, 0);
	const Color Color::TransparentWhite(4096, 4096, 4096, 0);

	// The standard web palette, as 12-bit channels: round(value / 255 * 4096).

	const Color Color::AliceBlue			( 3855, 3984, 4096 );		// 0xf0f8ff
	const Color Color::AntiqueWhite			( 4016, 3775, 3453 );		// 0xfaebd7
	const Color Color::Aqua					( 0, 4096, 4096 );		// 0x00ffff
	const Color Color::Aquamarine			( 2040, 4096, 3405 );		// 0x7fffd4
	const Color Color::Azure				( 3855, 4096, 4096 );		// 0xf0ffff
	const Color Color::Beige				( 3935, 3935, 3534 );		// 0xf5f5dc
	const Color Color::Bisque				( 4096, 3662, 3148 );		// 0xffe4c4
	const Color Color::Black				( 0, 0, 0 );		// 0x000000
	const Color Color::BlanchedAlmond		( 4096, 3775, 3293 );		// 0xffebcd
	const Color Color::Blue					( 0, 0, 4096 );		// 0x0000ff
	const Color Color::BlueViolet			( 2217, 691, 3630 );		// 0x8a2be2
	const Color Color::Brown				( 2650, 675, 675 );		// 0xa52a2a
	const Color Color::Burlywood			( 3566, 2956, 2168 );		// 0xdeb887
	const Color Color::CadetBlue			( 1526, 2538, 2570 );		// 0x5f9ea0
	const Color Color::Chartreuse			( 2040, 4096, 0 );		// 0x7fff00
	const Color Color::Chocolate			( 3373, 1687, 482 );		// 0xd2691e
	const Color Color::Coral				( 4096, 2040, 1285 );		// 0xff7f50
	const Color Color::CornflowerBlue		( 1606, 2393, 3807 );		// 0x6495ed
	const Color Color::Cornsilk				( 4096, 3984, 3534 );		// 0xfff8dc
	const Color Color::Crimson				( 3534, 321, 964 );		// 0xdc143c
	const Color Color::Cyan					( 0, 4096, 4096 );		// 0x00ffff
	const Color Color::DarkBlue				( 0, 0, 2233 );		// 0x00008b
	const Color Color::DarkCyan				( 0, 2233, 2233 );		// 0x008b8b
	const Color Color::DarkGoldenrod		( 2956, 2152, 177 );		// 0xb8860b
	const Color Color::DarkGray				( 2715, 2715, 2715 );		// 0xa9a9a9
	const Color Color::DarkGrey				( 2715, 2715, 2715 );		// 0xa9a9a9
	const Color Color::DarkGreen			( 0, 1606, 0 );		// 0x006400
	const Color Color::DarkKhaki			( 3036, 2939, 1719 );		// 0xbdb76b
	const Color Color::DarkMagenta			( 2233, 0, 2233 );		// 0x8b008b
	const Color Color::DarkOliveGreen		( 1365, 1719, 755 );		// 0x556b2f
	const Color Color::DarkOrange			( 4096, 2249, 0 );		// 0xff8c00
	const Color Color::DarkOrchid			( 2458, 803, 3277 );		// 0x9932cc
	const Color Color::DarkRed				( 2233, 0, 0 );		// 0x8b0000
	const Color Color::DarkSalmon			( 3743, 2409, 1960 );		// 0xe9967a
	const Color Color::DarkSeaGreen			( 2297, 3020, 2297 );		// 0x8fbc8f
	const Color Color::DarkSlateBlue		( 1157, 980, 2233 );		// 0x483d8b
	const Color Color::DarkSlateGray		( 755, 1269, 1269 );		// 0x2f4f4f
	const Color Color::DarkSlateGrey		( 755, 1269, 1269 );		// 0x2f4f4f
	const Color Color::DarkTurquoise		( 0, 3309, 3357 );		// 0x00ced1
	const Color Color::DarkViolet			( 2377, 0, 3389 );		// 0x9400d3
	const Color Color::DeepPink				( 4096, 321, 2361 );		// 0xff1493
	const Color Color::DeepSkyBlue			( 0, 3068, 4096 );		// 0x00bfff
	const Color Color::DimGray				( 1687, 1687, 1687 );		// 0x696969
	const Color Color::DimGrey				( 1687, 1687, 1687 );		// 0x696969
	const Color Color::DodgerBlue			( 482, 2313, 4096 );		// 0x1e90ff
	const Color Color::FireBrick			( 2859, 546, 546 );		// 0xb22222
	const Color Color::FloralWhite			( 4096, 4016, 3855 );		// 0xfffaf0
	const Color Color::ForestGreen			( 546, 2233, 546 );		// 0x228b22
	const Color Color::Fuchsia				( 4096, 0, 4096 );		// 0xff00ff
	const Color Color::Gainsboro			( 3534, 3534, 3534 );		// 0xdcdcdc
	const Color Color::GhostWhite			( 3984, 3984, 4096 );		// 0xf8f8ff
	const Color Color::Gold					( 4096, 3453, 0 );		// 0xffd700
	const Color Color::Goldenrod			( 3502, 2650, 514 );		// 0xdaa520
	const Color Color::Gray					( 2056, 2056, 2056 );		// 0x808080
	const Color Color::Grey					( 2056, 2056, 2056 );		// 0x808080
	const Color Color::Green				( 0, 2056, 0 );		// 0x008000
	const Color Color::GreenYellow			( 2779, 4096, 755 );		// 0xadff2f
	const Color Color::Honeydew				( 3855, 4096, 3855 );		// 0xf0fff0
	const Color Color::HotPink				( 4096, 1687, 2891 );		// 0xff69b4
	const Color Color::IndianRed			( 3293, 1478, 1478 );		// 0xcd5c5c
	const Color Color::Indigo				( 1205, 0, 2088 );		// 0x4b0082
	const Color Color::Ivory				( 4096, 4096, 3855 );		// 0xfffff0
	const Color Color::Khaki				( 3855, 3694, 2249 );		// 0xf0e68c
	const Color Color::Lavender				( 3694, 3694, 4016 );		// 0xe6e6fa
	const Color Color::LavenderBlush		( 4096, 3855, 3935 );		// 0xfff0f5
	const Color Color::LawnGreen			( 1992, 4048, 0 );		// 0x7cfc00
	const Color Color::LemonChiffon			( 4096, 4016, 3293 );		// 0xfffacd
	const Color Color::LightBlue			( 2779, 3470, 3694 );		// 0xadd8e6
	const Color Color::LightCoral			( 3855, 2056, 2056 );		// 0xf08080
	const Color Color::LightCyan			( 3598, 4096, 4096 );		// 0xe0ffff
	const Color Color::LightGoldenrodYellow	( 4016, 4016, 3373 );		// 0xfafad2
	const Color Color::LightGray			( 3389, 3389, 3389 );		// 0xd3d3d3
	const Color Color::LightGrey			( 3389, 3389, 3389 );		// 0xd3d3d3
	const Color Color::LightGreen			( 2313, 3823, 2313 );		// 0x90ee90
	const Color Color::LightPink			( 4096, 2923, 3100 );		// 0xffb6c1
	const Color Color::LightSalmon			( 4096, 2570, 1960 );		// 0xffa07a
	const Color Color::LightSeaGreen		( 514, 2859, 2731 );		// 0x20b2aa
	const Color Color::LightSkyBlue			( 2168, 3309, 4016 );		// 0x87cefa
	const Color Color::LightSlateGray		( 1911, 2185, 2458 );		// 0x778899
	const Color Color::LightSlateGrey		( 1911, 2185, 2458 );		// 0x778899
	const Color Color::LightSteelBlue		( 2827, 3148, 3566 );		// 0xb0c4de
	const Color Color::LightYellow			( 4096, 4096, 3598 );		// 0xffffe0
	const Color Color::Lime					( 0, 4096, 0 );		// 0x00ff00
	const Color Color::LimeGreen			( 803, 3293, 803 );		// 0x32cd32
	const Color Color::Linen				( 4016, 3855, 3694 );		// 0xfaf0e6
	const Color Color::Magenta				( 4096, 0, 4096 );		// 0xff00ff
	const Color Color::Maroon				( 2056, 0, 0 );		// 0x800000
	const Color Color::MediumAquamarine		( 1638, 3293, 2731 );		// 0x66cdaa
	const Color Color::MediumBlue			( 0, 0, 3293 );		// 0x0000cd
	const Color Color::MediumOrchid			( 2988, 1365, 3389 );		// 0xba55d3
	const Color Color::MediumPurple			( 2361, 1799, 3470 );		// 0x9370d8
	const Color Color::MediumSeaGreen		( 964, 2875, 1815 );		// 0x3cb371
	const Color Color::MediumSlateBlue		( 1976, 1671, 3823 );		// 0x7b68ee
	const Color Color::MediumSpringGreen	( 0, 4016, 2474 );		// 0x00fa9a
	const Color Color::MediumTurquoise		( 1157, 3357, 3277 );		// 0x48d1cc
	const Color Color::MediumVioletRed		( 3196, 337, 2136 );		// 0xc71585
	const Color Color::MidnightBlue			( 402, 402, 1799 );		// 0x191970
	const Color Color::MintCream			( 3935, 4096, 4016 );		// 0xf5fffa
	const Color Color::MistyRose			( 4096, 3662, 3614 );		// 0xffe4e1
	const Color Color::Moccasin				( 4096, 3662, 2907 );		// 0xffe4b5
	const Color Color::NavajoWhite			( 4096, 3566, 2779 );		// 0xffdead
	const Color Color::Navy					( 0, 0, 2056 );		// 0x000080
	const Color Color::OldLace				( 4064, 3935, 3694 );		// 0xfdf5e6
	const Color Color::Olive				( 2056, 2056, 0 );		// 0x808000
	const Color Color::OliveDrab			( 1719, 2281, 562 );		// 0x6b8e23
	const Color Color::Orange				( 4096, 2650, 0 );		// 0xffa500
	const Color Color::OrangeRed			( 4096, 1108, 0 );		// 0xff4500
	const Color Color::Orchid				( 3502, 1799, 3437 );		// 0xda70d6
	const Color Color::PaleGoldenrod		( 3823, 3727, 2731 );		// 0xeee8aa
	const Color Color::PaleGreen			( 2442, 4032, 2442 );		// 0x98fb98
	const Color Color::PaleTurquoise		( 2811, 3823, 3823 );		// 0xafeeee
	const Color Color::PaleVioletRed		( 3470, 1799, 2361 );		// 0xd87093
	const Color Color::PapayaWhip			( 4096, 3839, 3421 );		// 0xffefd5
	const Color Color::PeachPuff			( 4096, 3502, 2972 );		// 0xffdab9
	const Color Color::Peru					( 3293, 2136, 1012 );		// 0xcd853f
	const Color Color::Pink					( 4096, 3084, 3261 );		// 0xffc0cb
	const Color Color::Plum					( 3550, 2570, 3550 );		// 0xdda0dd
	const Color Color::PowderBlue			( 2827, 3598, 3694 );		// 0xb0e0e6
	const Color Color::Purple				( 2056, 0, 2056 );		// 0x800080
	const Color Color::Red					( 4096, 0, 0 );		// 0xff0000
	const Color Color::RosyBrown			( 3020, 2297, 2297 );		// 0xbc8f8f
	const Color Color::RoyalBlue			( 1044, 1687, 3614 );		// 0x4169e1
	const Color Color::SaddleBrown			( 2233, 1108, 305 );		// 0x8b4513
	const Color Color::Salmon				( 4016, 2056, 1831 );		// 0xfa8072
	const Color Color::SandyBrown			( 3919, 2634, 1542 );		// 0xf4a460
	const Color Color::SeaGreen				( 739, 2233, 1397 );		// 0x2e8b57
	const Color Color::Seashell				( 4096, 3935, 3823 );		// 0xfff5ee
	const Color Color::Sienna				( 2570, 1317, 723 );		// 0xa0522d
	const Color Color::Silver				( 3084, 3084, 3084 );		// 0xc0c0c0
	const Color Color::SkyBlue				( 2168, 3309, 3775 );		// 0x87ceeb
	const Color Color::SlateBlue			( 1703, 1446, 3293 );		// 0x6a5acd
	const Color Color::SlateGray			( 1799, 2056, 2313 );		// 0x708090
	const Color Color::SlateGrey			( 1799, 2056, 2313 );		// 0x708090
	const Color Color::Snow					( 4096, 4016, 4016 );		// 0xfffafa
	const Color Color::SpringGreen			( 0, 4096, 2040 );		// 0x00ff7f
	const Color Color::SteelBlue			( 1124, 2088, 2891 );		// 0x4682b4
	const Color Color::Tan					( 3373, 2891, 2249 );		// 0xd2b48c
	const Color Color::Teal					( 0, 2056, 2056 );		// 0x008080
	const Color Color::Thistle				( 3470, 3068, 3470 );		// 0xd8bfd8
	const Color Color::Tomato				( 4096, 1590, 1140 );		// 0xff6347
	const Color Color::Turquoise			( 1028, 3598, 3341 );		// 0x40e0d0
	const Color Color::Violet				( 3823, 2088, 3823 );		// 0xee82ee
	const Color Color::Wheat				( 3935, 3566, 2875 );		// 0xf5deb3
	const Color Color::White				( 4096, 4096, 4096 );		// 0xffffff
	const Color Color::WhiteSmoke			( 3935, 3935, 3935 );		// 0xf5f5f5
	const Color Color::Yellow				( 4096, 4096, 0 );		// 0xffff00
	const Color Color::YellowGreen			( 2474, 3293, 803 );		// 0x9acd32


	int16_t		Color::unpackSRGBTab[256];
	int16_t		Color::unpackLinearTab[256];

	uint8_t		Color::packSRGBTab[4097];
	uint8_t		Color::packLinearTab[4097];

	int16_t		Color::sRGBToLinearTab[4097];
	int16_t		Color::linearToSRGBTab[4097];

	float		Color::sRGBToLinearFloatTab[4097];


	//-------------------------------------------------------------------
	Color8 Color8::operator+(const Color8& k) const
	{
		Color8 kNewColor;
		kNewColor.r = Util::limitUint8(((int)r) + k.r);
		kNewColor.g = Util::limitUint8(((int)g) + k.g);
		kNewColor.b = Util::limitUint8(((int)b) + k.b);
		kNewColor.a = Util::limitUint8(((int)a) + k.a);
		return kNewColor;
	}

	//-------------------------------------------------------------------
	Color8 Color8::operator-(const Color8& k) const
	{
		Color8 kNewColor;
		kNewColor.r = Util::limitUint8(((int)r) - k.r);
		kNewColor.g = Util::limitUint8(((int)g) - k.g);
		kNewColor.b = Util::limitUint8(((int)b) - k.b);
		kNewColor.a = Util::limitUint8(((int)a) - k.a);
		return kNewColor;
	}

	//-------------------------------------------------------------------
	Color8 Color8::operator*(float f) const
	{
		Color8 kNewColor;
		kNewColor.r = (uint8_t)((float)r * f);
		kNewColor.g = (uint8_t)((float)g * f);
		kNewColor.b = (uint8_t)((float)b * f);
		kNewColor.a = (uint8_t)((float)a * f);
		return kNewColor;
	}

	//-------------------------------------------------------------------
	Color8 Color8::operator*(const Color8& k) const
	{
		Color8 kNewColor;
		kNewColor.r = (uint8_t)(((int)r * (int)k.r) / 255);
		kNewColor.g = (uint8_t)(((int)g * (int)k.g) / 255);
		kNewColor.b = (uint8_t)(((int)b * (int)k.b) / 255);
		kNewColor.a = (uint8_t)(((int)a * (int)k.a) / 255);
		return kNewColor;
	}

/*
	//____ setCMYK ___________________________________________________________________

	void Color::setCMYK( float c, float m, float y, float k, uint8_t alpha )
	{
		r = (uint8_t) (255*(1.f-c)*(1.f-k));
		g = (uint8_t) (255*(1.f-m)*(1.f-k));
		b = (uint8_t) (255*(1.f-y)*(1.f-k));
		a = alpha;
	}


	//____ getCMYK() _______________________________________________________________

	void Color::getCMYK( float* _c, float* _m, float* _y, float* _k )
	{
		float fr = r / 255.f;
		float fg = r / 255.f;
		float fb = r / 255.f;

		float k = 1.f - wax::max( fr, fg, fb );
		* _c = (1.f-fr-k) / (1.f-k);
		* _m = (1.f-fg-k) / (1.f-k);
		* _y = (1.f-fb-k) / (1.f-k);
		* _k = k;
	}
*/

	//____ constructor ________________________________________________________

	Color::Color(Color8 lowColor)
	{
		r = unpackLinearTab[lowColor.r];
		g = unpackLinearTab[lowColor.g];
		b = unpackLinearTab[lowColor.b];
		a = unpackLinearTab[lowColor.a];
	}


	//____ blend() ________________________________________________________________

	Color Color::mix( Color color1, Color color2, int balance )
	{
		Color col;

		col.r = color1.r + ((int(color2.r) - int(color1.r)) * balance / 4096);
		col.g = color1.g + ((int(color2.g) - int(color1.g)) * balance / 4096);
		col.b = color1.b + ((int(color2.b) - int(color1.b)) * balance / 4096);
		col.a = color1.a + ((int(color2.a) - int(color1.a)) * balance / 4096);

		return col;
	}

	//____ invert()_________________________________________________________________

	Color Color::invert( Color color, int grade )
	{
		color.r = ((4096-color.r)*grade + color.r*(4096-grade))/4096;
		color.g = ((4096-color.g)*grade + color.g*(4096-grade))/4096;
		color.b = ((4096-color.b)*grade + color.b*(4096-grade))/4096;
		return color;
	}

	//____ min() ___________________________________________________________________

	Color Color::min(Color color1, Color color2)
	{
		return Color(std::min(color1.r, color2.r),std::min(color1.g,color2.g),std::min(color1.b,color2.b),std::min(color1.a,color2.a));
	}

	//____ max() ____________________________________________________________________

	Color Color::max(Color color1, Color color2)
	{
		return Color(std::max(color1.r, color2.r), std::max(color1.g, color2.g), std::max(color1.b, color2.b), std::max(color1.a, color2.a));
	}


	//____ blend()___________________________________________________________________

	Color Color::blend( Color baseColor, Color blendColor, BlendMode operation )
	{
		switch( operation )
		{
			case BlendMode::Add:
				return baseColor + blendColor;
			case BlendMode::Morph:
				return mix(baseColor, blendColor, 2048);
			case BlendMode::Blend:
			case BlendMode::BlendFixedColor:
				return mix(baseColor, Color(blendColor.r, blendColor.g, blendColor.b, baseColor.a), blendColor.a);
			case BlendMode::Undefined:
			case BlendMode::Ignore:
				return baseColor;
			case BlendMode::Invert:
				return invert( baseColor, blendColor.a );
			case BlendMode::Multiply:
				return baseColor * blendColor;
			case BlendMode::Subtract:
				return baseColor - blendColor;
			case BlendMode::Min:
				return min(baseColor, blendColor);
			case BlendMode::Max:
				return max(baseColor, blendColor);
			case BlendMode::Replace:
			default:
				return blendColor;
		}
	}

	//____ operator Color8() ___________________________________________________

	Color::operator Color8() const
	{
		return Color8(packLinearTab[r], packLinearTab[g], packLinearTab[b], packLinearTab[a]);
	}

	//-------------------------------------------------------------------
	Color Color::operator+(const Color& k) const
	{
		Color kNewColor;
		kNewColor.r = r + k.r;
		kNewColor.g = g + k.g;
		kNewColor.b = b + k.b;
		kNewColor.a = a + k.a;

		return kNewColor;
	}

	//-------------------------------------------------------------------
	Color Color::operator-(const Color& k) const
	{
		Color kNewColor;
		kNewColor.r = r - k.r;
		kNewColor.g = g - k.g;
		kNewColor.b = b - k.b;
		kNewColor.a = a - k.a;

		return kNewColor;
	}

	//-------------------------------------------------------------------
	Color Color::operator*(float f) const
	{
		Color kNewColor;
		kNewColor.r = (uint16_t)((float)r * f);
		kNewColor.g = (uint16_t)((float)g * f);
		kNewColor.b = (uint16_t)((float)b * f);
		kNewColor.a = (uint16_t)((float)a * f);

		return kNewColor;
	}

	//-------------------------------------------------------------------
	Color Color::operator*(const Color& k) const
	{
		Color kNewColor;
		kNewColor.r = (uint16_t)(((int)r * (int)k.r) / 4096);
		kNewColor.g = (uint16_t)(((int)g * (int)k.g) / 4096);
		kNewColor.b = (uint16_t)(((int)b * (int)k.b) / 4096);
		kNewColor.a = (uint16_t)(((int)a * (int)k.a) / 4096);
		return kNewColor;
	}

	//-------------------------------------------------------------------
	Color& Color::operator+=(const Color& k)
	{
		r += k.r;
		g += k.g;
		b += k.b;
		a += k.a;

		return *this;
	}

	//-------------------------------------------------------------------
	Color& Color::operator-=(const Color& k)
	{
		r -= k.r;
		g -= k.g;
		b -= k.b;
		a -= k.a;

		return *this;
	}

	//-------------------------------------------------------------------
	Color& Color::operator*=(float f)
	{
		r = (uint16_t)((float)r * f);
		g = (uint16_t)((float)g * f);
		b = (uint16_t)((float)b * f);
		a = (uint16_t)((float)a * f);

		return *this;
	}


	//-------------------------------------------------------------------
	Color& Color::operator*=(const Color& k)
	{
		r = (uint16_t)(((int)r * (int)k.r) / 4096);
		g = (uint16_t)(((int)g * (int)k.g) / 4096);
		b = (uint16_t)(((int)b * (int)k.b) / 4096);
		a = (uint16_t)(((int)a * (int)k.a) / 4096);

		return *this;
	}


	//____ _initTables() ______________________________________________________

	// The sRGB transfer functions, as used by GPUs for sRGB textures and framebuffers.
	// A plain 2.2 power curve differs noticeably in dark tones.

	static double _sRGBToLinear(double c)
	{
		return c <= 0.04045 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
	}

	static double _linearToSRGB(double l)
	{
		return l <= 0.0031308 ? l * 12.92 : 1.055 * pow(l, 1 / 2.4) - 0.055;
	}

	void Color::_initTables()
	{
		for (int i = 0; i < 256; i++)
			Color::unpackSRGBTab[i] = int16_t(_sRGBToLinear(i / 255.0) * 4096 + 0.5);

		for (int i = 0; i < 256; i++)
			Color::unpackLinearTab[i] = int(i / 255.f * 4096 + 0.5f);

		for (int i = 0; i <= 4096; i++)
			Color::packSRGBTab[i] = uint8_t(_linearToSRGB(i / 4096.0) * 255 + 0.5);

		for (int i = 0; i <= 4096; i++)
			Color::packLinearTab[i] = uint8_t(i / 4096.f * 255.f + 0.5f);

		for (int i = 0; i <= 4096; i++)
		{
			Color::sRGBToLinearTab[i] = int16_t(_sRGBToLinear(i / 4096.0) * 4096 + 0.5);
			Color::linearToSRGBTab[i] = int16_t(_linearToSRGB(i / 4096.0) * 4096 + 0.5);
			Color::sRGBToLinearFloatTab[i] = float(_sRGBToLinear(i / 4096.0));
		}
	}


} // namespace wax
