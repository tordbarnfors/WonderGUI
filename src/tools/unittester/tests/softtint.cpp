#include <string.h>

#include "softtint.h"

#include <wondergui.h>
#include <wax_gfxdevice_gen2.h>
#include <wax_softbackend.h>
#include <wax_softkernels_default.h>
#include <wax_softsurface.h>
#include <wax_softtint.h>
#include <wax_tinttools.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace wg;

// SoftTint, which SoftBackend uses for tints, does integer math only so that it runs on CPUs
// without FPU. These tests compare it against references in double precision:
//
//  - Lookup tables against exact interpolation between stops.
//  - Rows of tint colors against the LUT entry for floor(position * N) at the pixel center, which
//    is what SoftTint calculates in fixed point. Positions within rounding of an entry boundary
//    may get the neighbor entry.
//  - Tinted fills and blits on SoftBackend against the tint evaluation of the GPU backends
//    (tint.hlsl), with the tolerance gfxdevicetester has between backends.

namespace
{
	//____ decode() ______________________________________________________________

	TintTools::DecodedTint decode(Tint* pTint, const RectSPX& rect)
	{
		uint16_t	words[TintTools::c_maxEncodedTintWords];
		HiColor		colors[TintTools::c_maxEncodedTintColors];
		int			nColors;

		TintTools::encodeTint(pTint, rect, words, colors, nColors);

		const HiColor* pColors = colors;
		TintTools::DecodedTint decoded;
		TintTools::decodeTint(words, pColors, decoded);
		return decoded;
	}

	//____ positionAt() __________________________________________________________
	// Position along the gradient at the center of pixel (x,y), no spread applied.

	double positionAt(const TintTools::TintLayer& layer, int x, int y)
	{
		TintTools::CanvasGeometry geo = TintTools::layerGeometry(layer);
		double px = x + 0.5, py = y + 0.5;

		if (geo.shape == TintShape::Linear)
			return double(geo.a) * px + double(geo.b) * py + double(geo.c);

		double u = (px - double(geo.centerX)) * double(geo.invRadiusX);
		double v = (py - double(geo.centerY)) * double(geo.invRadiusY);
		return std::sqrt(u * u + v * v);
	}

	//____ stopColorAt() _________________________________________________________
	// sRGB color (0.0 - 4096.0) between the stops at position t, no spread applied. Uses the last
	// stop at or before t, except that a hard edge at the first stop is to the left of it.

	void stopColorAt(const TintTools::TintLayer& layer, double t, double* pOut)
	{
		int n = layer.nStops;
		auto pos = [&](int s) { return layer.stopPos16[s] / 65535.0; };
		auto copy = [&](int s) { const HiColor& c = layer.stopColors[s]; pOut[0] = c.r; pOut[1] = c.g; pOut[2] = c.b; pOut[3] = c.a; };

		if (n == 1 || (t <= pos(0) && !(t == pos(0) && pos(1) == pos(0))))
			copy(0);
		else if (t >= pos(n - 1))
			copy(n - 1);
		else
		{
			int s = n - 1;
			while (pos(s) > t)
				s--;

			double f = (t - pos(s)) / (pos(s + 1) - pos(s));
			const HiColor& c0 = layer.stopColors[s];
			const HiColor& c1 = layer.stopColors[s + 1];

			pOut[0] = c0.r + (c1.r - c0.r) * f;
			pOut[1] = c0.g + (c1.g - c0.g) * f;
			pOut[2] = c0.b + (c1.b - c0.b) * f;
			pOut[3] = c0.a + (c1.a - c0.a) * f;
		}
	}

	//____ lutEntryColor() _______________________________________________________
	// Linear color of the LUT entry for index floor(position * N), spread applied. Entries are
	// sampled in the middle of the span they cover, except below 0.0 and above 1.0 with Pad.

	HiColor lutEntryColor(const TintTools::TintLayer& layer, int bits, int64_t idx, HiColor multiplier)
	{
		int64_t N = int64_t(1) << bits;
		double t;

		switch (layer.spread)
		{
			default:
			case TintSpread::Pad:
				t = idx < 0 ? 0.0 : idx >= N ? 1.0 : (idx + 0.5) / N;
				break;
			case TintSpread::Repeat:
				t = ((idx & (N - 1)) + 0.5) / N;
				break;
			case TintSpread::Reflect:
			{
				int64_t r = idx & (2 * N - 1);
				t = ((r >= N ? 2 * N - 1 - r : r) + 0.5) / N;
				break;
			}
		}

		double c[4];
		stopColorAt(layer, t, c);

		auto round = [](double v) { return int(std::floor(v + 0.5)); };
		HiColor color = HiColor(round(c[0]), round(c[1]), round(c[2]), round(c[3])).toLinear();
		return multiplier == HiColor::White ? color : color * multiplier;
	}

	//____ gpuTintColor() ________________________________________________________
	// Linear color (0.0 - 1.0) of a tint at the center of pixel (x,y), evaluated like tint.hlsl.
	// The tint color is sRGB, like it is given to GfxDevice.

	void gpuTintColor(const TintTools::DecodedTint& tint, int x, int y, HiColor tintColor, double* pOut)
	{
		auto toLinear = [](double c) { return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4); };

		for (int i = 0; i < 4; i++)
			pOut[i] = 0.0;

		for (int l = 0; l < tint.nLayers; l++)
		{
			const TintTools::TintLayer& layer = tint.layers[l];

			double t = positionAt(layer, x, y);

			switch (layer.spread)
			{
				default:
				case TintSpread::Pad:
					t = std::min(std::max(t, 0.0), 1.0);
					break;
				case TintSpread::Repeat:
					t = t - std::floor(t);
					break;
				case TintSpread::Reflect:
				{
					double r = t - 2.0 * std::floor(t * 0.5);
					t = r > 1.0 ? 2.0 - r : r;
					break;
				}
			}

			double c[4];
			stopColorAt(layer, t, c);

			double weight = layer.weight / 4096.0;
			for (int i = 0; i < 3; i++)
				pOut[i] += toLinear(c[i] / 4096.0) * weight;
			pOut[3] += c[3] / 4096.0 * weight;
		}

		pOut[0] *= toLinear(tintColor.r / 4096.0);
		pOut[1] *= toLinear(tintColor.g / 4096.0);
		pOut[2] *= toLinear(tintColor.b / 4096.0);
		pOut[3] *= tintColor.a / 4096.0;
	}

	//____ nearlyEqual() _________________________________________________________

	bool nearlyEqual(HiColor a, HiColor b, int tolerance)
	{
		return std::abs(a.r - b.r) <= tolerance && std::abs(a.g - b.g) <= tolerance &&
			   std::abs(a.b - b.b) <= tolerance && std::abs(a.a - b.a) <= tolerance;
	}
}

//____ SoftTintTest ___________________________________________________________

SoftTintTest::SoftTintTest()
{
	ADD_TEST(lookupTableTest);
	ADD_TEST(tintRowsTest);
	ADD_TEST(tintedRenderingTest);
}

SoftTintTest::~SoftTintTest()
{
}

bool SoftTintTest::init(std::ostream& output)
{
	return true;
}

//____ lookupTableTest() ______________________________________________________
//
// TintTools::buildLUT() interpolates in integers. Every entry should be the exact color in
// the middle of the span it covers, rounded to nearest, for all LUT sizes SoftTint uses.

bool SoftTintTest::lookupTableTest(std::ostream& output)
{
	HiColor red = Color::Red, white = Color::White, blue = Color::Blue, green = Color::Green, yellow = Color::Yellow;

	std::vector<std::vector<ColorStop>> stopSets = {
		{ {0.f, red}, {0.3f, yellow}, {0.6f, green}, {1.f, blue} },
		{ {0.f, white}, {0.5f, yellow}, {1.f, HiColor(0, 0, 4096, 2048)} },
		{ {0.f, red}, {0.5f, red}, {0.5f, white}, {1.f, blue} },
		{ {0.1f, HiColor(1234, 77, 4000, 3000)}, {0.13f, HiColor(17, 3999, 2, 4096)}, {0.77f, green}, {0.9f, HiColor(2048, 2048, 2048, 100)} },
		{ {0.f, red}, {0.f, white}, {0.25f, blue}, {1.f, blue}, {1.f, green} },
		{ {0.4f, red}, {0.40001f, blue} }
	};

	for (auto& stops : stopSets)
	{
		Tint::Blueprint bp;
		bp.stops = stops;
		bp.begin = { 0, 0 };
		bp.end = { 1, 0 };
		Tint_p pTint = Tint::create(bp);
		TEST_ASSERT(pTint != nullptr);

		TintTools::DecodedTint decoded = decode(pTint, RectSPX(0, 0, 64 * 64, 64));
		const TintTools::TintLayer& layer = decoded.layers[0];

		for (int bits = 1; bits <= 12; bits++)
		{
			int entries = (1 << bits) + 1;
			std::vector<HiColor> lut(entries);
			TintTools::buildLUT(layer, entries, lut.data());

			for (int i = 0; i < entries; i++)
			{
				double t = i < entries - 1 ? (i + 0.5) / (entries - 1) : 1.0;
				double c[4];
				stopColorAt(layer, t, c);

				int16_t* pChannels[4] = { &lut[i].r, &lut[i].g, &lut[i].b, &lut[i].a };

				for (int ch = 0; ch < 4; ch++)
				{
					// Exact values half way between integers can go either way in double precision.

					double expected = c[ch] + 0.5;
					bool bTie = std::abs(expected - std::floor(expected + 0.5)) < 1e-6;

					int got = *pChannels[ch];
					if (got != int(std::floor(expected)) && !(bTie && got == int(std::floor(expected + 0.5))))
					{
						output << "Entry " << i << " of " << entries << " has channel " << ch << " = " << got << ", expected " << c[ch] << ".";
						return false;
					}
				}
			}
		}

		// Colors below 0.0 and at 1.0, used by SoftTint for Pad.

		double c[4];
		stopColorAt(layer, 0.0, c);
		HiColor low = TintTools::layerColorAtFixed(layer, 0);
		TEST_ASSERT(low.r == int(std::floor(c[0] + 0.5)) && low.g == int(std::floor(c[1] + 0.5)) && low.b == int(std::floor(c[2] + 0.5)) && low.a == int(std::floor(c[3] + 0.5)));

		stopColorAt(layer, 1.0, c);
		HiColor high = TintTools::layerColorAtFixed(layer, 65535);
		TEST_ASSERT(high.r == int(std::floor(c[0] + 0.5)) && high.g == int(std::floor(c[1] + 0.5)) && high.b == int(std::floor(c[2] + 0.5)) && high.a == int(std::floor(c[3] + 0.5)));
	}

	return true;
}

//____ tintRowsTest() _________________________________________________________
//
// Rows and columns of colors from SoftTint for linear and radial tints with all spreads,
// placed in rects of different sizes, some far from the pixels generated so that positions
// are far outside the gradient. Every pixel should have the color of LUT entry
// floor(position * N), or of the neighbor entry when the position is within the error of
// SoftTint's fixed point from the boundary between them.
//
// Positions are exact at the start of rows of up to 128 pixels and stepped with errors
// of at most 2^-17 entries per pixel, which adds up to 1/1024 entry.

bool SoftTintTest::tintRowsTest(std::ostream& output)
{
	const double c_maxError = 2.0 / 1024;

	HiColor red = Color::Red, blue = Color::Blue, yellow = Color::Yellow, green = Color::Green, white = Color::White;

	std::vector<std::pair<std::string, Tint_p>> tints;

	TintSpread spreads[3] = { TintSpread::Pad, TintSpread::Repeat, TintSpread::Reflect };
	const char* spreadNames[3] = { "Pad", "Repeat", "Reflect" };

	for (int s = 0; s < 3; s++)
	{
		std::string spreadName = std::string(" ") + spreadNames[s];

		Tint::Blueprint bp;
		bp.spread = spreads[s];

		bp.stops = { {0.f, red}, {0.3f, yellow}, {0.6f, green}, {1.f, blue} };
		bp.begin = { 0.1f, 0.f };
		bp.end = { 0.9f, 1.f };
		tints.push_back({ "Linear diagonal" + spreadName, Tint::create(bp) });

		bp.begin = { 0.3f, 0.5f };
		bp.end = { 0.2f, 0.5f };
		tints.push_back({ "Linear reversed" + spreadName, Tint::create(bp) });

		bp.stops = { {0.f, red}, {0.5f, red}, {0.5f, white}, {1.f, blue} };
		bp.begin = { 0.f, 0.f };
		bp.end = { 0.25f, 0.1f };
		tints.push_back({ "Linear hard edges" + spreadName, Tint::create(bp) });

		bp.shape = TintShape::Radial;
		bp.radiusMode = TintRadius::Fit;
		bp.center = { 0.4f, 0.6f };
		bp.radius = { 0.5f, 0.3f };
		bp.stops = { {0.f, white}, {0.5f, yellow}, {1.f, HiColor(0, 0, 4096, 2048)} };
		tints.push_back({ "Radial ellipse" + spreadName, Tint::create(bp) });

		bp.radiusMode = TintRadius::Circle;
		bp.center = { 0.2f, 0.7f };
		bp.radius = { 0.03f, 0.03f };
		bp.stops = { {0.f, red}, {0.2f, red}, {0.2f, white}, {0.7f, green}, {1.f, blue} };
		tints.push_back({ "Radial small hard edges" + spreadName, Tint::create(bp) });

		bp.radiusMode = TintRadius::Fit;
		bp.center = { 1.5f, -0.3f };
		bp.radius = { 3.f, 0.7f };
		bp.stops = { {0.f, red}, {0.5f, white}, {1.f, blue} };
		tints.push_back({ "Radial off center" + spreadName, Tint::create(bp) });
	}

	// The last two rects are far from the pixels, the very last so far that radial tints
	// take the slow path of SoftTint.

	RectSPX rects[] = { RectSPX(0, 0, 128 * 64, 128 * 64), RectSPX(37 * 64 + 13, 21 * 64 + 50, 53 * 64 + 7, 77 * 64 + 31),
						RectSPX(60 * 64, 50 * 64, 3 * 64 + 20, 5 * 64 + 3), RectSPX(-5000 * 64, -3000 * 64, 4000 * 64, 6000 * 64),
						RectSPX(-20000 * 64, 100 * 64, 10 * 64, 10 * 64) };

	HiColor multipliers[2] = { HiColor::White, HiColor(4096, 2048, 3000, 3072) };

	const int size = 128;
	std::vector<HiColor> line(size);

	for (auto& tint : tints)
	{
		int nPixels = 0, nNeighbors = 0;

		for (auto& rect : rects)
		{
			TintTools::DecodedTint decoded = decode(tint.second, rect);
			const TintTools::TintLayer& layer = decoded.layers[0];

			int bits = TintTools::lutBitsForLayer(layer);
			TEST_ASSERT(bits >= 1);

			for (auto& multiplier : multipliers)
			{
				SoftTint softTint;
				softTint.set(tint.second, rect, multiplier);

				auto check = [&](int x, int y, HiColor color) -> bool
				{
					double position = positionAt(layer, x, y) * (1 << bits);
					int64_t idx = int64_t(std::floor(position));
					double fraction = position - std::floor(position);

					nPixels++;
					if (nearlyEqual(color, lutEntryColor(layer, bits, idx, multiplier), 2))
						return true;

					bool bLower = nearlyEqual(color, lutEntryColor(layer, bits, idx - 1, multiplier), 2);
					bool bHigher = nearlyEqual(color, lutEntryColor(layer, bits, idx + 1, multiplier), 2);

					// Within c_maxError of a boundary either entry is right.

					if ((bLower && fraction < c_maxError) || (bHigher && fraction > 1.0 - c_maxError))
						return true;

					if (bLower || bHigher)
					{
						nNeighbors++;
						return true;
					}

					output << tint.first << ": Pixel " << x << "," << y << " has color " << color.r << "," << color.g << "," << color.b << "," << color.a
						   << ", which isn't LUT entry " << idx << " or next to it.";
					return false;
				};

				for (int y = 0; y < size; y++)
				{
					softTint.generate(0, y, 1, 0, size, line.data());
					for (int x = 0; x < size; x++)
						if (!check(x, y, line[x]))
							return false;
				}

				for (int i = 0; i < size; i += 4)
				{
					softTint.generate(size - 1, i, -1, 0, size, line.data());
					for (int x = 0; x < size; x++)
						if (!check(size - 1 - x, i, line[x]))
							return false;

					softTint.generate(i, 0, 0, 1, size, line.data());
					for (int y = 0; y < size; y++)
						if (!check(i, y, line[y]))
							return false;

					softTint.generate(i, size - 1, 0, -1, size, line.data());
					for (int y = 0; y < size; y++)
						if (!check(i, size - 1 - y, line[y]))
							return false;

					softTint.generate(i, (i * 7) % size, 0, 0, 1, line.data());
					if (!check(i, (i * 7) % size, line[0]))
						return false;
				}
			}
		}

		if (nNeighbors * 10000 > nPixels)
		{
			output << tint.first << ": " << nNeighbors << " of " << nPixels << " pixels have the color of a neighbor LUT entry without being close to it, expected at most 0.01%.";
			return false;
		}
	}

	return true;
}

//____ tintedRenderingTest() __________________________________________________
//
// Fills and blits with tints on SoftBackend, compared against the tint evaluation of the GPU
// backends like gfxdevicetester compares backends: no more than 64 channel values may differ
// by more than 16 in 8-bit sRGB. Smooth gradients only, since hard edges legitimately end up
// on different sides of pixels.

bool SoftTintTest::tintedRenderingTest(std::ostream& output)
{
	HiColor red = Color::Red, blue = Color::Blue, yellow = Color::Yellow, green = Color::Green, white = Color::White;

	const SizeI	canvasSize = { 256, 256 };

	Tint::Blueprint multiStop;
	multiStop.stops = { {0.f, red}, {0.3f, yellow}, {0.6f, green}, {1.f, blue} };
	multiStop.begin = { 0.1f, 0.f };
	multiStop.end = { 0.9f, 1.f };

	Tint::Blueprint reflect;
	reflect.stops = { {0.f, red}, {1.f, HiColor(0, 4096, 4096, 2048)} };
	reflect.begin = { 0.2f, 0.2f };
	reflect.end = { 0.35f, 0.25f };
	reflect.spread = TintSpread::Reflect;

	Tint::Blueprint ellipse;
	ellipse.shape = TintShape::Radial;
	ellipse.center = { 0.4f, 0.6f };
	ellipse.radius = { 0.5f, 0.3f };
	ellipse.stops = { {0.f, white}, {0.5f, yellow}, {1.f, HiColor(0, 0, 4096, 2048)} };

	Tint::Blueprint circleRepeat;
	circleRepeat.shape = TintShape::Radial;
	circleRepeat.radiusMode = TintRadius::Circle;
	circleRepeat.spread = TintSpread::Repeat;
	circleRepeat.center = { 0.5f, 0.5f };
	circleRepeat.radius = { 0.15f, 0.15f };
	circleRepeat.stops = { {0.f, green}, {0.5f, blue}, {1.f, green} };

	struct Case
	{
		const char*	name;
		Tint_p		pTint;
		HiColor		tintColor;
	};

	Case cases[] = {
		{ "Multi stop linear", Tint::create(multiStop), HiColor::White },
		{ "Reflected linear", Tint::create(reflect), HiColor::White },
		{ "Radial ellipse", Tint::create(ellipse), HiColor::White },
		{ "Repeated circle", Tint::create(circleRepeat), HiColor::White },
		{ "Mix", Tint::mix(Tint::create(multiStop), Tint::create(ellipse), 0.4f), HiColor::White },
		{ "Radial ellipse with tint color", Tint::create(ellipse), HiColor(4096, 2048, 4096, 3072) }
	};

	RectSPX tintRects[] = { RectSPX(0, 0, canvasSize.w * 64, canvasSize.h * 64), RectSPX(20 * 64 + 21, 12 * 64 + 40, 150 * 64 + 13, 170 * 64 + 7) };

	auto pBackend = SoftBackend::create();
	addDefaultSoftKernels(pBackend);
	auto pDevice = GfxDeviceGen2::create(pBackend);

	auto pCanvas = SoftSurface::create(WGBP(Surface, _.size = canvasSize, _.format = PixelFormat::ARGB_8, _.colorSpace = ColorSpace::sRGB, _.canvas = true));
	auto pSource = SoftSurface::create(WGBP(Surface, _.size = canvasSize, _.format = PixelFormat::ARGB_8, _.colorSpace = ColorSpace::sRGB));
	pSource->fill(HiColor::White);

	RectSPX canvasRect(0, 0, canvasSize.w * 64, canvasSize.h * 64);

	auto toSRGB8 = [](double c)
	{
		c = std::min(std::max(c, 0.0), 1.0);
		double s = c <= 0.0031308 ? c * 12.92 : 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
		return int(std::floor(s * 255.0 + 0.5));
	};

	for (auto& c : cases)
	{
		for (auto& tintRect : tintRects)
		{
			for (int op = 0; op < 2; op++)
			{
				pDevice->beginRender();
				pDevice->beginCanvasUpdate(pCanvas);

				pDevice->setBlendMode(BlendMode::Replace);
				pDevice->fill(canvasRect, HiColor::Black);
				pDevice->setBlendMode(BlendMode::Blend);

				pDevice->setTint(tintRect, c.pTint);
				pDevice->setTintColor(c.tintColor);

				if (op == 0)
					pDevice->fill(canvasRect, HiColor::White);
				else
				{
					pDevice->setBlitSource(pSource);
					pDevice->blit({ 0, 0 });
				}

				pDevice->clearTint();
				pDevice->setTintColor(HiColor::White);

				pDevice->endCanvasUpdate();
				pDevice->endRender();

				// Read back as 8-bit sRGB, which is what gfxdevicetester compares.

				std::vector<uint8_t> pixels(canvasSize.w * canvasSize.h * 4);

				auto buffer = pCanvas->allocPixelBuffer();
				pCanvas->pushPixels(buffer);

				int srcPitchAdd = buffer.pitch - canvasSize.w * Util::pixelFormatToDescription(buffer.format).bits / 8;
				int dstPaletteEntries = 0;

				bool bOk = PixelTools::copyPixels(canvasSize.w, canvasSize.h, buffer.pixels, buffer.format, buffer.colorSpace, buffer.bigEndian, srcPitchAdd,
												  buffer.palette, pCanvas->paletteSize(), pixels.data(), PixelFormat::ARGB_8, ColorSpace::sRGB,
												  (WAX_IS_BIG_ENDIAN == 1), 0, nullptr, dstPaletteEntries, 0);
				pCanvas->freePixelBuffer(buffer);
				TEST_ASSERT(bOk);

				// White blended over black with the tint is the tint color times its alpha.

				TintTools::DecodedTint decoded = decode(c.pTint, tintRect);

				int nOver = 0;
				int maxDiff = 0;

				for (int y = 0; y < canvasSize.h; y++)
				{
					for (int x = 0; x < canvasSize.w; x++)
					{
						double tint[4];
						gpuTintColor(decoded, x, y, c.tintColor, tint);

						uint32_t pixel;
						memcpy(&pixel, &pixels[(y * canvasSize.w + x) * 4], 4);

						int got[3] = { int(pixel >> 16) & 0xFF, int(pixel >> 8) & 0xFF, int(pixel) & 0xFF };

						for (int ch = 0; ch < 3; ch++)
						{
							int diff = std::abs(got[ch] - toSRGB8(tint[ch] * tint[3]));
							maxDiff = std::max(maxDiff, diff);
							if (diff > 16)
								nOver++;
						}
					}
				}

				if (nOver > 64)
				{
					output << c.name << (op == 0 ? " fill" : " blit") << ": " << nOver << " channel values differ by more than 16 from the GPU evaluation, max difference " << maxDiff << ".";
					return false;
				}
			}
		}
	}

	return true;
}
