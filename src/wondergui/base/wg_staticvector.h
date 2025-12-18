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
#ifndef WG_STATICVECTOR_DOT_H
#define WG_STATICVECTOR_DOT_H
#pragma once

#include <vector>

namespace wg
{

	//____ StaticVector _____________________________________________________
	
	template<class EntryType>
	class StaticVector
	{
	public:
		
		using		iterator = typename std::vector<EntryType>::iterator;
		using		const_iterator = typename std::vector<EntryType>::const_iterator;

		using		reverse_iterator = typename std::vector<EntryType>::reverse_iterator;
		using		const_reverse_iterator = typename std::vector<EntryType>::const_reverse_iterator;


		StaticVector() {}


		//.____ Content _______________________________________________________

		inline int		size() const { return (int) m_entries.size(); }
		inline bool		isEmpty() const { return m_entries.empty(); }

		//.____ Misc _______________________________________________________

		inline iterator	begin() { return m_entries.begin(); }
		inline iterator	end() { return m_entries.end(); }

		inline const_iterator	begin() const { return m_entries.begin(); }
		inline const_iterator	end() const { return m_entries.end(); }

		inline reverse_iterator	rbegin() { return m_entries.rbegin(); }
		inline reverse_iterator	rend() { return m_entries.rend(); }

		inline const_reverse_iterator	rbegin() const { return m_entries.rbegin(); }
		inline const_reverse_iterator	rend() const { return m_entries.rend(); }


		inline EntryType& front() { return m_entries.front(); }
		inline const EntryType& front() const { return m_entries.front(); }

		inline EntryType& back() { return m_entries.back(); }
		inline const EntryType& back() const { return m_entries.back(); }

		//.____ Operators __________________________________________

		inline EntryType& operator[](int index) { return m_entries[index]; }
		inline const EntryType& operator[](int index) const { return m_entries[index]; }

		//.____ Internal ______________________________________________

		inline void _resize(int nb) { m_entries.resize(nb, EntryType()); }

		
	protected:
//		~StaticVector() {}
		
		std::vector<EntryType>	m_entries;
	};

} //namespace

#endif //WG_STATICVECTOR_DOT_H
