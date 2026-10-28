#include "wappimageloader.h"

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace wg;

//____ wappLoadSurface() ______________________________________________________

Surface_p wappLoadSurface( const std::string& path, SurfaceFactory * pFactory, const Surface::Blueprint& _bp )
{
	if( !pFactory )
		pFactory = Base::defaultSurfaceFactory().rawPtr();

	if (path.rfind(".surf") == path.size() - 5 || path.rfind(".srf") == path.size() - 4)
	{
		std::ifstream input(path, std::ios::binary);
		if (!input.good())
			return nullptr;

		auto pReader = SurfaceReader::create({ .factory = pFactory });
		Surface_p pSurface = pReader->readSurfaceFromStream(input, _bp);
		input.close();
		return pSurface;
	}

	int width, height, channels;
	stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
	if (!data)
		return nullptr;

	PixelFormat destFormat;
	PixelDescription pixelDesc;

	if (channels == 3)
	{
		pixelDesc.bits = 24;
		pixelDesc.type = PixelType::Chunky;
		pixelDesc.bigEndian = false;				// stb_image gives R, G, B in that order, which little endian masks describe.
		pixelDesc.R_mask = 0x000000FF;
		pixelDesc.G_mask = 0x0000FF00;
		pixelDesc.B_mask = 0x00FF0000;
		pixelDesc.A_mask = 0x00000000;

		destFormat = PixelFormat::XRGB_8;
	}
	else if (channels == 4)
	{
		pixelDesc.bits = 32;
		pixelDesc.type = PixelType::Chunky;
		pixelDesc.bigEndian = false;				// stb_image gives R, G, B, A in that order, which little endian masks describe.
		pixelDesc.R_mask = 0x000000FF;
		pixelDesc.G_mask = 0x0000FF00;
		pixelDesc.B_mask = 0x00FF0000;
		pixelDesc.A_mask = 0xFF000000;

		destFormat = PixelFormat::ARGB_8;
	}
	else
	{
		// Grey (with or without alpha) is expanded to RGB(A) by loading again.

		stbi_image_free(data);

		int wantedChannels = (channels == 2) ? 4 : 3;
		data = stbi_load(path.c_str(), &width, &height, &channels, wantedChannels);
		if (!data)
			return nullptr;

		Surface::Blueprint bp = _bp;
		bp.size = { width, height };

		pixelDesc.type = PixelType::Chunky;
		pixelDesc.bigEndian = false;
		pixelDesc.R_mask = 0x000000FF;
		pixelDesc.G_mask = 0x0000FF00;
		pixelDesc.B_mask = 0x00FF0000;

		if (wantedChannels == 4)
		{
			pixelDesc.bits = 32;
			pixelDesc.A_mask = 0xFF000000;
			bp.format = PixelFormat::ARGB_8;
		}
		else
		{
			pixelDesc.bits = 24;
			pixelDesc.A_mask = 0;
			bp.format = PixelFormat::XRGB_8;
		}

		Surface_p pSurface = pFactory->createSurface(bp, (uint8_t*)data, pixelDesc, 0, nullptr, 0);
		stbi_image_free(data);
		return pSurface;
	}

	auto bp = _bp;
	bp.format = destFormat;
	bp.size = { width, height };

	Surface_p pSurface = pFactory->createSurface(bp, (uint8_t*)data, pixelDesc, 0, nullptr, 0);

	stbi_image_free(data);
	return pSurface;
}
