// Warpmap prototype. Standalone, does not use the WonderGUI libraries.
//
// Renders a set of test cases to .ppm files in the current directory and prints position errors,
// image differences against an exact per pixel reference and timings.
//
// A Warpmap is a grid of nodes covering a destination rectangle of w x h pixels, one node every
// 'cell' pixels. Each node holds a normalized source position (u,v), 0..1 covering the source.
// Positions are interpolated bilinearly between nodes. uPeriod/vPeriod (0 = none) make an axis
// periodic: interpolation across the seam takes the short way and reads wrap around.
//
// Sources:
//   Edgemap  - same sample layout as wg::Edgemap (column major, edges per column; pixels here, spx there), optionally
//              tiling (wraps horizontally). Antialiased by signed distance to each edge in destination
//              pixels, one sample per pixel.
//   Surface  - RGBA image, bilinear sampling, supersampled where the warp minifies.
//
// The renderer works the way a software backend would: per cell, per row of the cell, the source
// position and its derivatives are stepped linearly along the row. Cells that no edge passes
// through are filled with the segment's color without per pixel work.
//
// Everything is float for clarity. A library version would use fixed point in the inner loops.

#define _USE_MATH_DEFINES		// For M_PI in MSVC.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

static inline float fastLen(float a, float b) { return std::sqrt(a * a + b * b); }

static double now() { return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(); }

//____ Image ____________________________________________________________________

struct RGBA { float r, g, b, a; };		// Premultiplied.

struct Image
{
	int w, h;
	std::vector<RGBA> px;
	Image(int w, int h, RGBA fill = { 0,0,0,0 }) : w(w), h(h), px(w * h, fill) {}
	RGBA& at(int x, int y) { return px[y * w + x]; }

	void savePPM(const std::string& name) const
	{
		FILE* f = fopen(name.c_str(), "wb");
		fprintf(f, "P6 %d %d 255\n", w, h);
		for (auto& p : px)
		{
			// Composite on dark gray for viewing.
			float bg = 0.18f;
			float r = p.r + bg * (1 - p.a), g = p.g + bg * (1 - p.a), b = p.b + bg * (1 - p.a);
			fputc(int(std::clamp(r, 0.f, 1.f) * 255 + 0.5f), f);
			fputc(int(std::clamp(g, 0.f, 1.f) * 255 + 0.5f), f);
			fputc(int(std::clamp(b, 0.f, 1.f) * 255 + 0.5f), f);
		}
		fclose(f);
	}
};

static inline void blendOver(RGBA& dst, const RGBA& src)
{
	float k = 1 - src.a;
	dst.r = src.r + dst.r * k; dst.g = src.g + dst.g * k; dst.b = src.b + dst.b * k; dst.a = src.a + dst.a * k;
}

//____ Warpmap __________________________________________________________________

struct Warpmap
{
	int w, h, cell;
	int nx, ny;						// Nodes per row/column.
	std::vector<float> u, v;		// Node positions, row major.
	float uPeriod = 0, vPeriod = 0;

	Warpmap(int w, int h, int cell) : w(w), h(h), cell(cell)
	{
		nx = (w + cell - 1) / cell + 1;
		ny = (h + cell - 1) / cell + 1;
		u.resize(nx * ny); v.resize(nx * ny);
	}
	int nodeX(int i) const { return std::min(i * cell, w); }
	int nodeY(int j) const { return std::min(j * cell, h); }
};

// Exact polar mapping, used to generate nodes and as per-pixel reference.
struct Polar
{
	float cx, cy, rIn, rOut;
	float startAngle;		// Radians, 0 = up, clockwise.
	float sweep;			// Radians covered by u = 0..1.

	void eval(float x, float y, float& u, float& v, float& dudx, float& dudy, float& dvdx, float& dvdy) const
	{
		float dx = x - cx, dy = y - cy;
		float r2 = dx * dx + dy * dy + 1e-12f;
		float r = std::sqrt(r2);
		float a = std::atan2(dx, -dy) - startAngle;			// 0 = up, clockwise.
		float period = float(2 * M_PI) / sweep;
		u = a / sweep;
		u = u - period * std::floor(u / period);
		v = (r - rIn) / (rOut - rIn);
		// d(atan2(dx,-dy)) = (-dy*ddx + dx*ddy) / r2  -> d/dx = -dy/r2, d/dy = dx/r2
		dudx = (-dy / r2) / sweep;
		dudy = (dx / r2) / sweep;
		dvdx = (dx / r) / (rOut - rIn);
		dvdy = (dy / r) / (rOut - rIn);
	}
	float uPeriod() const { return float(2 * M_PI) / sweep; }
};

static Warpmap makePolarWarpmap(int w, int h, int cell, const Polar& p)
{
	Warpmap m(w, h, cell);
	m.uPeriod = p.uPeriod();
	for (int j = 0; j < m.ny; j++)
		for (int i = 0; i < m.nx; i++)
		{
			float a, b, c, d;
			p.eval(float(m.nodeX(i)), float(m.nodeY(j)), m.u[j * m.nx + i], m.v[j * m.nx + i], a, b, c, d);
		}
	return m;
}

static Warpmap makeWaveWarpmap(int w, int h, int cell, float ampU, float ampV, float waves)
{
	Warpmap m(w, h, cell);
	for (int j = 0; j < m.ny; j++)
		for (int i = 0; i < m.nx; i++)
		{
			float x = float(m.nodeX(i)) / w, y = float(m.nodeY(j)) / h;
			m.u[j * m.nx + i] = x + ampU * std::sin(y * waves * 2 * float(M_PI));
			m.v[j * m.nx + i] = y + ampV * std::sin(x * waves * 2 * float(M_PI));
		}
	return m;
}

//____ Cell setup _______________________________________________________________

// Corner values of a cell, unwrapped across periodic seams so plain bilinear interpolation works.
struct Cell
{
	int x0, y0, cw, ch;
	float u00, u10, u01, u11;
	float v00, v10, v01, v11;
	float uMin, uMax, vMin, vMax;
	float gradU, gradV;				// Max |grad u|, |grad v| over the cell (at corners, bilinear => extremes there).
};

static inline float unwrapTo(float c, float ref, float period)
{
	if (period <= 0) return c;
	while (c - ref > period * 0.5f) c -= period;
	while (c - ref < -period * 0.5f) c += period;
	return c;
}

static void setupCell(const Warpmap& m, int i, int j, Cell& c, bool bUnwrap)
{
	c.x0 = m.nodeX(i); c.y0 = m.nodeY(j);
	c.cw = m.nodeX(i + 1) - c.x0; c.ch = m.nodeY(j + 1) - c.y0;
	int n = j * m.nx + i;
	c.u00 = m.u[n]; c.u10 = m.u[n + 1]; c.u01 = m.u[n + m.nx]; c.u11 = m.u[n + m.nx + 1];
	c.v00 = m.v[n]; c.v10 = m.v[n + 1]; c.v01 = m.v[n + m.nx]; c.v11 = m.v[n + m.nx + 1];

	if (bUnwrap)
	{
		c.u10 = unwrapTo(c.u10, c.u00, m.uPeriod); c.u01 = unwrapTo(c.u01, c.u00, m.uPeriod); c.u11 = unwrapTo(c.u11, c.u00, m.uPeriod);
		c.v10 = unwrapTo(c.v10, c.v00, m.vPeriod); c.v01 = unwrapTo(c.v01, c.v00, m.vPeriod); c.v11 = unwrapTo(c.v11, c.v00, m.vPeriod);
	}

	c.uMin = std::min({ c.u00, c.u10, c.u01, c.u11 }); c.uMax = std::max({ c.u00, c.u10, c.u01, c.u11 });
	c.vMin = std::min({ c.v00, c.v10, c.v01, c.v11 }); c.vMax = std::max({ c.v00, c.v10, c.v01, c.v11 });

	// Derivatives at corners: du/dx along top/bottom edge, du/dy along left/right edge.
	float uxT = (c.u10 - c.u00) / c.cw, uxB = (c.u11 - c.u01) / c.cw, uyL = (c.u01 - c.u00) / c.ch, uyR = (c.u11 - c.u10) / c.ch;
	float vxT = (c.v10 - c.v00) / c.cw, vxB = (c.v11 - c.v01) / c.cw, vyL = (c.v01 - c.v00) / c.ch, vyR = (c.v11 - c.v10) / c.ch;
	c.gradU = std::max({ fastLen(uxT, uyL), fastLen(uxT, uyR), fastLen(uxB, uyL), fastLen(uxB, uyR) });
	c.gradV = std::max({ fastLen(vxT, vyL), fastLen(vxT, vyR), fastLen(vxB, vyL), fastLen(vxB, vyR) });
}

// Calls pixel(x, y, u, v, dudx, dudy, dvdx, dvdy) for each pixel of the cell, stepping linearly along rows.
template<class F>
static inline void forCellPixels(const Cell& c, F&& pixel)
{
	float invW = 1.f / c.cw, invH = 1.f / c.ch;
	for (int y = 0; y < c.ch; y++)
	{
		float fy = (y + 0.5f) * invH;
		float uL = c.u00 + (c.u01 - c.u00) * fy, uR = c.u10 + (c.u11 - c.u10) * fy;
		float vL = c.v00 + (c.v01 - c.v00) * fy, vR = c.v10 + (c.v11 - c.v10) * fy;
		float dudx = (uR - uL) * invW, dvdx = (vR - vL) * invW;

		float fx = 0.5f * invW;
		float u = uL + dudx * 0.5f, v = vL + dvdx * 0.5f;
		float dudyL = (c.u01 - c.u00) * invH, dudyR = (c.u11 - c.u10) * invH;
		float dvdyL = (c.v01 - c.v00) * invH, dvdyR = (c.v11 - c.v10) * invH;
		float dudy = dudyL + (dudyR - dudyL) * fx, dvdy = dvdyL + (dvdyR - dvdyL) * fx;
		float dudyStep = (dudyR - dudyL) * invW, dvdyStep = (dvdyR - dvdyL) * invW;

		for (int x = 0; x < c.cw; x++)
		{
			pixel(c.x0 + x, c.y0 + y, u, v, dudx, dudy, dvdx, dvdy);
			u += dudx; v += dvdx; dudy += dudyStep; dvdy += dvdyStep;
		}
	}
}

//____ Edgemap source ___________________________________________________________

struct EdgemapSrc
{
	int w, h;					// Pixels.
	int nEdges;
	bool tiling;				// samples per edge = w if tiling, else w + 1.
	std::vector<float> s;		// Column major: s[col * nEdges + edge], in pixels (spx / 64). Always w + 1 columns, last mirrors first if tiling.
	std::vector<RGBA> colors;	// nEdges + 1, premultiplied.

	// Edge value and slope (pixels of y per pixel of x) at source x.
	inline void edgeAt(int col, float t, int e, float& y, float& slope) const
	{
		float a = s[col * nEdges + e], b = s[(col + 1) * nEdges + e];
		slope = b - a;
		y = a + slope * t;
	}
};

static EdgemapSrc makeGaugeEdgemap(int w, int h, bool tiling)
{
	EdgemapSrc e;
	e.w = w; e.h = h; e.nEdges = 4; e.tiling = tiling;
	e.s.resize((w + 1) * e.nEdges);
	for (int c = 0; c <= w; c++)
	{
		int cs = (tiling && c == w) ? 0 : c;
		float a = float(cs) / w * 2 * float(M_PI);
		float* p = &e.s[c * e.nEdges];
		p[0] = h * 0.10f;											// Thin inner rim.
		p[1] = h * 0.18f;
		p[2] = h * (0.55f + 0.18f * std::sin(a * 12));				// Wavy band, 12 waves around.
		p[3] = h * (0.82f + 0.06f * std::sin(a * 60));				// Fine ripples, 60 around.
	}
	e.colors = { {0,0,0,0}, {0.95f,0.95f,0.95f,1}, {0.10f * 0.9f,0.55f * 0.9f,0.95f * 0.9f,0.9f}, {0.98f,0.55f,0.10f,1}, {0,0,0,0} };
	return e;
}

static EdgemapSrc makeProgressEdgemap(int w, int h)
{
	// Non-tiling: one edge rising linearly from 0 to full, like a progress/level indicator.
	EdgemapSrc e;
	e.w = w; e.h = h; e.nEdges = 2; e.tiling = false;
	e.s.resize((w + 1) * e.nEdges);
	for (int c = 0; c <= w; c++)
	{
		float* p = &e.s[c * e.nEdges];
		p[0] = h * (1.f - 0.85f * c / w);
		p[1] = h * 1.0f;
	}
	e.colors = { {0.15f * 0.5f,0.15f * 0.5f,0.15f * 0.5f,0.5f}, {0.20f,0.85f,0.30f,1}, {0,0,0,0} };
	return e;
}

static EdgemapSrc makeFilledDiscEdgemap(int w, int h, float innerEdge)
{
	// Full disc with an opaque center: the first segment is filled, the lowest edge at 'innerEdge'
	// (0..1 of the height, which is the radius when warped into a disc).
	EdgemapSrc e;
	e.w = w; e.h = h; e.nEdges = 3; e.tiling = true;
	e.s.resize((w + 1) * e.nEdges);
	for (int c = 0; c <= w; c++)
	{
		int cs = (c == w) ? 0 : c;
		float a = float(cs) / w * 2 * float(M_PI);
		float* p = &e.s[c * e.nEdges];
		p[0] = h * innerEdge;
		p[1] = h * (0.55f + 0.12f * std::sin(a * 6));
		p[2] = h * (0.88f + 0.04f * std::sin(a * 40));
	}
	e.colors = { {0.20f,0.45f,0.85f,1}, {0.95f,0.95f,0.95f,1}, {0.98f,0.55f,0.10f,1}, {0,0,0,0} };
	return e;
}

struct EdgemapRenderStats { long long cells = 0, solidCells = 0, skippedCells = 0, pixelsShaded = 0; };

// Per pixel shading. Returns premultiplied color.
static inline RGBA shadeEdgemap(const EdgemapSrc& e, float u, float v, float dudx, float dudy, float dvdx, float dvdy, float uPeriod)
{
	// Position in source pixels.
	float xs = u * e.w;
	float ys = v * e.h;

	// Coverage of the source rectangle itself (outside of it is transparent). Distance to the
	// border in source units divided by gradient length gives distance in destination pixels.
	// Skipped when the pixel is clearly inside (same bound as for edges below).
	float inside = 1.f;
	float halfBoundV = 0.5f * e.h * (std::fabs(dvdx) + std::fabs(dvdy));
	if (ys < halfBoundV || e.h - ys < halfBoundV)
	{
		float gyLen = e.h * fastLen(dvdx, dvdy) + 1e-9f;
		inside = std::clamp(0.5f + ys / gyLen, 0.f, 1.f) * std::clamp(0.5f + (e.h - ys) / gyLen, 0.f, 1.f);
	}

	bool bWrapU = e.tiling && uPeriod == 1.f;
	if (bWrapU)
		xs -= e.w * std::floor(xs / e.w);
	else
	{
		float halfBoundU = 0.5f * e.w * (std::fabs(dudx) + std::fabs(dudy));
		if (xs < halfBoundU || e.w - xs < halfBoundU)
		{
			float gxLen = e.w * fastLen(dudx, dudy) + 1e-9f;
			inside *= std::clamp(0.5f + xs / gxLen, 0.f, 1.f) * std::clamp(0.5f + (e.w - xs) / gxLen, 0.f, 1.f);
		}
	}

	if (inside <= 0.f)
		return { 0,0,0,0 };

	int col = std::clamp(int(xs), 0, e.w - 1);
	float t = std::clamp(xs - col, 0.f, 1.f);

	// Upper bounds (L1 norm >= L2 norm) of how fast source x and y change per destination pixel.
	// An edge further away than half of its bound is fully on one side of the pixel, no need for
	// the exact gradient length.
	float boundU = e.w * (std::fabs(dudx) + std::fabs(dudy));
	float boundV = e.h * (std::fabs(dvdx) + std::fabs(dvdy));

	float covBelow[16];
	bool bMixed = inside < 1.f;
	for (int i = 0; i < e.nEdges; i++)
	{
		float y, slope;
		e.edgeAt(col, t, i, y, slope);
		float f = y - ys;
		float cb;
		float bound = std::fabs(slope) * boundU + boundV;
		if (f >= 0.5f * bound)
			cb = 0.f;
		else if (-f >= 0.5f * bound)
			cb = 1.f;
		else
		{
			float gx = slope * e.w * dudx - e.h * dvdx;
			float gy = slope * e.w * dudy - e.h * dvdy;
			float g = std::sqrt(gx * gx + gy * gy) + 1e-9f;
			cb = std::clamp(0.5f - f / g, 0.f, 1.f);
			bMixed = true;
		}
		covBelow[i] = (i > 0) ? std::min(cb, covBelow[i - 1]) : cb;		// Keep edges ordered.
	}

	if (!bMixed)
	{
		// Pixel entirely within one segment.
		int seg = 0;
		while (seg < e.nEdges && covBelow[seg] == 1.f) seg++;
		return e.colors[seg];
	}

	RGBA out = { 0,0,0,0 };
	float prev = 1.f;
	for (int k = 0; k <= e.nEdges; k++)
	{
		float next = (k < e.nEdges) ? covBelow[k] : 0.f;
		float frac = (prev - next) * inside;
		const RGBA& c = e.colors[k];
		out.r += c.r * frac; out.g += c.g * frac; out.b += c.b * frac; out.a += c.a * frac;
		prev = next;
	}
	return out;
}

// Classifies a cell: returns segment index if the whole cell (with antialiasing margin) is inside
// one segment and inside the source, -1 if it is entirely outside the source, -2 if it needs per
// pixel work.
static int classifyCell(const EdgemapSrc& e, const Cell& c, float uPeriod)
{
	bool bWrapU = e.tiling && uPeriod == 1.f;

	float marginV = e.h * c.gradV;		// One destination pixel in source y units (conservative 2x half pixel).
	float y0 = c.vMin * e.h, y1 = c.vMax * e.h;

	if (y1 + marginV < 0 || y0 - marginV > e.h)
		return -1;

	float uMin = c.uMin, uMax = c.uMax;
	if (!bWrapU && uPeriod > 0)
	{
		// Bring the cell's u range to start within [0, period). If it then crosses the period,
		// pixels in it read from both ends of the source.
		float k = std::floor(uMin / uPeriod);
		uMin -= k * uPeriod; uMax -= k * uPeriod;
		if (uMax >= uPeriod)
			return -2;
	}

	float x0 = uMin * e.w, x1 = uMax * e.w;
	if (!bWrapU)
	{
		float marginU = e.w * c.gradU;
		if (x1 + marginU < 0 || x0 - marginU > e.w)
			return -1;
		if (x0 - marginU < 0 || x1 + marginU > e.w)
			return -2;
	}
	if (y0 - marginV < 0 || y1 + marginV > e.h)
		return -2;

	// Columns touched.
	int cBeg = int(std::floor(x0)), cEnd = int(std::floor(x1)) + 1;
	if (cEnd - cBeg > e.w) return -2;

	int segment = -1;
	float maxSlope = 0;
	float eMin[16], eMax[16];
	for (int i = 0; i < e.nEdges; i++) { eMin[i] = 1e30f; eMax[i] = -1e30f; }
	for (int cc = cBeg; cc <= cEnd; cc++)
	{
		int col = bWrapU ? ((cc % e.w) + e.w) % e.w : std::clamp(cc, 0, e.w);
		for (int i = 0; i < e.nEdges; i++)
		{
			float val = e.s[col * e.nEdges + i];
			eMin[i] = std::min(eMin[i], val); eMax[i] = std::max(eMax[i], val);
			if (col < e.w) maxSlope = std::max(maxSlope, std::fabs(e.s[(col + 1) * e.nEdges + i] - val));
		}
	}
	float margin = marginV + maxSlope * e.w * c.gradU;

	int above = 0;
	for (int i = 0; i < e.nEdges; i++)
	{
		if (eMax[i] < y0 - margin) above++;				// Edge entirely above the cell.
		else if (eMin[i] > y1 + margin) {}				// Entirely below.
		else return -2;
	}
	segment = above;
	return segment;
}

static void renderEdgemapGrid(Image& dst, const Warpmap& m, const EdgemapSrc& e, bool bClassify, bool bUnwrap, EdgemapRenderStats* pStats, float uOffset = 0)
{
	for (int j = 0; j < m.ny - 1; j++)
		for (int i = 0; i < m.nx - 1; i++)
		{
			Cell c;
			setupCell(m, i, j, c, bUnwrap);
			if (uOffset != 0) { c.u00 += uOffset; c.u10 += uOffset; c.u01 += uOffset; c.u11 += uOffset; c.uMin += uOffset; c.uMax += uOffset; }
			if (pStats) pStats->cells++;

			if (bClassify)
			{
				int seg = classifyCell(e, c, m.uPeriod);
				if (seg == -1) { if (pStats) pStats->skippedCells++; continue; }
				if (seg >= 0)
				{
					if (pStats) pStats->solidCells++;
					const RGBA& col = e.colors[seg];
					if (col.a > 0)
						for (int y = 0; y < c.ch; y++)
							for (int x = 0; x < c.cw; x++)
								blendOver(dst.at(c.x0 + x, c.y0 + y), col);
					continue;
				}
			}

			// Read wrap for periodic u when the source doesn't wrap by itself (arcs): bring u to [0, period).
			float period = m.uPeriod;
			bool bWrapSrc = e.tiling && period == 1.f;
			forCellPixels(c, [&](int x, int y, float u, float v, float dudx, float dudy, float dvdx, float dvdy)
			{
				if (period > 0 && !bWrapSrc)
					u -= period * std::floor(u / period);
				RGBA col = shadeEdgemap(e, u, v, dudx, dudy, dvdx, dvdy, period);
				if (col.a > 0) blendOver(dst.at(x, y), col);
			});
			if (pStats) pStats->pixelsShaded += c.cw * c.ch;
		}
}

static void renderEdgemapExact(Image& dst, const Polar& p, const EdgemapSrc& e, float uOffset = 0)
{
	float period = p.uPeriod();
	bool bWrapSrc = e.tiling && period == 1.f;
	for (int y = 0; y < dst.h; y++)
		for (int x = 0; x < dst.w; x++)
		{
			float u, v, dudx, dudy, dvdx, dvdy;
			p.eval(x + 0.5f, y + 0.5f, u, v, dudx, dudy, dvdx, dvdy);
			u += uOffset;
			if (!bWrapSrc) u -= period * std::floor(u / period);
			RGBA col = shadeEdgemap(e, u, v, dudx, dudy, dvdx, dvdy, period);
			if (col.a > 0) blendOver(dst.at(x, y), col);
		}
}

//____ Surface source ___________________________________________________________

struct SurfaceSrc
{
	int w, h;
	std::vector<RGBA> px;
	RGBA at(int x, int y) const { return px[y * w + x]; }
};

static SurfaceSrc makeTestSurface(int w, int h)
{
	// Checkerboard with colored gradient and grid lines, good for seeing distortion and filtering.
	SurfaceSrc s{ w, h, std::vector<RGBA>(w * h) };
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
		{
			bool check = ((x / 16) + (y / 16)) & 1;
			float r = float(x) / w, b = float(y) / h;
			float l = check ? 1.f : 0.55f;
			RGBA c = { l * (0.3f + 0.7f * r), l * 0.45f, l * (0.3f + 0.7f * b), 1 };
			if (x % 64 == 0 || y % 64 == 0) c = { 1,1,1,1 };
			s.px[y * w + x] = c;
		}
	return s;
}

static inline RGBA sampleBilinear(const SurfaceSrc& s, float xs, float ys, bool wrapX, bool wrapY)
{
	xs -= 0.5f; ys -= 0.5f;
	int x0 = int(std::floor(xs)), y0 = int(std::floor(ys));
	float fx = xs - x0, fy = ys - y0;
	auto fetch = [&](int x, int y) -> RGBA
	{
		if (wrapX) x = ((x % s.w) + s.w) % s.w; else x = std::clamp(x, 0, s.w - 1);
		if (wrapY) y = ((y % s.h) + s.h) % s.h; else y = std::clamp(y, 0, s.h - 1);
		return s.at(x, y);
	};
	RGBA a = fetch(x0, y0), b = fetch(x0 + 1, y0), c = fetch(x0, y0 + 1), d = fetch(x0 + 1, y0 + 1);
	auto lerp = [](const RGBA& p, const RGBA& q, float t) { return RGBA{ p.r + (q.r - p.r) * t, p.g + (q.g - p.g) * t, p.b + (q.b - p.b) * t, p.a + (q.a - p.a) * t }; };
	return lerp(lerp(a, b, fx), lerp(c, d, fx), fy);
}

struct SurfaceRenderStats { long long pixels = 0, supersampled = 0; };

static void renderSurfaceGrid(Image& dst, const Warpmap& m, const SurfaceSrc& s, bool bSupersample, SurfaceRenderStats* pStats)
{
	bool wrapX = m.uPeriod == 1.f, wrapY = m.vPeriod == 1.f;
	for (int j = 0; j < m.ny - 1; j++)
		for (int i = 0; i < m.nx - 1; i++)
		{
			Cell c;
			setupCell(m, i, j, c, true);

			// Skip cells entirely outside the source.
			float mv = c.gradV, mu = c.gradU;
			if (!wrapY && (c.vMax + mv < 0 || c.vMin - mv > 1)) continue;
			if (!wrapX && (c.uMax + mu < 0 || c.uMin - mu > 1)) continue;

			// Footprint: source pixels covered by one destination pixel, worst case in cell.
			float footprint = std::max(c.gradU * s.w, c.gradV * s.h);
			int ss = !bSupersample || footprint < 1.5f ? 1 : footprint < 3.f ? 2 : 4;
			if (pStats) { pStats->pixels += c.cw * c.ch; if (ss > 1) pStats->supersampled += c.cw * c.ch; }

			forCellPixels(c, [&](int x, int y, float u, float v, float dudx, float dudy, float dvdx, float dvdy)
			{
				// Coverage of source rectangle for non-wrapping axes.
				float inside = 1.f;
				if (!wrapY) { float g = s.h * fastLen(dvdx, dvdy) + 1e-9f; float ys = v * s.h; inside *= std::clamp(0.5f + ys / g, 0.f, 1.f) * std::clamp(0.5f + (s.h - ys) / g, 0.f, 1.f); }
				if (!wrapX) { float g = s.w * fastLen(dudx, dudy) + 1e-9f; float xs = u * s.w; inside *= std::clamp(0.5f + xs / g, 0.f, 1.f) * std::clamp(0.5f + (s.w - xs) / g, 0.f, 1.f); }
				if (inside <= 0) return;

				RGBA acc = { 0,0,0,0 };
				if (ss == 1)
					acc = sampleBilinear(s, u * s.w, v * s.h, wrapX, wrapY);
				else
				{
					float inv = 1.f / ss;
					for (int sy = 0; sy < ss; sy++)
						for (int sx = 0; sx < ss; sx++)
						{
							float ox = (sx + 0.5f) * inv - 0.5f, oy = (sy + 0.5f) * inv - 0.5f;
							float uu = u + dudx * ox + dudy * oy, vv = v + dvdx * ox + dvdy * oy;
							RGBA c2 = sampleBilinear(s, uu * s.w, vv * s.h, wrapX, wrapY);
							acc.r += c2.r; acc.g += c2.g; acc.b += c2.b; acc.a += c2.a;
						}
					float k = inv * inv;
					acc.r *= k; acc.g *= k; acc.b *= k; acc.a *= k;
				}
				acc.r *= inside; acc.g *= inside; acc.b *= inside; acc.a *= inside;
				blendOver(dst.at(x, y), acc);
			});
		}
}

//____ Comparisons ______________________________________________________________

static void imageDiff(const Image& a, const Image& b, float& maxDiff, float& meanDiff, long long& nOver2)
{
	maxDiff = 0; double sum = 0; nOver2 = 0;
	for (size_t i = 0; i < a.px.size(); i++)
	{
		float d = std::max({ std::fabs(a.px[i].r - b.px[i].r), std::fabs(a.px[i].g - b.px[i].g), std::fabs(a.px[i].b - b.px[i].b), std::fabs(a.px[i].a - b.px[i].a) }) * 255;
		maxDiff = std::max(maxDiff, d); sum += d;
		if (d > 2.f) nOver2++;
	}
	meanDiff = float(sum / a.px.size());
}

// Position error of the grid compared to exact polar mapping, in destination pixels.
static void gridError(const Warpmap& m, const Polar& p, float vLo, float vHi, float& maxRadial, float& maxTangential, float& meanTotal)
{
	maxRadial = maxTangential = 0; double sum = 0; long long n = 0;
	for (int j = 0; j < m.ny - 1; j++)
		for (int i = 0; i < m.nx - 1; i++)
		{
			Cell c;
			setupCell(m, i, j, c, true);
			forCellPixels(c, [&](int x, int y, float u, float v, float, float, float, float)
			{
				float ue, ve, a, b, cc, d;
				p.eval(x + 0.5f, y + 0.5f, ue, ve, a, b, cc, d);
				if (ve < vLo || ve > vHi) return;
				float du = unwrapTo(u, ue, p.uPeriod()) - ue;
				float r = p.rIn + ve * (p.rOut - p.rIn);
				float tang = std::fabs(du) * p.sweep * r;
				float rad = std::fabs(v - ve) * (p.rOut - p.rIn);
				maxRadial = std::max(maxRadial, rad); maxTangential = std::max(maxTangential, tang);
				sum += fastLen(tang, rad); n++;
			});
		}
	meanTotal = float(sum / std::max(n, 1LL));
}

template<class F>
static double timeIt(F&& f, int reps)
{
	double best = 1e30;
	for (int r = 0; r < reps; r++) { double t0 = now(); f(); best = std::min(best, now() - t0); }
	return best;
}

//____ main _____________________________________________________________________

int main()
{
	const int S = 512;
	const RGBA clear = { 0,0,0,0 };

	// Ring: radius 110 -> 240. Edgemap 360 x 48, tiling, wraps around.
	Polar ring{ S / 2.f, S / 2.f, 110.f, 240.f, 0.f, float(2 * M_PI) };
	EdgemapSrc gauge = makeGaugeEdgemap(360, 48, true);

	printf("== Edgemap ring %dx%d, r %.0f..%.0f, edgemap %dx%d tiling, %d edges\n", S, S, ring.rIn, ring.rOut, gauge.w, gauge.h, gauge.nEdges);

	Image exact(S, S, clear);
	renderEdgemapExact(exact, ring, gauge);
	exact.savePPM("ring_exact.ppm");

	for (int cell : { 8, 16, 32 })
	{
		Warpmap m = makePolarWarpmap(S, S, cell, ring);
		Image img(S, S, clear);
		EdgemapRenderStats st;
		renderEdgemapGrid(img, m, gauge, true, true, &st);
		img.savePPM("ring_grid" + std::to_string(cell) + ".ppm");

		Image imgNoClass(S, S, clear);
		renderEdgemapGrid(imgNoClass, m, gauge, false, true, nullptr);

		float maxD, meanD, cmax, cmean; long long nOver, cOver;
		imageDiff(img, exact, maxD, meanD, nOver);
		imageDiff(img, imgNoClass, cmax, cmean, cOver);
		float eRad, eTan, eMean;
		gridError(m, ring, 0.f, 1.f, eRad, eTan, eMean);

		printf("cell %2d: nodes %dx%d (%zu bytes as float pairs). Cells: %lld, solid %lld, skipped %lld, per-pixel %lld (%lld px)\n",
			   cell, m.nx, m.ny, m.u.size() * 8, st.cells, st.solidCells, st.skippedCells, st.cells - st.solidCells - st.skippedCells, st.pixelsShaded);
		printf("         position error vs exact: max radial %.3f px, max tangential %.3f px, mean %.3f px\n", eRad, eTan, eMean);
		printf("         image vs exact: max %.1f, mean %.3f, pixels > 2/255: %lld.  Classification vs none: max %.1f\n", maxD, meanD, nOver, cmax);
	}

	// Timings

	printf("\n== Timings, %dx%d ring (best of 15)\n", S, S);
	{
		Image img(S, S, clear);
		double t = timeIt([&] { std::fill(img.px.begin(), img.px.end(), clear); renderEdgemapExact(img, ring, gauge); }, 15);
		printf("exact per pixel (atan2, sqrt)       : %7.2f ms\n", t * 1e3);
		for (int cell : { 8, 16, 32 })
		{
			Warpmap m = makePolarWarpmap(S, S, cell, ring);
			double t1 = timeIt([&] { std::fill(img.px.begin(), img.px.end(), clear); renderEdgemapGrid(img, m, gauge, false, true, nullptr); }, 15);
			double t2 = timeIt([&] { std::fill(img.px.begin(), img.px.end(), clear); renderEdgemapGrid(img, m, gauge, true, true, nullptr); }, 15);
			printf("grid %2d, every pixel shaded         : %7.2f ms\n", cell, t1 * 1e3);
			printf("grid %2d, solid/empty cells skipped   : %7.2f ms\n", cell, t2 * 1e3);
		}
		Image clr(S, S, clear);
		double tf = timeIt([&] { std::fill(img.px.begin(), img.px.end(), clear); for (auto& p : img.px) blendOver(p, RGBA{ 0.5f,0.2f,0.1f,0.8f }); }, 15);
		printf("reference: blend-fill whole image   : %7.2f ms\n", tf * 1e3);
	}

	// Seam: without period handling.
	{
		Warpmap m = makePolarWarpmap(S, S, 16, ring);
		Image img(S, S, clear);
		renderEdgemapGrid(img, m, gauge, false, false, nullptr);
		img.savePPM("ring_noseam_fix.ppm");
	}

	// Spinner: same map, u offset.
	{
		Warpmap m = makePolarWarpmap(S, S, 16, ring);
		Image img(S, S, clear);
		renderEdgemapGrid(img, m, gauge, true, true, nullptr, 0.125f);
		img.savePPM("ring_rotated.ppm");
	}

	// 270 degree arc, non-tiling progress edgemap, start at -135 degrees.
	{
		Polar arc{ S / 2.f, S / 2.f, 150.f, 230.f, float(-0.75 * M_PI), float(1.5 * M_PI) };
		EdgemapSrc prog = makeProgressEdgemap(300, 40);
		Warpmap m = makePolarWarpmap(S, S, 16, arc);
		Image img(S, S, clear);
		EdgemapRenderStats st;
		renderEdgemapGrid(img, m, prog, true, true, &st);
		img.savePPM("arc270.ppm");
		Image ex(S, S, clear);
		renderEdgemapExact(ex, arc, prog);
		float maxD, meanD; long long nOver;
		imageDiff(img, ex, maxD, meanD, nOver);
		printf("\n== 270 degree arc, uPeriod %.3f, cell 16: vs exact max %.1f mean %.3f, pixels > 2/255: %lld\n", arc.uPeriod(), maxD, meanD, nOver);
	}

	// Full disc (no hole): shows the center singularity.
	{
		Polar disc{ S / 2.f, S / 2.f, 0.f, 240.f, 0.f, float(2 * M_PI) };
		for (int cell : { 8, 16 })
		{
			Warpmap m = makePolarWarpmap(S, S, cell, disc);
			Image img(S, S, clear);
			renderEdgemapGrid(img, m, gauge, true, true, nullptr);
			img.savePPM("disc_grid" + std::to_string(cell) + ".ppm");
			float eRad, eTan, eMean;
			gridError(m, disc, 0.f, 1.f, eRad, eTan, eMean);
			float eRad2, eTan2, eMean2;
			gridError(m, disc, 2.f * cell / 240.f, 1.f, eRad2, eTan2, eMean2);
			printf("== Disc, cell %2d: max position error %.2f px radial, %.2f px tangential; outside r > 2 cells: %.3f / %.3f\n", cell, eRad, eTan, eRad2, eTan2);
		}
	}

	// Full disc with filled center, edges kept away from the center.
	{
		Polar disc{ S / 2.f, S / 2.f, 0.f, 240.f, 0.f, float(2 * M_PI) };
		printf("\n== Filled disc (no hole), radius 240\n");
		for (float innerEdge : { 0.05f, 0.14f, 0.25f })
		{
			EdgemapSrc filled = makeFilledDiscEdgemap(360, 48, innerEdge);
			Image ex(S, S, clear);
			renderEdgemapExact(ex, disc, filled);
			{
				char name[64];
				snprintf(name, sizeof(name), "filleddisc_%02d_exact.ppm", int(innerEdge * 100 + 0.5f));
				ex.savePPM(name);
			}
			for (int cell : { 8, 16 })
			{
				Warpmap m = makePolarWarpmap(S, S, cell, disc);
				Image img(S, S, clear);
				renderEdgemapGrid(img, m, filled, true, true, nullptr);
				char name[64];
				snprintf(name, sizeof(name), "filleddisc_%02d_grid%d.ppm", int(innerEdge * 100 + 0.5f), cell);
				img.savePPM(name);
				float maxD, meanD; long long nOver;
				imageDiff(img, ex, maxD, meanD, nOver);
				printf("lowest edge at r = %3.0f px (%.1f cells), cell %2d: vs exact max %5.1f, mean %.3f, pixels > 2/255: %lld\n",
					   innerEdge * 240, innerEdge * 240 / cell, cell, maxD, meanD, nOver);
			}
		}
	}

	// Surfaces
	printf("\n== Surface warps\n");
	{
		SurfaceSrc surf = makeTestSurface(512, 128);
		Polar sring{ S / 2.f, S / 2.f, 60.f, 240.f, 0.f, float(2 * M_PI) };
		Warpmap m = makePolarWarpmap(S, S, 16, sring);

		Image a(S, S, clear), b(S, S, clear);
		SurfaceRenderStats st;
		renderSurfaceGrid(a, m, surf, false, nullptr);
		renderSurfaceGrid(b, m, surf, true, &st);
		a.savePPM("surface_ring_bilinear.ppm");
		b.savePPM("surface_ring_supersampled.ppm");
		double t1 = timeIt([&] { std::fill(a.px.begin(), a.px.end(), clear); renderSurfaceGrid(a, m, surf, false, nullptr); }, 15);
		double t2 = timeIt([&] { std::fill(b.px.begin(), b.px.end(), clear); renderSurfaceGrid(b, m, surf, true, nullptr); }, 15);
		printf("ring 512x128 surface, cell 16: bilinear %.2f ms, with supersampling %.2f ms (%.0f%% of pixels supersampled)\n",
			   t1 * 1e3, t2 * 1e3, 100.0 * st.supersampled / std::max(st.pixels, 1LL));

		SurfaceSrc surf2 = makeTestSurface(512, 512);
		Warpmap wave = makeWaveWarpmap(S, S, 16, 0.03f, 0.03f, 3);
		Image c(S, S, clear);
		renderSurfaceGrid(c, wave, surf2, true, nullptr);
		c.savePPM("surface_wave.ppm");
		double t3 = timeIt([&] { std::fill(c.px.begin(), c.px.end(), clear); renderSurfaceGrid(c, wave, surf2, true, nullptr); }, 15);
		printf("wave 512x512, cell 16: %.2f ms\n", t3 * 1e3);
	}

	return 0;
}
