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
#include <wg_glwarpmap.h>
#include <wg_gfxbase.h>

#include <algorithm>
#include <cfloat>

namespace wg
{
	const TypeInfo GlWarpmap::TYPEINFO = { "GlWarpmap", &Warpmap::TYPEINFO };

	//____ create() ___________________________________________________________

	GlWarpmap_p GlWarpmap::create( const Blueprint& bp )
	{
		if (!_validateBlueprint(bp))
			return nullptr;

		return GlWarpmap_p(new GlWarpmap(bp));
	}

	GlWarpmap_p GlWarpmap::create( const Blueprint& bp, const CoordSPX * pNodes, int pitch )
	{
		if (!_validateBlueprint(bp) || !pNodes)
			return nullptr;

		return GlWarpmap_p(new GlWarpmap(bp, pNodes, pitch));
	}

	//____ constructor ________________________________________________________

	GlWarpmap::GlWarpmap( const Blueprint& bp, const CoordSPX * pNodes, int pitch ) : Warpmap(bp, pNodes, pitch)
	{
		SizeI cells = _cells();
		SizeI blocks = _blocks();

		glGenBuffers(1, &m_bufferId);
		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);
		glBufferData(GL_TEXTURE_BUFFER, (cells.w * cells.h + blocks.w * blocks.h) * 2 * 4 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_BUFFER, m_textureId);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_bufferId);
		glBindTexture(GL_TEXTURE_BUFFER, 0);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		_nodesUpdated({ 0, 0, m_nodes.w, m_nodes.h });
	}

	//____ destructor _________________________________________________________

	GlWarpmap::~GlWarpmap()
	{
		glDeleteTextures(1, &m_textureId);
		glDeleteBuffers(1, &m_bufferId);
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& GlWarpmap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ _nodesUpdated() ____________________________________________________
	// Rebuilds and uploads the rows of cells touching the nodes that changed, and their blocks.

	void GlWarpmap::_nodesUpdated( const RectI& region )
	{
		SizeI cells = _cells();

		int rowBeg = std::max(0, region.y - 1);
		int rowEnd = std::min(cells.h, region.y + region.h);

		if (rowBeg >= rowEnd)
			return;

		int bytes = (rowEnd - rowBeg) * cells.w * 2 * 4 * sizeof(GLfloat);
		auto pBuffer = (GLfloat*) GfxBase::memStackAlloc(bytes);
		GLfloat* pOut = pBuffer;

		for (int row = rowBeg; row < rowEnd; row++)
		{
			for (int column = 0; column < cells.w; column++)
			{
				CoordSPX corners[4];
				cellCorners(column, row, corners);

				for (int i = 0; i < 4; i++)
				{
					*pOut++ = corners[i].x / 64.f;
					*pOut++ = corners[i].y / 64.f;
				}
			}
		}

		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);
		glBufferSubData(GL_TEXTURE_BUFFER, rowBeg * cells.w * 2 * 4 * sizeof(GLfloat), bytes, pBuffer);

		GfxBase::memStackFree(bytes);

		// Blocks of the cells that changed, from the corners as uploaded.

		int blockCells = _blockCells(m_cellSize);
		SizeI blocks = _blocks();

		int blockRowBeg = rowBeg / blockCells;
		int blockRowEnd = (rowEnd - 1) / blockCells + 1;

		int blockBytes = (blockRowEnd - blockRowBeg) * blocks.w * 2 * 4 * sizeof(GLfloat);
		pBuffer = (GLfloat*) GfxBase::memStackAlloc(blockBytes);
		pOut = pBuffer;

		for (int blockRow = blockRowBeg; blockRow < blockRowEnd; blockRow++)
		{
			for (int blockColumn = 0; blockColumn < blocks.w; blockColumn++)
			{
				float lo[2] = { FLT_MAX, FLT_MAX }, hi[2] = { -FLT_MAX, -FLT_MAX }, reach[2] = { 0.f, 0.f };

				int rowEndInBlock = std::min((blockRow + 1) * blockCells, cells.h);
				int columnEndInBlock = std::min((blockColumn + 1) * blockCells, cells.w);

				for (int row = blockRow * blockCells; row < rowEndInBlock; row++)
				{
					float h = float(nodeY(row + 1) - nodeY(row));

					for (int column = blockColumn * blockCells; column < columnEndInBlock; column++)
					{
						float w = float(nodeX(column + 1) - nodeX(column));

						CoordSPX corners[4];
						cellCorners(column, row, corners);

						float cellLo[2] = { FLT_MAX, FLT_MAX }, cellHi[2] = { -FLT_MAX, -FLT_MAX };
						for (int i = 0; i < 4; i++)
						{
							float v[2] = { corners[i].x / 64.f, corners[i].y / 64.f };
							for (int a = 0; a < 2; a++)
							{
								cellLo[a] = std::min(cellLo[a], v[a]);
								cellHi[a] = std::max(cellHi[a], v[a]);
							}
						}

						for (int a = 0; a < 2; a++)
						{
							lo[a] = std::min(lo[a], cellLo[a]);
							hi[a] = std::max(hi[a], cellHi[a]);
							reach[a] = std::max(reach[a], 0.5f * (cellHi[a] - cellLo[a]) * (1.f / w + 1.f / h));
						}
					}
				}

				*pOut++ = lo[0];
				*pOut++ = lo[1];
				*pOut++ = hi[0];
				*pOut++ = hi[1];
				*pOut++ = reach[0];
				*pOut++ = reach[1];
				*pOut++ = 0.f;
				*pOut++ = 0.f;
			}
		}

		glBufferSubData(GL_TEXTURE_BUFFER, (cells.w * cells.h + blockRowBeg * blocks.w) * 2 * 4 * sizeof(GLfloat), blockBytes, pBuffer);
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		GfxBase::memStackFree(blockBytes);
	}

	//____ _blocks() __________________________________________________________

	SizeI GlWarpmap::_blocks() const
	{
		SizeI cells = _cells();
		int blockCells = _blockCells(m_cellSize);
		return { (cells.w + blockCells - 1) / blockCells, (cells.h + blockCells - 1) / blockCells };
	}

} // namespace wg
