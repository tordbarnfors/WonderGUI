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

#include <wg_color.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

#include <algorithm>
#include <math.h>

namespace wg
{

	const Color8 Color8::Transparent		( 0x00000000 );
	const Color8 Color8::AliceBlue		( 0xFFF0F8FF );
	const Color8 Color8::AntiqueWhite		( 0xFFFAEBD7 );
	const Color8 Color8::Aqua				( 0xFF00ffff );
	const Color8 Color8::Aquamarine		( 0xFF7fffd4 );
	const Color8 Color8::Azure			( 0xFFf0ffff );
	const Color8 Color8::Beige			( 0xFFf5f5dc );
	const Color8 Color8::Bisque  			( 0xFFffe4c4 );
	const Color8 Color8::Black	  		( 0xFF000000 );
	const Color8 Color8::BlanchedAlmond 	( 0xFFffebcD );
	const Color8 Color8::Blue 	  		( 0xFF0000ff );
	const Color8 Color8::BlueViolet 		( 0xFF8a2be2 );
	const Color8 Color8::Brown   			( 0xFFa52a2a );
	const Color8 Color8::Burlywood  		( 0xFFdeb887 );
	const Color8 Color8::CadetBlue  		( 0xFF5f9ea0 );
	const Color8 Color8::Chartreuse 		( 0xFF7fff00 );
	const Color8 Color8::Chocolate  		( 0xFFd2691e );
	const Color8 Color8::Coral   			( 0xFFff7f50 );
	const Color8 Color8::CornflowerBlue 	( 0xFF6495eD );
	const Color8 Color8::Cornsilk 		( 0xFFfff8dc );
	const Color8 Color8::Crimson  		( 0xFFdc143c );
	const Color8 Color8::Cyan  			( 0xFF00ffff );
	const Color8 Color8::DarkBlue  		( 0xFF00008b );
	const Color8 Color8::DarkCyan  		( 0xFF008b8b );
	const Color8 Color8::DarkGoldenrod  	( 0xFFb8860b );
	const Color8 Color8::DarkGray  		( 0xFFa9a9a9 );
	const Color8 Color8::DarkGrey  		( 0xFFa9a9a9 );
	const Color8 Color8::DarkGreen  		( 0xFF006400 );
	const Color8 Color8::DarkKhaki  		( 0xFFbdb76b );
	const Color8 Color8::DarkMagenta  	( 0xFF8b008b );
	const Color8 Color8::DarkOliveGreen 	( 0xFF556b2F );
	const Color8 Color8::DarkOrange  		( 0xFFff8c00 );
	const Color8 Color8::DarkOrchid  		( 0xFF9932cc );
	const Color8 Color8::DarkRed  		( 0xFF8b0000 );
	const Color8 Color8::DarkSalmon  		( 0xFFe9967a );
	const Color8 Color8::DarkSeaGreen  	( 0xFF8fbc8f );
	const Color8 Color8::DarkSlateBlue  	( 0xFF483d8b );
	const Color8 Color8::DarkSlateGray  	( 0xFF2f4f4f );
	const Color8 Color8::DarkSlateGrey  	( 0xFF2f4f4f );
	const Color8 Color8::DarkTurquoise  	( 0xFF00ced1 );
	const Color8 Color8::DarkViolet  		( 0xFF9400d3 );
	const Color8 Color8::DeepPink  		( 0xFFff1493 );
	const Color8 Color8::DeepSkyBlue 		( 0xFF00bfff );
	const Color8 Color8::DimGray  		( 0xFF696969 );
	const Color8 Color8::DimGrey  		( 0xFF696969 );
	const Color8 Color8::DodgerBlue  		( 0xFF1e90ff );
	const Color8 Color8::FireBrick  		( 0xFFb22222 );
	const Color8 Color8::FloralWhite 		( 0xFFfffaf0 );
	const Color8 Color8::ForestGreen 		( 0xFF228b22 );
	const Color8 Color8::Fuchsia  		( 0xFFff00ff );
	const Color8 Color8::Gainsboro  		( 0xFFdcdcdc );
	const Color8 Color8::GhostWhite  		( 0xFFf8f8ff );
	const Color8 Color8::Gold  			( 0xFFffd700 );
	const Color8 Color8::Goldenrod  		( 0xFFdaa520 );
	const Color8 Color8::Gray  			( 0xFF808080 );
	const Color8 Color8::Grey  			( 0xFF808080 );
	const Color8 Color8::Green  			( 0xFF008000 );
	const Color8 Color8::GreenYellow 		( 0xFFadff2f );
	const Color8 Color8::Honeydew  		( 0xFFf0fff0 );
	const Color8 Color8::HotPink  		( 0xFFff69b4 );
	const Color8 Color8::IndianRed   		( 0xFFcd5c5c );
	const Color8 Color8::Indigo   		( 0xFF4b0082 );
	const Color8 Color8::Ivory  			( 0xFFfffff0 );
	const Color8 Color8::Khaki  			( 0xFFf0e68c );
	const Color8 Color8::Lavender  		( 0xFFe6e6fa );
	const Color8 Color8::LavenderBlush  	( 0xFFfff0f5 );
	const Color8 Color8::LawnGreen  		( 0xFF7cfc00 );
	const Color8 Color8::LemonCiffon		( 0xFFfffacd );
	const Color8 Color8::LightBlue  		( 0xFFadd8e6 );
	const Color8 Color8::LightCoral  		( 0xFFf08080 );
	const Color8 Color8::LightCyan  		( 0xFFe0ffff );
	const Color8 Color8::LightGoldenrodYellow	( 0xFFfAFAD2 );
	const Color8 Color8::LightGray  		( 0xFFd3d3d3 );
	const Color8 Color8::LightGrey  		( 0xFFd3d3d3 );
	const Color8 Color8::LightGreen  		( 0xFF90ee90 );
	const Color8 Color8::LightPink  		( 0xFFffb6c1 );
	const Color8 Color8::LightSalmon  	( 0xFFffa07a );
	const Color8 Color8::LightSeaGreen  	( 0xFF20b2aa );
	const Color8 Color8::LightSkyBlue  	( 0xFF87cefa );
	const Color8 Color8::LightSlateGray 	( 0xFF778899 );
	const Color8 Color8::LightSlateGrey 	( 0xFF778899 );
	const Color8 Color8::LightSteelBlue 	( 0xFFb0c4dE );
	const Color8 Color8::LightYellow  	( 0xFFffffe0 );
	const Color8 Color8::Lime  			( 0xFF00ff00 );
	const Color8 Color8::LimeGreen  		( 0xFF32cd32 );
	const Color8 Color8::Linen  			( 0xFFfaf0e6 );
	const Color8 Color8::Magenta  		( 0xFFff00ff );
	const Color8 Color8::Maroon  			( 0xFF800000 );
	const Color8 Color8::MediumAquamarine ( 0xFF66cdaA );
	const Color8 Color8::MediumBlue  		( 0xFF0000cd );
	const Color8 Color8::MediumOrchid		( 0xFFba55d3 );
	const Color8 Color8::MediumPurple		( 0xFF9370d8 );
	const Color8 Color8::MediumSeaGreen 	( 0xFF3cb371 );
	const Color8 Color8::MediumSlateBlue  ( 0xFF7b68eE );
	const Color8 Color8::MediumSpringGreen( 0xFF00fa9A );
	const Color8 Color8::MediumTurquoise  ( 0xFF48d1cC );
	const Color8 Color8::MediumVioletRed  ( 0xFFc71585 );
	const Color8 Color8::MidnightBlue  	( 0xFF191970 );
	const Color8 Color8::MintCream  		( 0xFFf5fffa );
	const Color8 Color8::MistyRose  		( 0xFFffe4e1 );
	const Color8 Color8::Moccasin  		( 0xFFffe4b5 );
	const Color8 Color8::NavajoWhite  	( 0xFFffdead );
	const Color8 Color8::Navy  			( 0xFF000080 );
	const Color8 Color8::OldLace  		( 0xFFfdf5e6 );
	const Color8 Color8::Olive  			( 0xFF808000 );
	const Color8 Color8::OliveDrab  		( 0xFF6b8e23 );
	const Color8 Color8::Orange  			( 0xFFffa500 );
	const Color8 Color8::OrangeRed  		( 0xFFff4500 );
	const Color8 Color8::Orchid  			( 0xFFda70d6 );
	const Color8 Color8::PaleGoldenrod  	( 0xFFeee8aa );
	const Color8 Color8::PaleGreen  		( 0xFF98fb98 );
	const Color8 Color8::PaleTurquoise  	( 0xFFafeeee );
	const Color8 Color8::PaleVioletRed  	( 0xFFd87093 );
	const Color8 Color8::PapayaWhip  		( 0xFFffefd5 );
	const Color8 Color8::PeachPuff  		( 0xFFffdab9 );
	const Color8 Color8::Peru  			( 0xFFcd853f );
	const Color8 Color8::Pink  			( 0xFFffc0cb );
	const Color8 Color8::Plum  			( 0xFFdda0dd );
	const Color8 Color8::PowderBlue  		( 0xFFb0e0e6 );
	const Color8 Color8::Purple  			( 0xFF800080 );
	const Color8 Color8::Red  			( 0xFFff0000 );
	const Color8 Color8::RosyBrown  		( 0xFFbc8f8f );
	const Color8 Color8::RoyalBlue  		( 0xFF4169e1 );
	const Color8 Color8::SaddleBrown 		( 0xFF8b4513 );
	const Color8 Color8::Salmon  			( 0xFFfa8072 );
	const Color8 Color8::SandyBrown  		( 0xFFf4a460 );
	const Color8 Color8::SeaGreen  		( 0xFF2e8b57 );
	const Color8 Color8::Seashell  		( 0xFFfff5ee );
	const Color8 Color8::Sienna  			( 0xFFa0522d );
	const Color8 Color8::Silver  			( 0xFFc0c0c0 );
	const Color8 Color8::SkyBlue  		( 0xFF87ceeb );
	const Color8 Color8::SlateBlue  		( 0xFF6a5acd );
	const Color8 Color8::SlateGray  		( 0xFF708090 );
	const Color8 Color8::SlateGrey  		( 0xFF708090 );
	const Color8 Color8::Snow  			( 0xFFfffafa );
	const Color8 Color8::SpringGreen 		( 0xFF00ff7f );
	const Color8 Color8::SteelBlue  		( 0xFF4682b4 );
	const Color8 Color8::Tan  			( 0xFFd2b48c );
	const Color8 Color8::Teal  			( 0xFF008080 );
	const Color8 Color8::Thistle  		( 0xFFd8bfd8 );
	const Color8 Color8::Tomato  			( 0xFFff6347 );
	const Color8 Color8::Turquoise  		( 0xFF40e0d0 );
	const Color8 Color8::Violet  			( 0xFFee82ee );
	const Color8 Color8::Wheat  			( 0xFFf5deb3 );
	const Color8 Color8::White  			( 0xFFffffff );
	const Color8 Color8::WhiteSmoke  		( 0xFFf5f5f5 );
	const Color8 Color8::Yellow  			( 0xFFffff00 );
	const Color8 Color8::YellowGreen 		( 0xFF9acd32 );

	const HiColor HiColor::Undefined(-1, -1, -1, -1);
	const HiColor HiColor::Transparent(0, 0, 0, 0);
	const HiColor HiColor::TransparentBlack(0, 0, 0, 0);
	const HiColor HiColor::TransparentWhite(4096, 4096, 4096, 0);
	const HiColor HiColor::Black(0, 0, 0, 4096);
	const HiColor HiColor::White(4096, 4096, 4096, 4096);


	int16_t		HiColor::unpackSRGBTab[256];
	int16_t		HiColor::unpackLinearTab[256];

	uint8_t		HiColor::packSRGBTab[4097];
	uint8_t		HiColor::packLinearTab[4097];


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

	void HiColor::setCMYK( float c, float m, float y, float k, uint8_t alpha )
	{
		r = (uint8_t) (255*(1.f-c)*(1.f-k));
		g = (uint8_t) (255*(1.f-m)*(1.f-k));
		b = (uint8_t) (255*(1.f-y)*(1.f-k));
		a = alpha;
	}


	//____ getCMYK() _______________________________________________________________

	void HiColor::getCMYK( float* _c, float* _m, float* _y, float* _k )
	{
		float fr = r / 255.f;
		float fg = r / 255.f;
		float fb = r / 255.f;

		float k = 1.f - wg::max( fr, fg, fb );
		* _c = (1.f-fr-k) / (1.f-k);
		* _m = (1.f-fg-k) / (1.f-k);
		* _y = (1.f-fb-k) / (1.f-k);
		* _k = k;
	}
*/

	//____ constructor ________________________________________________________

	HiColor::HiColor(Color8 lowColor)
	{
		const int16_t* pUnpackTab = GfxBase::defaultToSRGB() ? unpackSRGBTab : unpackLinearTab;

		r = pUnpackTab[lowColor.r];
		g = pUnpackTab[lowColor.g];
		b = pUnpackTab[lowColor.b];
		a = unpackLinearTab[lowColor.a];
	}


	//____ blend() ________________________________________________________________

	HiColor HiColor::mix( HiColor color1, HiColor color2, int balance )
	{
		HiColor col;

		col.r = color1.r + ((int(color2.r) - int(color1.r)) * balance / 4096);
		col.g = color1.g + ((int(color2.g) - int(color1.g)) * balance / 4096);
		col.b = color1.b + ((int(color2.b) - int(color1.b)) * balance / 4096);
		col.a = color1.a + ((int(color2.a) - int(color1.a)) * balance / 4096);

		return col;
	}

	//____ invert()_________________________________________________________________

	HiColor HiColor::invert( HiColor color, int grade )
	{
		color.r = ((4096-color.r)*grade + color.r*(4096-grade))/4096;
		color.g = ((4096-color.g)*grade + color.g*(4096-grade))/4096;
		color.b = ((4096-color.b)*grade + color.b*(4096-grade))/4096;
		return color;
	}

	//____ min() ___________________________________________________________________

	HiColor HiColor::min(HiColor color1, HiColor color2)
	{
		return HiColor(std::min(color1.r, color2.r),std::min(color1.g,color2.g),std::min(color1.b,color2.b),std::min(color1.a,color2.a));
	}

	//____ max() ____________________________________________________________________

	HiColor HiColor::max(HiColor color1, HiColor color2)
	{
		return HiColor(std::max(color1.r, color2.r), std::max(color1.g, color2.g), std::max(color1.b, color2.b), std::max(color1.a, color2.a));
	}


	//____ blend()___________________________________________________________________

	HiColor HiColor::blend( HiColor baseColor, HiColor blendColor, BlendMode operation )
	{
		switch( operation )
		{
			case BlendMode::Add:
				return baseColor + blendColor;
			case BlendMode::Morph:
				return mix(baseColor, blendColor, 2048);
			case BlendMode::Blend:
			case BlendMode::BlendFixedColor:
				return mix(baseColor, HiColor(blendColor.r, blendColor.g, blendColor.b, baseColor.a), blendColor.a);
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

	HiColor::operator Color8() const
	{
		const uint8_t* pPackTab = GfxBase::defaultToSRGB() ? packSRGBTab : packLinearTab;
		return Color8(pPackTab[r], pPackTab[g], pPackTab[b], packLinearTab[a]);
	}

	//-------------------------------------------------------------------
	HiColor HiColor::operator+(const HiColor& k) const
	{
		HiColor kNewColor;
		kNewColor.r = r + k.r;
		kNewColor.g = g + k.g;
		kNewColor.b = b + k.b;
		kNewColor.a = a + k.a;

		return kNewColor;
	}

	//-------------------------------------------------------------------
	HiColor HiColor::operator-(const HiColor& k) const
	{
		HiColor kNewColor;
		kNewColor.r = r - k.r;
		kNewColor.g = g - k.g;
		kNewColor.b = b - k.b;
		kNewColor.a = a - k.a;

		return kNewColor;
	}

	//-------------------------------------------------------------------
	HiColor HiColor::operator*(float f) const
	{
		HiColor kNewColor;
		kNewColor.r = (uint16_t)((float)r * f);
		kNewColor.g = (uint16_t)((float)g * f);
		kNewColor.b = (uint16_t)((float)b * f);
		kNewColor.a = (uint16_t)((float)a * f);

		return kNewColor;
	}

	//-------------------------------------------------------------------
	HiColor HiColor::operator*(const HiColor& k) const
	{
		HiColor kNewColor;
		kNewColor.r = (uint16_t)(((int)r * (int)k.r) / 4096);
		kNewColor.g = (uint16_t)(((int)g * (int)k.g) / 4096);
		kNewColor.b = (uint16_t)(((int)b * (int)k.b) / 4096);
		kNewColor.a = (uint16_t)(((int)a * (int)k.a) / 4096);
		return kNewColor;
	}

	//-------------------------------------------------------------------
	HiColor& HiColor::operator+=(const HiColor& k)
	{
		r += k.r;
		g += k.g;
		b += k.b;
		a += k.a;

		return *this;
	}

	//-------------------------------------------------------------------
	HiColor& HiColor::operator-=(const HiColor& k)
	{
		r -= k.r;
		g -= k.g;
		b -= k.b;
		a -= k.a;

		return *this;
	}

	//-------------------------------------------------------------------
	HiColor& HiColor::operator*=(float f)
	{
		r = (uint16_t)((float)r * f);
		g = (uint16_t)((float)g * f);
		b = (uint16_t)((float)b * f);
		a = (uint16_t)((float)a * f);

		return *this;
	}


	//-------------------------------------------------------------------
	HiColor& HiColor::operator*=(const HiColor& k)
	{
		r = (uint16_t)(((int)r * (int)k.r) / 4096);
		g = (uint16_t)(((int)g * (int)k.g) / 4096);
		b = (uint16_t)(((int)b * (int)k.b) / 4096);
		a = (uint16_t)(((int)a * (int)k.a) / 4096);

		return *this;
	}


	//____ _initTables() ______________________________________________________

	void HiColor::_initTables()
	{
		float max = powf(255, 2.2f);

		for (int i = 0; i < 256; i++)
			HiColor::unpackSRGBTab[i] = int((powf(float(i), 2.2f) / max) * 4096 + 0.5f);

		for (int i = 0; i < 256; i++)
			HiColor::unpackLinearTab[i] = int(i / 255.f * 4096 + 0.5f);

		for (int i = 0; i <= 4096; i++)
			HiColor::packSRGBTab[i] = uint8_t(powf(i * max / 4096, 1 / 2.2f) + 0.5f);

		for (int i = 0; i <= 4096; i++)
			HiColor::packLinearTab[i] = uint8_t(i / 4096.f * 255.f + 0.5f);
	}


} // namespace wg
