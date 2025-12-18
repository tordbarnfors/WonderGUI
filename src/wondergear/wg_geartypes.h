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
#ifndef	WG_GEARTYPES_DOT_H
#define WG_GEARTYPES_DOT_H
#pragma once

#include <cstdint>

namespace wg
{

#if defined(_WIN32)
#	if defined(_M_X64) || defined(_M_IX86)
#		define WG_IS_BIG_ENDIAN 0
#		define WG_IS_LITTLE_ENDIAN 1
#	endif
#else
	#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && defined(__ORDER_LITTLE_ENDIAN__)
	#	 if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	#		define WG_IS_BIG_ENDIAN 1
	#		define WG_IS_LITTLE_ENDIAN 0
	#	elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	#		define WG_IS_BIG_ENDIAN 0
	#		define WG_IS_LITTLE_ENDIAN 1
	#	endif
	#else
	#	ifdef __BIG_ENDIAN__
	#		if __BIG_ENDIAN__
	#			define WG_IS_BIG_ENDIAN 1
	#			define WG_IS_LITTLE_ENDIAN 0
	#		endif
	#	endif

	#	ifdef __LITTLE_ENDIAN__
	#		if __LITTLE_ENDIAN__
	#			define WG_IS_BIG_ENDIAN 0
	#			define WG_IS_LITTLE_ENDIAN 1
	#		endif
	#	endif
	#endif
#endif


	#ifndef WG_IS_BIG_ENDIAN
	#error Could not detect endianness. You need to define WG_IS_BIG_ENDIAN and WG_IS_LITTLE_ENDIAN in wg_types.h
	#define WG_IS_BIG_ENDIAN 0
	#define WG_IS_LITTLE_ENDIAN 0
	#endif


	#if INTPTR_MAX == INT64_MAX
	#		define	WG_IS_64_BITS 1
	#elif INTPTR_MAX == INT32_MAX
	#		define WG_IS_64_BITS 0	
	#else
	#error Could not detect if target is 32 or 64 bits. You need to manually set WG_IS_64_BITS in wg_types.h
	#endif



//-----------------------------------------------------------------------------


template<typename T, typename T2, typename T3> inline void limit(T& x, T2 min, T3 max) { if( x < min) x = min; if( x > max) x = max; }

#define WGBP(T, ...)\
	([&]{ wg::T::Blueprint _{}; __VA_ARGS__; return _; }())

#define WGOVR(T, ...)\
([&]{ auto _ = T; __VA_ARGS__; return _; }())

#define WGCREATE(T, ...)\
	wg::T::create([&]{ wg::T::Blueprint _{}; __VA_ARGS__; return _; }())



#define WGTRACK(pObj) wg::Base::_trackObj(pObj, __FILE__, __LINE__ ) 


	//____ ErrorLevel ________________________________________________

	enum class ErrorLevel		//. autoExtras
	{
		Warning,
		SilentError,
		Error,
		Critical
	};

	//____ ErrorCode ____________________________________________________

	enum class ErrorCode			//. autoExtras
	{
		OutOfRange,
		InvalidIterator,
		InvalidParam,
		FailedPrerequisite,
		OpenGL,						// OpenGL has issued a GLerror
		Internal,
		SystemIntegrity,
		IllegalCall,
		ResourceExhausted,			// A limited internal resource has been exhausted
		RenderFailure,
		Performance,				// Only used with ErrorLevel::Warning for performance warnings.
		Other
	};



	//____ Bitmask ____________________________________________________

	template<typename T> class Bitmask
	{
	public:
	  Bitmask() { m_mask = 0; }
		Bitmask(T v) { m_mask = v; }

		inline void setBit(int index) { m_mask |= (T(1) << index); }
		inline void setBit(int index, bool value) { m_mask &= ~(T(1) << index); m_mask |= (T(value) << index); }
		inline bool bit(int index) const { return ((m_mask & (T(1) << index)) != 0); }
		inline void clearBit(int index) { m_mask &= ~(T(1) << index); }
		inline bool isEmpty() const { return m_mask == 0; }
		inline void flipBit(int index) { m_mask ^= (T(1) << index);  }

		inline Bitmask<T> operator=(const Bitmask<T>& r) { m_mask = r.m_mask; return *this; }
		inline Bitmask<T> operator=(T r) { m_mask = r; return *this; }

		inline Bitmask<T> operator&(const Bitmask<T>& r) { return Bitmask<T>(m_mask & r.m_mask); }
		inline Bitmask<T> operator|(const Bitmask<T>& r) { return Bitmask<T>(m_mask | r.m_mask); }
		inline Bitmask<T> operator^(const Bitmask<T>& r) { return Bitmask<T>(m_mask ^ r.m_mask); }
		inline Bitmask<T> operator~() { return Bitmask<T>(~m_mask); }

		inline Bitmask<T> operator&=(const Bitmask<T>& r) { m_mask &= r.m_mask; return *this; }
		inline Bitmask<T> operator|=(const Bitmask<T>& r) { m_mask |= r.m_mask; return *this; }
		inline Bitmask<T> operator^=(const Bitmask<T>& r) { m_mask ^= r.m_mask; return *this; }


		inline operator T() const { return m_mask; }

		T	 mask() const { return m_mask; }

	private:
		T	m_mask;
	};


	//____ TypeInfo __________________________________________________________

	struct TypeInfo
	{
		const char * className;
		const TypeInfo * pSuperClass;

		inline bool operator==(const TypeInfo& other) const { return (this == &other); }		// Same address is same structure.
		inline bool operator!=(const TypeInfo& other) const { return (this != &other); }		// Same address is same structure.
	};

} // namespace wg

#include <wg_gearenumextras.h>

#endif // WG_GEARTYPES_DOT_H
