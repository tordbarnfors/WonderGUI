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
#ifndef WAX_SOFTWARPMAP_DOT_H
#define	WAX_SOFTWARPMAP_DOT_H
#pragma once

#include <wax_warpmap.h>

namespace wax
{
	class SoftWarpmap;
	typedef	StrongPtr<SoftWarpmap>	SoftWarpmap_p;
	typedef	WeakPtr<SoftWarpmap>	SoftWarpmap_wp;

	//____ SoftWarpmap _________________________________________________________
	/**
	 * @brief Warpmap for SoftBackend, LinearBackend and ReferenceBackend.
	 *
	 * The software backends read the nodes directly, so there is nothing more to it than
	 * the Warpmap itself.
	 */

	class SoftWarpmap : public Warpmap
	{
	public:

		//.____ Creation __________________________________________

		static SoftWarpmap_p	create( const Blueprint& blueprint );
		static SoftWarpmap_p	create( const Blueprint& blueprint, const CoordF * pNodes, int pitch = 0 );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		SoftWarpmap( const Blueprint& bp ) : Warpmap(bp) {}
		virtual ~SoftWarpmap() {}

		void	_nodesUpdated( const RectI& region ) override {}
	};

} // namespace wax
#endif //WAX_SOFTWARPMAP_DOT_H
