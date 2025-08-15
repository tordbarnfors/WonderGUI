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
#ifndef WG_SIZECAPSULE_DOT_H
#define WG_SIZECAPSULE_DOT_H
#pragma once

#include <wg_capsule.h>

//TODO: Need to support skin padding.

namespace wg
{

	class SizeCapsule;
	typedef	StrongPtr<SizeCapsule>		SizeCapsule_p;
	typedef	WeakPtr<SizeCapsule>		SizeCapsule_wp;

	//____ SizeCapsule ________________________________________________________
	/**
	* @brief	A widget that controls the size of a single child.
	*
	* The SizeCapsule is used to change the size of a widget. This is done either by imposing
	* limits on its minimum and maximum size or by setting a specific default size.
	*
	* Width and height can be controlled separately
	* The SizeCapsule overrides the min, max and default size reported by its child, thus
	* affecting its geometry. Exactly how the geometry is affected depends on the parent widget.
	*
	*/

	class SizeCapsule : public Capsule
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{

			Object_p		baggage;
			Widget_p		child;
			Size			defaultSize = { -1,-1 };
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			Size			maxSize = { -1,-1 };
			Size			minSize = { 0,0 };
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
			bool			usePickHandles = false;

		};

		//.____ Creation __________________________________________

		static SizeCapsule_p	create();
		static SizeCapsule_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________


		bool	setSizes( Size minSize, Size defaultSize, Size maxSize );

		void	setDefaultSize( Size size );
		void	setMinSize( Size size );
		void	setMaxSize( Size size );

		//.____ Internal _________________________________________________

		spx		_matchingHeight(spx width, int scale) const override;
		spx		_matchingWidth(spx height, int scale) const override;

		SizeSPX	_defaultSize(int scale) const override;
		SizeSPX	_minSize(int scale) const override;
		SizeSPX	_maxSize(int scale) const override;


	protected:
		SizeCapsule();
		
		template<class BP> SizeCapsule( const BP& bp ) : Capsule(bp)
		{
			m_minSize		= bp.minSize;
			m_maxSize		= bp.maxSize;
			m_defaultSize	= bp.defaultSize;
		}

		virtual ~SizeCapsule();


	private:

		Size			m_minSize;
		Size			m_maxSize = { -1, -1 };
		Size			m_defaultSize = { -1, -1 };
	};


} // namespace wg
#endif //WG_SIZECAPSULE_DOT_H
