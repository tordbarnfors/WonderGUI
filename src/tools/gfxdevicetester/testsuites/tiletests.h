#include "testsuite.h"

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
	}

	bool init(GfxDevice * pDevice, const RectI& canvas, wapp::API * pAppAPI)
	{
		m_pImg = pAppAPI->loadSurface("resources/small_tile.png", pDevice->surfaceFactory(), { .sampleMethod = SampleMethod::Bilinear, .tiling = true });
		if (!m_pImg)
			return false;

		return true;
	}
	 
	bool exit(GfxDevice * pDevice, const RectI& canvas)
	{
		m_pImg = nullptr;
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


private:
	Surface_p	m_pImg;
};
