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

#include <wg_edgemap.h>
#include <cstring>

namespace wg
{

	const TypeInfo Edgemap::TYPEINFO = { "Edgemap", &Object::TYPEINFO };


	//____ typeInfo() _________________________________________________________

	const TypeInfo& Edgemap::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setRenderSegments() ________________________________________________

	bool Edgemap::setRenderSegments(int segments)
	{
		if( segments < 1 || segments > m_nbSegments )
			return false;

		m_nbRenderSegments = segments;
		return true;
	}

	//____ setColors() ____________________________________________________________

	bool Edgemap::setColors( int begin, int end, const HiColor * pColors )
	{
		if( !m_pColors )
		{
			//TODO: Error handling!
			return false;
		}

		if( begin < 0 || end < begin || end > m_nbSegments )
		{
			//TODO: Error handling!

			return false;
		}

		memcpy( m_pColors + begin, pColors, (end-begin)*sizeof(HiColor) );
		return true;
	}

	//____ setGradients() _________________________________________________________

	bool Edgemap::setGradients( int begin, int end, const Gradient * pGradients )
	{
		if( !m_pGradients )
		{
			//TODO: Error handling!
			return false;
		}

		if( begin < 0 || end < begin || end > m_nbSegments )
		{
			//TODO: Error handling!

			return false;
		}

		memcpy( m_pGradients + begin, pGradients, (end-begin)*sizeof(Gradient) );
		return true;
	}

	//____ setTintmaps() _________________________________________________________

	bool Edgemap::setTintmaps( int begin, int end, const Tintmap_p * pTintmaps )
	{
		if( !m_pTintmaps )
		{
			//TODO: Error handling!
			return false;
		}

		if( begin < 0 || end < begin || end > m_nbSegments )
		{
			//TODO: Error handling!

			return false;
		}
		
		for( int i = begin ; i < end ; i++ )
		{
			m_pTintmaps[i]->release();
			m_pTintmaps[i] = * pTintmaps++;
			m_pTintmaps[i]->retain();
		}
			
		return true;
	}




	//____ colors() ______________________________________________________________

	const HiColor * Edgemap::colors() const
	{
		return m_pColors;
	}

	//____ gradients() ___________________________________________________________

	const Gradient * Edgemap::gradients() const
	{
		return m_pGradients;
	}

	//____ tintmaps() ___________________________________________________________

	Tintmap * const * Edgemap::tintmaps() const
	{
		return m_pTintmaps;
	}


	//____ color() ____________________________________________________________

	HiColor Edgemap::color(int segment) const
	{
		if( segment < 0 || segment >= m_nbSegments || m_pColors == nullptr )
			return HiColor::Undefined;

		return m_pColors[segment];
	}

	//____ gradient() ____________________________________________________________

	Gradient Edgemap::gradient(int segment) const
	{
		if( segment < 0 || segment >= m_nbSegments || m_pGradients == nullptr )
			return Gradient();

		return m_pGradients[segment];
	}

	//____ tintmap() ____________________________________________________________

	Tintmap_p Edgemap::tintmap(int segment) const
	{
		if( segment < 0 || segment >= m_nbSegments || m_pTintmaps == nullptr )
			return nullptr;

		return m_pTintmaps[segment];
	}

	//____ _validateBlueprint() __________________________________________________

	bool Edgemap::_validateBlueprint(const Blueprint& bp)
	{
		//TODO: Throw errors, not just return false.

		if( bp.size.w <= 0 || bp.size.h <= 0 )
			return false;

		if( bp.segments <= 0 || bp.segments > maxSegments )
			return false;

		if( int(bp.colors != nullptr) + int(bp.gradients != nullptr) + int(bp.tintmaps != nullptr) != 1 )
			return false;

		return true;
	}


} // namespace wg
