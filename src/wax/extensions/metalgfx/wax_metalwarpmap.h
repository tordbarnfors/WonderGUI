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
#ifndef WAX_METALWARPMAP_DOT_H
#define WAX_METALWARPMAP_DOT_H
#pragma once

#import "MetalKit/MetalKit.h"

#include <wax_warpmap.h>


namespace wax
{

	class MetalWarpmap;
	typedef StrongPtr<MetalWarpmap> MetalWarpmap_p;
	typedef WeakPtr<MetalWarpmap> MetalWarpmap_wp;

	//____ MetalWarpmap ________________________________________________________
	/**
	 * @brief Warpmap for MetalBackend.
	 *
	 * The cells are kept in a buffer for the shaders, two float4 entries per cell, row by row:
	 * the top-left and top-right corners, then the bottom-left and bottom-right corners, as
	 * given by cellCorners(), so already moved by whole periods to be close to the top-left one.
	 */

	class MetalWarpmap : public Warpmap
	{
		friend class MetalBackend;
	public:

		//.____ Creation __________________________________________

		static MetalWarpmap_p	create( const Blueprint& blueprint );
		static MetalWarpmap_p	create( const Blueprint& blueprint, const CoordF * pNodes, int pitch = 0 );

		//.____ Identification __________________________________________

		const TypeInfo&     typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

	protected:

		MetalWarpmap( const Blueprint& bp );
		~MetalWarpmap();

		void	_nodesUpdated( const RectI& region ) override;

		inline SizeI	_cells() const { return { m_nodes.w - 1, m_nodes.h - 1 }; }

		id<MTLBuffer>   m_bufferId = nil;
		float *      	m_pBuffer = nullptr;
	};

} // namespace wax
#endif // WAX_METALWARPMAP_DOT_H
