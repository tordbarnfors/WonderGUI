/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#include <wg_edgemapshape.h>
#include <wg_gfxdevice.h>
#include <wg_gfxbase.h>
#include <wg_gfxutil.h>

#include <algorithm>
#include <cmath>

namespace wg
{
	const TypeInfo EdgemapShape::TYPEINFO = { "EdgemapShape", &Object::TYPEINFO };

	//____ typeInfo() ____________________________________________________________

	const TypeInfo& EdgemapShape::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ bounds() ______________________________________________________________
	/**
	 * @brief Rectangle covered by the pieces of the shape.
	 */

	RectSPX EdgemapShape::bounds() const
	{
		if( m_pieces.empty() )
			return RectSPX();

		RectSPX bounds = _pieceRect(m_pieces[0]);

		for( size_t i = 1 ; i < m_pieces.size() ; i++ )
			bounds.growToContain(_pieceRect(m_pieces[i]));

		return bounds;
	}

	//____ draw() ________________________________________________________________
	/**
	 * @brief Draw the shape.
	 *
	 * Draws the edgemaps of all pieces, moved by offset, which is rounded to whole pixels.
	 */

	void EdgemapShape::draw( GfxDevice * pDevice, CoordSPX offset ) const
	{
		offset = Util::roundToPixels(offset) * 64;

		for( auto& piece : m_pieces )
			pDevice->flipDrawEdgemap(piece.pos + offset, piece.pEdgemap, piece.flip);
	}

	//____ _pieceRect() __________________________________________________________

	RectSPX EdgemapShape::_pieceRect( const Piece& piece )
	{
		return { piece.pos, Util::flipSize(piece.pEdgemap->pixelSize(), piece.flip) * 64 };
	}

	//____ _factoryOrDefault() ___________________________________________________

	EdgemapFactory_p EdgemapShape::_factoryOrDefault( EdgemapFactory * pFactory )
	{
		return pFactory ? EdgemapFactory_p(pFactory) : GfxBase::defaultEdgemapFactory();
	}

	//____ _buildPieces() ________________________________________________________
	/*
		Makes the pieces of the shape from the cells of grid and the regions along lines through
		them, reusing the edgemaps of the current pieces where possible.

		Cells with the same orientation are joined into larger pieces where the regions along
		their lines can still be lined up as the segments of one edgemap: first along rows of
		cells for pieces running along canvas rows and along columns of cells for the others,
		then across. Cells where nothing is drawn are left out.

		Returns false if an edgemap couldn't be created.
	*/

	bool EdgemapShape::_buildPieces( const Grid& grid, const RegionFunc& regions, const HiColor * pColors, EdgemapFactory * pFactory )
	{
		std::vector<Build>	builds;
		Description shape = { regions, pColors };

		int nx = int(grid.x.size()) - 1;
		int ny = int(grid.y.size()) - 1;

		// Build each cell on its own. Cells that can't be built in one piece are split and kept apart.

		struct Cell
		{
			int		orientation = -1;
			Build	build;
		};

		std::vector<Cell> cells(std::max(nx, 0) * std::max(ny, 0));

		auto cellRect = [&grid](int i, int j) { return RectI(grid.x[i], grid.y[j], grid.x[i + 1] - grid.x[i], grid.y[j + 1] - grid.y[j]); };

		for (int j = 0; j < ny; j++)
		{
			for (int i = 0; i < nx; i++)
			{
				RectI rect = cellRect(i, j);
				if (rect.w <= 0 || rect.h <= 0)
					continue;

				int orientation = grid.orientation(rect);
				if (orientation < 0)
					continue;

				Cell& cell = cells[j * nx + i];

				if (!_buildPiece(rect, orientation == 1, shape, false, cell.build))
					_buildCell(rect, orientation == 1, shape, builds);
				else if (!cell.build.bEmpty)
					cell.orientation = orientation;
			}
		}

		// Join cells into runs, along rows of cells for row pieces and columns of cells for column pieces.

		std::vector<Build> rowRuns, columnRuns;

		auto join = [&](Build& run, const RectI& rect)
		{
			RectI bounds = RectI::bounds(run.rect, rect);

			for (auto& barrier : run.bRows ? grid.barrierRows : grid.barrierColumns)
			{
				int begin = run.bRows ? bounds.y : bounds.x;
				int end = begin + (run.bRows ? bounds.h : bounds.w);
				int acrossBegin = run.bRows ? bounds.x : bounds.y;
				int acrossEnd = acrossBegin + (run.bRows ? bounds.w : bounds.h);

				if (barrier.line > begin && barrier.line < end && barrier.begin < acrossEnd && barrier.end > acrossBegin)
					return false;
			}

			Build joined;
			if (!_buildPiece(bounds, run.bRows, shape, false, joined))
				return false;

			run = std::move(joined);
			return true;
		};

		for (int j = 0; j < ny; j++)
		{
			for (int i = 0; i < nx; i++)
			{
				Cell& cell = cells[j * nx + i];
				if (cell.orientation != 1)
					continue;

				Build run = std::move(cell.build);

				while (i + 1 < nx && cells[j * nx + i + 1].orientation == 1 && join(run, cellRect(i + 1, j)))
					i++;

				rowRuns.push_back(std::move(run));
			}
		}

		for (int i = 0; i < nx; i++)
		{
			for (int j = 0; j < ny; j++)
			{
				Cell& cell = cells[j * nx + i];
				if (cell.orientation != 0)
					continue;

				Build run = std::move(cell.build);

				while (j + 1 < ny && cells[(j + 1) * nx + i].orientation == 0 && join(run, cellRect(i, j + 1)))
					j++;

				columnRuns.push_back(std::move(run));
			}
		}

		// Join runs across, row runs that line up vertically and column runs that line up horizontally.

		auto joinAcross = [&](std::vector<Build>& runs, bool bRows)
		{
			std::sort(runs.begin(), runs.end(), [bRows](const Build& a, const Build& b)
			{
				return bRows ? std::make_tuple(a.rect.x, a.rect.w, a.rect.y) < std::make_tuple(b.rect.x, b.rect.w, b.rect.y)
							 : std::make_tuple(a.rect.y, a.rect.h, a.rect.x) < std::make_tuple(b.rect.y, b.rect.h, b.rect.x);
			});

			for (size_t k = 0; k < runs.size(); )
			{
				Build run = std::move(runs[k++]);

				while (k < runs.size())
				{
					const RectI& next = runs[k].rect;
					bool bLinedUp = bRows ? (next.x == run.rect.x && next.w == run.rect.w && next.y == run.rect.y + run.rect.h)
										  : (next.y == run.rect.y && next.h == run.rect.h && next.x == run.rect.x + run.rect.w);

					if (!bLinedUp || !join(run, next))
						break;
					k++;
				}

				builds.push_back(std::move(run));
			}
		};

		joinAcross(rowRuns, true);
		joinAcross(columnRuns, false);

		return _applyPieces(builds, pFactory);
	}

	//____ _buildCell() __________________________________________________________
	// Builds a cell that can't be built as one piece, by splitting it in halves until the halves
	// can be built. Cells of a single pixel are built anyway, as well as it goes.

	void EdgemapShape::_buildCell( const RectI& rect, bool bRows, const Description& shape, std::vector<Build>& out )
	{
		Build build;

		bool bSingle = rect.w == 1 && rect.h == 1;

		if (_buildPiece(rect, bRows, shape, bSingle, build))
		{
			if (!build.bEmpty)
				out.push_back(std::move(build));
			return;
		}

		if (rect.w >= rect.h)
		{
			int half = rect.w / 2;
			_buildCell({ rect.x, rect.y, half, rect.h }, bRows, shape, out);
			_buildCell({ rect.x + half, rect.y, rect.w - half, rect.h }, bRows, shape, out);
		}
		else
		{
			int half = rect.h / 2;
			_buildCell({ rect.x, rect.y, rect.w, half }, bRows, shape, out);
			_buildCell({ rect.x, rect.y + half, rect.w, rect.h - half }, bRows, shape, out);
		}
	}

	//____ _buildPiece() _________________________________________________________
	/*
		Builds the segments and samples of a piece from the regions along its lines.

		The regions are lined up in the order they appear along all lines, regions missing from a
		line getting no width there. Neighbouring regions with the same color become one segment.

		Fails if the regions can't be lined up in one order or need more segments than an
		edgemap has, unless bForce is set. Then the order is taken from where the regions are on
		average and the narrowest segments are joined with their neighbours.
	*/

	bool EdgemapShape::_buildPiece( const RectI& rect, bool bRows, const Description& shape, bool bForce, Build& out )
	{
		const RegionFunc& regions = shape.regions;
		const HiColor* pColors = shape.pColors;

		static const double c_inset = 1.0 / 4096;			// How far lines at the edges are moved into the piece.

		int nLines = (bRows ? rect.h : rect.w) + 1;
		int lineLength = bRows ? rect.w : rect.h;
		double begin = bRows ? rect.x : rect.y;
		int firstLine = bRows ? rect.y : rect.x;

		// Regions along each line, empty ones dropped and neighbours with the same key joined.

		std::vector<std::vector<Region>> lines(nLines);

		for (int i = 0; i < nLines; i++)
		{
			double pos = firstLine + i;
			if (i == 0)
				pos += c_inset;
			else if (i == nLines - 1)
				pos -= c_inset;

			auto& line = lines[i];
			regions(rect, bRows, pos, begin, begin + lineLength, line);

			size_t n = 0;
			double prevEnd = 0.0;
			for (auto& region : line)
			{
				double end = std::min(region.end, double(lineLength));
				if (end <= prevEnd)
					continue;

				if (n > 0 && line[n - 1].key == region.key)
					line[n - 1].end = end;
				else
				{
					line[n] = region;
					line[n].end = end;
					n++;
				}
				prevEnd = end;
			}
			line.resize(n);
		}

		// Collect the regions and which ones follow which.

		struct Node
		{
			uint32_t			key;
			int					color;
			double				position = 0.0;			// Sum of where it starts on the lines it is on.
			int					count = 0;				// Lines it is on.
			int					before = 0;				// Regions that must come before it, not yet placed.
			std::vector<int>	after;					// Regions that must come after it.
		};

		std::vector<Node> nodes;

		auto nodeOf = [&nodes](const Region& region)
		{
			for (int k = 0; k < (int)nodes.size(); k++)
				if (nodes[k].key == region.key)
					return k;

			nodes.push_back({ region.key, region.color });
			return int(nodes.size()) - 1;
		};

		for (auto& line : lines)
		{
			int prev = -1;
			double start = 0.0;
			for (auto& region : line)
			{
				int k = nodeOf(region);
				nodes[k].position += start / std::max(lineLength, 1);
				nodes[k].count++;

				if (prev >= 0 && std::find(nodes[prev].after.begin(), nodes[prev].after.end(), k) == nodes[prev].after.end())
				{
					nodes[prev].after.push_back(k);
					nodes[k].before++;
				}

				prev = k;
				start = region.end;
			}
		}

		// Order them. Of the regions free to go next, the one furthest back on average goes first.

		std::vector<int> order;
		std::vector<bool> placed(nodes.size(), false);

		while (order.size() < nodes.size())
		{
			int best = -1;
			bool bFree = false;

			for (int k = 0; k < (int)nodes.size(); k++)
			{
				if (placed[k])
					continue;

				bool bKFree = nodes[k].before == 0;
				double pos = nodes[k].position / nodes[k].count;

				if (best < 0 || (bKFree && !bFree) || (bKFree == bFree && pos < nodes[best].position / nodes[best].count))
				{
					best = k;
					bFree = bKFree;
				}
			}

			if (!bFree && !bForce)
				return false;

			placed[best] = true;
			order.push_back(best);
			for (int k : nodes[best].after)
				nodes[k].before--;
		}

		// Group neighbouring regions with the same color into segments.

		auto colorOf = [pColors](int color) { return color >= 0 ? pColors[color] : HiColor::Transparent; };

		std::vector<int> groupOf(nodes.size());				// Segment of each region.
		std::vector<HiColor> colors;
		bool bEmpty = true;

		for (int k : order)
		{
			HiColor color = colorOf(nodes[k].color);
			if (colors.empty() || color != colors.back())
				colors.push_back(color);

			groupOf[k] = int(colors.size()) - 1;

			if (nodes[k].color >= 0)
				bEmpty = false;
		}

		// End of each segment on each line.

		int nGroups = (int)colors.size();
		std::vector<double> ends(nLines * nGroups);

		for (int i = 0; i < nLines; i++)
		{
			double* pEnds = &ends[i * nGroups];
			double pos = 0.0;

			for (int k : order)
			{
				for (auto& region : lines[i])
				{
					if (region.key == nodes[k].key)
					{
						pos = std::max(pos, region.end);
						break;
					}
				}

				pEnds[groupOf[k]] = pos;
			}
		}

		// A piece one pixel across gets the exact coverage of each segment in its first and last
		// pixel. Nothing else depends on its outermost lines, so the widths of the segments there
		// are set to make their average across the pixel match the area they cover in it.

		if (lineLength == 1)
		{
			static const int c_steps = 16;

			auto areasOfPixel = [&](int pixel, std::vector<double>& areas)
			{
				areas.assign(nGroups, 0.0);

				std::vector<Region> sub;
				for (int k = 0; k < c_steps; k++)
				{
					regions(rect, bRows, firstLine + pixel + (k + 0.5) / c_steps, begin, begin + 1, sub);

					double start = 0.0;
					for (auto& region : sub)
					{
						double end = std::min(region.end, 1.0);
						if (end > start)
						{
							for (int n = 0; n < (int)nodes.size(); n++)
								if (nodes[n].key == region.key)
									areas[groupOf[n]] += (end - start) / c_steps;
						}
						start = std::max(start, end);
					}
				}
			};

			auto setWidths = [&](int line, const std::vector<double>& widths)
			{
				double sum = 0.0;
				for (double w : widths)
					sum += w;

				if (sum <= 0.0)
					return;

				double end = 0.0;
				for (int g = 0; g < nGroups; g++)
				{
					end += widths[g] / sum;
					ends[line * nGroups + g] = end;
				}
			};

			auto widthsAt = [&](int line)
			{
				std::vector<double> widths(nGroups);
				for (int g = 0; g < nGroups; g++)
					widths[g] = ends[line * nGroups + g] - (g > 0 ? ends[line * nGroups + g - 1] : 0.0);
				return widths;
			};

			std::vector<double> areas;

			if (nLines == 2)
			{
				// A single pixel: the same widths along both lines.

				areasOfPixel(0, areas);
				setWidths(0, areas);
				setWidths(1, areas);
			}
			else
			{
				for (int outer : { 0, nLines - 1 })
				{
					int inner = outer == 0 ? 1 : nLines - 2;
					areasOfPixel(outer == 0 ? 0 : nLines - 2, areas);

					std::vector<double> innerWidths = widthsAt(inner);
					std::vector<double> widths(nGroups);
					for (int g = 0; g < nGroups; g++)
						widths[g] = std::max(0.0, 2 * areas[g] - innerWidths[g]);

					setWidths(outer, widths);
				}
			}
		}

		// Too many segments? Join the narrowest with a neighbour until they fit.

		if (nGroups > Edgemap::maxSegments)
		{
			if (!bForce)
				return false;

			while (nGroups > Edgemap::maxSegments)
			{
				int narrowest = 0;
				double narrowestWidth = 0.0;

				for (int g = 0; g < nGroups; g++)
				{
					double width = 0.0;
					for (int i = 0; i < nLines; i++)
						width = std::max(width, ends[i * nGroups + g] - (g > 0 ? ends[i * nGroups + g - 1] : 0.0));

					if (g == 0 || width < narrowestWidth)
					{
						narrowest = g;
						narrowestWidth = width;
					}
				}

				// The segment before takes its place, or the one after for the first.

				int dropEnd = narrowest > 0 ? narrowest - 1 : 0;
				colors.erase(colors.begin() + narrowest);

				std::vector<double> newEnds(nLines * (nGroups - 1));
				for (int i = 0; i < nLines; i++)
				{
					int n = 0;
					for (int g = 0; g < nGroups; g++)
						if (g != dropEnd)
							newEnds[i * (nGroups - 1) + n++] = ends[i * nGroups + g];
				}

				ends.swap(newEnds);
				nGroups--;
			}
		}

		// Edges in spx. A single segment gets a second one with no width, edgemaps need edges.

		if (nGroups == 1)
			colors.push_back(colors.back());

		int nEdges = (int)colors.size() - 1;

		out.rect = rect;
		out.bRows = bRows;
		out.bEmpty = bEmpty;
		out.colors = std::move(colors);
		out.samples.resize(nLines * nEdges);

		for (int i = 0; i < nLines; i++)
		{
			spx prev = 0;
			for (int e = 0; e < nEdges; e++)
			{
				spx edge = nGroups == 1 ? lineLength * 64 : spx(std::lround(ends[i * nGroups + e] * 64));
				edge = std::clamp(edge, prev, lineLength * 64);
				out.samples[i * nEdges + e] = edge;
				prev = edge;
			}
		}

		return true;
	}

	//____ _applyPieces() ________________________________________________________
	/*
		Makes builds the pieces of the shape. The edgemap of a piece already in the same place
		is kept if it has segments enough, and only colors and samples that differ are updated
		in it. Otherwise a new edgemap is created.

		Returns false if an edgemap couldn't be created.
	*/

	bool EdgemapShape::_applyPieces( std::vector<Build>& builds, EdgemapFactory * pFactory )
	{
		m_pieces.resize(builds.size());
		m_stored.resize(builds.size());

		for (size_t i = 0; i < builds.size(); i++)
		{
			Build& build = builds[i];
			Piece& piece = m_pieces[i];
			Stored& stored = m_stored[i];

			int nSegments = (int)build.colors.size();
			int nEdges = nSegments - 1;
			int nSamples = (build.bRows ? build.rect.h : build.rect.w) + 1;

			bool bSamePlace = piece.pEdgemap && stored.rect == build.rect && stored.bRows == build.bRows;

			if (bSamePlace && stored.capacity >= nSegments)
			{
				Edgemap* pEdgemap = piece.pEdgemap;

				if (stored.colors != build.colors)
					pEdgemap->setColors(0, nSegments, build.colors.data());

				if (pEdgemap->renderSegments() != nSegments)
					pEdgemap->setRenderSegments(nSegments);

				// Samples that differ, edge by edge.

				int nOldEdges = (int)stored.colors.size() - 1;

				for (int e = 0; e < nEdges; e++)
				{
					int first = -1, last = -1;

					for (int s = 0; s < nSamples; s++)
					{
						if (e >= nOldEdges || stored.samples[s * nOldEdges + e] != build.samples[s * nEdges + e])
						{
							if (first < 0)
								first = s;
							last = s;
						}
					}

					if (first >= 0)
						pEdgemap->importSamples(SampleOrigo::Top, &build.samples[first * nEdges + e], e, e + 1, first, last + 1, 1, nEdges);
				}
			}
			else
			{
				// Room for a couple more segments if it outgrew its edgemap, so it doesn't need to be recreated again.

				int capacity = bSamePlace ? std::min(nSegments + 2, int(Edgemap::maxSegments)) : nSegments;

				std::vector<HiColor> colors = build.colors;
				colors.resize(capacity, build.colors.back());

				SizeI size = build.bRows ? SizeI(build.rect.h, build.rect.w) : SizeI(build.rect.w, build.rect.h);

				auto pEdgemap = pFactory->createEdgemap(WGBP(Edgemap, _.colors = colors.data(), _.segments = capacity, _.size = size));
				if (!pEdgemap)
				{
					m_pieces.resize(i);
					m_stored.resize(i);
					return false;
				}

				if (capacity > nSegments)
					pEdgemap->setRenderSegments(nSegments);

				pEdgemap->importSamples(SampleOrigo::Top, build.samples.data(), 0, nEdges, 0, nSamples, 1, nEdges);

				piece.pEdgemap = pEdgemap;
				stored.capacity = capacity;
			}

			stored.rect = build.rect;
			stored.bRows = build.bRows;
			stored.colors = std::move(build.colors);
			stored.samples = std::move(build.samples);

			piece.pos = build.rect.pos() * 64;
			piece.flip = build.bRows ? GfxFlip::Rot90FlipX : GfxFlip::None;		// Transposed: edgemap columns along canvas rows.
		}

		return true;
	}

} // namespace wg
