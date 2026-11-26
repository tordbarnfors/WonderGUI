#include "testsuite.h"

#include <wax_tint.h>

#include <cmath>
#include <initializer_list>
#include <vector>

// Polylines, see Polylines in wax_device.h. They are semi-transparent where it matters, so that
// pixels blended twice at a joint would show as darker spots and background showing through
// between two segments as lighter ones.
//
// Polylines that cross themselves or turn back within less than their thickness may be blended
// once or twice where they do, depending on the backend, so the tests with those are in opaque
// colors and only there to show that nothing else goes wrong.

class PolylineTests : public TestSuite
{
public:
	PolylineTests()
	{
		name = "PolylineTests";

		addTest("Angles", &PolylineTests::angles);
		addTest("Thin angles", &PolylineTests::thinAngles);
		addTest("Thick angles", &PolylineTests::thickAngles);
		addTest("Caps", &PolylineTests::caps);
		addTest("Wires", &PolylineTests::wires);
		addTest("Zigzags", &PolylineTests::zigzags);
		addTest("Curves", &PolylineTests::curves);
		addTest("Short segments", &PolylineTests::shortSegments);
		addTest("Clipped", &PolylineTests::clipped);
		addTest("Tinted", &PolylineTests::setTint, &PolylineTests::angles, &PolylineTests::clearTint);
		addTest("BlendModes", &PolylineTests::blendModes);
		addTest("Crossing itself, opaque", &PolylineTests::crossing);
		addTest("Tight turns, opaque", &PolylineTests::tightTurns);
	}

	bool init(GfxDevice * pDevice, const RectSPX& canvas, wapp::API * pAppAPI)
	{
		m_pTint = Tint::create( { {0.f, Color(4096, 1000, 1000, 4096)}, {0.5f, Color(4096, 4096, 1000, 4096)}, {1.f, Color(1000, 1000, 4096, 2500)} }, {0.f, 0.f}, {1.f, 1.f} );
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
		pDevice->setTintColor(Color(4096, 4096, 4096, 3500));
		return true;
	}

	bool clearTint(GfxDevice * pDevice, const RectSPX& canvas)
	{
		pDevice->clearTint();
		pDevice->setTintColor(Color::White);
		return true;
	}

	// Two segments at every angle to each other, in eight directions.

	bool angles(GfxDevice * pDevice, const RectSPX& canvas)
	{
		angleGrid(pDevice, 7.0, 24.0);
		return true;
	}

	bool thinAngles(GfxDevice * pDevice, const RectSPX& canvas)
	{
		angleGrid(pDevice, 1.3, 26.0);
		return true;
	}

	bool thickAngles(GfxDevice * pDevice, const RectSPX& canvas)
	{
		angleGrid(pDevice, 13.0, 22.0);
		return true;
	}

	// Each cap at the ends of polylines of three segments, turning both ways.

	bool caps(GfxDevice * pDevice, const RectSPX& canvas)
	{
		const LineCap capList[8] = { LineCap::None, LineCap::Flat, LineCap::Square, LineCap::Round, LineCap::Vertical,
									 LineCap::Horizontal, LineCap::Spear(50), LineCap::Spear(300) };

		for (int c = 0; c < 8; c++)
		{
			for (int r = 0; r < 8; r++)
			{
				double cx = c * 64 + 32.3;
				double cy = r * 64 + 32.6;
				double angle = (r * 45 + 11) * c_pi / 180;
				double ux = std::cos(angle), uy = std::sin(angle);

				RectSPX cell = { c * 64 * 64, r * 64 * 64, 64 * 64, 64 * 64 };
				pDevice->pushClipList(1, &cell);

				polyline(pDevice, { cx - ux * 22 + uy * 8, cy - uy * 22 - ux * 8,  cx - ux * 6, cy - uy * 6,
									cx + ux * 6 - uy * 9, cy + uy * 6 + ux * 9,  cx + ux * 22 - uy * 4, cy + uy * 22 + ux * 4 },
						 6.0, LineCaps(capList[c], capList[(c + r) % 8]), Color(4096, 3000, 1200, 3000));

				pDevice->popClipList();
			}
		}
		return true;
	}

	// Wires between nodes: along x and y with a stub at each end, at positions between pixels,
	// and some straight across.

	bool wires(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < 12; i++)
		{
			double thickness = 1.0 + (i % 6) * 0.9;
			double x1 = 20 + i * 3.37, y1 = 20 + i * 40.21;
			double x2 = 480 - i * 5.13, y2 = 480 - i * 36.4;
			double mid = 120 + i * 24.6;

			Color color = (i & 1) ? Color(4096, 3200, 800, 2400) : Color(1000, 3000, 4096, 2800);

			polyline(pDevice, { x1, y1,  x1 + 12, y1,  mid, y1,  mid, y2,  x2 - 12, y2,  x2, y2 }, thickness, LineCap::Flat, color);
		}

		for (int i = 0; i < 6; i++)
		{
			double y = 30 + i * 80.3;
			polyline(pDevice, { 40.2, y,  60.2, y,  440.7, 500 - y,  460.7, 500 - y }, 2.0 + i * 0.5, LineCap::Round, Color(3000, 4096, 2000, 2000));
		}
		return true;
	}

	// Many segments, long compared to the thickness.

	bool zigzags(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int row = 0; row < 6; row++)
		{
			std::vector<double> points;

			double y = 40 + row * 80;
			double step = 14 + row * 9;
			double height = 10 + row * 9;

			for (int i = 0; 20 + i * step < 492; i++)
			{
				points.push_back(20 + i * step + row * 0.21);
				points.push_back(y + ((i & 1) ? height : -height) * (1.0 - i * 0.02));
			}

			polyline(pDevice, points, 1.5 + row * 1.1, LineCaps(LineCap::Round, LineCap::Spear(200)), Color(4096, 2500, 1000, 2600));
		}
		return true;
	}

	// Curves as many short segments, bending less than their thickness allows.

	bool curves(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < 5; i++)
		{
			std::vector<double> points;

			double thickness = 1.5 + i * 2.6;
			double radius = 30 + i * 42;

			for (int k = 0; k <= 48; k++)
			{
				double angle = (20 + k * 5.5 + i * 9) * c_pi / 180;
				points.push_back(256.3 + std::cos(angle) * radius);
				points.push_back(256.6 + std::sin(angle) * radius * 0.9);
			}

			polyline(pDevice, points, thickness, LineCaps(LineCap::Flat, LineCap::Round), Color(1200, 3600, 4096, 2800));
		}

		for (int i = 0; i < 3; i++)
		{
			std::vector<double> points;

			for (int k = 0; k <= 120; k++)
			{
				points.push_back(16.2 + k * 4.0);
				points.push_back(60 + i * 190 + std::sin(k * 0.21 + i) * (18 + i * 8) * std::cos(k * 0.013));
			}

			polyline(pDevice, points, 2.0 + i * 3.0, LineCap::Round, Color(4096, 1500, 3000, 2400));
		}
		return true;
	}

	// Segments shorter than the polyline is thick, and shorter than a pixel, where it hardly turns.

	bool shortSegments(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int row = 0; row < 8; row++)
		{
			std::vector<double> points;

			double length = 0.4 + row * 0.9;
			double x = 20.3, y = 34.6 + row * 60;
			double angle = 0;

			while (x < 490)
			{
				points.push_back(x);
				points.push_back(y);

				angle = std::sin(x * 0.05 + row) * 0.5;
				x += std::cos(angle) * length;
				y += std::sin(angle) * length;
			}

			polyline(pDevice, points, 3.0 + row * 1.7, LineCaps(LineCap::Square, LineCap::Flat), Color(3200, 4096, 1500, 2500));
		}
		return true;
	}

	// One polyline through a clip list of separate rectangles.

	bool clipped(GfxDevice * pDevice, const RectSPX& canvas)
	{
		std::vector<RectSPX> clip;

		for (int y = 0; y < 5; y++)
			for (int x = 0; x < 5; x++)
				clip.push_back(RectSPX(spx(10 + x * 100) * 64, spx(14 + y * 100) * 64, spx(70 + x * 5) * 64, spx(60 + y * 6) * 64));

		pDevice->pushClipList(int(clip.size()), clip.data());

		polyline(pDevice, { 12.3, 20.2,  480.4, 60.1,  30.6, 150.7,  470.2, 240.3,  40.8, 330.9,  490.1, 420.2,  20.5, 500.4 },
				 22.0, LineCap::Round, Color(4096, 2200, 1200, 2800));

		polyline(pDevice, { 250.3, 4.4,  250.3, 180.2,  330.7, 180.2,  330.7, 400.6,  120.2, 400.6,  120.2, 506.1 },
				 9.0, LineCap::Flat, Color(1200, 2200, 4096, 2800));

		pDevice->popClipList();
		return true;
	}

	bool blendModes(GfxDevice * pDevice, const RectSPX& canvas)
	{
		const BlendMode modes[9] = { BlendMode::Replace, BlendMode::Blend, BlendMode::Add, BlendMode::Subtract, BlendMode::Multiply,
									 BlendMode::Invert, BlendMode::Min, BlendMode::Max, BlendMode::Morph };

		// Stripes to blend onto.

		for (int i = 0; i < 8; i++)
			pDevice->fill(RectSPX(0, i * 64 * 64, canvas.w, 32 * 64), Color(i * 500, 2000, 4096 - i * 500, 4096));

		pDevice->setMorphFactor(0.4f);

		for (int m = 0; m < 9; m++)
		{
			pDevice->setBlendMode(modes[m]);

			double y = 30 + m * 54;
			polyline(pDevice, { 30.3, y,  90.3, y + 20,  150.3, y - 4,  230.3, y + 14 }, 12.0, LineCap::Round, Color(4096, 2000, 1000, 3000));
			polyline(pDevice, { 280.6, y + 14,  330.6, y + 14,  330.6, y - 6,  420.6, y - 6,  480.6, y + 18 }, 5.0,
					 LineCaps(LineCap::Square, LineCap::Spear(300)), Color(1000, 4096, 2000, 3000));
		}

		pDevice->setBlendMode(BlendMode::Blend);
		pDevice->setMorphFactor(0.5f);
		return true;
	}

	// Stars and loops, crossing themselves.

	bool crossing(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int i = 0; i < 4; i++)
		{
			std::vector<double> points;

			double cx = 130.4 + (i & 1) * 250, cy = 130.7 + (i >> 1) * 250;
			int nTips = 5 + i * 2;

			for (int k = 0; k <= nTips; k++)
			{
				double angle = k * 2 * c_pi * ((nTips - 1) / 2) / nTips;
				points.push_back(cx + std::sin(angle) * 110);
				points.push_back(cy - std::cos(angle) * 110);
			}

			polyline(pDevice, points, 3.0 + i * 3.0, LineCap::Round, Color(4096, 3000 - i * 600, 800 + i * 900, 4096));
		}
		return true;
	}

	// Turning back within less than the thickness: zigzags closing in, and a spiral.

	bool tightTurns(GfxDevice * pDevice, const RectSPX& canvas)
	{
		for (int row = 0; row < 4; row++)
		{
			std::vector<double> points;

			for (int i = 0; i < 40; i++)
			{
				points.push_back(20.3 + i * (12 - row * 2.5) * (1.0 - i * 0.012));
				points.push_back(40.6 + row * 70 + ((i & 1) ? 24 : -24));
			}

			polyline(pDevice, points, 4.0 + row * 4.0, LineCap::Flat, Color(1000 + row * 900, 3500, 4096, 4096));
		}

		std::vector<double> points;

		for (int k = 0; k < 200; k++)
		{
			double angle = k * 0.35;
			double radius = 2 + k * 0.45;
			points.push_back(256.2 + std::cos(angle) * radius);
			points.push_back(400.4 + std::sin(angle) * radius);
		}

		polyline(pDevice, points, 14.0, LineCap::Round, Color(4096, 2000, 2000, 4096));
		return true;
	}

private:

	static constexpr double c_pi = 3.14159265358979323846;

	void polyline(GfxDevice * pDevice, const std::vector<double>& coords, double thickness, LineCaps caps, Color color)
	{
		std::vector<CoordSPX> points;

		for (size_t i = 0; i + 1 < coords.size(); i += 2)
			points.push_back({ spx(std::lround(coords[i] * 64)), spx(std::lround(coords[i + 1] * 64)) });

		pDevice->drawPolyline(int(points.size()), points.data(), color, spx(std::lround(thickness * 64)), caps);
	}

	void polyline(GfxDevice * pDevice, std::initializer_list<double> coords, double thickness, LineCaps caps, Color color)
	{
		polyline(pDevice, std::vector<double>(coords), thickness, caps, color);
	}

	// A first segment in each of eight directions (rows), a second turning from it by eight
	// angles (columns), half of them to each side.

	void angleGrid(GfxDevice * pDevice, double thickness, double length)
	{
		const double turns[8] = { 8, -25, 45, -72, 90, -118, 145, -168 };

		for (int c = 0; c < 8; c++)
		{
			for (int r = 0; r < 8; r++)
			{
				double cx = c * 64 + 32.3;
				double cy = r * 64 + 32.6;

				double first = (r * 45 + 7) * c_pi / 180;
				double second = first + turns[c] * c_pi / 180;

				// The joint is placed so that the two ends and the joint are centered in the cell.

				double ax = -std::cos(first) * length, ay = -std::sin(first) * length;
				double bx = std::cos(second) * length, by = std::sin(second) * length;
				double jx = cx - (ax + bx) / 3, jy = cy - (ay + by) / 3;

				RectSPX cell = { c * 64 * 64, r * 64 * 64, 64 * 64, 64 * 64 };
				pDevice->pushClipList(1, &cell);
				polyline(pDevice, { jx + ax, jy + ay,  jx, jy,  jx + bx, jy + by }, thickness, LineCap::Flat, Color(4096, 3000, 1200, 3000));
				pDevice->popClipList();
			}
		}
	}

	Tint_p	m_pTint;
};
