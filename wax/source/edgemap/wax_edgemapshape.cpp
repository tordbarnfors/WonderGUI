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
#include <wax_edgemapshape.h>
#include <wax_device.h>
#include <wax_core.h>
#include <wax_util.h>

#include <algorithm>
#include <cmath>

namespace wax
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

	void EdgemapShape::draw( Device * pDevice, CoordSPX offset ) const
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
		return pFactory ? EdgemapFactory_p(pFactory) : Core::defaultEdgemapFactory();
	}

	//____ _buildPieces() ________________________________________________________
	/*
		Makes the pieces of the shape as laid out, from the regions along lines through them,
		reusing the edgemaps of the current pieces where possible. Pieces where nothing is drawn
		are left out.

		All pieces are built before any edgemap is touched, so the shape is left as it was if
		the regions of a piece don't fit the segments of an edgemap.
	*/

	EdgemapShape::BuildResult EdgemapShape::_buildPieces( const std::vector<Layout>& layout, const RegionFunc& regions, const HiColor * pColors, EdgemapFactory * pFactory )
	{
		std::vector<Build>	builds;

		for (auto& piece : layout)
		{
			if (piece.rect.w <= 0 || piece.rect.h <= 0)
				continue;

			Build build;
			if (!_buildPiece(piece, regions, pColors, build))
				return BuildResult::TooManySegments;

			if (!build.bEmpty)
				builds.push_back(std::move(build));
		}

		return _applyPieces(builds, pFactory) ? BuildResult::Ok : BuildResult::Failed;
	}

	//____ _buildPiece() _________________________________________________________
	/*
		Builds the segments and samples of a piece from the regions along its lines.

		The regions are lined up in the order they appear along all lines, regions missing from a
		line getting no width there. Neighbouring regions with the same color become one segment.

		Fails if the regions can't be lined up in one order or need more segments than an
		edgemap has.
	*/

	bool EdgemapShape::_buildPiece( const Layout& layout, const RegionFunc& regions, const HiColor * pColors, Build& out )
	{
		const RectI& rect = layout.rect;
		bool bRows = layout.bRows;

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

			if (!bFree)
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

		if (nGroups > Edgemap::maxSegments)
			return false;

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

				auto pEdgemap = pFactory->createEdgemap(WAXBP(Edgemap, _.colors = colors.data(), _.segments = capacity, _.size = size));
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

} // namespace wax
