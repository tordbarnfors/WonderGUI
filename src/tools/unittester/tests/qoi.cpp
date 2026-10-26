#include <string.h>

#include "qoi.h"

#include <wondergui.h>
#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_qoicompression.h>

using namespace wg;
using namespace wax;

// Test image of 16x6 pixels in R, G, B, A byte order, which covers all chunk types,
// wrap-around of differences, a run longer than 62 and a run of the initial pixel.
// The QOI files are what the reference implementation (github.com/phoboslab/qoi)
// makes of it, with 4 and 3 channels.

static const uint8_t c_pixelsRGBA[96*4] = {
	0,0,0,255,0,0,0,255,1,0,255,255,11,9,250,255,200,100,50,255,200,100,50,128,1,0,255,255,0,0,0,0,
	200,100,50,128,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,6,7,7,
	40,30,20,7,255,255,255,255,0,1,0,255,249,159,69,255,252,164,76,255,255,169,83,255,2,174,90,255,5,179,97,255,
	8,184,104,255,11,189,111,255,14,194,118,255,17,199,125,255,20,204,132,255,23,209,139,255,26,214,146,255,29,219,153,255 };

static const uint8_t c_qoiFileRGBA[87] = {
	0x71,0x6F,0x69,0x66,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x06,0x04,0x00,0xC1,0x79,
	0xFE,0x0B,0x09,0xFA,0xFE,0xC8,0x64,0x32,0xFF,0xC8,0x64,0x32,0x80,0x31,0x00,0x2A,
	0xFF,0x07,0x07,0x07,0x07,0xFD,0xC6,0x76,0xFE,0x28,0x1E,0x14,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xA2,0x77,0xFE,0xF9,0x9F,0x45,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,
	0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x01 };

static const uint8_t c_qoiFileRGB[81] = {
	0x71,0x6F,0x69,0x66,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x06,0x03,0x00,0xC1,0x79,
	0xFE,0x0B,0x09,0xFA,0xFE,0xC8,0x64,0x32,0xC0,0x31,0x5B,0x1F,0xFE,0x07,0x07,0x07,
	0xFD,0xC6,0x76,0xFE,0x28,0x1E,0x14,0xFE,0xFF,0xFF,0xFF,0xA2,0x77,0xFE,0xF9,0x9F,
	0x45,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,
	0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0xA5,0x6A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01 };

// Pixels in the byte order of little endian (B, G, R, A) and big endian (A, R, G, B) ARGB_8.

static std::vector<uint8_t> littleEndianPixels()
{
	std::vector<uint8_t> v(sizeof(c_pixelsRGBA));
	for( size_t i = 0 ; i < v.size() ; i += 4 )
	{
		v[i] = c_pixelsRGBA[i+2];
		v[i+1] = c_pixelsRGBA[i+1];
		v[i+2] = c_pixelsRGBA[i];
		v[i+3] = c_pixelsRGBA[i+3];
	}
	return v;
}

static std::vector<uint8_t> bigEndianPixels()
{
	std::vector<uint8_t> v(sizeof(c_pixelsRGBA));
	for( size_t i = 0 ; i < v.size() ; i += 4 )
	{
		v[i] = c_pixelsRGBA[i+3];
		v[i+1] = c_pixelsRGBA[i];
		v[i+2] = c_pixelsRGBA[i+1];
		v[i+3] = c_pixelsRGBA[i+2];
	}
	return v;
}

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

QOITest::QOITest()
{
	ADD_TEST(compressTest);
	ADD_TEST(decompressTest);
	ADD_TEST(writeQOITest);
	ADD_TEST(readQOITest);
	ADD_TEST(surfaceFileTest);
}

QOITest::~QOITest()
{
}


bool QOITest::init(std::ostream& output)
{
	return true;
}

//____ compressTest() _________________________________________________________

bool QOITest::compressTest(std::ostream& output)
{
	const uint8_t * pChunks = c_qoiFileRGBA + c_qoiHeaderSize;
	int chunkBytes = sizeof(c_qoiFileRGBA) - c_qoiHeaderSize;

	auto le = littleEndianPixels();
	auto be = bigEndianPixels();

	auto pCompressorLE = QOICompressor::create( { .format = PixelFormat::ARGB_8, .bigEndian = false } );
	auto pCompressorBE = QOICompressor::create( { .format = PixelFormat::ARGB_8, .bigEndian = true } );

	TEST_ASSERT( pCompressorLE->idToken() == QOICompressor::ID_TOKEN );
	TEST_ASSERT( pCompressorBE->idToken() == QOICompressor::ID_TOKEN_BIG_ENDIAN );

	std::vector<uint8_t> buffer(pCompressorLE->maxCompressedSize(int(le.size())));

	int bytes = pCompressorLE->compress(buffer.data(), le.data(), le.data() + le.size());
	TEST_ASSERT( bytes == chunkBytes && memcmp(buffer.data(), pChunks, bytes) == 0 );

	bytes = pCompressorBE->compress(buffer.data(), be.data(), be.data() + be.size());
	TEST_ASSERT( bytes == chunkBytes && memcmp(buffer.data(), pChunks, bytes) == 0 );

	// Stream ends with end marker.

	TEST_ASSERT( memcmp(buffer.data() + bytes - 8, "\0\0\0\0\0\0\0\1", 8) == 0 );

	// XRGB_8 is compressed as QOI with 3 channels, alpha being ignored.

	for( size_t i = 3 ; i < le.size() ; i += 4 )
		le[i] = uint8_t(i * 13);

	auto pCompressorX = QOICompressor::create( { .format = PixelFormat::XRGB_8, .bigEndian = false } );

	bytes = pCompressorX->compress(buffer.data(), le.data(), le.data() + le.size());
	TEST_ASSERT( bytes == int(sizeof(c_qoiFileRGB)) - c_qoiHeaderSize && memcmp(buffer.data(), c_qoiFileRGB + c_qoiHeaderSize, bytes) == 0 );

	// Empty data is just the end marker.

	bytes = pCompressorLE->compress(buffer.data(), le.data(), le.data());
	TEST_ASSERT( bytes == 8 );

	return true;
}

//____ decompressTest() _______________________________________________________

bool QOITest::decompressTest(std::ostream& output)
{
	const uint8_t * pChunks = c_qoiFileRGBA + c_qoiHeaderSize;
	int chunkBytes = sizeof(c_qoiFileRGBA) - c_qoiHeaderSize;

	auto le = littleEndianPixels();
	auto be = bigEndianPixels();
	int pixelBytes = int(le.size());

	auto pDecompressorLE = QOIDecompressor::create( { .bigEndian = false } );
	auto pDecompressorBE = QOIDecompressor::create( { .bigEndian = true } );

	std::vector<uint8_t> buffer(pixelBytes + 16, 0xCD);

	int bytes = pDecompressorLE->decompress(buffer.data(), pChunks, pChunks + chunkBytes);
	TEST_ASSERT( bytes == pixelBytes && memcmp(buffer.data(), le.data(), pixelBytes) == 0 && buffer[pixelBytes] == 0xCD );

	bytes = pDecompressorBE->decompress(buffer.data(), pChunks, pChunks + chunkBytes);
	TEST_ASSERT( bytes == pixelBytes && memcmp(buffer.data(), be.data(), pixelBytes) == 0 );

	// Decompression stops at the end marker, even if more data follows.

	std::vector<uint8_t> extended(pChunks, pChunks + chunkBytes);
	extended.insert(extended.end(), { 0x00, 0x55, 0xC5, 0xFF, 1, 2, 3, 4 } );

	std::fill(buffer.begin(), buffer.end(), 0xCD);
	bytes = pDecompressorLE->decompress(buffer.data(), extended.data(), extended.data() + extended.size());
	TEST_ASSERT( bytes == pixelBytes && memcmp(buffer.data(), le.data(), pixelBytes) == 0 && buffer[pixelBytes] == 0xCD );

	// Decompression stops when given number of bytes are written, also in the middle of a run.

	std::fill(buffer.begin(), buffer.end(), 0xCD);
	bytes = pDecompressorLE->decompress(buffer.data(), 20*4, pChunks, pChunks + chunkBytes);
	TEST_ASSERT( bytes == 20*4 && memcmp(buffer.data(), le.data(), bytes) == 0 && buffer[bytes] == 0xCD );

	// In place, with compressed data at end of buffer, as is done in GfxStream.

	auto pCompressor = QOICompressor::create();
	std::vector<uint8_t> inPlace(pCompressor->maxCompressedSize(pixelBytes));
	int dataStart = int(inPlace.size()) - chunkBytes;
	memcpy(inPlace.data() + dataStart, pChunks, chunkBytes);

	bytes = pDecompressorLE->decompress(inPlace.data(), inPlace.data() + dataStart, inPlace.data() + inPlace.size());
	TEST_ASSERT( bytes == pixelBytes && memcmp(inPlace.data(), le.data(), pixelBytes) == 0 );

	// 3 channels, alpha is 255.

	bytes = pDecompressorLE->decompress(buffer.data(), c_qoiFileRGB + c_qoiHeaderSize, c_qoiFileRGB + sizeof(c_qoiFileRGB));
	TEST_ASSERT( bytes == pixelBytes );

	for( int i = 0 ; i < pixelBytes ; i += 4 )
		TEST_ASSERT( buffer[i] == le[i] && buffer[i+1] == le[i+1] && buffer[i+2] == le[i+2] && buffer[i+3] == 255 );

	// wax::Core knows about both byte orders.

	TEST_ASSERT( wax::Core::getDecompressor(QOIDecompressor::ID_TOKEN)->idToken() == QOIDecompressor::ID_TOKEN );
	TEST_ASSERT( wax::Core::getDecompressor(QOIDecompressor::ID_TOKEN_BIG_ENDIAN)->idToken() == QOIDecompressor::ID_TOKEN_BIG_ENDIAN );

	return true;
}

//____ writeQOITest() _________________________________________________________

bool QOITest::writeQOITest(std::ostream& output)
{
	auto le = littleEndianPixels();
	auto pWriter = SurfaceWriter::create();

	// ARGB_8 is saved with 4 channels.

	auto pSurface = SoftSurface::create( { .format = PixelFormat::ARGB_8, .size = {16,6} }, le.data(), Util::pixelFormatToDescription(PixelFormat::ARGB_8, false) );

	auto pBlob = pWriter->writeSurfaceToQOIBlob(pSurface);
	TEST_ASSERT( pBlob && pBlob->size() == int(sizeof(c_qoiFileRGBA)) && memcmp(pBlob->data(), c_qoiFileRGBA, sizeof(c_qoiFileRGBA)) == 0 );

	std::ostringstream stream;
	TEST_ASSERT( pWriter->writeSurfaceToQOIStream(stream, pSurface) );
	TEST_ASSERT( stream.str().size() == sizeof(c_qoiFileRGBA) && memcmp(stream.str().data(), c_qoiFileRGBA, sizeof(c_qoiFileRGBA)) == 0 );

	// Big endian gives the same file.

	auto pSurfaceBE = SoftSurface::create( { .bigEndian = true, .format = PixelFormat::ARGB_8, .size = {16,6} }, le.data(), Util::pixelFormatToDescription(PixelFormat::ARGB_8, false) );

	pBlob = pWriter->writeSurfaceToQOIBlob(pSurfaceBE);
	TEST_ASSERT( pBlob && pBlob->size() == int(sizeof(c_qoiFileRGBA)) && memcmp(pBlob->data(), c_qoiFileRGBA, sizeof(c_qoiFileRGBA)) == 0 );

	// XRGB_8 is saved with 3 channels.

	auto pSurfaceX = SoftSurface::create( { .format = PixelFormat::XRGB_8, .size = {16,6} }, le.data(), Util::pixelFormatToDescription(PixelFormat::ARGB_8, false) );

	pBlob = pWriter->writeSurfaceToQOIBlob(pSurfaceX);
	TEST_ASSERT( pBlob && pBlob->size() == int(sizeof(c_qoiFileRGB)) && memcmp(pBlob->data(), c_qoiFileRGB, sizeof(c_qoiFileRGB)) == 0 );

	// Linear color space is marked in header.

	auto pSurfaceLinear = SoftSurface::create( { .colorSpace = ColorSpace::Linear, .format = PixelFormat::ARGB_8, .size = {16,6} }, le.data(), Util::pixelFormatToDescription(PixelFormat::ARGB_8, false) );

	pBlob = pWriter->writeSurfaceToQOIBlob(pSurfaceLinear);
	TEST_ASSERT( pBlob && ((uint8_t*)pBlob->data())[13] == 1 && memcmp(((uint8_t*)pBlob->data()) + 14, c_qoiFileRGBA + 14, sizeof(c_qoiFileRGBA) - 14) == 0 );

	return true;
}

//____ readQOITest() __________________________________________________________

bool QOITest::readQOITest(std::ostream& output)
{
	auto le = littleEndianPixels();
	auto pReader = SurfaceReader::create( { .factory = SoftSurfaceFactory::create() } );

	// From stream, which should be left right after the image.

	std::string data( (const char*) c_qoiFileRGBA, sizeof(c_qoiFileRGBA) );
	data += "NEXT";

	std::istringstream stream(data);

	auto pSurface = pReader->readSurfaceFromStream(stream);
	TEST_ASSERT( pSurface && pSurface->pixelFormat() == PixelFormat::ARGB_8 && pSurface->colorSpace() == ColorSpace::sRGB );
	TEST_ASSERT( pSurface->pixelSize() == SizeI(16,6) && surfacePixels(pSurface) == le );

	std::string rest;
	stream >> rest;
	TEST_ASSERT( rest == "NEXT" );

	// From blob and memory.

	auto pBlob = Blob::create(sizeof(c_qoiFileRGBA));
	memcpy(pBlob->data(), c_qoiFileRGBA, sizeof(c_qoiFileRGBA));

	pSurface = pReader->readSurfaceFromBlob(pBlob);
	TEST_ASSERT( pSurface && surfacePixels(pSurface) == le );

	pSurface = pReader->readSurfaceFromMemory((const char*) c_qoiFileRGBA);
	TEST_ASSERT( pSurface && surfacePixels(pSurface) == le );

	// 3 channels gives XRGB_8.

	pSurface = pReader->readSurfaceFromMemory((const char*) c_qoiFileRGB);
	TEST_ASSERT( pSurface && pSurface->pixelFormat() == PixelFormat::XRGB_8 );

	auto pixels = surfacePixels(pSurface);
	TEST_ASSERT( pixels.size() == le.size() );

	for( size_t i = 0 ; i < pixels.size() ; i += 4 )
		TEST_ASSERT( pixels[i] == le[i] && pixels[i+1] == le[i+1] && pixels[i+2] == le[i+2] );

	// Truncated image fails.

	std::istringstream truncated( std::string( (const char*) c_qoiFileRGBA, sizeof(c_qoiFileRGBA) - 12 ) );
	TEST_ASSERT( pReader->readSurfaceFromStream(truncated) == nullptr );

	return true;
}

//____ surfaceFileTest() ______________________________________________________

bool QOITest::surfaceFileTest(std::ostream& output)
{
	auto le = littleEndianPixels();
	auto pSurface = SoftSurface::create( { .format = PixelFormat::ARGB_8, .size = {16,6} }, le.data(), Util::pixelFormatToDescription(PixelFormat::ARGB_8, false) );

	// Pixel data of a surface file with QOI compression is the same as in a QOI file.

	auto pWriter = SurfaceWriter::create( { .pixelCompressor = QOICompressor::create( { .bigEndian = false } ) } );
	auto pBlob = pWriter->writeSurfaceToBlob(pSurface);

	auto pHeader = (const SurfaceFileHeader*) pBlob->data();
	auto pPixelData = ((const uint8_t*) pBlob->data()) + pHeader->headerBytes + pHeader->paletteBytes;

	TEST_ASSERT( pHeader->pixelCompression == QOICompressor::ID_TOKEN );
	TEST_ASSERT( pHeader->pixelBytes - pHeader->pixelDataPadding == int(sizeof(c_qoiFileRGBA)) - c_qoiHeaderSize );
	TEST_ASSERT( memcmp(pPixelData, c_qoiFileRGBA + c_qoiHeaderSize, sizeof(c_qoiFileRGBA) - c_qoiHeaderSize) == 0 );

	auto pReader = SurfaceReader::create( { .factory = SoftSurfaceFactory::create() } );
	auto pLoaded = pReader->readSurfaceFromBlob(pBlob);
	TEST_ASSERT( pLoaded && surfacePixels(pLoaded) == le );

	return true;
}
