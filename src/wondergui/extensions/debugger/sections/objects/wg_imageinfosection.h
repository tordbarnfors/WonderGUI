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
#ifndef	WG_IMAGEINFOSECTION_DOT_H
#define WG_IMAGEINFOSECTION_DOT_H
#pragma once

#include <wg_typedinfosection.h>
#include <wg_image.h>

namespace wg
{
	class ImageInfoSection;
	typedef	StrongPtr<ImageInfoSection>	ImageInfoSection_p;
	typedef	WeakPtr<ImageInfoSection>	ImageInfoSection_wp;



	class ImageInfoSection : public TypedInfoSection<Image>
	{
	public:

		//.____ Creation __________________________________________

		static ImageInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Image * pInspected) { return ImageInfoSection_p(new ImageInfoSection(theme, pContext, pInspected) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;


	protected:
		ImageInfoSection(const DebugTheme& theme, IDebugContext* pContext, Image * pInspected );
		~ImageInfoSection() {}

		DrawerPanel_p	m_pImageRectDrawer;
		DrawerPanel_p	m_pImageTintDrawer;

		Rect			m_displayedImageRect;
		HiColor			m_displayedImageTint;
	};

} // namespace wg
#endif //WG_IMAGEINFOSECTION_DOT_H
