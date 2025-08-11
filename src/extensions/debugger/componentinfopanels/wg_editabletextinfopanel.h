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

#ifndef	WG_EDITABLETEXTINFOPANEL_DOT_H
#define WG_EDITABLETEXTINFOPANEL_DOT_H
#pragma once

#include <wg_tablepanel.h>
#include <wg_debugpanel.h>
#include <wg_editabletext.h>

namespace wg
{
	class EditableTextInfoPanel;
	typedef	StrongPtr<EditableTextInfoPanel>	EditableTextInfoPanel_p;
	typedef	WeakPtr<EditableTextInfoPanel>	EditableTextInfoPanel_wp;



	class EditableTextInfoPanel : public DebugPanel
	{
	public:

		//.____ Creation __________________________________________

		static EditableTextInfoPanel_p		create(const Blueprint& blueprint, IDebugger* pHolder, EditableText* pEditableText) { return EditableTextInfoPanel_p(new EditableTextInfoPanel(blueprint, pHolder, pEditableText)); }

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Control ____________________________________________________

		void refresh() override;

	protected:
		EditableTextInfoPanel(const Blueprint& blueprint, IDebugger* pHolder, EditableText* pEditableText);
		~EditableTextInfoPanel() {}

		TablePanel_p	m_pTable;
		EditableText *	m_pInspected;

	};

} // namespace wg
#endif //WG_EDITABLETEXTINFOPANEL_DOT_H

