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
#ifndef	WG_OBJECTINSPECTOR_DOT_H
#define WG_OBJECTINSPECTOR_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugwindow.h>

namespace wg
{
	class ObjectInspector;
	typedef	StrongPtr<ObjectInspector>	ObjectInspector_p;
	typedef	WeakPtr<ObjectInspector>	ObjectInspector_wp;



	class ObjectInspector : public DebugWindow
	{
	public:

		//.____ Creation __________________________________________

		static ObjectInspector_p		create( const Blueprint& blueprint, IDebugger * pHolder, Object * pObject) { return ObjectInspector_p(new ObjectInspector(blueprint, pHolder, pObject) ); }

		//.____ Identification __________________________________________

		const TypeInfo&			typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

	protected:
		ObjectInspector(const Blueprint& blueprint, IDebugger * pHolder, Object * pObject );
		~ObjectInspector() {}

		Object_p	m_pObject;			// Keep our object alive as long as we are, so refresh won't fail.

		ToggleButton_p	m_pAutoRefreshButton;
	};

} // namespace wg
#endif //WG_OBJECTINSPECTOR_DOT_H


