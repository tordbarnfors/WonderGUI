#pragma once

#include <wax_softshapes.h>

#include <climits>
#include <functional>
#include <string>
#include <vector>

//____ ShapeCoverage __________________________________________________________
// Coverage of each segment of a SoftShapes shape in each pixel of an area.
// SoftShapes mixes the colors of segments sharing a pixel, so the shape is
// rasterized once for each segment, drawn as the only one, in white. Checks
// that runs are well formed and pixels aren't given twice.

class ShapeCoverage : public wax::SoftShapes::RowSink
{
public:
	ShapeCoverage(const wax::RectI& area, int nSegments)
		: m_area(area), m_nSegments(nSegments), m_coverage(size_t(nSegments) * area.w * area.h, 0), m_hits(size_t(area.w) * area.h, 0) {}

	// Calls rasterize once for each segment, which rasterizes the shape into the given sink.

	void collect(const std::function<void(wax::SoftShapes::RowSink*)>& rasterize)
	{
		for (m_segment = 0; m_segment < m_nSegments; m_segment++)
		{
			std::fill(m_hits.begin(), m_hits.end(), 0);
			rasterize(this);
		}
	}

	const int16_t* segmentColors(int segment, int y, int begin, int end, bool& bFlat) override
	{
		static const int16_t white[4] = { 4096, 4096, 4096, 4096 };

		bFlat = true;
		return segment == m_segment ? white : nullptr;
	}

	void row(int y, int nRuns, const wax::SoftShapes::Run* pRuns, const wax::SoftShapes::CoveragePixel* pPixels) override
	{
		int prevEnd = INT_MIN;

		for (int i = 0; i < nRuns; i++)
		{
			const wax::SoftShapes::Run& run = pRuns[i];

			if (run.begin >= run.end || run.begin < prevEnd || run.begin < m_area.x || run.end > m_area.x + m_area.w || y < m_area.y || y >= m_area.y + m_area.h)
			{
				_error("Bad run " + std::to_string(run.begin) + "-" + std::to_string(run.end) + " on row " + std::to_string(y));
				return;
			}

			if (run.segment >= 0 && run.segment != m_segment)
				_error("Run of segment " + std::to_string(run.segment) + ", which isn't drawn, on row " + std::to_string(y));

			prevEnd = run.end;

			for (int x = run.begin; x < run.end; x++)
			{
				size_t idx = size_t(y - m_area.y) * m_area.w + (x - m_area.x);

				if (++m_hits[idx] > 1)
					_error("Pixel " + std::to_string(x) + "," + std::to_string(y) + " given twice.");

				int coverage = wax::SoftShapes::c_fullCoverage;

				if (run.segment < 0)
				{
					const wax::SoftShapes::CoveragePixel& px = *pPixels++;
					coverage = px.coverage;

					if (px.coverage <= 0 || px.coverage > wax::SoftShapes::c_fullCoverage || px.b != px.coverage || px.a != px.coverage)
						_error("Bad coverage pixel " + std::to_string(x) + "," + std::to_string(y));
				}

				m_coverage[size_t(m_segment) * m_area.w * m_area.h + idx] += coverage;
			}
		}
	}

	int coverage(int segment, int x, int y) const { return m_coverage[size_t(segment) * m_area.w * m_area.h + size_t(y - m_area.y) * m_area.w + (x - m_area.x)]; }

	bool			hasError() const { return !m_error.empty(); }
	std::string		error() const { return m_error; }

private:
	void _error(const std::string& msg) { if (m_error.empty()) m_error = msg; }

	wax::RectI			m_area;
	int					m_nSegments;
	int					m_segment = 0;
	std::vector<int>	m_coverage;
	std::vector<int>	m_hits;
	std::string			m_error;
};
