#ifndef WAPPIMAGELOADER_DOT_H
#define WAPPIMAGELOADER_DOT_H
#pragma once

#include <wondergui.h>

#include <string>

// Loads a bitmap from a .surf/.srf file, or from an image in any format
// stb_image supports (PNG, JPEG, BMP, GIF, TGA, ...). Images are loaded as
// ARGB_8, or XRGB_8 if they have no alpha channel.
//
// pFactory may be null, in which case the default bitmap factory is used.

wg::Bitmap_p	wappLoadBitmap( const std::string& path, wg::BitmapFactory * pFactory, const wg::Bitmap::Blueprint& bp );

#endif //WAPPIMAGELOADER_DOT_H
