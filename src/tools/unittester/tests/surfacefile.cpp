#include <string.h>

#include "surfacefile.h"

#include <wondergui.h>
#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_lzcompression.h>
#include <wax_rlecompression.h>
#include <wax_qoicompression.h>

using namespace wg;

//____ surfacePixels() ________________________________________________________

static std::vector<uint8_t> surfacePixels(Surface * pSurface)
{
	auto pixbuf = pSurface->allocPixelBuffer();
	pSurface->pushPixels(pixbuf);

	int lineBytes = pixbuf.rect.w * pSurface->pixelBits() / 8;

	std::vector<uint8_t> v;
	for( int y = 0 ; y < pixbuf.rect.h ; y++ )
		v.insert(v.end(), pixbuf.pixels + y * pixbuf.pitch, pixbuf.pixels + y * pixbuf.pitch + lineBytes);

	pSurface->freePixelBuffer(pixbuf);
	return v;
}

SurfaceFileTest::SurfaceFileTest()
{
	ADD_TEST(paddedPixelsTest);
	ADD_TEST(compressedPaletteTest);
}

SurfaceFileTest::~SurfaceFileTest()
{
}


bool SurfaceFileTest::init(std::ostream& output)
{
	return true;
}

//____ paddedPixelsTest() _____________________________________________________
//
// Pixels with padding at the end of each line, as surfaces of some backends have,
// must be saved without the padding, also when compressed.

bool SurfaceFileTest::paddedPixelsTest(std::ostream& output)
{
	const int width = 5;
	const int height = 7;
	const int pitch = 32;				// 20 bytes of pixels and 12 of padding per line.

	std::vector<uint8_t> memory(pitch * height);
	std::vector<uint8_t> expected;

	for( int y = 0 ; y < height ; y++ )
	{
		for( int x = 0 ; x < pitch ; x++ )
		{
			uint8_t value = x < width * 4 ? uint8_t(x / 8 + y * 3) : uint8_t(0xA0 + x);		// Padding differs from pixels.
			memory[y * pitch + x] = value;

			if( x < width * 4 )
				expected.push_back(value);
		}
	}

	auto pSurface = SoftSurface::createInPlace( { .format = PixelFormat::ARGB_8, .size = {width,height} }, memory.data(), pitch );
	auto pReader = SurfaceReader::create( { .factory = SoftSurfaceFactory::create() } );

	Compressor_p compressors[] = { nullptr, LZCompressor::create(), RLECompressor::create( { .primSize = 4 } ), QOICompressor::create() };

	for( auto& pCompressor : compressors )
	{
		auto pWriter = SurfaceWriter::create( { .pixelCompressor = pCompressor } );
		auto pBlob = pWriter->writeSurfaceToBlob(pSurface);
		TEST_ASSERT( pBlob != nullptr );

		auto pLoaded = pReader->readSurfaceFromBlob(pBlob);
		TEST_ASSERT( pLoaded && pLoaded->pixelSize() == SizeI(width,height) );
		TEST_ASSERT( surfacePixels(pLoaded) == expected );
	}

	return true;
}

//____ compressedPaletteTest() ________________________________________________

bool SurfaceFileTest::compressedPaletteTest(std::ostream& output)
{
	const int width = 9;
	const int height = 4;
	const int paletteSize = 37;

	Color8 palette[paletteSize];
	for( int i = 0 ; i < paletteSize ; i++ )
		palette[i] = Color8( uint8_t(i * 7), uint8_t(i < 10 ? 0 : 255), uint8_t(i * 3), 255 );

	uint8_t pixels[width * height];
	for( int i = 0 ; i < width * height ; i++ )
		pixels[i] = uint8_t(i % paletteSize);

	auto pSurface = SoftSurface::create( { .palette = palette, .paletteSize = paletteSize, .format = PixelFormat::Index_8, .size = {width,height} },
										 pixels, PixelFormat::Index_8, 0, palette, paletteSize );
	TEST_ASSERT( pSurface != nullptr );

	auto pReader = SurfaceReader::create( { .factory = SoftSurfaceFactory::create() } );

	Compressor_p compressors[] = { LZCompressor::create(), RLECompressor::create( { .primSize = 1 } ), RLECompressor::create( { .primSize = 4 } ) };

	for( auto& pCompressor : compressors )
	{
		auto pWriter = SurfaceWriter::create( { .paletteCompressor = pCompressor } );
		auto pBlob = pWriter->writeSurfaceToBlob(pSurface);
		TEST_ASSERT( pBlob != nullptr );

		auto pHeader = (const SurfaceFileHeader*) pBlob->data();
		TEST_ASSERT( pHeader->paletteCompression == pCompressor->idToken() );

		auto pLoaded = pReader->readSurfaceFromBlob(pBlob);
		TEST_ASSERT( pLoaded && pLoaded->pixelFormat() == PixelFormat::Index_8 && pLoaded->paletteSize() == paletteSize );
		TEST_ASSERT( memcmp(pLoaded->palette(), palette, sizeof(palette)) == 0 );
		TEST_ASSERT( surfacePixels(pLoaded) == std::vector<uint8_t>(pixels, pixels + sizeof(pixels)) );
	}

	return true;
}
