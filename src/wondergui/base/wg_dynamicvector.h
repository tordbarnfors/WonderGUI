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
#ifndef WG_DYNAMICVECTOR_DOT_H
#define WG_DYNAMICVECTOR_DOT_H
#pragma once


#include <wg_staticvector.h>

namespace wg
{

	//____ DynamicVector _____________________________________________________
	
	template<class EntryType>
	class DynamicVector : public StaticVector<EntryType>
	{
	public:

		using iterator = typename StaticVector<EntryType>::iterator;
		using const_iterator = typename StaticVector<EntryType>::const_iterator;

		class Holder	/** @private */
		{
		public:
			virtual void	_didAddEntries(EntryType * pEntry, int nb) = 0;
			virtual void	_didMoveEntries(EntryType * pFrom, EntryType * pTo, int nb) = 0;
			virtual void	_willEraseEntries(EntryType * pEntry, int nb) = 0;
		};
		
		DynamicVector(Holder * pHolder) : StaticVector<EntryType>(), m_pHolder(pHolder) {}

		//.____ Content _______________________________________________________

		inline int		capacity() const { return (int) StaticVector<EntryType>::m_entries.capacity(); }

		inline void		reserve(int amount) { StaticVector<EntryType>::m_entries.reserve( amount ); }
		inline void		shrinkToFit() { StaticVector<EntryType>::m_entries.shrink_to_fit(); }

		void			resize( int nb);

		iterator		pushBack( int nb = 1 );
		iterator		pushBack( int nb, const struct EntryType::Blueprint& entry );
		iterator		pushBack( const struct EntryType::Blueprint& entry );
		iterator		pushBack( const std::initializer_list<struct EntryType::Blueprint>& entries );

		template<typename Iterator>
		iterator		pushBack(Iterator beg, Iterator end);
		
		iterator		insert( int index, int nb = 1 );
		iterator		insert( int index, int nb, const struct EntryType::Blueprint& entry );
		iterator		insert( int index, const struct EntryType::Blueprint& entry );
		iterator		insert( int index, const std::initializer_list<struct EntryType::Blueprint>& entries );

		template<typename Iterator>
		iterator		insert( int index, Iterator beg, Iterator end);

		iterator		insert( const_iterator pos, int nb = 1 );
		iterator		insert( const_iterator pos, int nb, const struct EntryType::Blueprint& entry );
		iterator		insert( const_iterator pos, const struct EntryType::Blueprint& entry );
		iterator		insert( const_iterator pos, const std::initializer_list<struct EntryType::Blueprint>& entries );

		template<typename Iterator>
		iterator		insert( const_iterator pos, Iterator beg, Iterator end);

		
		iterator		erase( int index );
		iterator		erase( int index, int amount );
		iterator		erase( const_iterator pos );
		iterator		erase( const_iterator begin, const_iterator end );
		
		void			clear();

		//.____ Operators __________________________________________

		inline void operator=(const struct EntryType::Blueprint& entry) { clear(); pushBack(entry); }
		inline void operator=(const std::initializer_list<struct EntryType::Blueprint>& entries) { clear(); pushBack(entries); }

		
		inline DynamicVector<EntryType>& operator<<(const struct EntryType::Blueprint& entry) { pushBack(entry); return * this; }
		
	protected:

		Holder* m_pHolder;


		virtual void	_didAddEntries(EntryType* pEntry, int nb) { m_pHolder->_didAddEntries(pEntry, nb); }
		virtual void	_didMoveEntries(EntryType* pFrom, EntryType* pTo, int nb) { m_pHolder->_didMoveEntries(pFrom, pTo, nb); }
		virtual void	_willEraseEntries(EntryType* pEntry, int nb) { m_pHolder->_willEraseEntries(pEntry, nb); }
	};

	//____ resize() __________________________________________________________________

	template < class EntryType>
	void DynamicVector<EntryType>::resize(int nb)
	{
		int oldSize = (int)StaticVector<EntryType>::m_entries.size();

		if( nb < oldSize )
			_willEraseEntries(&StaticVector<EntryType>::m_entries[nb], oldSize-nb);

		StaticVector<EntryType>::m_entries.resize(nb);

		if( nb > oldSize )
			_didAddEntries(&StaticVector<EntryType>::m_entries[oldSize], nb-oldSize);
	}

	//____ pushBack() _________________________________________________________________

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::pushBack(int nb)
	{
		int oldSize = (int)StaticVector<EntryType>::m_entries.size();

		StaticVector<EntryType>::m_entries.resize(oldSize + nb);
		_didAddEntries(&StaticVector<EntryType>::m_entries[oldSize], nb);
		return StaticVector<EntryType>::m_entries.begin() + oldSize;
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::pushBack(int nb, const struct EntryType::Blueprint& entry)
	{
		int oldSize = (int)StaticVector<EntryType>::m_entries.size();

		StaticVector<EntryType>::m_entries.resize(oldSize + nb, entry);
		_didAddEntries(&StaticVector<EntryType>::m_entries[oldSize], nb);
		return StaticVector<EntryType>::m_entries.begin() + oldSize;
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::pushBack(const struct EntryType::Blueprint& entry)
	{
		
		StaticVector<EntryType>::m_entries.emplace_back(entry);
		_didAddEntries(&StaticVector<EntryType>::m_entries.back(), 1);
		return StaticVector<EntryType>::m_entries.end() - 1;
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::pushBack( const std::initializer_list<struct EntryType::Blueprint>& entries)
	{
		int oldSize = (int)StaticVector<EntryType>::m_entries.size();

		StaticVector<EntryType>::m_entries.reserve(StaticVector<EntryType>::m_entries.size() + entries.size());
		
		for (auto& entry : entries)
			StaticVector<EntryType>::m_entries.emplace_back(entry);

		_didAddEntries(&StaticVector<EntryType>::m_entries[oldSize], int(StaticVector<EntryType>::m_entries.size()) - oldSize);
		return StaticVector<EntryType>::m_entries.begin() + oldSize;
	}

	template < class EntryType>
	template< typename Iterator>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::pushBack(Iterator beg, Iterator end)
	{
		static_assert(std::is_convertible<typename std::iterator_traits<Iterator>::iterator_category, std::input_iterator_tag>::value &&
			std::is_convertible<typename std::iterator_traits<Iterator>::value_type, struct EntryType::Blueprint>::value,
			"Begin and end parameters must be iterators or raw pointers to blueprint.");

		int oldSize = StaticVector<EntryType>::m_entries.size();

		StaticVector<EntryType>::m_entries.reserve(StaticVector<EntryType>::m_entries.size() + (end - beg));

		Iterator it = beg;
		while (it != end)
			StaticVector<EntryType>::m_entries.emplace_back(*it++);

		_didAddEntries(&StaticVector<EntryType>::m_entries[oldSize], int(StaticVector<EntryType>::m_entries.size()) - oldSize);
		return StaticVector<EntryType>::m_entries.begin() + oldSize;
	}


	//____ insert() ______________________________________________________________

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::insert(int index, int nb)
	{
		//TODO: Add error checking

		auto it = StaticVector<EntryType>::m_entries.insert(StaticVector<EntryType>::m_entries.begin() + index, nb, EntryType() );
		_didAddEntries(&StaticVector<EntryType>::m_entries[index], nb);
		return it;
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::insert(int index, int nb, const struct EntryType::Blueprint& entry)
	{
		//TODO: Add error checking

		auto it = StaticVector<EntryType>::m_entries.insert(StaticVector<EntryType>::m_entries.begin() + index, nb, EntryType(entry) );
		_didAddEntries(&StaticVector<EntryType>::m_entries[index], nb);
		return it;
	}


	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::insert(int index, const struct EntryType::Blueprint& entry)
	{
		//TODO: Add error checking

		auto it = StaticVector<EntryType>::m_entries.emplace(StaticVector<EntryType>::m_entries.begin() + index, entry);
		_didAddEntries(&StaticVector<EntryType>::m_entries[index], 1);
		return it;
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::insert( int index, const std::initializer_list<struct EntryType::Blueprint>& entries)
	{
		//TODO: Add error checking

		int nEntries = (int) entries.size();
		auto insertPos = StaticVector<EntryType>::m_entries.insert(StaticVector<EntryType>::m_entries.begin() + index, nEntries, EntryType() );

		auto it = insertPos;
		for (auto& entry : entries)
			* it++ = EntryType(entry);

		_didAddEntries(&insertPos[0], nEntries);
		return insertPos;
	}

	template < class EntryType>
	template< typename Iterator>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::insert( int index, Iterator beg, Iterator end)
	{
		//TODO: Add error checking

		static_assert(std::is_convertible<typename std::iterator_traits<Iterator>::iterator_category, std::input_iterator_tag>::value &&
			std::is_convertible<typename std::iterator_traits<Iterator>::value_type, struct EntryType::Blueprint>::value,
			"Begin and end parameters must be iterators or raw pointers to blueprint.");

		int nEntries = (int) (end - beg);

		auto insertPos = StaticVector<EntryType>::m_entries.insert(StaticVector<EntryType>::m_entries.begin() + index, nEntries, EntryType() );

		
		auto dst = insertPos;
		Iterator src = beg;
		while (src != end)
			* dst++ = * src++;

		_didAddEntries(&insertPos[0], nEntries);
		return insertPos;
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::insert(const_iterator pos, int nb)
	{
		//TODO: Add error checking

		auto it = StaticVector<EntryType>::m_entries.insert(pos, nb, EntryType() );
		_didAddEntries(&it[0], nb);
		return it;
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::insert(const_iterator pos, int nb, const struct EntryType::Blueprint& entry)
	{
		//TODO: Add error checking

		auto it = StaticVector<EntryType>::m_entries.insert(pos, nb, EntryType(entry) );
		_didAddEntries(&it[0], nb);
		return it;
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::insert(const_iterator pos, const struct EntryType::Blueprint& entry)
	{
		//TODO: Add error checking

		auto it = StaticVector<EntryType>::m_entries.emplace(pos, entry);
		_didAddEntries(&it[0], 1);
		return it;
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::insert( const_iterator pos, const std::initializer_list<struct EntryType::Blueprint>& entries)
	{
		//TODO: Add error checking

		int nEntries = (int) entries.size();
		auto insertPos = StaticVector<EntryType>::m_entries.insert(pos, nEntries, EntryType() );

		auto it = insertPos;
		for (auto& entry : entries)
			* it++ = EntryType(entry);

		_didAddEntries(&insertPos[0], nEntries);
		return insertPos;
	}

	template < class EntryType>
	template< typename Iterator>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::insert( const_iterator pos, Iterator beg, Iterator end)
	{
		//TODO: Add error checking

		static_assert(std::is_convertible<typename std::iterator_traits<Iterator>::iterator_category, std::input_iterator_tag>::value &&
			std::is_convertible<typename std::iterator_traits<Iterator>::value_type, struct EntryType::Blueprint>::value,
			"Begin and end parameters must be iterators or raw pointers to blueprint.");

		int nEntries = (int) (end - beg);

		auto insertPos = StaticVector<EntryType>::m_entries.insert(pos, nEntries, EntryType() );

		auto dst = insertPos;
		Iterator src = beg;
		while (src != end)
			* dst++ = * src++;

		_didAddEntries(&insertPos[0], nEntries);
		return insertPos;
	}


	//____ erase() ________________________________________________________________

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::erase(int index)
	{
		//TODO: Add error checking

		_willEraseEntries(&StaticVector<EntryType>::m_entries[index], 1);
		return StaticVector<EntryType>::m_entries.erase(StaticVector<EntryType>::m_entries.begin() + index);
	}


	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::erase(int index, int amount)
	{
		//TODO: Add error checking

		_willEraseEntries(&StaticVector<EntryType>::m_entries[index], amount);
		return StaticVector<EntryType>::m_entries.erase(StaticVector<EntryType>::m_entries.begin() + index, StaticVector<EntryType>::m_entries.begin() + index + amount);
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::erase(const_iterator pos)
	{
		//TODO: Add error checking

		_willEraseEntries(pos, 1);
		return StaticVector<EntryType>::m_entries.erase(pos);
	}

	template < class EntryType>
	typename DynamicVector<EntryType>::iterator DynamicVector<EntryType>::erase(const_iterator begin, const_iterator end)
	{
		//TODO: Add error checking

		_willEraseEntries(begin, end - begin);
		return StaticVector<EntryType>::m_entries.erase(begin,end);
	}



	//____ clear() ____________________________________________________________

	template < class EntryType>
	void DynamicVector<EntryType>::clear()
	{
		if( !StaticVector<EntryType>::m_entries.empty() )
		{
			_willEraseEntries(&StaticVector<EntryType>::m_entries[0], (int)StaticVector<EntryType>::m_entries.size());
			StaticVector<EntryType>::m_entries.clear();
		}
	}



} //namespace

#endif //WG_DYNAMICVECTOR_DOT_H
