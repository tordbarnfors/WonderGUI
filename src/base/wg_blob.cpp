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
#include <wg_blob.h>

namespace wg
{

	const char Blob::CLASSNAME[] = {"Blob"};


	//____ Create _________________________________________________________________
	/**
	 * @brief Create an empty blob of the specified size.
	 *
	 * Create an empty blob of the specified size.
	 *
	 * @param size Size in bytes of area to allocate.
	 *
	 * Allocates a memory buffer of the given size and wraps it into a Blob.
	 * The memory buffer will not move and can be accessed randomly at any
	 * time. When the last reference to the Blob is destroyed, the memory
	 * buffer is released.
	 */

	Blob_p Blob::create( int size )
	{
		Blob * pBlob = new(size) Blob( size );
		return Blob_p(pBlob);
	}

	/**
	 * @brief Wrap a non-WonderGUI object or arbritary data into a Blob.
	 *
	 * Wrap a non-WonderGUI object or arbitrary data into a Blob.
	 *
	 * @param pData			Pointer at the data to be considered the content of the Blob.
	 *
	 * @param destructor 	Lambda function that is called to destroy the data
	 * 						when the Blob is destroyed.
	 *
	 * This method creates a Blob that refers to the object or data structure given as its content.
	 * When the Blob is destroyed, the destructor function is called.
	 *
	 * Since the size of the object/data is unknown to the blob, a call to size() will return 0.
	 *
	 */


	Blob_p Blob::create(void * pData, std::function<void()> destructor )
	{
		Blob * pBlob = new(0) Blob( pData, 0, destructor );
		return Blob_p(pBlob);
	}

	/**
	 * @brief Wrap a non-WonderGUI object or arbritary data into a Blob.
	 *
	 * Wrap a non-WonderGUI object or arbitrary data into a Blob.
	 *
	 * @param pData			Pointer at the data to be considered the content of the Blob.
	 *
	 * @param size			Size of the data pointed to by pData.
	 *
	 * @param destructor 	Lambda function that is called to destroy the data
	 * 						when the Blob is destroyed.
	 *
	 * This method creates a Blob that refers to the object or data structure given as its content.
	 * When the Blob is destroyed, the destructor function is called.
	 *
	 * The size parameter is not used internally, just kept and returned when size() is called.
	 *
	 */


	Blob_p Blob::create(void * pData, int size, std::function<void()> destructor )
	{
		Blob * pBlob = new(0) Blob( pData, size, destructor );
		return Blob_p(pBlob);
	}


	//____ Constructor ____________________________________________________________

	Blob::Blob( int size )
	{
		m_size = size;
		m_pData = ((char*)this) + sizeof(Blob);
		m_destructor = nullptr;
	}

	Blob::Blob( void * pData, int size, std::function<void()> destructor )
	{
		m_size = size;
		m_pData = pData;
		m_destructor = destructor;
	}


	//____ Destructor _____________________________________________________________

	Blob::~Blob()
	{
		if( m_destructor != nullptr )
			m_destructor();
	}

	//____ isInstanceOf() _________________________________________________________

	bool Blob::isInstanceOf( const char * pClassName ) const
	{
		if( pClassName==CLASSNAME )
			return true;

		return Object::isInstanceOf(pClassName);
	}

	//____ className() ____________________________________________________________

	const char * Blob::className( void ) const
	{
		return CLASSNAME;
	}

	//____ cast() _________________________________________________________________

	Blob_p Blob::cast( Object * pObject )
	{
		if( pObject && pObject->isInstanceOf(CLASSNAME) )
			return Blob_p( static_cast<Blob*>(pObject) );

		return 0;
	}

} // namespace wg
