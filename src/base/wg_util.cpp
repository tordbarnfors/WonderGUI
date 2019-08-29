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

#include <wg_util.h>
#include <wg_geo.h>
#include <wg_surface.h>
#include <wg_gfxdevice.h>
#include <wg_patches.h>

namespace wg
{

	uint8_t Util::_limitUint8Table[768] = {	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
										0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

										0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
										0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
										0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
										0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
										0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
										0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
										0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
										0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
										0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
										0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
										0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
										0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
										0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
										0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
										0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
										0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,

										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
										0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };



/*
	LICENSE NOTE!

	Functions squareRoot() and powerOfTen() is from www.codeproject.com and licensed
	under The Code Project Open License (CPOL). See https://www.codeproject.com/info/cpol10.aspx
*/

// powerOfTen function is needed for squareRoot function (IMPORTANT)

double Util::squareRoot(double a)
{
	/*
		find more detail of this method on wiki methods_of_computing_square_roots

		*** Babylonian method cannot get exact zero but approximately value of the square_root
	*/
	double z = a;
	double rst = 0.0;
	int max = 8;	// to define maximum digit
	int i;
	double j = 1.0;
	for(i = max ; i > 0 ; i--){
		// value must be bigger then 0
		if(z - (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)) >= 0)
		{
			while( z - (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)) >= 0)
			{
				j++;
				if(j >= 10) break;

			}
			j--; //correct the extra value by minus one to j
			z -= (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)); //find value of z

			rst += j * powerOfTen(i);	// find sum of a

			j = 1.0;


		}

	}

	for(i = 0 ; i >= 0 - max ; i--){
		if(z - (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)) >= 0)
		{
			while( z - (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)) >= 0)
			{
				j++;
				if(j >= 10) break;
			}
			j--;
			z -= (( 2 * rst ) + ( j * powerOfTen(i)))*( j * powerOfTen(i)); //find value of z

			rst += j * powerOfTen(i);	// find sum of a
			j = 1.0;
		}
	}
	// find the number on each digit
	return rst;
}

double Util::powerOfTen(int num){
	double rst = 1.0;
	if(num >= 0){
		for(int i = 0; i < num ; i++){
			rst *= 10.0;
		}
	}else{
		for(int i = 0; i < (0 - num ); i++){
			rst *= 0.1;
		}
	}

	return rst;
}

	//____ markTestStretchRect() __________________________________________________

	bool Util::markTestStretchRect( CoordI ofs, Surface * pSurface, const RectI& source, const RectI& area, int opacityTreshold )
	{
		// Sanity check & shortcuts.
		if( !pSurface || !area.contains(ofs) || source.isEmpty() || area.isEmpty() || opacityTreshold > 255 )
			return false;

		if( pSurface->isOpaque() || opacityTreshold <= 0 )
			return true;

		// Make cordinates relative area.

		ofs.x -= area.x;
		ofs.y -= area.y;

		// Convert offset in area to offset in bitmap.

		ofs.x = (int) (ofs.x/((double)area.w) * source.w);
		ofs.y = (int) (ofs.y/((double)area.h) * source.h);

		// Do alpha test

		int alpha = pSurface->alpha(source.x+ofs.x, source.y+ofs.y);

		return (alpha >= opacityTreshold);
	}

	//____ markTestNinePatch() ________________________________________________

	bool Util::markTestNinePatch(CoordI ofs, Surface * pSurface, const RectI& _source, const RectI& _dest, int opacityTreshold, BorderI sourceFrame)
	{
		// Sanity check & shortcuts.

		if (sourceFrame.isEmpty())
			return markTestStretchRect(ofs, pSurface, _source, _dest, opacityTreshold);

		if (!pSurface || !_dest.contains(ofs) || _source.isEmpty() || _dest.isEmpty() || opacityTreshold > 255)
			return false;

		if (pSurface->isOpaque() || opacityTreshold <= 0)
			return true;

		//

		BorderI destFrame = pointsToPixels(sourceFrame);

		RectI source;
		RectI dest;

		if (ofs.x < _dest.x + destFrame.left)
		{
			// left section
			source.x = _source.x;
			source.w = sourceFrame.left;
			dest.x = _dest.x;
			dest.w = destFrame.left;
		}
		else if (ofs.x < _dest.x + _dest.w - destFrame.right)
		{
			// mid section
			source.x = _source.x + sourceFrame.left;
			source.w = _source.w - sourceFrame.width();
			dest.x = _dest.x + destFrame.left;
			dest.w = _dest.w - destFrame.width();
		}
		else
		{
			// right section
			source.x = _source.x + _source.w - sourceFrame.right;
			source.w = sourceFrame.right;
			dest.x = _dest.x + _dest.w - destFrame.right;
			dest.w = destFrame.right;
		}

		if (ofs.y < _dest.y + destFrame.top)
		{
			// left section
			source.y = _source.y;
			source.h = sourceFrame.top;
			dest.y = _dest.y;
			dest.h = destFrame.top;
		}
		else if (ofs.y < _dest.y + _dest.h - destFrame.bottom)
		{
			// mid section
			source.y = _source.y + sourceFrame.top;
			source.h = _source.h - sourceFrame.height();
			dest.y = _dest.y + destFrame.top;
			dest.h = _dest.h - destFrame.height();
		}
		else
		{
			// right section
			source.y = _source.y + _source.h - sourceFrame.bottom;
			source.h = sourceFrame.bottom;
			dest.y = _dest.y + _dest.h - destFrame.bottom;
			dest.h = destFrame.right;
		}

		// Make coordinates relative area.

		ofs.x -= dest.x;
		ofs.y -= dest.y;

		// Convert offset in area to offset in bitmap.

		ofs.x = (int)(ofs.x / ((double)dest.w) * source.w);
		ofs.y = (int)(ofs.y / ((double)dest.h) * source.h);

		// Do alpha test

		int alpha = pSurface->alpha(source.x + ofs.x, source.y + ofs.y);

		return (alpha >= opacityTreshold);
	}




	//____ pixelFormatToDescription() _____________________________________________________

	bool Util::pixelFormatToDescription( PixelFormat format, PixelDescription& wFormat )
	{
		switch( format )
		{
			case PixelFormat::BGR_8:
				wFormat.format = format;
				wFormat.bits = 24;
				wFormat.bIndexed = false;

				wFormat.R_bits = 8;
				wFormat.G_bits = 8;
				wFormat.B_bits = 8;
				wFormat.A_bits = 0;

				wFormat.R_loss = 0;
				wFormat.G_loss = 0;
				wFormat.B_loss = 0;
				wFormat.A_loss = 8;

	#if IS_LITTLE_ENDIAN
				wFormat.R_mask = 0xFF0000;
				wFormat.G_mask = 0xFF00;
				wFormat.B_mask = 0xFF;
				wFormat.A_mask = 0x0;

				wFormat.R_shift = 16;
				wFormat.G_shift = 8;
				wFormat.B_shift = 0;
				wFormat.A_shift = 0;
	#else
				wFormat.R_mask = 0xFF;
				wFormat.G_mask = 0xFF00;
				wFormat.B_mask = 0xFF0000;
				wFormat.A_mask = 0x0;

				wFormat.R_shift = 0;
				wFormat.G_shift = 8;
				wFormat.B_shift = 16;
				wFormat.A_shift = 0;
	#endif
				return true;

			case PixelFormat::BGRA_8:
				wFormat.format = format;
				wFormat.bits = 32;
				wFormat.bIndexed = false;

				wFormat.R_bits = 8;
				wFormat.G_bits = 8;
				wFormat.B_bits = 8;
				wFormat.A_bits = 8;

				wFormat.R_loss = 0;
				wFormat.G_loss = 0;
				wFormat.B_loss = 0;
				wFormat.A_loss = 0;

	#if IS_LITTLE_ENDIAN
				wFormat.A_mask = 0xFF000000;
				wFormat.R_mask = 0xFF0000;
				wFormat.G_mask = 0xFF00;
				wFormat.B_mask = 0xFF;

				wFormat.A_shift = 24;
				wFormat.R_shift = 16;
				wFormat.G_shift = 8;
				wFormat.B_shift = 0;
	#else
				wFormat.A_mask = 0xFF;
				wFormat.R_mask = 0xFF00;
				wFormat.G_mask = 0xFF0000;
				wFormat.B_mask = 0xFF000000;

				wFormat.A_shift = 0;
				wFormat.R_shift = 8;
				wFormat.G_shift = 16;
				wFormat.B_shift = 24;
	#endif
				return true;

			case PixelFormat::BGRX_8:
				wFormat.format = format;
				wFormat.bits = 32;
				wFormat.bIndexed = false;

				wFormat.R_bits = 8;
				wFormat.G_bits = 8;
				wFormat.B_bits = 8;
				wFormat.A_bits = 0;

				wFormat.R_loss = 0;
				wFormat.G_loss = 0;
				wFormat.B_loss = 0;
				wFormat.A_loss = 0;

#if IS_LITTLE_ENDIAN
				wFormat.A_mask = 0x00000000;
				wFormat.R_mask = 0xFF0000;
				wFormat.G_mask = 0xFF00;
				wFormat.B_mask = 0xFF;

				wFormat.A_shift = 0;
				wFormat.R_shift = 16;
				wFormat.G_shift = 8;
				wFormat.B_shift = 0;
#else
				wFormat.A_mask = 0x00;
				wFormat.R_mask = 0xFF00;
				wFormat.G_mask = 0xFF0000;
				wFormat.B_mask = 0xFF000000;

				wFormat.A_shift = 0;
				wFormat.R_shift = 8;
				wFormat.G_shift = 16;
				wFormat.B_shift = 24;
#endif
				return true;


			case PixelFormat::BGRA_4:
				wFormat.format = format;
				wFormat.bits = 16;
				wFormat.bIndexed = false;

				wFormat.R_bits = 4;
				wFormat.G_bits = 4;
				wFormat.B_bits = 4;
				wFormat.A_bits = 4;

				wFormat.R_loss = 4;
				wFormat.G_loss = 4;
				wFormat.B_loss = 4;
				wFormat.A_loss = 4;

#if IS_LITTLE_ENDIAN
				wFormat.A_mask = 0xF000;
				wFormat.R_mask = 0x0F00;
				wFormat.G_mask = 0x00F0;
				wFormat.B_mask = 0x000F;

				wFormat.A_shift = 12;
				wFormat.R_shift = 8;
				wFormat.G_shift = 4;
				wFormat.B_shift = 0;
#else
				wFormat.A_mask = 0x00F0;
				wFormat.R_mask = 0x000F;
				wFormat.G_mask = 0xF000;
				wFormat.B_mask = 0x0F00;

				wFormat.A_shift = 4;
				wFormat.R_shift = 0;
				wFormat.G_shift = 12;
				wFormat.B_shift = 8;
#endif
				return true;

			case PixelFormat::BGR_565:
				wFormat.format = format;
				wFormat.bits = 16;
				wFormat.bIndexed = false;

				wFormat.R_bits = 5;
				wFormat.G_bits = 6;
				wFormat.B_bits = 5;
				wFormat.A_bits = 0;

				wFormat.R_loss = 3;
				wFormat.G_loss = 2;
				wFormat.B_loss = 3;
				wFormat.A_loss = 8;

#if IS_LITTLE_ENDIAN
				wFormat.A_mask = 0x0000;
				wFormat.R_mask = 0xF800;
				wFormat.G_mask = 0x07E0;
				wFormat.B_mask = 0x001F;

				wFormat.A_shift = 0;
				wFormat.R_shift = 11;
				wFormat.G_shift = 5;
				wFormat.B_shift = 0;
#else
				wFormat.A_mask = 0x0000;
				wFormat.R_mask = 0x00F8;
				wFormat.G_mask = 0xE007;
				wFormat.B_mask = 0x1F00;

				wFormat.A_shift = 0;
				wFormat.R_shift = 3;
				wFormat.G_shift = 13;
				wFormat.B_shift = 8;
#endif
				return true;


			case PixelFormat::A8:
				wFormat.format = format;
				wFormat.bits = 8;
				wFormat.bIndexed = false;

				wFormat.R_bits = 0;
				wFormat.G_bits = 0;
				wFormat.B_bits = 0;
				wFormat.A_bits = 8;

				wFormat.R_loss = 8;
				wFormat.G_loss = 8;
				wFormat.B_loss = 8;
				wFormat.A_loss = 0;

				wFormat.R_mask = 0x00;
				wFormat.G_mask = 0x00;
				wFormat.B_mask = 0x00;
				wFormat.A_mask = 0xFF;

				wFormat.R_shift = 0;
				wFormat.G_shift = 0;
				wFormat.B_shift = 0;
				wFormat.A_shift = 0;
				return true;

			case PixelFormat::I8:
				wFormat.format = format;
				wFormat.bits = 8;
				wFormat.bIndexed = true;

				wFormat.R_bits = 8;
				wFormat.G_bits = 8;
				wFormat.B_bits = 8;
				wFormat.A_bits = 8;

				wFormat.R_loss = 0;
				wFormat.G_loss = 0;
				wFormat.B_loss = 0;
				wFormat.A_loss = 0;

				wFormat.R_mask = 0x00;
				wFormat.G_mask = 0x00;
				wFormat.B_mask = 0x00;
				wFormat.A_mask = 0x00;

				wFormat.R_shift = 0;
				wFormat.G_shift = 0;
				wFormat.B_shift = 0;
				wFormat.A_shift = 0;
				return true;

			default:
				wFormat.format = PixelFormat::Unknown;
				wFormat.bits = 0;
				wFormat.bIndexed = false;

				wFormat.R_bits = 0;
				wFormat.G_bits = 0;
				wFormat.B_bits = 0;
				wFormat.A_bits = 0;

				wFormat.R_loss = 0;
				wFormat.G_loss = 0;
				wFormat.B_loss = 0;
				wFormat.A_loss = 0;

				wFormat.R_mask = 0;
				wFormat.G_mask = 0;
				wFormat.B_mask = 0;
				wFormat.A_mask = 0;

				wFormat.R_shift = 0;
				wFormat.G_shift = 0;
				wFormat.B_shift = 0;
				wFormat.A_shift = 0;


				return false;
		}


	}

	//____ sizeFromPolicy() __________________________________________________________

	int Util::sizeFromPolicy( int defaultSize, int specifiedSize, SizePolicy policy )
	{
		switch( policy )
		{
			case SizePolicy::Default:
				return defaultSize;
			case SizePolicy::Bound:
				return specifiedSize;
			case SizePolicy::Confined:
				if( defaultSize > specifiedSize )
					return specifiedSize;
			case SizePolicy::Expanded:
				if( defaultSize < specifiedSize )
					return specifiedSize;
		}
		return defaultSize;
	}

	//____ bestStateIndexMatch() ___________________________________________________

	int Util::bestStateIndexMatch(int wantedStateIndex, Bitmask<uint32_t> availableStateIndices)
	{
		static uint32_t mask[StateEnum_Nb] = {	1,
											2+1,
											4+1,
											8 + 4 + 2 + 1,
											16 + 4 + 1,
											32 + 16 + 8 + 4 + 2 + 1,
											64 + 1,
											128 + 64 + 2 + 1,
											256 + 64 + 4 + 1,
											512 + 256 + 128 + 64 + 8 + 4 + 2 + 1,
											1024 + 256 + 64 + 16 + 4 + 1,
											2048 + 1024 + 512 + 256 + 128 + 64 + 32 + 16 + 8 + 4 + 2 + 1,
											4096 + 4 + 1,
											8192 + 4096 + 8 + 4 + 2 + 1,
											16384 + 4096 + 256 + 64 + 4 + 1,
											32768 + 16384 + 8192 + 4096 + 512 + 256 + 128 + 64 + 8 + 4 + 2 + 1,
											65536 + 1,
											65536*2 + 65536 + 64 + 1
										};

		int candidates = uint32_t(availableStateIndices) & mask[wantedStateIndex];

		return mostSignificantBit(candidates);
	}



	//____ Checksum8::add() ________________________________________________________

	void Util::Checksum8::add( const void * pData, uint32_t nBytes )
	{
		uint32_t x = remainder;

		for( uint32_t i = 0 ; i < nBytes ; i++ )
			x = ((x << 8) + ((uint8_t*)pData)[i])%dividend;

		remainder = x;
	}




	//____ origoToOfs() ________________________________________________________

	CoordI Util::origoToOfs( Origo origo, SizeI base )
	{
		switch( origo )
		{
			default:
			case Origo::NorthWest:
				return CoordI(0,0);

			case Origo::North:
				return CoordI( base.w/2,0 );

			case Origo::NorthEast:
				return CoordI( base.w,0 );

			case Origo::East:
				return CoordI( base.w, base.h/2 );

			case Origo::SouthEast:
				return CoordI( base.w, base.h );

			case Origo::South:
				return CoordI( base.w/2, base.h );

			case Origo::SouthWest:
				return CoordI( 0, base.h );

			case Origo::West:
				return CoordI( 0, base.h/2 );

			case Origo::Center:
				return CoordI( base.w/2, base.h/2 );
		}
	}

	//____ origoToRect() ________________________________________________________

	RectI Util::origoToRect( Origo origo, SizeI base, SizeI rect )
	{
		switch( origo )
		{
			default:
			case Origo::NorthWest:
				return RectI(0,0, rect);

			case Origo::North:
				return RectI( base.w/2 - rect.w/2, 0, rect );

			case Origo::NorthEast:
				return RectI( base.w - rect.w, 0, rect );

			case Origo::East:
				return RectI( base.w - rect.w, base.h/2 - rect.h/2, rect );

			case Origo::SouthEast:
				return RectI( base.w - rect.w, base.h - rect.h, rect );

			case Origo::South:
				return RectI( base.w/2 - rect.w/2, base.h - rect.h, rect );

			case Origo::SouthWest:
				return RectI( 0, base.h - rect.h, rect );

			case Origo::West:
				return RectI( 0, base.h/2 - rect.h/2, rect );

			case Origo::Center:
				return RectI( base.w/2 - rect.w/2, base.h/2 - rect.h/2, rect );
		}
	}

	//____ scaleToFit() _______________________________________________________

	SizeI Util::scaleToFit(SizeI object, SizeI boundaries)
	{
		float wScale = object.w / (float)boundaries.w;
		float hScale = object.h / (float)boundaries.h;

		float useScale = max(wScale, hScale);

		return object / useScale;
	}

	//____ mostSignificantBit() _______________________________________________

	// Uses the de Bruijn algorithm for finding the most significant bit in an integer.

	uint32_t Util::mostSignificantBit(uint32_t value)
	{
		static const int MultiplyDeBruijnBitPosition[32] =
		{
			0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
			8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
		};

		value |= value >> 1; // first round down to one less than a power of 2
		value |= value >> 2;
		value |= value >> 4;
		value |= value >> 8;
		value |= value >> 16;

		return MultiplyDeBruijnBitPosition[(uint32_t)(value * 0x07C4ACDDU) >> 27];
	}

	//____ patchesToClipList() ____________________________________________________________________

	Util::ClipPopData Util::patchesToClipList( GfxDevice * pDevice, const RectI& clip, const Patches& patches )
	{
		int nOldRects 				= pDevice->clipListSize();
		const RectI * pOldRects 	= pDevice->clipList();

		int nRects = patches.size();
		const RectI * pRects = patches.begin();
		
		int allocSize = nRects * sizeof(RectI);
		
		RectI * pNewRects = (RectI*) Base::memStackAlloc(allocSize);
		int nNewRects = 0;
		
		for( int i = 0 ; i < nRects ; i++ )
		{
			pNewRects[nNewRects] = rawToPixels( RectI(pRects[i], clip) );
			if( !pNewRects[nNewRects].isEmpty() )
				nNewRects++;
		}
		
		pDevice->setClipList(nNewRects, pNewRects);
		return { nOldRects, pOldRects, allocSize };
	}

	Util::ClipPopData Util::patchesToClipList( GfxDevice * pDevice, const Patches& patches )
	{
		int nOldRects 				= pDevice->clipListSize();
		const RectI * pOldRects 	= pDevice->clipList();
		
		int nRects = patches.size();
		const RectI * pRects = patches.begin();
		
		int allocSize = nRects * sizeof(RectI);
		
		RectI * pNewRects = (RectI*) Base::memStackAlloc(allocSize);
		
		for( int i = 0 ; i < nRects ; i++ )
			pNewRects[i] = rawToPixels( pRects[i] );
		
		pDevice->setClipList(nRects, pNewRects);
		return { nOldRects, pOldRects, allocSize };
	}

	//____ limitClipList() ____________________________________________________________________

	Util::ClipPopData Util::limitClipList( GfxDevice * pDevice, const RectI& clip )
	{
		if( clip.contains(pDevice->clipBounds()))
			return { 0, nullptr, 0 };
		
		int nRects 				= pDevice->clipListSize();
		const RectI * pRects 	= pDevice->clipList();
		int allocSize = nRects * sizeof(RectI);

		RectI * pNewRects = (RectI*) Base::memStackAlloc(allocSize);
		int nNewRects = 0;
		
		for( int i = 0 ; i < nRects ; i++ )
		{
			if( pNewRects[nNewRects].intersection( pRects[i], clip ) )
				nNewRects++;
		}
		
		pDevice->setClipList(nNewRects, pNewRects);
		return { nRects, pRects, allocSize };
	}
	
	//____ pushClipList() __________________________________________________________________

	Util::ClipPopData Util::pushClipList(GfxDevice * pDevice)
	{
		return { pDevice->clipListSize(), pDevice->clipList(), 0 };
	}


	//____ popClipList() ____________________________________________________________________
	
	void Util::popClipList( GfxDevice * pDevice, const ClipPopData& popData )
	{
		if( popData.bInitialized )
		{
			pDevice->setClipList(popData.nRects, popData.pRects);
			Base::memStackRelease(popData.reservedMem);
		}
	}

} // namespace wg
