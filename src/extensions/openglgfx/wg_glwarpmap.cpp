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

		auto p = GlWarpmap_p(new GlWarpmap(bp));
		SizeI nodes = p->nodes();
		p->importNodes({ 0,0,nodes.w,nodes.h }, pNodes, pitch);
		return p;
	}

	//____ constructor ________________________________________________________

	GlWarpmap::GlWarpmap( const Blueprint& bp ) : Warpmap(bp)
	{
		SizeI cells = _cells();

		glGenBuffers(1, &m_bufferId);
		glBindBuffer(GL_TEXTURE_BUFFER, m_bufferId);
		glBufferData(GL_TEXTURE_BUFFER, cells.w * cells.h * 2 * 4 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

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
	// Rebuilds and uploads the rows of cells touching the nodes that changed.

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
		glBindBuffer(GL_TEXTURE_BUFFER, 0);

		GfxBase::memStackFree(bytes);
	}

} // namespace wg
