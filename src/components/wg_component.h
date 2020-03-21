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
#ifndef WG_COMPONENT_DOT_H
#define WG_COMPONENT_DOT_H
#pragma once


namespace wg
{
	class Component;
	template<class T> class StrongComponentPtr;
	typedef StrongComponentPtr<Component>	Component_p;

	template<class T> class WeakComponentPtr;
	typedef WeakComponentPtr<Component>		Component_wp;

	class Object;
	template<class T> class StrongPtr;
	typedef StrongPtr<Object>				Object_p;

	struct TypeInfo;

	/**
	 * @brief Provides access to a component of a widget or object.
	 *
	 * Interfaces are provided by Widgets and other reference counted Objects
	 * to provide API access to their embedded components such as labels and
	 * icons.
	 *
	 * The interface concept of WonderGUI serves two purposes:
	 *
	 * First it
	 * provides a nice API-level abstraction to keep methods for accessing
	 * different components of a widget logically separated while providing
	 * an identical way to access same type of component in all widgets that
	 * contains them.
	 *
	 * Secondly it provides a safe way to pass around pointers to the embedded
	 * components since interface pointers do reference counting on the object.
	 *
	 **/

	class Component
	{
		template<class T> friend class StrongComponentPtr;
		template<class T> friend class WeakComponentPtr;
	public:

		//.____ Identification _________________________________________________

		virtual const TypeInfo& typeInfo(void) const;
		const static TypeInfo	TYPEINFO;

		bool		isInstanceOf(const TypeInfo& typeInfo) const;

		//.____ Misc __________________________________________________

		Component_p				ptr();
		Object_p				object();

	protected:
		virtual Object * 			_object() = 0;
		virtual const Object * 		_object() const = 0;
	};



} // namespace wg
#endif //WG_COMPONENT_DOT_H
