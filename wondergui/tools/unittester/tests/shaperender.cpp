#include "shaperender.h"
#include "shapecoverage.h"

#include <wondergui.h>
#include <wax_device_gen2.h>
#include <wax_c_device.h>
#include <wax_util.h>
#include <wax_softbackend.h>
#include <wax_linearbackend.h>
#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_softedgemapfactory.h>
#include <wax_softkernels_default.h>
#include <wax_softshapes.h>
#include <wax_referencebackend.h>
#include <wax_streambackend.h>
#include <wax_streamplayer.h>
#include <wax_streamencoder.h>

#include <algorithm>
#include <cmath>
#include <functional>

using namespace wg;
using namespace wax;

/*
	Draws shapes through GfxDeviceGen2 and the software backends and checks the pixels
	against the coverage from SoftShapes and the rule for partly covered pixels:

	  out = dst * (1 - coverage) + blend(src, dst) * coverage

	Canvases are ARGB_8 in linear color space, so channels can be compared without
	color space conversions.
*/

namespace
{
	const SizeI c_canvasSize = { 64, 48 };

	struct C12
	{
		int		b, g, r, a;		// 0 -> 4096, linear.
	};

	C12 toC12(HiColor c)
	{
		HiColor l = c.toLinear();
		return { l.b, l.g, l.r, l.a };
	}

	//____ Canvas _____________________________________________________________

	class Canvas
	{
	public:
		Canvas()
		{
			surface = SoftSurface::create(WGBP(Surface, _.size = c_canvasSize, _.format = PixelFormat::ARGB_8, _.colorSpace = ColorSpace::Linear, _.canvas = true));
		}

		std::vector<uint8_t> read() const
		{
			std::vector<uint8_t> out(c_canvasSize.w * c_canvasSize.h * 4);

			auto buffer = surface->allocPixelBuffer();
			surface->pushPixels(buffer);

			for (int y = 0; y < c_canvasSize.h; y++)
				std::memcpy(out.data() + y * c_canvasSize.w * 4, buffer.pixels + y * buffer.pitch, c_canvasSize.w * 4);

			surface->freePixelBuffer(buffer);
			return out;
		}

		SoftSurface_p	surface;
	};

	// Channel of 8-bit BGRA pixel as 12-bit linear.

	inline int unpack(const std::vector<uint8_t>& pixels, int x, int y, int channel)
	{
		return HiColor::unpackLinearTab[pixels[(y * c_canvasSize.w + x) * 4 + channel]];
	}

	inline int byteAt(const std::vector<uint8_t>& pixels, int x, int y, int channel)
	{
		return pixels[(y * c_canvasSize.w + x) * 4 + channel];
	}

	GfxDeviceGen2_p createSoftDevice()
	{
		auto pBackend = SoftBackend::create();
		addDefaultSoftKernels(pBackend);
		return GfxDeviceGen2::create(pBackend);
	}

	//____ Coverage _________________________________________________________

	struct Coverage
	{
		std::vector<int>	fill;
		std::vector<int>	outline;
	};

	Coverage coverageOf(const RoundedBoxSpec& spec)
	{
		SoftShapes::RoundedBox box;
		box.rect = spec.rect;
		for (int i = 0; i < 4; i++)
			box.radius[i] = spec.radius[i];
		box.outline = spec.outline;

		RectI area(0, 0, c_canvasSize.w, c_canvasSize.h);

		ShapeCoverage grid(area, 2);
		SoftShapes shapes;
		grid.collect([&](SoftShapes::RowSink* pSink) { shapes.rasterizeRoundedBox(box, area, pSink); });

		Coverage cov;
		cov.fill.resize(area.w * area.h);
		cov.outline.resize(area.w * area.h);

		for (int y = 0; y < area.h; y++)
			for (int x = 0; x < area.w; x++)
			{
				cov.fill[y * area.w + x] = grid.coverage(0, x, y);
				cov.outline[y * area.w + x] = grid.coverage(1, x, y);
			}

		return cov;
	}

	//____ blendFull() ________________________________________________________
	// Result of blending src onto dst with full coverage, by the rules of "Blending and
	// transparency" in the guide. BlendFixedColor blends onto the fixed color, as SoftBackend
	// does to save reading the canvas, with straight colors.

	C12 blendFull(BlendMode mode, C12 s, C12 d, int morph, C12 fixed)
	{
		auto lim = [](int v) { return std::clamp(v, 0, 4096); };

		// Premultiplied colors weighted by wSrc and wDst, divided by the alpha of the result.

		auto paint = [](C12 s, int wSrc, C12 d, int wDst)
		{
			int a = (s.a * wSrc + d.a * wDst) / 4096;
			if (a == 0)
				return C12{ 0, 0, 0, 0 };

			auto ch = [&](int sc, int dc) { return (sc * s.a / 4096 * wSrc + dc * d.a / 4096 * wDst) / a; };
			return C12{ ch(s.b, d.b), ch(s.g, d.g), ch(s.r, d.r), a };
		};

		switch (mode)
		{
			case BlendMode::Replace:
				return s;
			case BlendMode::Blend:
				return paint(s, 4096, d, 4096 - s.a);
			case BlendMode::BlendFixedColor:
				return { (fixed.b * (4096 - s.a) + s.b * s.a) / 4096, (fixed.g * (4096 - s.a) + s.g * s.a) / 4096,
						 (fixed.r * (4096 - s.a) + s.r * s.a) / 4096, (fixed.a * (4096 - s.a) + 4096 * s.a) / 4096 };
			case BlendMode::Add:
				return { lim(d.b + s.b * s.a / 4096), lim(d.g + s.g * s.a / 4096), lim(d.r + s.r * s.a / 4096), d.a };
			case BlendMode::Subtract:
				return { lim(d.b - s.b * s.a / 4096), lim(d.g - s.g * s.a / 4096), lim(d.r - s.r * s.a / 4096), d.a };
			case BlendMode::Multiply:
				return { d.b * (4096 - s.a + s.b * s.a / 4096) / 4096, d.g * (4096 - s.a + s.g * s.a / 4096) / 4096,
						 d.r * (4096 - s.a + s.r * s.a / 4096) / 4096, d.a };
			case BlendMode::Invert:
			{
				auto inv = [&](int sc, int dc) { int e = sc * s.a / 4096; return (e * (4096 - dc) + dc * (4096 - e)) / 4096; };
				return { inv(s.b, d.b), inv(s.g, d.g), inv(s.r, d.r), d.a };
			}
			case BlendMode::Min:
				return { std::min(d.b, 4096 - s.a + s.b * s.a / 4096), std::min(d.g, 4096 - s.a + s.g * s.a / 4096),
						 std::min(d.r, 4096 - s.a + s.r * s.a / 4096), d.a };
			case BlendMode::Max:
				return { std::max(d.b, s.b * s.a / 4096), std::max(d.g, s.g * s.a / 4096), std::max(d.r, s.r * s.a / 4096), d.a };
			case BlendMode::Morph:
				return paint(s, morph, d, 4096 - morph);
			default:
				return d;
		}
	}

	// A partly covered pixel: the average of m (what the blend mode gives the covered part)
	// and the original pixel d, weighted by coverage and averaged as premultiplied colors.

	C12 mix(C12 d, C12 m, int coverage)
	{
		int inv = 4096 - coverage;
		int a = (d.a * inv + m.a * coverage) / 4096;
		if (a == 0)
			return C12{ 0, 0, 0, 0 };

		auto ch = [&](int dc, int mc) { return (dc * d.a / 4096 * inv + mc * m.a / 4096 * coverage) / a; };
		return C12{ ch(d.b, m.b), ch(d.g, m.g), ch(d.r, m.r), a };
	}

	// As mix(), but with straight colors, as SoftBackend mixes BlendFixedColor.

	C12 mixStraight(C12 d, C12 m, int coverage)
	{
		int inv = 4096 - coverage;
		return { (d.b * inv + m.b * coverage) / 4096, (d.g * inv + m.g * coverage) / 4096,
				 (d.r * inv + m.r * coverage) / 4096, (d.a * inv + m.a * coverage) / 4096 };
	}

	// Compares canvas to expected 12-bit colors, with tolerance in 8-bit units.

	bool compare(std::ostream& output, const char* pWhat, const std::vector<uint8_t>& pixels, int tolerance,
				 const std::function<C12(int x, int y)>& expected)
	{
		for (int y = 0; y < c_canvasSize.h; y++)
		{
			for (int x = 0; x < c_canvasSize.w; x++)
			{
				C12 e = expected(x, y);
				int want[4] = { HiColor::packLinearTab[std::clamp(e.b, 0, 4096)], HiColor::packLinearTab[std::clamp(e.g, 0, 4096)],
								HiColor::packLinearTab[std::clamp(e.r, 0, 4096)], HiColor::packLinearTab[std::clamp(e.a, 0, 4096)] };

				for (int ch = 0; ch < 4; ch++)
				{
					int got = byteAt(pixels, x, y, ch);
					if (std::abs(got - want[ch]) > tolerance)
					{
						output << pWhat << ": pixel " << x << "," << y << " channel " << ch << " (BGRA) is " << got << ", expected " << want[ch] << "." << std::endl;
						return false;
					}
				}
			}
		}
		return true;
	}

	RoundedBoxSpec testBox()
	{
		RoundedBoxSpec spec;
		spec.rect = { 64 * 5 + 19, 64 * 4 + 45, 64 * 40 + 32, 64 * 30 + 13 };
		spec.radius[0] = 64 * 9 + 32;
		spec.radius[1] = 64 * 4;
		spec.radius[2] = 64 * 12 + 7;
		spec.radius[3] = 64 * 2 + 10;
		return spec;
	}

	const HiColor c_background = HiColor(800, 2500, 3600, 4096);
}

//____ Constructor ____________________________________________________________

ShapeRenderTest::ShapeRenderTest()
{
	ADD_TEST(blendModesTest);
	ADD_TEST(fillAndOutlineTest);
	ADD_TEST(tintColorTest);
	ADD_TEST(gradientTintTest);
	ADD_TEST(clipListTest);
	ADD_TEST(linearBackendTest);
	ADD_TEST(streamTest);
	ADD_TEST(cApiTest);
	ADD_TEST(alignedBoxTest);
	ADD_TEST(boxSkinTest);
	ADD_TEST(ringTest);
	ADD_TEST(undefinedColorTest);
	ADD_TEST(elipseTest);
	ADD_TEST(referenceCoverageTest);
}

ShapeRenderTest::~ShapeRenderTest()
{
}

//____ blendModesTest() _______________________________________________________
// Single segment box in all blend modes, partly covered pixels blended against an opaque background.

bool ShapeRenderTest::blendModesTest(std::ostream& output)
{
	BlendMode modes[] = { BlendMode::Replace, BlendMode::Blend, BlendMode::BlendFixedColor, BlendMode::Add, BlendMode::Subtract,
						  BlendMode::Multiply, BlendMode::Invert, BlendMode::Min, BlendMode::Max, BlendMode::Morph };

	RoundedBoxSpec spec = testBox();

	Coverage cov = coverageOf(spec);

	HiColor fixedColor(1000, 3000, 2000, 2500);
	float morphFactor = 0.3f;

	// Second color is bright enough to saturate Add.

	HiColor colors[2] = { HiColor(3000, 1500, 600, 3000), HiColor(4096, 3500, 3900, 3500) };

	for (auto color : colors)
	for (auto mode : modes)
	{
		spec.fillColor = color;

		Canvas canvas;
		auto pDevice = createSoftDevice();

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(canvas.surface);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(c_background);
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		auto before = canvas.read();

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(canvas.surface);
		pDevice->setBlendMode(mode);
		pDevice->setFixedBlendColor(fixedColor);
		pDevice->setMorphFactor(morphFactor);
		pDevice->drawRoundedBox(spec);
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		auto after = canvas.read();

		C12 src = toC12(spec.fillColor);
		C12 fixed = toC12(fixedColor);
		int morph = int(morphFactor * 4096);

		std::string what = std::string("BlendMode::") + toString(mode);

		// Paint modes average the covered and uncovered parts of a pixel, effect modes are applied
		// with a strength scaled by coverage, which fades the alpha of the source.

		bool bEffect = (mode == BlendMode::Add || mode == BlendMode::Subtract || mode == BlendMode::Multiply ||
						mode == BlendMode::Invert || mode == BlendMode::Min || mode == BlendMode::Max);

		bool bOk = compare(output, what.c_str(), after, 3, [&](int x, int y)
		{
			C12 dst = { unpack(before, x, y, 0), unpack(before, x, y, 1), unpack(before, x, y, 2), unpack(before, x, y, 3) };
			int c = cov.fill[y * c_canvasSize.w + x];

			if (bEffect)
				return blendFull(mode, { src.b, src.g, src.r, src.a * c / 4096 }, dst, morph, fixed);

			C12 full = blendFull(mode, src, dst, morph, fixed);
			return mode == BlendMode::BlendFixedColor ? mixStraight(dst, full, c) : mix(dst, full, c);
		});

		if (!bOk)
			return false;
	}

	return true;
}

//____ fillAndOutlineTest() ___________________________________________________
// Fill and semi-transparent outline in Blend mode, colors mixed where they share pixels.

bool ShapeRenderTest::fillAndOutlineTest(std::ostream& output)
{
	RoundedBoxSpec spec = testBox();
	spec.outline = { 64 + 20, 64 * 3 + 5, 40, 64 * 2 };
	spec.fillColor = HiColor(3000, 1500, 600, 4096);
	spec.outlineColor = HiColor(200, 4000, 1000, 2600);

	Coverage cov = coverageOf(spec);

	Canvas canvas;
	auto pDevice = createSoftDevice();

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(canvas.surface);
	pDevice->setBlendMode(BlendMode::Replace);
	pDevice->fill(c_background);
	pDevice->setBlendMode(BlendMode::Blend);
	pDevice->drawRoundedBox(spec);
	pDevice->endCanvasUpdate();
	pDevice->endRender();

	auto pixels = canvas.read();

	C12 bg = toC12(c_background);
	C12 f = toC12(spec.fillColor);
	C12 o = toC12(spec.outlineColor);

	return compare(output, "Fill and outline", pixels, 3, [&](int x, int y)
	{
		int cf = cov.fill[y * c_canvasSize.w + x] * f.a / 4096;
		int co = cov.outline[y * c_canvasSize.w + x] * o.a / 4096;
		int back = 4096 - cf - co;

		return C12{ (bg.b * back + f.b * cf + o.b * co) / 4096, (bg.g * back + f.g * cf + o.g * co) / 4096,
					(bg.r * back + f.r * cf + o.r * co) / 4096, (bg.a * back + 4096 * (cf + co)) / 4096 };
	});
}

//____ tintColorTest() ________________________________________________________

bool ShapeRenderTest::tintColorTest(std::ostream& output)
{
	RoundedBoxSpec spec = testBox();
	spec.outline = { 64 * 2, 64 * 2, 64 * 2, 64 * 2 };
	spec.fillColor = HiColor(3000, 1500, 600, 4096);
	spec.outlineColor = HiColor(200, 4000, 1000, 4096);

	HiColor tintColor(4096, 2048, 1024, 4096);

	Coverage cov = coverageOf(spec);

	Canvas canvas;
	auto pDevice = createSoftDevice();

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(canvas.surface);
	pDevice->setBlendMode(BlendMode::Replace);
	pDevice->fill(c_background);
	pDevice->setBlendMode(BlendMode::Blend);
	pDevice->setTintColor(tintColor);
	pDevice->drawRoundedBox(spec);
	pDevice->endCanvasUpdate();
	pDevice->endRender();

	auto pixels = canvas.read();

	C12 bg = toC12(c_background);
	C12 t = toC12(tintColor);
	C12 f = toC12(spec.fillColor);
	C12 o = toC12(spec.outlineColor);

	f = { f.b * t.b / 4096, f.g * t.g / 4096, f.r * t.r / 4096, f.a * t.a / 4096 };
	o = { o.b * t.b / 4096, o.g * t.g / 4096, o.r * t.r / 4096, o.a * t.a / 4096 };

	return compare(output, "Tint color", pixels, 3, [&](int x, int y)
	{
		int cf = cov.fill[y * c_canvasSize.w + x];
		int co = cov.outline[y * c_canvasSize.w + x];
		int back = 4096 - cf - co;

		return C12{ (bg.b * back + f.b * cf + o.b * co) / 4096, (bg.g * back + f.g * cf + o.g * co) / 4096,
					(bg.r * back + f.r * cf + o.r * co) / 4096, 4096 };
	});
}

//____ gradientTintTest() _____________________________________________________
// Gradient tints on fill and outline.

bool ShapeRenderTest::gradientTintTest(std::ostream& output)
{
	RoundedBoxSpec spec = testBox();
	spec.outline = { 64 * 3, 64 * 3, 64 * 3, 64 * 3 };
	spec.fillColor = HiColor::White;
	spec.outlineColor = HiColor::White;

	auto pFillTint = Tint::create(HiColor(4096, 0, 0, 4096), HiColor(0, 0, 4096, 4096));							// Vertical
	auto pOutlineTint = Tint::create(HiColor(0, 4096, 0, 4096), HiColor(4096, 4096, 0, 4096), { 0.f, 0.f }, { 1.f, 0.f });	// Horizontal

	spec.pFillTint = pFillTint;
	spec.pOutlineTint = pOutlineTint;

	Coverage cov = coverageOf(spec);

	Canvas canvas;
	auto pDevice = createSoftDevice();

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(canvas.surface);
	pDevice->setBlendMode(BlendMode::Replace);
	pDevice->fill(c_background);
	pDevice->setBlendMode(BlendMode::Blend);
	pDevice->drawRoundedBox(spec);
	pDevice->endCanvasUpdate();
	pDevice->endRender();

	auto pixels = canvas.read();

	C12 bg = toC12(c_background);

	return compare(output, "Gradient tints", pixels, 4, [&](int x, int y)
	{
		C12 f = toC12(pFillTint->colorAt({ x * 64, y * 64 }, spec.rect));
		C12 o = toC12(pOutlineTint->colorAt({ x * 64, y * 64 }, spec.rect));

		int cf = cov.fill[y * c_canvasSize.w + x];
		int co = cov.outline[y * c_canvasSize.w + x];
		int back = 4096 - cf - co;

		return C12{ (bg.b * back + f.b * cf + o.b * co) / 4096, (bg.g * back + f.g * cf + o.g * co) / 4096,
					(bg.r * back + f.r * cf + o.r * co) / 4096, 4096 };
	});
}

//____ clipListTest() _________________________________________________________

bool ShapeRenderTest::clipListTest(std::ostream& output)
{
	RoundedBoxSpec spec = testBox();
	spec.outline = { 64 * 2, 64 * 2, 64 * 2, 64 * 2 };
	spec.fillColor = HiColor(3000, 1500, 600, 3500);
	spec.outlineColor = HiColor(200, 4000, 1000, 4096);

	RectSPX clips[2] = { { 64 * 3, 64 * 2, 64 * 20, 64 * 15 }, { 64 * 30, 64 * 20, 64 * 30, 64 * 25 } };

	auto render = [&](bool bClip)
	{
		Canvas canvas;
		auto pDevice = createSoftDevice();

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(canvas.surface);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(c_background);
		pDevice->setBlendMode(BlendMode::Blend);

		if (bClip)
			pDevice->setClipList(2, clips);

		pDevice->drawRoundedBox(spec);
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		return canvas.read();
	};

	auto clipped = render(true);
	auto whole = render(false);

	C12 bg = toC12(c_background);

	return compare(output, "Clipped", clipped, 0, [&](int x, int y)
	{
		CoordSPX pos(x * 64, y * 64);
		if (clips[0].contains(pos) || clips[1].contains(pos))
			return C12{ unpack(whole, x, y, 0), unpack(whole, x, y, 1), unpack(whole, x, y, 2), unpack(whole, x, y, 3) };
		else
			return bg;
	});
}

//____ Scene for comparing backends _______________________________________

namespace
{
	// Shapes drawn directly or through the C API, which plugins draw through.

	void drawRoundedBox(GfxDevice* pDevice, const RoundedBoxSpec& spec, bool bCApi)
	{
		if (!bCApi)
		{
			pDevice->drawRoundedBox(spec);
			return;
		}

		wax_roundedBoxSpec box;
		box.rect = *(const wax_rectSPX*)&spec.rect;

		for (int i = 0; i < 4; i++)
			box.radius[i] = spec.radius[i];

		box.outline = *(const wax_borderSPX*)&spec.outline;
		box.fillColor = *(const wax_color*)&spec.fillColor;
		box.outlineColor = *(const wax_color*)&spec.outlineColor;
		box.fillTint = static_cast<Object*>(spec.pFillTint);
		box.outlineTint = static_cast<Object*>(spec.pOutlineTint);

		wax_drawRoundedBox(static_cast<Object*>(pDevice), &box);
	}

	void drawRing(GfxDevice* pDevice, const RingSpec& spec, bool bCApi)
	{
		if (!bCApi)
		{
			pDevice->drawRing(spec);
			return;
		}

		std::vector<wax_ringSlice> slices(spec.nSlices);

		for (int i = 0; i < spec.nSlices; i++)
			slices[i] = { spec.pSlices[i].size, *(const wax_color*)&spec.pSlices[i].color, static_cast<Object*>(spec.pSlices[i].pTint) };

		wax_ringSpec ring;
		ring.rect = *(const wax_rectSPX*)&spec.rect;
		ring.thickness = spec.thickness;
		ring.outline = spec.outline;
		ring.start = spec.start;
		ring.nSlices = spec.nSlices;
		ring.pSlices = slices.data();
		ring.bSquare = spec.bSquare;
		ring.outlineColor = *(const wax_color*)&spec.outlineColor;
		ring.outlineTint = static_cast<Object*>(spec.pOutlineTint);
		ring.hubColor = *(const wax_color*)&spec.hubColor;
		ring.hubTint = static_cast<Object*>(spec.pHubTint);
		ring.backColor = *(const wax_color*)&spec.backColor;
		ring.backTint = static_cast<Object*>(spec.pBackTint);

		wax_drawRing(static_cast<Object*>(pDevice), &ring);
	}

	void drawScene(GfxDevice* pDevice, bool bCApi = false)
	{
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(c_background);

		RoundedBoxSpec spec = testBox();
		spec.outline = { 64 + 20, 64 * 3 + 5, 40, 64 * 2 };
		spec.fillColor = HiColor(3000, 1500, 600, 3500);
		spec.outlineColor = HiColor(200, 4000, 1000, 2600);

		auto pTint = Tint::create(HiColor(4096, 0, 0, 4096), HiColor(0, 0, 4096, 2000));
		spec.pOutlineTint = pTint;

		pDevice->setBlendMode(BlendMode::Blend);
		drawRoundedBox(pDevice, spec, bCApi);

		RoundedBoxSpec pill;
		pill.rect = { 64 * 2 + 10, 64 * 30 + 50, 64 * 50, 64 * 12 + 20 };
		pill.radius[0] = pill.radius[1] = pill.radius[2] = pill.radius[3] = 64 * 1000;
		pill.fillColor = HiColor(4096, 4096, 0, 2048);

		pDevice->setBlendMode(BlendMode::Replace);
		drawRoundedBox(pDevice, pill, bCApi);

		pDevice->setBlendMode(BlendMode::Add);
		pDevice->setTintColor(HiColor(2048, 4096, 4096, 4096));
		pill.rect.y -= 64 * 20 + 13;
		drawRoundedBox(pDevice, pill, bCApi);
		pDevice->clearTintColor();

		auto pSliceTint = Tint::create(HiColor(4096, 4096, 4096, 4096), HiColor(0, 0, 0, 4096), { 0.f, 0.f }, { 1.f, 1.f });

		RingSlice slices[4] = { { 0.2f, HiColor(4096, 0, 0, 4096) }, { 0.3f, HiColor(0, 4096, 0, 3000), pSliceTint },
								{ 0.1f, HiColor(0, 0, 4096, 4096) }, { 0.15f, HiColor(4096, 4096, 0, 4096) } };

		RingSpec ring;
		ring.rect = { 64 * 30 + 21, 64 * 3 + 9, 64 * 30 + 40, 64 * 30 + 40 };
		ring.thickness = 64 * 9;
		ring.outline = 64 + 20;
		ring.start = 0.85f;
		ring.nSlices = 4;
		ring.pSlices = slices;
		ring.outlineColor = HiColor(1000, 1000, 1000, 4096);
		ring.hubColor = HiColor(4096, 4096, 4096, 2000);
		ring.backColor = HiColor(0, 0, 0, 1500);

		pDevice->setBlendMode(BlendMode::Blend);
		drawRing(pDevice, ring, bCApi);
	}
}

//____ linearBackendTest() ____________________________________________________
// LinearBackend must draw exactly what SoftBackend draws.

bool ShapeRenderTest::linearBackendTest(std::ostream& output)
{
	Canvas canvas;
	auto pDevice = createSoftDevice();

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(canvas.surface);
	drawScene(pDevice);
	pDevice->endCanvasUpdate();
	pDevice->endRender();

	auto softPixels = canvas.read();

	// Same with LinearBackend, rendered in two update rectangles, created from a blueprint
	// with the mirror buffer allocated up front.

	std::vector<uint8_t> buffer(c_canvasSize.w * c_canvasSize.h * 4 * 2);
	std::vector<uint8_t> linearPixels(c_canvasSize.w * c_canvasSize.h * 4, 0);

	auto pLinearBackend = LinearBackend::create(
		WAXBP(LinearBackend, _.mirrorBuffer = SoftShapes::c_defaultMaxMirrorBuffer, _.maxMirrorBuffer = SoftShapes::c_defaultMaxMirrorBuffer),
		[&](CanvasRef ref, int bytes)
		{
			return (void*)buffer.data();
		},
		[&](CanvasRef ref, int nSegments, const LinearBackend::Segment* pSegments)
		{
			for (int i = 0; i < nSegments; i++)
			{
				auto& seg = pSegments[i];
				for (int y = 0; y < seg.rect.h; y++)
					std::memcpy(linearPixels.data() + ((seg.rect.y + y) * c_canvasSize.w + seg.rect.x) * 4, seg.pBuffer + y * seg.pitch, seg.rect.w * 4);
			}
		});

	addDefaultSoftKernels(pLinearBackend);
	pLinearBackend->defineCanvas(CanvasRef::Default, c_canvasSize * 64, PixelFormat::ARGB_8, 64, ColorSpace::Linear);

	auto pLinearDevice = GfxDeviceGen2::create(pLinearBackend);

	RectSPX updateRects[2] = { { 0, 0, c_canvasSize.w * 64, 64 * 20 }, { 0, 64 * 20, c_canvasSize.w * 64, (c_canvasSize.h - 20) * 64 } };

	pLinearDevice->beginRender();
	pLinearDevice->beginCanvasUpdate(CanvasRef::Default, 2, updateRects);
	drawScene(pLinearDevice);
	pLinearDevice->endCanvasUpdate();
	pLinearDevice->endRender();

	for (size_t i = 0; i < softPixels.size(); i++)
	{
		if (softPixels[i] != linearPixels[i])
		{
			int pixel = int(i / 4);
			output << "Pixel " << pixel % c_canvasSize.w << "," << pixel / c_canvasSize.w << " channel " << i % 4
				   << " is " << int(linearPixels[i]) << " with LinearBackend, " << int(softPixels[i]) << " with SoftBackend." << std::endl;
			return false;
		}
	}

	return true;
}

//____ streamTest() ___________________________________________________________
// Shapes streamed through StreamBackend and StreamPlayer must draw exactly as directly.

bool ShapeRenderTest::streamTest(std::ostream& output)
{
	Canvas directCanvas;
	auto pDevice = createSoftDevice();

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(directCanvas.surface);
	drawScene(pDevice);
	pDevice->endCanvasUpdate();
	pDevice->endRender();

	auto directPixels = directCanvas.read();

	// Through a stream

	Canvas streamedCanvas;

	auto pBackend = SoftBackend::create();
	addDefaultSoftKernels(pBackend);
	pBackend->defineCanvas(CanvasRef::Default, streamedCanvas.surface);

	auto pStreamPlayer = StreamPlayer::create(pBackend, SoftSurfaceFactory::create(), SoftEdgemapFactory::create());
	auto pStreamEncoder = StreamEncoder::create({ pStreamPlayer, pStreamPlayer->input });
	auto pStreamBackend = StreamBackend::create(pStreamEncoder);
	pStreamBackend->defineCanvas(CanvasRef::Default, c_canvasSize, PixelFormat::ARGB_8, 64, ColorSpace::Linear);

	auto pStreamDevice = GfxDeviceGen2::create(pStreamBackend);

	pStreamDevice->beginRender();
	pStreamDevice->beginCanvasUpdate(CanvasRef::Default);
	drawScene(pStreamDevice);
	pStreamDevice->endCanvasUpdate();
	pStreamDevice->endRender();

	auto streamedPixels = streamedCanvas.read();

	for (size_t i = 0; i < directPixels.size(); i++)
	{
		if (directPixels[i] != streamedPixels[i])
		{
			int pixel = int(i / 4);
			output << "Pixel " << pixel % c_canvasSize.w << "," << pixel / c_canvasSize.w << " channel " << i % 4
				   << " is " << int(streamedPixels[i]) << " streamed, " << int(directPixels[i]) << " direct." << std::endl;
			return false;
		}
	}

	return true;
}

//____ cApiTest() _____________________________________________________________
// Shapes drawn through the C API, as plugins do, must draw exactly as directly.

bool ShapeRenderTest::cApiTest(std::ostream& output)
{
	auto pDevice = createSoftDevice();
	std::vector<uint8_t> pixels[2];

	for (int i = 0; i < 2; i++)
	{
		Canvas canvas;

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(canvas.surface);
		drawScene(pDevice, i == 1);
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		pixels[i] = canvas.read();
	}

	for (size_t i = 0; i < pixels[0].size(); i++)
	{
		if (pixels[0][i] != pixels[1][i])
		{
			int pixel = int(i / 4);
			output << "Pixel " << pixel % c_canvasSize.w << "," << pixel / c_canvasSize.w << " channel " << i % 4
				   << " is " << int(pixels[1][i]) << " through the C API, " << int(pixels[0][i]) << " direct." << std::endl;
			return false;
		}
	}

	return true;
}

//____ alignedBoxTest() _______________________________________________________
// Square boxes aligned to pixels are drawn as fills.

bool ShapeRenderTest::alignedBoxTest(std::ostream& output)
{
	RoundedBoxSpec spec;
	spec.rect = { 64 * 4, 64 * 5, 64 * 30, 64 * 20 };
	spec.outline = { 64 * 2, 64 * 3, 64 * 1, 64 * 4 };
	spec.fillColor = HiColor(3000, 1500, 600, 4096);
	spec.outlineColor = HiColor(200, 4000, 1000, 4096);

	Canvas canvas;
	auto pDevice = createSoftDevice();

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(canvas.surface);
	pDevice->setBlendMode(BlendMode::Replace);
	pDevice->fill(c_background);
	pDevice->setBlendMode(BlendMode::Blend);
	pDevice->drawRoundedBox(spec);
	pDevice->endCanvasUpdate();
	pDevice->endRender();

	auto pixels = canvas.read();

	RectI outer = spec.rect / 64;
	RectI inner = (spec.rect - spec.outline) / 64;

	return compare(output, "Aligned box", pixels, 1, [&](int x, int y)
	{
		if (inner.contains(CoordI(x, y)))
			return toC12(spec.fillColor);
		if (outer.contains(CoordI(x, y)))
			return toC12(spec.outlineColor);
		return toC12(c_background);
	});
}

//____ boxSkinTest() __________________________________________________________
// BoxSkin with corner radii draws a rounded box and respects the corners when mark testing.

bool ShapeRenderTest::boxSkinTest(std::ostream& output)
{
	auto pSkin = BoxSkin::create(WGBP(BoxSkin,
		_.color = HiColor(3000, 1500, 600, 4096),
		_.outlineColor = HiColor(200, 4000, 1000, 4096),
		_.outlineThickness = { 2, 3, 2, 1 },
		_.cornerRadius = { 8, 8, 3, 0 }));

	RectSPX canvas = { 64 * 5, 64 * 4, 64 * 40, 64 * 30 };
	int scale = 64;

	// Skin

	Canvas skinCanvas;
	auto pDevice = createSoftDevice();

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(skinCanvas.surface);
	pDevice->setBlendMode(BlendMode::Replace);
	pDevice->fill(c_background);
	pDevice->setBlendMode(BlendMode::Blend);
	pSkin->_render(pDevice, canvas, scale, State::Default);
	pDevice->endCanvasUpdate();
	pDevice->endRender();

	// Same box drawn directly

	RoundedBoxSpec spec;
	spec.rect = canvas;
	spec.radius = { 64 * 8, 64 * 8, 64 * 3, 0 };
	spec.outline = { 64 * 2, 64 * 3, 64 * 2, 64 * 1 };
	spec.fillColor = HiColor(3000, 1500, 600, 4096);
	spec.outlineColor = HiColor(200, 4000, 1000, 4096);

	Canvas boxCanvas;

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(boxCanvas.surface);
	pDevice->setBlendMode(BlendMode::Replace);
	pDevice->fill(c_background);
	pDevice->setBlendMode(BlendMode::Blend);
	pDevice->drawRoundedBox(spec);
	pDevice->endCanvasUpdate();
	pDevice->endRender();

	auto skinPixels = skinCanvas.read();
	auto boxPixels = boxCanvas.read();

	TEST_ASSERT(skinPixels == boxPixels);

	// Mark test

	TEST_ASSERT(!pSkin->_markTest({ canvas.x + 64, canvas.y + 64 }, canvas, scale, State::Default));					// Outside top-left corner.
	TEST_ASSERT(pSkin->_markTest({ canvas.x + 64 * 3, canvas.y + 64 * 3 }, canvas, scale, State::Default));				// Inside it.
	TEST_ASSERT(pSkin->_markTest({ canvas.x + 64, canvas.y + canvas.h - 64 }, canvas, scale, State::Default));			// Square bottom-left corner.
	TEST_ASSERT(pSkin->_markTest({ canvas.x + canvas.w / 2, canvas.y + canvas.h / 2 }, canvas, scale, State::Default));

	// Coverage must lie inside the rounded box.

	RectSPX coverage = pSkin->_coverage(canvas, scale, State::Default);
	TEST_ASSERT(!coverage.isEmpty());

	spx radius[4] = { 64 * 8, 64 * 8, 64 * 3, 0 };
	TEST_ASSERT(wax::Util::isInsideRoundedBox(coverage.pos(), canvas, radius));
	TEST_ASSERT(wax::Util::isInsideRoundedBox({ coverage.x + coverage.w - 1, coverage.y }, canvas, radius));
	TEST_ASSERT(wax::Util::isInsideRoundedBox({ coverage.x, coverage.y + coverage.h - 1 }, canvas, radius));
	TEST_ASSERT(wax::Util::isInsideRoundedBox({ coverage.x + coverage.w - 1, coverage.y + coverage.h - 1 }, canvas, radius));

	return true;
}

//____ ringTest() _____________________________________________________________
// Ring with slices, outline, hub and back, against coverage from SoftShapes.

bool ShapeRenderTest::ringTest(std::ostream& output)
{
	RingSlice slices[5] = { { 0.1f, HiColor(4096, 0, 0, 4096) }, { 0.25f, HiColor(0, 4096, 0, 4096) },
							{ 0.05f, HiColor(0, 0, 4096, 4096) }, { 0.3f, HiColor(4096, 4096, 0, 4096) }, { 0.15f, HiColor(0, 4096, 4096, 4096) } };

	RingSpec ring;
	ring.rect = { 64 * 3 + 21, 64 * 2 + 9, 64 * 50 + 40, 64 * 42 + 10 };
	ring.thickness = 64 * 12 + 30;
	ring.outline = 64 + 20;
	ring.start = 0.9f;
	ring.nSlices = 5;
	ring.pSlices = slices;
	ring.outlineColor = HiColor(1000, 1000, 1000, 4096);
	ring.hubColor = HiColor(4096, 4096, 4096, 4096);
	ring.backColor = HiColor(2000, 0, 2000, 4096);

	// Coverage, converted as GfxDeviceGen2 does.

	uint32_t sizes[5];
	for (int i = 0; i < 5; i++)
		sizes[i] = uint32_t(double(slices[i].size) * GfxBackend::c_shapeFullTurn);

	SoftShapes::Ring shape;
	shape.rect = ring.rect;
	shape.thickness = ring.thickness;
	shape.outline = ring.outline;
	shape.start = uint32_t(double(ring.start - std::floor(ring.start)) * GfxBackend::c_shapeFullTurn) % GfxBackend::c_shapeFullTurn;
	shape.nSlices = 5;
	shape.pSliceSizes = sizes;

	RectI area(0, 0, c_canvasSize.w, c_canvasSize.h);

	ShapeCoverage cov(area, SoftShapes::c_ringFirstSliceSegment + 5);
	SoftShapes shapes;
	cov.collect([&](SoftShapes::RowSink* pSink) { shapes.rasterizeRing(shape, area, pSink); });

	// Render

	Canvas canvas;
	auto pDevice = createSoftDevice();

	pDevice->beginRender();
	pDevice->beginCanvasUpdate(canvas.surface);
	pDevice->setBlendMode(BlendMode::Replace);
	pDevice->fill(c_background);
	pDevice->setBlendMode(BlendMode::Blend);
	pDevice->drawRing(ring);
	pDevice->endCanvasUpdate();
	pDevice->endRender();

	auto pixels = canvas.read();

	HiColor segmentColors[8] = { ring.backColor, ring.outlineColor, ring.hubColor, slices[0].color, slices[1].color, slices[2].color, slices[3].color, slices[4].color };
	C12 colors[8];
	for (int i = 0; i < 8; i++)
		colors[i] = toC12(segmentColors[i]);

	C12 bg = toC12(c_background);

	return compare(output, "Ring", pixels, 3, [&](int x, int y)
	{
		int idx = y * c_canvasSize.w + x;
		int total = 0;
		C12 sum = { 0, 0, 0, 0 };

		for (int s = 0; s < 8; s++)
		{
			int c = cov.coverage(s, x, y);
			total += c;
			sum.b += colors[s].b * c;
			sum.g += colors[s].g * c;
			sum.r += colors[s].r * c;
		}

		int back = 4096 - total;
		return C12{ (bg.b * back + sum.b) / 4096, (bg.g * back + sum.g) / 4096, (bg.r * back + sum.r) / 4096, 4096 };
	});
}

//____ undefinedColorTest() ___________________________________________________
// Fill, hub and back left undefined are not drawn, also in Replace mode where a
// transparent color is written as is.

bool ShapeRenderTest::undefinedColorTest(std::ostream& output)
{
	RoundedBoxSpec box;
	box.rect = { 64 * 4, 64 * 4, 64 * 40, 64 * 30 };
	box.radius = 64 * 8;
	box.outline = { 64 * 3, 64 * 3, 64 * 3, 64 * 3 };

	RingSlice slice = { 1.f, HiColor(4096, 0, 0, 4096) };

	RingSpec ring;
	ring.rect = { 64 * 4, 64 * 2, 64 * 44, 64 * 44 };
	ring.thickness = 64 * 10;
	ring.nSlices = 1;
	ring.pSlices = &slice;

	struct Probe { const char* what; int x, y; };

	auto render = [&](bool bRing, bool bTransparent)
	{
		Canvas canvas;
		auto pDevice = createSoftDevice();

		RoundedBoxSpec b = box;
		RingSpec r = ring;

		if (bTransparent)
		{
			b.fillColor = HiColor::Transparent;
			r.hubColor = HiColor::Transparent;
			r.backColor = HiColor::Transparent;
		}

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(canvas.surface);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(c_background);
		if (bRing)
			pDevice->drawRing(r);
		else
			pDevice->drawRoundedBox(b);
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		return canvas.read();
	};

	auto check = [&](const char* shape, bool bRing, std::initializer_list<Probe> probes)
	{
		auto untouched = render(bRing, false);
		auto cleared = render(bRing, true);

		for (auto& p : probes)
		{
			int ofs = (p.y * c_canvasSize.w + p.x) * 4;

			if (std::memcmp(untouched.data() + ofs, untouched.data(), 4) != 0)		// Pixel 0,0 is outside shape.
			{
				output << shape << ": " << p.what << " left undefined was drawn." << std::endl;
				return false;
			}

			if (cleared[ofs + 3] != 0)
			{
				output << shape << ": transparent " << p.what << " was not written in Replace mode." << std::endl;
				return false;
			}
		}
		return true;
	};

	return check("RoundedBox", false, { { "fill", 24, 19 } }) &&
		   check("Ring", true, { { "hub", 26, 24 }, { "back", 5, 3 } });
}

//____ elipseTest() ___________________________________________________________
// Deprecated drawElipse(), now drawn with drawRing(): edges at radius r, r - outline,
// r - outline - thickness and r - 2 * outline - thickness.

bool ShapeRenderTest::elipseTest(std::ostream& output)
{
	const HiColor fill = HiColor(0, 0, 4096, 4096);
	const HiColor outline = HiColor(4096, 0, 0, 4096);

	struct Case { const char* name; spx thickness; spx outline; HiColor center; HiColor at9; HiColor at16; };

	const Case cases[] = {
		{ "ring", 64 * 4, 64 * 2, c_background, c_background, fill },			// Edges at 20, 18, 14, 12.
		{ "outline reaching center", 64 * 10, 64 * 6, outline, fill, outline },	// 20, 14, 4, -2.
		{ "fill reaching center", 64 * 19, 64 * 1, fill, fill, fill },			// 20, 19, 0, -1.
		{ "nothing", 0, 0, c_background, c_background, c_background }
	};

	for (auto& c : cases)
	{
		Canvas canvas;
		auto pDevice = createSoftDevice();

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(canvas.surface);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(c_background);
		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->drawElipse({ 64 * 4, 64 * 4, 64 * 40, 64 * 40 }, c.thickness, fill, c.outline, outline);
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		auto pixels = canvas.read();

		struct Probe { int x, y; HiColor color; const char* where; };
		const Probe probes[] = { { 24, 24, c.center, "center" }, { 24 + 9, 24, c.at9, "9 pixels from center" }, { 24, 24 - 16 - 1, c.at16, "16 pixels from center" } };

		for (auto& p : probes)
		{
			C12 want = toC12(p.color);
			for (int ch = 0; ch < 3; ch++)
			{
				int wanted = ch == 0 ? want.b : ch == 1 ? want.g : want.r;
				if (std::abs(unpack(pixels, p.x, p.y, ch) - wanted) > 64)
				{
					output << c.name << ": pixel " << p.where << " has wrong color." << std::endl;
					return false;
				}
			}
		}
	}

	return true;
}

//____ referenceCoverageTest() ________________________________________________
// ReferenceBackend draws shapes by the exact area of each pixel that is covered. Opaque white
// shapes drawn with Blend onto a transparent canvas leave that area in the alpha of the pixels
// and keep their color white, also where only partly covered. The area is checked against one
// worked out on its own here, from the span of the shape along vertical lines across the canvas.

namespace
{
	// Span from top to bottom of a shape along the vertical line at u, false if none.

	using Span = std::function<bool(double u, double& top, double& bottom)>;

	// Coverage of each pixel, integrated across it by the midpoint rule.

	std::vector<double> spanCoverage(const Span& span)
	{
		const int steps = 1024;
		std::vector<double> coverage(c_canvasSize.w * c_canvasSize.h, 0.0);

		for (int i = 0; i < c_canvasSize.w * steps; i++)
		{
			double top, bottom;
			if (!span((i + 0.5) / steps, top, bottom))
				continue;

			int x = i / steps;
			for (int y = std::max(0, int(std::floor(top))); y < std::min(c_canvasSize.h, int(std::ceil(bottom))); y++)
				coverage[y * c_canvasSize.w + x] += std::max(0.0, std::min(bottom, y + 1.0) - std::max(top, double(y))) / steps;
		}

		return coverage;
	}

	std::vector<uint8_t> renderWithReference(const std::function<void(GfxDevice* pDevice)>& draw)
	{
		Canvas canvas;
		auto pDevice = GfxDeviceGen2::create(ReferenceBackend::create());

		pDevice->beginRender();
		pDevice->beginCanvasUpdate(canvas.surface);
		pDevice->setBlendMode(BlendMode::Replace);
		pDevice->fill(HiColor::Transparent);
		pDevice->setBlendMode(BlendMode::Blend);
		draw(pDevice);
		pDevice->endCanvasUpdate();
		pDevice->endRender();

		return canvas.read();
	}
}

bool ShapeRenderTest::referenceCoverageTest(std::ostream& output)
{
	auto check = [&output](const char* pWhat, const std::vector<uint8_t>& pixels, const Span& span)
	{
		std::vector<double> coverage = spanCoverage(span);

		for (int y = 0; y < c_canvasSize.h; y++)
		{
			for (int x = 0; x < c_canvasSize.w; x++)
			{
				int alpha = byteAt(pixels, x, y, 3);
				int wanted = int(std::lround(coverage[y * c_canvasSize.w + x] * 255));

				if (std::abs(alpha - wanted) > 1)
				{
					output << pWhat << ": alpha of pixel " << x << "," << y << " is " << alpha << ", expected " << wanted << "." << std::endl;
					return false;
				}

				for (int ch = 0; ch < 3 && alpha > 0; ch++)
				{
					if (byteAt(pixels, x, y, ch) != 255)
					{
						output << pWhat << ": pixel " << x << "," << y << " isn't white." << std::endl;
						return false;
					}
				}
			}
		}

		return true;
	};

	// Rounded box, with an uneven outline. Fill and outline together cover the box.

	RoundedBoxSpec box = testBox();
	box.outline = { 64 * 2 + 13, 64 * 3 + 40, 64 + 5, 64 * 4 + 22 };
	box.fillColor = HiColor::White;
	box.outlineColor = HiColor::White;

	auto boxPixels = renderWithReference([&](GfxDevice* pDevice) { pDevice->drawRoundedBox(box); });

	bool bBoxOk = check("Rounded box", boxPixels, [&box](double u, double& top, double& bottom)
	{
		double left = box.rect.x / 64.0;
		double right = (box.rect.x + box.rect.w) / 64.0;

		if (u < left || u >= right)
			return false;

		top = box.rect.y / 64.0;
		bottom = (box.rect.y + box.rect.h) / 64.0;

		// How far in from the side a corner of radius r reaches at u, d from the side.

		auto corner = [](double r, double d) { return d < r ? r - std::sqrt(r * r - (r - d) * (r - d)) : 0.0; };

		top += std::max(corner(box.radius[0] / 64.0, u - left), corner(box.radius[1] / 64.0, right - u));
		bottom -= std::max(corner(box.radius[3] / 64.0, u - left), corner(box.radius[2] / 64.0, right - u));
		return true;
	});

	if (!bBoxOk)
		return false;

	// Ring with outline, hub and slices all white, which together cover a disc.

	const RectSPX ringRect = { 64 * 6 + 13, 64 * 4 + 40, 64 * 50 + 7, 64 * 39 + 50 };

	const double centerX = (ringRect.x + ringRect.w / 2.0) / 64.0;
	const double centerY = (ringRect.y + ringRect.h / 2.0) / 64.0;
	const double radius = std::min(ringRect.w, ringRect.h) / 128.0;

	auto disc = [=](double u, double& top, double& bottom)
	{
		double dx = u - centerX;
		if (std::abs(dx) >= radius)
			return false;

		double half = std::sqrt(radius * radius - dx * dx);
		top = centerY - half;
		bottom = centerY + half;
		return true;
	};

	RingSlice slices[3] = { { 0.25f }, { 0.375f }, { 0.375f } };

	RingSpec ring;
	ring.rect = ringRect;
	ring.thickness = 64 * 7 + 20;
	ring.outline = 64 + 50;
	ring.start = 0.37f;
	ring.nSlices = 3;
	ring.pSlices = slices;
	ring.outlineColor = HiColor::White;
	ring.hubColor = HiColor::White;

	auto ringPixels = renderWithReference([&](GfxDevice* pDevice) { pDevice->drawRing(ring); });

	if (!check("Ring", ringPixels, disc))
		return false;

	// A quarter of a pie facing 3 o'clock, between edges at 45 degrees.

	RingSlice quarter = { 0.25f };

	RingSpec pie;
	pie.rect = ringRect;
	pie.start = 0.125f;
	pie.nSlices = 1;
	pie.pSlices = &quarter;

	auto piePixels = renderWithReference([&](GfxDevice* pDevice) { pDevice->drawRing(pie); });

	return check("Pie", piePixels, [&](double u, double& top, double& bottom)
	{
		double dx = u - centerX;
		if (dx <= 0.0 || !disc(u, top, bottom))
			return false;

		top = std::max(top, centerY - dx);
		bottom = std::min(bottom, centerY + dx);
		return true;
	});
}
