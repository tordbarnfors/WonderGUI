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
#ifndef WG_SCALECAPSULE_DOT_H
#define WG_SCALECAPSULE_DOT_H
#pragma once

#include <wg_capsule.h>

//TODO: Need to support skin padding.

namespace wg
{

	class ScaleCapsule;
	typedef	StrongPtr<ScaleCapsule>		ScaleCapsule_p;
	typedef	WeakPtr<ScaleCapsule>		ScaleCapsule_wp;

	//____ ScaleCapsule ________________________________________________________
	/**
	* @brief	A widget that controls the scale of size of a single child.
	*
	* The ScaleCapsule is used to change the size of a widget. This is done either by imposing
	* limits on its minimum and maximum size or by setting a specific default size.
	*
	* Width and height can be controlled separately
	* The ScaleCapsule overrides the min, max and default size reported by its child, thus
	* affecting its geometry. Exactly how the geometry is affected depends on the parent widget.
	*
	*/

	class ScaleCapsule : public Capsule
	{
	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{

			Object_p		baggage;
			Widget_p		child;
			bool			disabled = false;
			bool			dropTarget = false;
			Finalizer_p		finalizer = nullptr;
			int				id = 0;
			MarkPolicy		markPolicy = MarkPolicy::AlphaTest;
			bool			pickable = false;
			uint8_t			pickCategory = 0;
			bool			pickHandle = false;
			PointerStyle	pointer = PointerStyle::Undefined;
			int				scale = 0;
			bool			selectable = false;
			Skin_p			skin;
			bool			stickyFocus = false;
			bool			tabLock = false;
			String			tooltip;
			bool			usePickHandles = false;

		};

		//.____ Creation __________________________________________

		static ScaleCapsule_p	create();
		static ScaleCapsule_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry _________________________________________________

		bool				setScale(int scale);
		inline void			clearScale() { setScale(0); }
		inline bool			isScaleSet() const { return m_bScaleSet; }	

		//.____ Internal _________________________________________________

		spx		_matchingHeight(spx width, int scale) const override;
		spx		_matchingWidth(spx height, int scale) const override;

		SizeSPX	_defaultSize(int scale) const override;
		SizeSPX	_minSize(int scale) const override;
		SizeSPX	_maxSize(int scale) const override;


	protected:
		ScaleCapsule();
		
		template<class BP> ScaleCapsule( const BP& bp ) : Capsule(bp)
		{
			if (bp.scale != 0)
			{
				m_scale = bp.scale;
				m_bScaleSet = true;
			}

			if (bp.child)
				slot.setWidget(bp.child);
		}
		
		virtual ~ScaleCapsule();

		void		_resize(const SizeSPX& size, int scale) override;


	private:

		bool			m_bScaleSet = false;
		uint16_t		m_scaleFromParent = 64;
	
	};


} // namespace wg
#endif //WG_SCALECAPSULE_DOT_H
