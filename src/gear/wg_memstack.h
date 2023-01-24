/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/
#ifndef WG_MEMSTACK_DOT_H
#define WG_MEMSTACK_DOT_H
#pragma once

#include <wg_chain.h>

//#include <vector>

namespace wg
{

	class MemStack
	{
	public:
		//.____ Creation __________________________________________

		MemStack( int blockSize );
		~MemStack() {}

		//.____ Misc _______________________________________________________

		char *  allocBytes( int bytes );
		void	releaseBytes( int bytes );
		void	clear();
		bool	isEmpty() const;

	private:

		class BlockHeader : public Link
		{
		public:
			BlockHeader(int bytes);
			~BlockHeader();

			char *	pBlock;
			int		size;
			int		allocated;
		};

		int					m_blockSize;
		Chain<BlockHeader>	m_blocks;
		
//		std::vector<int>	m_allocSizes;	// Debug code, should be disabled by default.
		
	};


} // namespace wg
#endif //WG_MEMSTACK_DOT_H
