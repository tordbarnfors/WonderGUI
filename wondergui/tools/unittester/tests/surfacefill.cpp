#include <string.h>

#include "surfacefill.h"

#include <wondergui.h>
#include <waxgfxstream.h>
#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_softedgemapfactory.h>
#include <wax_softbackend.h>

using namespace wg;
using namespace wax;

static const Color8 s_palette[4] = { Color8(0, 0, 0, 255), Color8(255, 0, 0, 255), Color8(40, 160, 220, 255), Color8(255, 255, 255, 255) };

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

//____ blueprint() ____________________________________________________________

static Surface::Blueprint blueprint(PixelFormat format, HiColor fillColor)
{
	bool bIndexed = (format == PixelFormat::Index_8);

	return WGBP(Surface, _.size = { 5, 3 }, _.format = format, _.fillColor = fillColor, _.palette = bIndexed ? s_palette : nullptr, _.paletteSize = bIndexed ? 4 : 0);
}

//____ filledPixels() _________________________________________________________
//
// What Surface::fill() makes of the color, which is what the fill color should give.

static std::vector<uint8_t> filledPixels(PixelFormat format, HiColor color)
{
	auto pSurface = SoftSurface::create(blueprint(format, HiColor::Undefined));
	pSurface->fill(color);
	return surfacePixels(pSurface);
}

SurfaceFillTest::SurfaceFillTest()
{
	ADD_TEST(softSurfaceTest);
	ADD_TEST(suppliedPixelsTest);
	ADD_TEST(streamTest);
	ADD_TEST(oldStreamTest);
}

SurfaceFillTest::~SurfaceFillTest()
{
}

bool SurfaceFillTest::init(std::ostream& output)
{
	return true;
}

//____ softSurfaceTest() ______________________________________________________

bool SurfaceFillTest::softSurfaceTest(std::ostream& output)
{
	const PixelFormat formats[] = { PixelFormat::ARGB_8, PixelFormat::XRGB_8, PixelFormat::RGB_565, PixelFormat::BGR_565, PixelFormat::Alpha_8, PixelFormat::Index_8 };

	const HiColor colors[] = { HiColor(Color8(200, 100, 50, 128)), HiColor(Color8(40, 160, 220, 255)), HiColor::TransparentBlack };

	for( PixelFormat format : formats )
	{
		for( HiColor color : colors )
		{
			auto pSurface = SoftSurface::create(blueprint(format, color));

			TEST_ASSERT( pSurface );
			TEST_ASSERT( surfacePixels(pSurface) == filledPixels(format, color) );
		}
	}

	// The blueprint of a surface doesn't carry the fill color on: it says how the surface
	// was to start out, not what it is.

	auto pSurface = SoftSurface::create(blueprint(PixelFormat::ARGB_8, HiColor::White));
	TEST_ASSERT( pSurface->blueprint().fillColor.isUndefined() );

	return true;
}

//____ suppliedPixelsTest() ___________________________________________________
//
// Pixels that are supplied are what the surface gets, whatever the fill color.

bool SurfaceFillTest::suppliedPixelsTest(std::ostream& output)
{
	uint8_t pixels[5 * 3 * 4];
	for( int i = 0 ; i < int(sizeof(pixels)) ; i++ )
		pixels[i] = uint8_t(i * 7 + 3);

	auto bp = blueprint(PixelFormat::ARGB_8, HiColor::White);

	auto pSurface = SoftSurface::create(bp, pixels, PixelFormat::ARGB_8, 5 * 4);

	TEST_ASSERT( pSurface );
	TEST_ASSERT( surfacePixels(pSurface) == std::vector<uint8_t>(pixels, pixels + sizeof(pixels)) );

	return true;
}

//____ streamTest() ___________________________________________________________
//
// A surface created with a fill color at one end of a stream starts out filled at the other.

bool SurfaceFillTest::streamTest(std::ostream& output)
{
	auto pPlayer = StreamPlayer::create(SoftBackend::create(), SoftSurfaceFactory::create(), SoftEdgemapFactory::create());
	auto pEncoder = StreamEncoder::create(StreamSink_p(pPlayer, pPlayer->input));

	HiColor color = HiColor(Color8(200, 100, 50, 128));

	auto pFilled = RemoteSurface::create(pEncoder, blueprint(PixelFormat::ARGB_8, color));
	auto pIndexed = RemoteSurface::create(pEncoder, blueprint(PixelFormat::Index_8, HiColor(s_palette[2])));
	pEncoder->flush();

	std::vector<Surface*> surfaces;
	for( auto& pObject : pPlayer->objects() )
	{
		if( pObject )
			surfaces.push_back(static_cast<Surface*>(pObject.rawPtr()));
	}

	TEST_ASSERT( surfaces.size() == 2 );

	TEST_ASSERT( surfaces[0]->pixelFormat() == PixelFormat::ARGB_8 );
	TEST_ASSERT( surfacePixels(surfaces[0]) == filledPixels(PixelFormat::ARGB_8, color) );

	TEST_ASSERT( surfaces[1]->pixelFormat() == PixelFormat::Index_8 );
	TEST_ASSERT( surfacePixels(surfaces[1]) == filledPixels(PixelFormat::Index_8, HiColor(s_palette[2])) );

	return true;
}

//____ oldStreamTest() ________________________________________________________
//
// CreateSurface chunks from before the fill color are 8 bytes shorter. They still give a
// surface, and what follows them is read from the right place.

bool SurfaceFillTest::oldStreamTest(std::ostream& output)
{
	auto pPlayer = StreamPlayer::create(SoftBackend::create(), SoftSurfaceFactory::create(), SoftEdgemapFactory::create());
	auto pEncoder = StreamEncoder::create(StreamSink_p(pPlayer, pPlayer->input));

	uint16_t oldId = pEncoder->allocObjectId();

	*pEncoder << GfxStream::Header{ GfxStream::ChunkId::CreateSurface, 0, 40 };
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

	HiColor color = HiColor(Color8(40, 160, 220, 255));

	auto pFilled = RemoteSurface::create(pEncoder, blueprint(PixelFormat::ARGB_8, color));
	pEncoder->flush();

	std::vector<Surface*> surfaces;
	for( auto& pObject : pPlayer->objects() )
	{
		if( pObject )
			surfaces.push_back(static_cast<Surface*>(pObject.rawPtr()));
	}

	TEST_ASSERT( surfaces.size() == 2 );

	TEST_ASSERT( surfaces[0]->pixelSize() == SizeI(7, 2) );

	TEST_ASSERT( surfaces[1]->pixelSize() == SizeI(5, 3) );
	TEST_ASSERT( surfacePixels(surfaces[1]) == filledPixels(PixelFormat::ARGB_8, color) );

	return true;
}
