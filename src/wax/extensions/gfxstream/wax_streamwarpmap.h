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
#ifndef WG_STREAMWARPMAP_DOT_H
#define WG_STREAMWARPMAP_DOT_H
#pragma once

#include <wg_warpmap.h>
#include <wg_streamencoder.h>

namespace wg
{
	class StreamWarpmap;
	typedef StrongPtr<StreamWarpmap> StreamWarpmap_p;
	typedef WeakPtr<StreamWarpmap> StreamWarpmap_wp;

	//____ StreamWarpmap _______________________________________________________
	/**
	 * @brief Warpmap that is mirrored in a stream.
	 *
	 * Creation, nodes, periods and deletion are sent in CreateWarpmap, WarpmapNodes,
	 * SetWarpmapPeriods and DeleteWarpmap chunks.
	 */

	class StreamWarpmap : public Warpmap
	{
	public:

		//.____ Creation __________________________________________

		static StreamWarpmap_p	create( StreamEncoder * pEncoder, const Blueprint& blueprint );
		static StreamWarpmap_p	create( StreamEncoder * pEncoder, const Blueprint& blueprint, const CoordF * pNodes, int pitch = 0 );

		//.____ Identification __________________________________________

		const TypeInfo&     typeInfo(void) const override;
		const static TypeInfo   TYPEINFO;

		uint16_t		inStreamId() const { return m_inStreamId; }

		//.____ Content _______________________________________________________

		bool	setPeriods( float uPeriod, float vPeriod ) override;

		//.____ Internal ______________________________________________________

		static constexpr int	c_maxNodesPerChunk = 128;

	protected:
		StreamWarpmap( StreamEncoder * pEncoder, const Blueprint& bp );
		~StreamWarpmap();

		void	_nodesUpdated( const RectI& region ) override;

		StreamEncoder_p		m_pEncoder;
		uint16_t			m_inStreamId;		// Id of this warpmap in the stream.
	};

} // namespace wg
#endif // WG_STREAMWARPMAP_DOT_H
