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
#ifndef WG_DATASET_DOT_H
#define	WG_DATASET_DOT_H
#pragma once

#include <cstdint>
#include <wg_pointers.h>

#include <functional>

namespace wg
{

	//____ BasicDataset _______________________________________________________

	class BasicDataset;
	typedef StrongPtr<BasicDataset>   BasicDataset_p;
	typedef WeakPtr<BasicDataset>   BasicDataset_wp;

	class BasicDataset : public Object
	{
	public:

		//.____ Creation __________________________________________

		static BasicDataset_p	create() { return BasicDataset_p(new BasicDataset()); }

		//.____ Identification _________________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		BasicDataset() {}
		~BasicDataset() {}
	};

	//____ Dataset<> _________________________________________________________________

	template<class Type> class Dataset;

	template<class Type> using Dataset_p = StrongPtr<Dataset<Type>>;
	template<class Type> using Dataset_wp = WeakPtr<Dataset<Type>>;

/**
	@Template class for embedding any class into a WonderGUI object.

	Dataset is a template class that can be used to embedd a class or struct
	into a reference counted WonderGUI object. The main purpose is to allow
	any arbitrary class to be used as baggage for a Widget or Surface, providing
	additional application specific data, but it can be used for anything.

	The embedded class is provided as a public member named "data".
*/

	template<class Type> class Dataset : public BasicDataset
	{
	public:

		//.____ Creation __________________________________________

		static Dataset_p<Type> create() { return Dataset_p<Type>(new Dataset<Type>()); }
		static Dataset_p<Type> create( const Type& load) { auto p = new Dataset<Type>(); p->data = load; return Dataset_p<Type>(p); }

		//.____ Identification _________________________________________________

		const TypeInfo&			typeInfo(void) const override { return TYPEINFO; }
		static TypeInfo	TYPEINFO;

        //.____ Content _______________________________________________________

        Type    data;

	protected:

		static char s_className[64];

	};

	// Generating

	template<class Type> char Dataset<Type>::s_className[64] = { "Dataset type 0x" };
	template<class Type> TypeInfo Dataset<Type>::TYPEINFO = [](char* p)
	{
		uintptr_t x = reinterpret_cast<uintptr_t>(p);

		char * pWrite = p + 15 + 16;
		*pWrite-- = 0;
		for (int i = 0; i < 16; i++)
		{
			char c = char(x & 0xF);
			*pWrite-- = c + (c < 10 ? '0' : 'a' - 10);
			x >>= 4;
		}

		TypeInfo ti = { p, &BasicDataset::TYPEINFO };
		return ti;
	}(s_className);

} // namespace wg
#endif //WG_DATASET_DOT_H
