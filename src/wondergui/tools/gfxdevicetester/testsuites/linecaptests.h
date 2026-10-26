#include "testsuite.h"

#include <wax_tint.h>

#include <cmath>

// Lines with caps, see LineCap. Lines are semi-transparent where it matters, so that pixels drawn
// twice, where parts of a line or lines meeting at a cap overlap, would show as darker spots.
//
// Vertical and horizontal cuts reach far beyond the end of a line that nearly follows them, so in
// the grids of caps each line is clipped to its cell.

class LineCapTests : public TestSuite
{
public:
	LineCapTests()
	{
		name = "LineCapTests";

		addTest("All caps", &LineCapTests::allCaps);
		addTest("Thin caps", &LineCapTests::thinCaps);
		addTest("Mixed caps", &LineCapTests::mixedCaps);
		addTest("Round solaris", &LineCapTests::roundSolaris);
		addTest("Spears", &LineCapTests::spears);
		addTest("Dots", &LineCapTests::dots);
		addTest("Chart joins", &LineCapTests::chartJoins);
		addTest("Straight lines", &LineCapTests::straightLines);
		addTest("Tinted", &LineCapTests::setTint, &LineCapTests::allCaps, &LineCapTests::clearTint);
		addTest("BlendModes", &LineCapTests::blendModes);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		m_pTint = Tint::create( { {0.f, HiColor(4096, 1000, 1000, 4096)}, {0.5f, HiColor(4096, 4096, 1000, 4096)}, {1.f, HiColor(1000, 1000, 4096, 2500)} }, {0.f, 0.f}, {1.f, 1.f} );
		return true;
	}

	bool exit(GfxDevice * pDevice, const RectSPX& canvas)
	{
		m_pTint = nullptr;
		return true;
	}

	bool setTint(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->setTint(canvas, m_pTint);
		pDevice->setTintColor(HiColor(4096, 4096, 4096, 3500));
		return true;
	}

	bool clearTint(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->clearTint();
		pDevice->setTintColor(Color::White);
		return true;
	}

	// Every cap in eight directions, one cap per column.

	bool allCaps(GfxDevice * pDevice, const RectSPX& canvas)
	{
		capGrid(pDevice, canvas, 9.0, 34.0, false);
		return true;
	}

	bool thinCaps(GfxDevice * pDevice, const RectSPX& canvas)
	{
		capGrid(pDevice, canvas, 2.5, 40.0, false);
		return true;
	}

	// Each cap at the begin of a line with each at its end.

	bool mixedCaps(GfxDevice * pDevice, const RectSPX& canvas)
	{
		capGrid(pDevice, canvas, 7.0, 36.0, true);
		return true;
	}

	// Lines in all directions with round caps, from a ring around the center.

	bool roundSolaris(GfxDevice * pDevice, const RectSPX& canvas)
	{
		const int nLines = 36;
		CoordF center = { canvas.w / 128.f, canvas.h / 128.f };

		for (int i = 0; i < nLines; i++)
		{
			double angle = i * 2 * c_pi / nLines;
			double c = std::cos(angle), s = std::sin(angle);
			double thickness = 1.0 + (i % 6) * 2.3;

			lineFrom(pDevice, center.x + c * 60, center.y + s * 60, center.x + c * 230, center.y + s * 230, thickness, LineCap::Round, HiColor(4096, 3500, 1500, 3200));
		}
		return true;
	}

	// Spears from 25 to 800 percent of the thickness, in two directions.

	bool spears(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < 16; i++)
		{
			uint16_t length = uint16_t(25 + i * 775 / 15);
			double y = 20 + i * 30.3;

			lineFrom(pDevice, 20.4, y, 240.4, y + 4, 8.0, { LineCap::Flat, LineCap::Spear(length) }, HiColor(1500, 4096, 2500, 3500));
			lineFrom(pDevice, 280.7, y + 10, 490.7, y - 6, 4.0, LineCap::Spear(length), HiColor(4096, 2000, 3500, 3500));
		}
		return true;
	}

	// Lines of no length: round dots and squares of growing size, at fractions of pixels.

	bool dots(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < 12; i++)
		{
			for (int j = 0; j < 12; j++)
			{
				double size = 0.5 + (i * 12 + j) * 0.25;
				double x = 22 + j * 40 + j * 0.13;
				double y = 22 + i * 40 + i * 0.29;

				LineCap cap = ((i + j) % 2) == 0 ? LineCap::Round : LineCap::Square;
				lineFrom(pDevice, x, y, x, y, size, cap, HiColor(4096, 4096, 4096, 4096));
			}
		}
		return true;
	}

	// Polylines of a chart, segments meeting at vertical caps on whole pixels, where they neither
	// overlap nor leave gaps, with flat ends. Semi-transparent, so overlaps would show.

	bool chartJoins(GfxDevice * pDevice, const RectSPX& canvas)
	{
		const int nSegments = 40;

		for (int chart = 0; chart < 4; chart++)
		{
			double base = 70 + chart * 120;
			double thickness = 1.5 + chart * 3.0;
			HiColor color = chart % 2 ? HiColor(4096, 2500, 500, 2800) : HiColor(800, 3000, 4096, 2800);

			auto valueAt = [&](int i) { return base - 45 * std::sin(i * 0.37 + chart) * std::cos(i * 0.11); };

			for (int i = 0; i < nSegments; i++)
			{
				LineCaps caps = { i == 0 ? LineCap::Flat : LineCap::Vertical, i == nSegments - 1 ? LineCap::Flat : LineCap::Vertical };
				lineFrom(pDevice, 10 + i * 12, valueAt(i), 10 + (i + 1) * 12, valueAt(i + 1), thickness, caps, color);
			}
		}
		return true;
	}

	// Lines along x and y, with GfxDevice::drawLine(begin, direction, length...).

	bool straightLines(GfxDevice * pDevice, const RectSPX& canvas)
	{
		const LineCap caps[4] = { LineCap::None, LineCap::Flat, LineCap::Round, LineCap::Spear(150) };
		const Direction dirs[4] = { Direction::Right, Direction::Down, Direction::Left, Direction::Up };

		for (int c = 0; c < 4; c++)
		{
			for (int d = 0; d < 4; d++)
			{
				CoordSPX begin = { spx((60 + c * 120 + (d == 2 ? 80 : 0)) * 64 + 13), spx((40 + d * 120 + (d == 3 ? 80 : 0)) * 64 + 29) };
				pDevice->drawLine(begin, dirs[d], 80 * 64, HiColor(4096, 4096, 2000, 3500), 64 * 6 + 20, caps[c]);
			}
		}
		return true;
	}

	bool blendModes(GfxDevice * pDevice, const RectSPX& canvas)
	{
		const BlendMode modes[9] = { BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::Subtract, BlendMode::Multiply,
									 BlendMode::Invert, BlendMode::Min, BlendMode::Max, BlendMode::Morph };

		// Stripes to blend onto.

		for (int i = 0; i < 8; i++)
			pDevice->fill(RectSPX(0, i * 64 * 64, canvas.w, 32 * 64), HiColor(i * 500, 2000, 4096 - i * 500, 4096));

		pDevice->setMorphFactor(0.4f);

		for (int m = 0; m < 9; m++)
		{
			pDevice->setBlendMode(modes[m]);

			double y = 30 + m * 54;
			lineFrom(pDevice, 30.3, y, 230.3, y + 14, 12.0, LineCap::Round, HiColor(4096, 2000, 1000, 3000));
			lineFrom(pDevice, 280.6, y + 14, 480.6, y, 9.0, { LineCap::Square, LineCap::Spear(300) }, HiColor(1000, 4096, 2000, 3000));
		}

		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->setMorphFactor(0.5f);
		return true;
	}

private:

	static constexpr double c_pi = 3.14159265358979323846;

	void lineFrom(GfxDevice * pDevice, double x1, double y1, double x2, double y2, double thickness, LineCaps caps, HiColor color)
	{
		pDevice->drawLine({ spx(std::lround(x1 * 64)), spx(std::lround(y1 * 64)) }, { spx(std::lround(x2 * 64)), spx(std::lround(y2 * 64)) },
						  color, spx(std::lround(thickness * 64)), caps);
	}

	void capGrid(GfxDevice * pDevice, const RectSPX& canvas, double thickness, double length, bool bMixed)
	{
		const LineCap caps[8] = { LineCap::None, LineCap::Flat, LineCap::Square, LineCap::Round, LineCap::Vertical,
								  LineCap::Horizontal, LineCap::Spear(50), LineCap::Spear(300) };
		const double angles[8] = { 0, 17, 45, 70, 90, 128, 200, 333 };

		for (int c = 0; c < 8; c++)
		{
			for (int r = 0; r < 8; r++)
			{
				double cx = c * 64 + 32.3;
				double cy = r * 64 + 32.6;
				double angle = (bMixed ? (c * 8 + r) * 23.0 : angles[r]) * c_pi / 180;
				double dx = std::cos(angle) * length / 2;
				double dy = std::sin(angle) * length / 2;

				LineCaps lineCaps = bMixed ? LineCaps(caps[c], caps[r]) : LineCaps(caps[c]);

				RectSPX cell = { c * 64 * 64, r * 64 * 64, 64 * 64, 64 * 64 };
				pDevice->pushClipList(1, &cell);
				lineFrom(pDevice, cx - dx, cy - dy, cx + dx, cy + dy, thickness, lineCaps, HiColor(4096, 3000, 1200, 3000));
				pDevice->popClipList();
			}
		}
	}

	Tint_p	m_pTint;
};
