#include "testsuite.h"

#include <vector>

class TileTests : public TestSuite
{
public:
	TileTests()
	{
		name = "TileTests";

		addTest("Tile", &TileTests::setup, &TileTests::tile, &TileTests::dummy);
		addTest("TileShift", &TileTests::setup, &TileTests::tileShift, &TileTests::dummy);
		addTest("TileRot90", &TileTests::setup, &TileTests::tileRot90, &TileTests::dummy);
		addTest("TileRot90Shift", &TileTests::setup, &TileTests::tileRot90Shift, &TileTests::dummy);
		addTest("TileRot180", &TileTests::setup, &TileTests::tileRot180, &TileTests::dummy);
		addTest("TileRot180Shift", &TileTests::setup, &TileTests::tileRot180Shift, &TileTests::dummy);
		addTest("TileRot270", &TileTests::setup, &TileTests::tileRot270, &TileTests::dummy);
		addTest("TileRot270Shift", &TileTests::setup, &TileTests::tileRot270Shift, &TileTests::dummy);
		addTest("TileFlipX", &TileTests::setup, &TileTests::tileFlipX, &TileTests::dummy);
		addTest("TileFlipXShift", &TileTests::setup, &TileTests::tileFlipXShift, &TileTests::dummy);
		addTest("TileFlipY", &TileTests::setup, &TileTests::tileFlipY, &TileTests::dummy);
		addTest("TileFlipYShift", &TileTests::setup, &TileTests::tileFlipYShift, &TileTests::dummy);

		addTest("TileScale", &TileTests::setup, &TileTests::tileScale, &TileTests::dummy);
		addTest("TileScaleShift", &TileTests::setup, &TileTests::tileScaleShift, &TileTests::dummy);
		addTest("TileScaleRot90", &TileTests::setup, &TileTests::tileScaleRot90, &TileTests::dummy);
		addTest("TileScaleRot90Shift", &TileTests::setup, &TileTests::tileScaleRot90Shift, &TileTests::dummy);
		addTest("TileScaleRot180", &TileTests::setup, &TileTests::tileScaleRot180, &TileTests::dummy);
		addTest("TileScaleRot180Shift", &TileTests::setup, &TileTests::tileScaleRot180Shift, &TileTests::dummy);
		addTest("TileScaleRot270", &TileTests::setup, &TileTests::tileScaleRot270, &TileTests::dummy);
		addTest("TileScaleRot270Shift", &TileTests::setup, &TileTests::tileScaleRot270Shift, &TileTests::dummy);
		addTest("TileScaleFlipX", &TileTests::setup, &TileTests::tileScaleFlipX, &TileTests::dummy);
		addTest("TileScaleFlipXShift", &TileTests::setup, &TileTests::tileScaleFlipXShift, &TileTests::dummy);
		addTest("TileScaleFlipY", &TileTests::setup, &TileTests::tileScaleFlipY, &TileTests::dummy);
		addTest("TileScaleFlipYShift", &TileTests::setup, &TileTests::tileScaleFlipYShift, &TileTests::dummy);

		addTest("TileRotScale1", &TileTests::setup, &TileTests::tileRotScale1, &TileTests::dummy);
		addTest("TileRotScale2", &TileTests::setup, &TileTests::tileRotScale2, &TileTests::dummy);

		// Tiling where the destination doesn't start or end on a whole tile, with negative shifts,
		// and from sources whose sizes aren't powers of two. The ramps and checker of the pattern
		// make a source pixel read from one pixel off show.

		addTest("TileOddRect", &TileTests::tileOddRect);
		addTest("TileNegativeShift", &TileTests::tileNegativeShift);
		addTest("TileNPOT", &TileTests::tileNPOT);
		addTest("TileNPOTNegativeShift", &TileTests::tileNPOTNegativeShift);
		addTest("TileFlips", &TileTests::tileFlips);
		addTest("TileFlipsNPOT", &TileTests::tileFlipsNPOT);
		addTest("TileScaleNPOT", &TileTests::tileScaleNPOT);

		// Scaled and flipped, nearest sampling. At scale 1.3 every 13th pixel reads exactly
		// at the boundary between two source pixels, stepping backwards when flipped or rotated.

		addTest("TileScaleFlipsNearest", &TileTests::tileScaleFlipsNearest);
		addTest("TileScaleFlipsNearestNPOT", &TileTests::tileScaleFlipsNearestNPOT);
		addTest("TileScaleDownFlipsNearestNPOT", &TileTests::tileScaleDownFlipsNearestNPOT);
		addTest("TileScaleFlipsBilinearNPOT", &TileTests::tileScaleFlipsBilinearNPOT);

		// Tinted, which some backends render in two passes, starting each chunk of lines anew.

		addTest("TileNPOTTinted", &TileTests::tileNPOTTinted);
		addTest("TileFlipsNPOTTinted", &TileTests::tileFlipsNPOTTinted);
		addTest("TileScaleFlipsNearestNPOTTinted", &TileTests::tileScaleFlipsNearestNPOTTinted);
	}

	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{
		m_pImg = pAppAPI->loadSurface("resources/small_tile.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Bilinear, .tiling = true });
		if (!m_pImg)
			return false;

		auto pFactory = pDevice->surfaceFactory();

		for (int i = 0; i < 2; i++)
		{
			SampleMethod method = i == 0 ? SampleMethod::Nearest : SampleMethod::Bilinear;

			m_pPow2[i] = _createPattern(pFactory, { 64, 64 }, method);
			m_pNPOT[i] = _createPattern(pFactory, { 60, 50 }, method);

			if (!m_pPow2[i] || !m_pNPOT[i])
				return false;
		}

		return true;
	}

	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pImg = nullptr;

		for (int i = 0; i < 2; i++)
		{
			m_pPow2[i] = nullptr;
			m_pNPOT[i] = nullptr;
		}
		return true;
	}

	bool dummy(GfxDevice * pDevice, const RectI& canvas)
	{
		return true;
	}

	bool setup(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pImg);
		return true;
	}

	bool	tile(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->tile(canvas);
		return true;
	}

	bool	tileRot90(GfxDevice * pDevice, const RectI& canvas)
	{
		pDevice->flipTile(canvas, GfxFlip::Rot90);
		return true;
	}

	bool	tileRot180(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->flipTile(canvas, GfxFlip::Rot180);
		return true;
	}

	bool	tileRot270(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->flipTile(canvas, GfxFlip::Rot270);
		return true;
	}

	bool	tileFlipX(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->flipTile(canvas, GfxFlip::FlipX);
		return true;
	}

	bool	tileFlipY(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->flipTile(canvas, GfxFlip::FlipY);
		return true;
	}

	bool	tileShift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->tile(canvas, { 20*64,10*64 } );
		return true;
	}

	bool	tileRot90Shift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->flipTile(canvas, GfxFlip::Rot90, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileRot180Shift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->flipTile(canvas, GfxFlip::Rot180, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileRot270Shift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->flipTile(canvas, GfxFlip::Rot270, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileFlipXShift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->flipTile(canvas, GfxFlip::FlipX, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileFlipYShift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->flipTile(canvas, GfxFlip::FlipY, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileScale(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleTile(canvas, 1.5f);
		return true;
	}

	bool	tileScaleRot90(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleFlipTile(canvas, 1.5f, GfxFlip::Rot90);
		return true;
	}

	bool	tileScaleRot180(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleFlipTile(canvas, 1.5f, GfxFlip::Rot180);
		return true;
	}

	bool	tileScaleRot270(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleFlipTile(canvas, 1.5f, GfxFlip::Rot270);
		return true;
	}

	bool	tileScaleFlipX(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleFlipTile(canvas, 1.5f, GfxFlip::FlipX);
		return true;
	}

	bool	tileScaleFlipY(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleFlipTile(canvas, 1.5f, GfxFlip::FlipY);
		return true;
	}

	bool	tileScaleShift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleTile(canvas, 1.5f, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileScaleRot90Shift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleFlipTile(canvas, 1.5f, GfxFlip::Rot90, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileScaleRot180Shift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleFlipTile(canvas, 1.5f, GfxFlip::Rot180, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileScaleRot270Shift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleFlipTile(canvas, 1.5f, GfxFlip::Rot270, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileScaleFlipXShift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleFlipTile(canvas, 1.5f, GfxFlip::FlipX, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileScaleFlipYShift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->scaleFlipTile(canvas, 1.5f, GfxFlip::FlipY, { 20 * 64,10 * 64 });
		return true;
	}

	bool	tileRotScale1(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->rotScaleBlit(canvas, 0.f, 1.f);
		return true;
	}

	bool	tileRotScale2(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->rotScaleBlit(canvas, 30.f, 1.5f);
		return true;
	}

	bool	tileOddRect(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pPow2[NEAREST]);
		pDevice->tile(_oddRect(canvas));
		return true;
	}

	bool	tileNegativeShift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pPow2[NEAREST]);
		pDevice->tile(_oddRect(canvas), { -20 * 64, -10 * 64 });
		return true;
	}

	bool	tileNPOT(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pNPOT[NEAREST]);
		pDevice->tile(_oddRect(canvas));
		return true;
	}

	bool	tileNPOTNegativeShift(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pNPOT[NEAREST]);
		pDevice->tile(_oddRect(canvas), { -27 * 64, 41 * 64 });
		return true;
	}

	bool	tileFlips(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pPow2[NEAREST]);
		_forEachFlip(canvas, [&](const RectI& cell, GfxFlip flip) { pDevice->flipTile(cell, flip, { 5 * 64, -7 * 64 }); });
		return true;
	}

	bool	tileFlipsNPOT(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pNPOT[NEAREST]);
		_forEachFlip(canvas, [&](const RectI& cell, GfxFlip flip) { pDevice->flipTile(cell, flip, { 5 * 64, -7 * 64 }); });
		return true;
	}

	bool	tileScaleNPOT(GfxDevice* pDevice, const RectI& canvas)
	{
		RectI left = { canvas.x, canvas.y, canvas.w / 2, canvas.h };
		RectI right = { canvas.x + canvas.w / 2, canvas.y, canvas.w - canvas.w / 2, canvas.h };

		pDevice->setBlitSource(m_pNPOT[NEAREST]);
		pDevice->scaleTile(left, 1.5f, { -11 * 64, 3 * 64 });
		pDevice->setBlitSource(m_pNPOT[BILINEAR]);
		pDevice->scaleTile(right, 1.5f, { -11 * 64, 3 * 64 });
		return true;
	}

	bool	tileScaleFlipsNearest(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pPow2[NEAREST]);
		_forEachFlip(canvas, [&](const RectI& cell, GfxFlip flip) { pDevice->scaleFlipTile(cell, 1.3f, flip); });
		return true;
	}

	bool	tileScaleFlipsNearestNPOT(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pNPOT[NEAREST]);
		_forEachFlip(canvas, [&](const RectI& cell, GfxFlip flip) { pDevice->scaleFlipTile(cell, 1.3f, flip); });
		return true;
	}

	bool	tileScaleDownFlipsNearestNPOT(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pNPOT[NEAREST]);
		_forEachFlip(canvas, [&](const RectI& cell, GfxFlip flip) { pDevice->scaleFlipTile(cell, 0.7f, flip); });
		return true;
	}

	bool	tileScaleFlipsBilinearNPOT(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setBlitSource(m_pNPOT[BILINEAR]);
		_forEachFlip(canvas, [&](const RectI& cell, GfxFlip flip) { pDevice->scaleFlipTile(cell, 1.3f, flip); });
		return true;
	}

	bool	tileNPOTTinted(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setTintColor(c_tint);
		pDevice->setBlitSource(m_pNPOT[NEAREST]);
		pDevice->tile(_oddRect(canvas), { -27 * 64, 41 * 64 });
		pDevice->clearTintColor();
		return true;
	}

	bool	tileFlipsNPOTTinted(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setTintColor(c_tint);
		pDevice->setBlitSource(m_pNPOT[NEAREST]);
		_forEachFlip(canvas, [&](const RectI& cell, GfxFlip flip) { pDevice->flipTile(cell, flip, { 5 * 64, -7 * 64 }); });
		pDevice->clearTintColor();
		return true;
	}

	bool	tileScaleFlipsNearestNPOTTinted(GfxDevice* pDevice, const RectI& canvas)
	{
		pDevice->setTintColor(c_tint);
		pDevice->setBlitSource(m_pNPOT[NEAREST]);
		_forEachFlip(canvas, [&](const RectI& cell, GfxFlip flip) { pDevice->scaleFlipTile(cell, 1.3f, flip); });
		pDevice->clearTintColor();
		return true;
	}

private:

	static const int NEAREST = 0, BILINEAR = 1;

	inline static const HiColor c_tint = HiColor(4096, 3000, 2000, 4096);

	// Opaque pattern where each pixel differs from its neighbors: red and green ramps
	// along x and y, and blue that changes with every step in either direction.

	Surface_p _createPattern(SurfaceFactory* pFactory, SizeI size, SampleMethod method)
	{
		std::vector<Color8> pixels(size.w * size.h);

		for (int y = 0; y < size.h; y++)
			for (int x = 0; x < size.w; x++)
				pixels[y * size.w + x] = Color8(uint8_t(x * 255 / (size.w - 1)), uint8_t(y * 255 / (size.h - 1)), uint8_t(((x ^ y) & 7) * 36), 255);

		return pFactory->createSurface(WGBP(Surface, _.size = size, _.format = PixelFormat::ARGB_8, _.sampleMethod = method, _.tiling = true),
									   (uint8_t*)pixels.data(), PixelFormat::ARGB_8);
	}

	// A destination that neither starts nor ends on a whole tile.

	RectI _oddRect(const RectI& canvas)
	{
		return { canvas.x + 13 * 64, canvas.y + 7 * 64, canvas.w - 31 * 64, canvas.h - 17 * 64 };
	}

	// Draws in a grid of 4x3 cells, one for each GfxFlip, of sizes that aren't whole tiles.

	template<typename Func>
	void _forEachFlip(const RectI& canvas, Func draw)
	{
		const int cellW = canvas.w / 4, cellH = canvas.h / 3;

		for (int i = 0; i < 12; i++)
		{
			RectI cell = { canvas.x + (i % 4) * cellW + 64, canvas.y + (i / 4) * cellH + 2 * 64, cellW - 3 * 64, cellH - 5 * 64 };
			draw(cell, GfxFlip(i));
		}
	}

	Surface_p	m_pImg;
	Surface_p	m_pPow2[2];
	Surface_p	m_pNPOT[2];
};
