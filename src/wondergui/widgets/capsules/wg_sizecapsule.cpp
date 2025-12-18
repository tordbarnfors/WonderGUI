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
#include <wg_sizecapsule.h>

#include <algorithm>

namespace wg
{
   using namespace Util;

   const TypeInfo SizeCapsule::TYPEINFO = { "SizeCapsule", &Capsule::TYPEINFO };

   //____ create() _______________________________________________________________

   SizeCapsule_p SizeCapsule::create()
   {
	   return SizeCapsule_p(new SizeCapsule());
   }

   SizeCapsule_p SizeCapsule::create(const Blueprint& blueprint)
   {
	   return SizeCapsule_p(new SizeCapsule(blueprint));
   }


   //____ constructor ____________________________________________________________

   SizeCapsule::SizeCapsule()
   {
   }


   //____ destructor _____________________________________________________________

   SizeCapsule::~SizeCapsule()
   {
   }

   //____ typeInfo() _________________________________________________________

   const TypeInfo& SizeCapsule::typeInfo(void) const
   {
	   return TYPEINFO;
   }

   //____ setDefaultSize() _____________________________________________________

   void SizeCapsule::setDefaultSize( Size size )
   {
	   if( size != m_defaultSize )
	   {
		   m_defaultSize = size;
		   _requestResize();
	   }
   }

   //____ setSizes() _____________________________________________________________

   bool SizeCapsule::setSizes( Size minSize, Size defaultSize, Size maxSize )
   {
	   if ( (defaultSize.w >= 0 && (defaultSize.w > maxSize.w || defaultSize.w < minSize.w )) ||
			(defaultSize.h >= 0 && (defaultSize.h > maxSize.h || defaultSize.h < minSize.h )) )
		   return false;

	   if (minSize != m_minSize || defaultSize != m_defaultSize || maxSize != m_maxSize)
	   {
		   m_minSize = minSize;
		   m_defaultSize = defaultSize;
		   m_maxSize = maxSize;
		   _requestResize();
	   }
	   return true;
   }

   //____ setMinSize() ___________________________________________________________

   void SizeCapsule::setMinSize( Size size )
   {
	   //TODO: Add error handling.

	   if( size != m_minSize )
	   {
		   m_minSize = size;
		   _requestResize();
	   }
   }

   //____ setMaxSize() ___________________________________________________________

   void SizeCapsule::setMaxSize( Size size )
   {
	   //TODO: Add error handling.

	   if( size != m_maxSize )
	   {
		   m_maxSize = size;
		   _requestResize();
	   }
   }

   //____ _defaultSize() ________________________________________________________

   SizeSPX SizeCapsule::_defaultSize(int scale) const
   {
	   Widget * pChild = slot._widget();
	   
	   // Special handling of m_maxSize to allow -1 to equal MAX.
	  
	   SizeSPX maxSize;
	   maxSize.w = m_maxSize.w < 0 ? 0x3FFFFFC0 : align(ptsToSpx(m_maxSize.w, scale));
	   maxSize.h = m_maxSize.h < 0 ? 0x3FFFFFC0 : align(ptsToSpx(m_maxSize.h, scale));

	   //
	   
	   SizeSPX defaultSize	= align(ptsToSpx(m_defaultSize, scale));
	   SizeSPX minSize		= align(ptsToSpx(m_minSize, scale));

	   if (!pChild)
	   {
		   SizeSPX size = { std::max(0, defaultSize.w), std::max(0, defaultSize.h) };
		   size += m_skin.contentBorderSize(scale);

		   return size;
	   }

	   //

	   SizeSPX pref;

	   if (defaultSize.w >= 0)
	   {
		   // Default width is forced, we only need to adapt height.

		   pref.w = defaultSize.w;
		   if (defaultSize.h >= 0)
			   pref.h = defaultSize.h;
		   else
		   {
			   pref.h = pChild->_matchingHeight(pref.w, scale);
		   }
	   }
	   else if (defaultSize.h >= 0)
	   {
		   // Default height is forced, we only need to adapt width.

		   pref.h = defaultSize.h;
		   pref.w = pChild->_matchingWidth(pref.h, scale);
	   }
	   else
	   {
		   // Default size not set in size capsule.
		   // We take default from child and check against our min/max.

		   pref = pChild->_defaultSize(scale);

		   if (pref.w > maxSize.w && pref.h > maxSize.h)
		   {
			   // Both width and height surpasses our max, get matching values for both and limit them to our min.

			   spx matchW = pChild->_matchingWidth(maxSize.h,scale);
			   spx matchH = pChild->_matchingHeight(maxSize.w,scale);

			   //

			   if (matchW > maxSize.w && matchH > maxSize.h)
				   pref = maxSize;							// Both matching values are too big, so we will max in both dimensions.
			   else if (matchW <= maxSize.w && matchH <= maxSize.h)
			   {
				   // Both matching values are below max, so we can go either way here.
				   // Lets choose the one that gives the largest area.

				   if (matchW*maxSize.h > matchH*maxSize.w)
				   {
					   pref.w = matchW;
					   pref.h = maxSize.h;
				   }
				   else
				   {
					   pref.w = maxSize.w;
					   pref.h = matchH;
				   }
			   }
			   else
			   {
				   // If we get here, only one matching value is small enough.

				   if (matchW <= maxSize.w)
				   {
					   pref.w = matchW;
					   pref.h = maxSize.h;
				   }
				   else
				   {
					   pref.w = maxSize.w;
					   pref.h = matchH;
				   }
			   }
		   }
		   else if (pref.w > maxSize.w)
		   {
			   // Only width surpasses our max.

			   pref.w = maxSize.w;
			   pref.h = pChild->_matchingHeight(pref.w,scale);
		   }
		   else if (pref.h > maxSize.h)
		   {
			   // Only height surpasses our max

			   pref.h = maxSize.h;
			   pref.w = pChild->_matchingWidth(pref.h,scale);
		   }
		   else
		   {
			   // Neither dimension surpasses our max, let's check against our min values.

			   if (pref.w < minSize.w)
			   {
				   pref.w = minSize.w;
				   pref.h = pChild->_matchingHeight(pref.w,scale);
			   }
			   else if (pref.h < minSize.h)
			   {
				   pref.h = minSize.h;
				   pref.w = pChild->_matchingWidth(pref.h,scale);
			   }
		   }
	   }

	   limit(pref.w, minSize.w, maxSize.w);
	   limit(pref.h, minSize.h, maxSize.h);

	   pref += m_skin.contentBorderSize(scale);
	   return pref;
   }

   //____ _minSize() ______________________________________________________________

   SizeSPX SizeCapsule::_minSize(int scale) const
   {
	   SizeSPX size;

	   if( slot._widget() )
		   size = SizeSPX::max(align(ptsToSpx(m_minSize,scale)),slot._widget()->_minSize(scale));
	   else
		   size = align(ptsToSpx(m_minSize, scale));
	   
	   return size + m_skin.contentBorderSize(scale);
   }

   //____ _maxSize() ______________________________________________________________

   SizeSPX SizeCapsule::_maxSize(int scale) const
   {
	   SizeSPX size;
	   size.w = m_maxSize.w < 0 ? 0x3FFFFFC0 : align(ptsToSpx(m_maxSize.w, scale));
	   size.h = m_maxSize.h < 0 ? 0x3FFFFFC0 : align(ptsToSpx(m_maxSize.h, scale));

	   if( slot._widget() )
		   size = SizeSPX::min(size,slot._widget()->_maxSize(scale));

	   return size + m_skin.contentBorderSize(scale);
   }

   //____ _matchingHeight() _______________________________________________________

   spx SizeCapsule::_matchingHeight(spx width, int scale) const
   {
	   SizeSPX contentBorder = m_skin.contentBorderSize(scale);
	   
	   width -= contentBorder.w;
	   
	   spx result;
	   if (m_defaultSize.h >= 0)
	   {
		   spx h = ptsToSpx(m_defaultSize.h, scale);

		   if (slot._widget())
		   {
			   spx max = slot._widget()->_maxSize(scale).h;
			   spx min = slot._widget()->_minSize(scale).h;
			   limit(h, min, max);
		   }
		   result = align(h);
	   }
	   else if (slot._widget())
	   {
		   spx max = m_maxSize.h < 0 ? 0x3FFFFFC0 : align(ptsToSpx(m_maxSize.h, scale));

		   spx h = slot._widget()->_matchingHeight(width, scale);
		   limit(h, ptsToSpx(m_minSize.h,scale), max);
		   result = align(h);
	   }
	   else
		   result = align(ptsToSpx(m_minSize.h, scale));
	   
	   return result + contentBorder.h;
   }

   //____ _matchingWidth() _______________________________________________________

   spx SizeCapsule::_matchingWidth( spx height, int scale ) const
   {
	   SizeSPX contentBorder = m_skin.contentBorderSize(scale);
	   
	   height -= contentBorder.h;

	   spx result;
	   if( m_defaultSize.w >= 0 )
	   {
		   spx w = ptsToSpx(m_defaultSize.w, scale);

		   if( slot._widget() )
		   {
			   spx max = slot._widget()->_maxSize(scale).w;
			   spx min = slot._widget()->_minSize(scale).w;
			   limit( w, min, max );
		   }
		   result = align(w);
	   }
	   else if( slot._widget() )
	   {
		   spx max = m_maxSize.w < 0 ? 0x3FFFFFC0 : align(ptsToSpx(m_maxSize.w, scale));

		   spx w = slot._widget()->_matchingWidth(height,scale);
		   limit( w, ptsToSpx(m_minSize.w,scale), max );
		   result = align(w);
	   }
	   else
		   result = align(ptsToSpx(m_minSize.w, scale));

	   return result + contentBorder.w;
   }


} // namespace wg
