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

#include <wg_textstylemanager.h>
#include <wg_base.h>
#include <wg_dummyfont.h>


namespace wg
{
	TextStyle ** TextStyleManager::s_pLookupTable = nullptr;// Pointer at handle->pointer lookup table.
	int			TextStyleManager::s_capacity = 0;			// SizeI in entries for lookup table.
	int			TextStyleManager::s_size = 0;
	int			TextStyleManager::s_nextAvailable = -1;		// Offset in table for next available entry.

	int			TextStyleManager::s_instanceCounter = 0;		// Offset in table for next available entry.

	const TypeInfo	TextStyleManager::TYPEINFO = { "TextStyleManager", nullptr };


	//____ getPointer()_________________________________________________________

	TextStyle_p TextStyleManager::getPointer( TextStyle_h handle )
	{
		if (handle == 0 || handle >= s_capacity)
		{
			Base::throwError(ErrorLevel::Warning, ErrorCode::SystemIntegrity, "TextStyle handle out of bounds. TextStyle does not exist. Returning nullptr.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return nullptr;
		}

		return s_pLookupTable[handle-1];
	}


	//____ init()_______________________________________________________________

	void TextStyleManager::init()
	{
		// If s_pLookupTable is set, we assume we have bailed out on an earlier exit, thus
		// still have a lookup table with entries that are still alive.
        
		if( s_capacity == 0 )			// This is correct, we DO want to check s_capacity instead of s_instanceCounter here.
		{
			int capacity = 16;

			s_pLookupTable = new TextStyle*[capacity];
			s_capacity = capacity;
			s_nextAvailable = 0;

			for( int i = 0 ; i < capacity ; i++ )
				* (int*)(&s_pLookupTable[i]) = i+1;
			

			TextStyle::Blueprint textStyleBP;
			textStyleBP.font = DummyFont::create();

			TextStyle::s_pDefaultStyle = TextStyle::create( textStyleBP );
		}
		
		s_instanceCounter++;
	}

	//____ exit()_______________________________________________________________

	void TextStyleManager::exit()
	{
		if( s_instanceCounter == 0 )
		{
			Base::throwError(ErrorLevel::Error, ErrorCode::SystemIntegrity, "Exiting TextStyleManager more times than it has been initialized.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		s_instanceCounter--;
		
		if( s_instanceCounter == 0 )
		{
			wg::TextStyle::s_pDefaultStyle = nullptr;
			
			if (s_size > 0)
			{
				Base::throwError(ErrorLevel::Warning, ErrorCode::SystemIntegrity, "Exiting TextStyleManager before all TextStyles have been deleted. This can cause crashes later on.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
				return;
			}
			
			//TODO: Something if s_size > 0 (there are still existing TextStyles)
			
			delete [] s_pLookupTable;
			
			s_pLookupTable = nullptr;
			s_capacity = 0;
			s_size = 0;
			s_nextAvailable = -1;
		}
	}

	//____ _reserveHandle()_____________________________________________________

	uint16_t TextStyleManager::_reserveHandle(TextStyle * p)
	{
		if( s_size == s_capacity )
		{
			if (s_capacity == c_maxCapacity)
			{
				Base::throwError(ErrorLevel::Critical, ErrorCode::ResourceExhausted, "Maximum number of TextStyles already created. Can have no more than 65535. Will return 0.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
				return 0;
			}

			int newCapacity = s_capacity * 2 < c_maxCapacity ? s_capacity * 2 : c_maxCapacity;

			TextStyle ** pNewTable = new TextStyle*[newCapacity];

			for( int i = 0 ; i < s_capacity ; i++ )
				pNewTable[i] = s_pLookupTable[i];

			for( int i = s_capacity ; i < newCapacity-1 ; i++ )
				* (int*)(&pNewTable[i]) = i+1;

			* (int*)(&pNewTable[newCapacity-1]) = -1;

			delete [] s_pLookupTable;

			s_pLookupTable = pNewTable;
			s_nextAvailable = s_capacity;
			s_capacity = newCapacity;
		}

		int idx = s_nextAvailable;
		s_nextAvailable = * (int *)(&s_pLookupTable[idx]);
		s_pLookupTable[idx] = p;
		s_size++;

		return static_cast<TextStyle_h>(idx+1);
	}

	//____ _releaseHandle()_____________________________________________________

	void TextStyleManager::_releaseHandle( TextStyle_h handle )
	{
        if( s_pLookupTable == nullptr )
        {
			Base::throwError(ErrorLevel::Warning, ErrorCode::SystemIntegrity, "TextStyle destroyed after TextStyleManager has exited (or possibly before being initialized).", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
            return;
        }

		int idx = handle - 1;

		if (idx < 0 || idx >= s_capacity)
		{
			Base::throwError(ErrorLevel::Warning, ErrorCode::SystemIntegrity, "Attempt to release invalid TextStyle-handle. Something has gone wrong.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
			return;
		}

		* (int*)(&s_pLookupTable[idx]) = s_nextAvailable;
		s_nextAvailable = idx;
		s_size--;
	}


} //namespace wg
