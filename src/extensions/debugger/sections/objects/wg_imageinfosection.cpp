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
#include "wg_imageinfosection.h"
#include <wg_packpanel.h>


namespace wg
{

	const TypeInfo ImageInfoSection::TYPEINFO = { "ImageInfoSection", &InfoSection::TYPEINFO };


	//____ constructor _____________________________________________________________

	ImageInfoSection::ImageInfoSection(const DebugTheme& theme, IDebugContext* pContext, Image * pInspected)
		: TypedInfoSection<Image>( theme, pContext, Image::TYPEINFO.className, pInspected )
	{
		//TODO: Size policy and placement (no public getters)

		auto pPanel = WGCREATE(PackPanel, _.axis = Axis::Y);

		auto pTable = _createRows({
			objectRow( "Image surface: ",    [](Image* i) -> Object* { return i->imageSurface().rawPtr(); } ),
			intRow   ( "Image mark alpha: ", [](Image* i) { return i->imageMarkAlpha(); } )
		});

		m_displayedImageRect = pInspected->imageRect();
		m_displayedImageTint = pInspected->imageTint();

		m_pImageRectDrawer = _createRectDrawer("Image rect: ", m_displayedImageRect);
		m_pImageTintDrawer = _createColorDrawer("Image tint: ", m_displayedImageTint);

		pPanel->slots.pushBack({ pTable, m_pImageRectDrawer, m_pImageTintDrawer });
		this->slot = pPanel;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& ImageInfoSection::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ refresh() _____________________________________________________________

	void ImageInfoSection::refresh()
	{
		TypedInfoSection<Image>::refresh();

		_refreshRectDrawer(m_pImageRectDrawer, inspected()->imageRect(), m_displayedImageRect);
		_refreshColorDrawer(m_pImageTintDrawer, inspected()->imageTint(), m_displayedImageTint);
	}

} // namespace wg
