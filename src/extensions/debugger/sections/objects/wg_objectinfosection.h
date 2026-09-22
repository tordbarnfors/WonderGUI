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
#ifndef	WG_OBJECTINFOSECTION_DOT_H
#define WG_OBJECTINFOSECTION_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_infosection.h>

namespace wg
{
	class ObjectInfoSection;
	typedef	StrongPtr<ObjectInfoSection>	ObjectInfoSection_p;
	typedef	WeakPtr<ObjectInfoSection>	ObjectInfoSection_wp;



	class ObjectInfoSection : public InfoSection
	{
	public:

		//.____ Creation __________________________________________

		static ObjectInfoSection_p		create( const DebugTheme& theme, IDebugContext* pContext, Object * pObject) { return ObjectInfoSection_p(new ObjectInfoSection(theme, pContext, pObject) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		ObjectInfoSection(const DebugTheme& theme, IDebugContext* pContext, Object * pObject );
		~ObjectInfoSection() {}

		Object *		m_pInspected;
		TablePanel_p	m_pTable;
		void *			m_pFinalizer = nullptr;

	};

} // namespace wg
#endif //WG_OBJECTINFOSECTION_DOT_H

