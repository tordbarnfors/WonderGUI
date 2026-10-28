#ifndef WAPPIMAGELOADER_DOT_H
#define WAPPIMAGELOADER_DOT_H
#pragma once

#include <wondergui.h>

#include <string>

// Loads a surface from a .surf/.srf file, or from an image in any format
// stb_image supports (PNG, JPEG, BMP, GIF, TGA, ...). Images are loaded as
// ARGB_8, or XRGB_8 if they have no alpha channel.
//
// pFactory may be null, in which case the default surface factory is used.

wg::Surface_p	wappLoadSurface( const std::string& path, wg::SurfaceFactory * pFactory, const wg::Surface::Blueprint& bp );

#endif //WAPPIMAGELOADER_DOT_H
