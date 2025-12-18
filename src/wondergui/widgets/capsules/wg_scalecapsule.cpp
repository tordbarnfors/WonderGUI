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
#include <wg_scalecapsule.h>

#include <algorithm>

namespace wg
{
	using namespace Util;

	const TypeInfo ScaleCapsule::TYPEINFO = { "ScaleCapsule", &Capsule::TYPEINFO };


	//____ create() _______________________________________________________________

	ScaleCapsule_p ScaleCapsule::create() 
	{ 
		return ScaleCapsule_p(new ScaleCapsule()); 
	}

	ScaleCapsule_p ScaleCapsule::create(const Blueprint& blueprint)
	{ 
		return ScaleCapsule_p(new ScaleCapsule(blueprint));
	}


	//____ constructor ____________________________________________________________

	ScaleCapsule::ScaleCapsule()
	{
	}

	//____ Destructor _____________________________________________________________

	ScaleCapsule::~ScaleCapsule()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ScaleCapsule::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setScale() ___________________________________________________________
	/**
	* @brief Explicitly set scale.
	*
	* Explicitly set the scale of this widget and indirectly of its children.
	*
	* @param scale Scale to be set or 0 to clear, returning widget to the
	*              scale determined by parent.
	*
	* Scale defines the size of a point, measured in 1/64ths of a pixel. Thus
	* is 64 the default size where one point equals one pixel and 128 is a
	* 200% zoom factor.
	*
	* Setting the scale on a ScaleCapsule or RootWidget will propagate the scale
	* to all children that doesn't have an explicitly set scale.
	*
	*  @return True if scale was accepted.
	*/

	bool ScaleCapsule::setScale(int scale)
	{
		if (scale < 0 || scale > 64 * 64)
			return false;

		if (scale > 0)
			m_bScaleSet = true;
		else
		{
			scale = m_pHolder ? m_pHolder->_scale() : m_scale;
			m_bScaleSet = false;
		}

		if (scale != m_scale)
		{
			m_scale = scale;
			_requestResize();
		}

		return true;
	}

	//____ _matchingHeight() ___________________________________________________________

	spx ScaleCapsule::_matchingHeight(spx width, int scale) const
	{
		if (m_bScaleSet)
			scale = m_scale;

		return Capsule::_matchingHeight(width, scale);
	}

	//____ _matchingWidth() ___________________________________________________________

	spx ScaleCapsule::_matchingWidth(spx height, int scale) const
	{
		if (m_bScaleSet)
			scale = m_scale;

		return Capsule::_matchingWidth(height, scale);
	}

	//____ _defaultSize() ___________________________________________________________

	SizeSPX ScaleCapsule::_defaultSize(int scale) const
	{
		if (m_bScaleSet)
			scale = m_scale;

		return Capsule::_defaultSize(scale);
	}

	//____ _minSize() ___________________________________________________________

	SizeSPX ScaleCapsule::_minSize(int scale) const
	{
		if (m_bScaleSet)
			scale = m_scale;

		return Capsule::_minSize(scale);
	}

	//____ _maxSize() ___________________________________________________________

	SizeSPX ScaleCapsule::_maxSize(int scale) const
	{
		if (m_bScaleSet)
			scale = m_scale;

		return Capsule::_maxSize(scale);
	}


	//____ _resize() __________________________________________________________

	void ScaleCapsule::_resize(const SizeSPX& size, int scale)
	{
		if (m_bScaleSet)
			scale = m_scale;

		return Capsule::_resize(size, scale);
	}


} // namespace wg
