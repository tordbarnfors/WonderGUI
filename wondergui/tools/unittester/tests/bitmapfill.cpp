#include <string.h>

#include "bitmapfill.h"

#include <wondergui.h>
#include <waxgfxstream.h>
#include <wax_softbitmap.h>
#include <wax_softbitmapfactory.h>
#include <wax_softedgemapfactory.h>
#include <wax_softbackend.h>

using namespace wg;
using namespace wax;

static const Color8 s_palette[4] = { Color8(0, 0, 0, 255), Color8(255, 0, 0, 255), Color8(40, 160, 220, 255), Color8(255, 255, 255, 255) };

//____ bitmapPixels() ________________________________________________________

static std::vector<uint8_t> bitmapPixels(Bitmap * pBitmap)
{
	auto pixbuf = pBitmap->allocPixelBuffer();
	pBitmap->pushPixels(pixbuf);

	int lineBytes = pixbuf.rect.w * pBitmap->pixelBits() / 8;

	std::vector<uint8_t> v;
	for( int y = 0 ; y < pixbuf.rect.h ; y++ )
		v.insert(v.end(), pixbuf.pixels + y * pixbuf.pitch, pixbuf.pixels + y * pixbuf.pitch + lineBytes);

	pBitmap->freePixelBuffer(pixbuf);
	return v;
}

//____ blueprint() ____________________________________________________________

static Bitmap::Blueprint blueprint(PixelFormat format, Color fillColor)
{
	bool bIndexed = (format == PixelFormat::Index_8);

	return WGBP(Bitmap, _.size = { 5, 3 }, _.format = format, _.fillColor = fillColor, _.palette = bIndexed ? s_palette : nullptr, _.paletteSize = bIndexed ? 4 : 0);
}

//____ filledPixels() _________________________________________________________
//
// What Bitmap::fill() makes of the color, which is what the fill color should give.

static std::vector<uint8_t> filledPixels(PixelFormat format, Color color)
{
	auto pBitmap = SoftBitmap::create(blueprint(format, Color::Undefined));
	pBitmap->fill(color);
	return bitmapPixels(pBitmap);
}

BitmapFillTest::BitmapFillTest()
{
	ADD_TEST(softBitmapTest);
	ADD_TEST(suppliedPixelsTest);
	ADD_TEST(streamTest);
	ADD_TEST(oldStreamTest);
}

BitmapFillTest::~BitmapFillTest()
{
}

bool BitmapFillTest::init(std::ostream& output)
{
	return true;
}

//____ softBitmapTest() ______________________________________________________

bool BitmapFillTest::softBitmapTest(std::ostream& output)
{
	const PixelFormat formats[] = { PixelFormat::ARGB_8, PixelFormat::XRGB_8, PixelFormat::RGB_565, PixelFormat::BGR_565, PixelFormat::Alpha_8, PixelFormat::Index_8 };

	const Color colors[] = { Color(Color8(200, 100, 50, 128)), Color(Color8(40, 160, 220, 255)), Color::TransparentBlack };

	for( PixelFormat format : formats )
	{
		for( Color color : colors )
		{
			auto pBitmap = SoftBitmap::create(blueprint(format, color));

			TEST_ASSERT( pBitmap );
			TEST_ASSERT( bitmapPixels(pBitmap) == filledPixels(format, color) );
		}
	}

	// The blueprint of a bitmap doesn't carry the fill color on: it says how the bitmap
	// was to start out, not what it is.

	auto pBitmap = SoftBitmap::create(blueprint(PixelFormat::ARGB_8, Color::White));
	TEST_ASSERT( pBitmap->blueprint().fillColor.isUndefined() );

	return true;
}

//____ suppliedPixelsTest() ___________________________________________________
//
// Pixels that are supplied are what the bitmap gets, whatever the fill color.

bool BitmapFillTest::suppliedPixelsTest(std::ostream& output)
{
	uint8_t pixels[5 * 3 * 4];
	for( int i = 0 ; i < int(sizeof(pixels)) ; i++ )
		pixels[i] = uint8_t(i * 7 + 3);

	auto bp = blueprint(PixelFormat::ARGB_8, Color::White);

	auto pBitmap = SoftBitmap::create(bp, pixels, PixelFormat::ARGB_8, 5 * 4);

	TEST_ASSERT( pBitmap );
	TEST_ASSERT( bitmapPixels(pBitmap) == std::vector<uint8_t>(pixels, pixels + sizeof(pixels)) );

	return true;
}

//____ streamTest() ___________________________________________________________
//
// A bitmap created with a fill color at one end of a stream starts out filled at the other.

bool BitmapFillTest::streamTest(std::ostream& output)
{
	auto pPlayer = StreamPlayer::create(SoftBackend::create(), SoftBitmapFactory::create(), SoftEdgemapFactory::create());
	auto pEncoder = StreamEncoder::create(StreamSink_p(pPlayer, pPlayer->input));

	Color color = Color(Color8(200, 100, 50, 128));

	auto pFilled = RemoteBitmap::create(pEncoder, blueprint(PixelFormat::ARGB_8, color));
	auto pIndexed = RemoteBitmap::create(pEncoder, blueprint(PixelFormat::Index_8, Color(s_palette[2])));
	pEncoder->flush();

	std::vector<Bitmap*> bitmaps;
	for( auto& pObject : pPlayer->objects() )
	{
		if( pObject )
			bitmaps.push_back(static_cast<Bitmap*>(pObject.rawPtr()));
	}

	TEST_ASSERT( bitmaps.size() == 2 );

	TEST_ASSERT( bitmaps[0]->pixelFormat() == PixelFormat::ARGB_8 );
	TEST_ASSERT( bitmapPixels(bitmaps[0]) == filledPixels(PixelFormat::ARGB_8, color) );

	TEST_ASSERT( bitmaps[1]->pixelFormat() == PixelFormat::Index_8 );
	TEST_ASSERT( bitmapPixels(bitmaps[1]) == filledPixels(PixelFormat::Index_8, Color(s_palette[2])) );

	return true;
}

//____ oldStreamTest() ________________________________________________________
//
// CreateBitmap chunks from before the fill color are 8 bytes shorter. They still give a
// bitmap, and what follows them is read from the right place.

bool BitmapFillTest::oldStreamTest(std::ostream& output)
{
	auto pPlayer = StreamPlayer::create(SoftBackend::create(), SoftBitmapFactory::create(), SoftEdgemapFactory::create());
	auto pEncoder = StreamEncoder::create(StreamSink_p(pPlayer, pPlayer->input));

	uint16_t oldId = pEncoder->allocObjectId();

	*pEncoder << GfxStream::Header{ GfxStream::ChunkId::CreateBitmap, 0, 40 };
	*pEncoder << oldId;
	*pEncoder << false;							// canvas
	*pEncoder << false;							// dynamic
	*pEncoder << PixelFormat::ARGB_8;
	*pEncoder << ColorSpace::sRGB;
	*pEncoder << uint8_t(0);					// bigEndian
	*pEncoder << int(0);						// identity
	*pEncoder << false;							// mipmap
	*pEncoder << SampleMethod::Nearest;
	*pEncoder << int(64);						// scale
	*pEncoder << SizeI(7, 2);
	*pEncoder << false;							// tiling
	*pEncoder << int(0);						// paletteCapacity
	*pEncoder << int(0);						// paletteSize

	Color color = Color(Color8(40, 160, 220, 255));

	auto pFilled = RemoteBitmap::create(pEncoder, blueprint(PixelFormat::ARGB_8, color));
	pEncoder->flush();

	std::vector<Bitmap*> bitmaps;
	for( auto& pObject : pPlayer->objects() )
	{
		if( pObject )
			bitmaps.push_back(static_cast<Bitmap*>(pObject.rawPtr()));
	}

	TEST_ASSERT( bitmaps.size() == 2 );

	TEST_ASSERT( bitmaps[0]->pixelSize() == SizeI(7, 2) );

	TEST_ASSERT( bitmaps[1]->pixelSize() == SizeI(5, 3) );
	TEST_ASSERT( bitmapPixels(bitmaps[1]) == filledPixels(PixelFormat::ARGB_8, color) );

	return true;
}
