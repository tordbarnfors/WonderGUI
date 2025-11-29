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
#ifndef	WG_BLOB_DOT_H
#define	WG_BLOB_DOT_H
#pragma once

#include <stddef.h>
#include <functional>

#include <wg_pointers.h>
#include <wg_geartypes.h>

namespace wg
{
	class Blob;
	typedef	StrongPtr<Blob>		Blob_p;
	typedef	WeakPtr<Blob>		Blob_wp;


	/**
	 * @brief Reference counted container of arbitrary data.
	 *
	 * Blob is a container that can be used to wrap a non-WonderGUI
	 * object or any set of data into a reference counted WonderGUI object.
	 *
	 * When the Blob is destroyed, the external object is destroyed or the memory area
	 * released.
	 */

	class Blob : public Object
	{
	public:
		//.____ Creation __________________________________________

		static Blob_p	create( int size );
		static Blob_p	create( void * pData, std::function<void()> destructor );
		static Blob_p	create( void * pData, int size, std::function<void()> destructor );

		//.____ Identification __________________________________________

		const TypeInfo&	typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Content ___________________________________________________________

		inline int			size() const;

		inline const void*	data() const;
		inline void*		data();

		inline const void*	begin() const;
		inline void*		begin();

		inline const void*	end() const;
		inline void*		end();


	protected:
		Blob( int bytes );
		Blob( void * pData, int size, std::function<void()> destructor );
		virtual ~Blob();

		void* operator new (size_t s, int bytes) { return new char[sizeof(Blob) + bytes]; }
		void operator delete(void * p, int bytes)  { delete[] (char*) p; }
		void operator delete(void * p)             { delete[] (char*) p; }

		int		m_size;
		void *	m_pData;

		std::function<void()> m_destructor;
	};


	/** @brief Get the size of the blobs data.
		
		Get the size of the blobs data.

		The size of the content can only be retrieved if known by the blob.
		
		@return Size of blob data or 0 if unknown.
	**/

	int		Blob::size() const 
	{ 
		return m_size; 
	}


	/**
		Get a raw pointer to the data of the blob, which is either the object
		wrapped or beginning of the reserved memory area.

		@return Pointer to data of the blob.
	
	**/

	const void* Blob::data() const 
	{ 
		return m_pData; 
	}

	void* Blob::data() 
	{ 
		return m_pData; 
	}				


	/** @brief Get pointer to the data of the blob.
	
		Get a raw pointer to the beginning of the data of the blob, which is either the object
		wrapped or beginning of the reserved memory area.

		This method behaves identical to data() and is just provided for API completeness.

		@return Pointer to beginning of data of the blob.															
	**/

	const void* Blob::begin() const 
	{ 
		return m_pData; 
	}

	void* Blob::begin()
	{
		return m_pData;
	}


	/** @brief Get pointer to the end of the data of the blob.
	* 
		Get a raw pointer to the end of the data of the blob, which is the address + length of the data.

		If the size of the data is not defined this method will return the address of the data.
	
		@return Pointer to end of data of the blob if size is known, otherwise pointer to beginning of data.
	**/

	const void* Blob::end() const 
	{ 
		return ((char*)m_pData) + m_size; 
	}	
	
	void* Blob::end()
	{
		return ((char*)m_pData) + m_size;
	}




} // namespace wg
#endif //WG_BLOB_DOT_H
