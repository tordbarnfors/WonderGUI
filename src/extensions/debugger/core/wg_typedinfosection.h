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
#ifndef	WG_TYPEDINFOSECTION_DOT_H
#define WG_TYPEDINFOSECTION_DOT_H
#pragma once

#include <functional>
#include <vector>
#include <type_traits>

#include <wg_infosection.h>
#include <wg_base.h>
#include <wg_msgrouter.h>

namespace wg
{

	//____ TypedInfoSection ______________________________________________________
	//
	// InfoSection for one inspected object of type T whose table rows are
	// described once, as a list of label + getter. The same list builds the
	// table and refreshes it, so the two can't drift apart.
	//
	//	m_pTable = _createRows({
	//		intRow ( "Id: ",          [](Widget* w) { return w->id(); } ),
	//		ptsRow ( "Width (pts): ", [](Widget* w) { return w->size().w; } ),
	//		...
	//	});
	//
	// refresh() updates all rows. Subclasses with more content (drawers etc.)
	// override refresh() and call TypedInfoSection<T>::refresh() first.

	template<class T>
	class TypedInfoSection : public InfoSection
	{
	public:

		//.____ Control _____________________________________________________

		void		refresh() override { _refreshRows(); }

		// Only slot sections follow a new slot; for others this does nothing.

		void		setInspectedSlot(StaticSlot* pSlot) override
		{
			if constexpr (std::is_base_of<StaticSlot, T>::value)
				m_pInspected = static_cast<T*>(pSlot);
		}

		//.____ Content _____________________________________________________

		void		setInspected(T* pInspected) { m_pInspected = pInspected; }
		T*			inspected() const { return m_pInspected; }

	protected:

		//____ Row ______________________________________________________________

		struct Row
		{
			const char*		pLabel;
			std::function<void(TypedInfoSection* pSection, TablePanel* pTable, int row)>							init;
			std::function<void(TypedInfoSection* pSection, TablePanel* pTable, int row, T* pInspected, Row& state)>	update;

			// Per-row state, used by pointer rows.

			void*			pDisplayedPointer = nullptr;
			Object_p		pDisplayedObject;
			RouteId			linkRoute = 0;
		};

		TypedInfoSection(const DebugTheme& theme, IDebugContext* pContext, const char* pLabel, T* pInspected)
			: InfoSection(theme, pContext, pLabel), m_pInspected(pInspected) {}

		~TypedInfoSection()
		{
			auto pRouter = Base::msgRouter();
			if (pRouter)
			{
				for (auto& row : m_rows)
					if (row.linkRoute != 0)
						pRouter->deleteRoute(row.linkRoute);
			}
		}

		//____ _createRows() ______________________________________________________
		//
		// Creates the table for the rows, fills it in from the inspected object
		// and returns it for the subclass to place.

		TablePanel_p _createRows(std::vector<Row> rows)
		{
			m_rows = std::move(rows);
			m_pRowTable = _createTable((int)m_rows.size(), 2);

			for (int i = 0; i < (int)m_rows.size(); i++)
				m_rows[i].init(this, m_pRowTable, i);

			_refreshRows();
			return m_pRowTable;
		}

		//____ _refreshRows() _____________________________________________________

		void _refreshRows()
		{
			if (!m_pRowTable || !m_pInspected)
				return;

			for (int i = 0; i < (int)m_rows.size(); i++)
				m_rows[i].update(this, m_pRowTable, i, m_pInspected, m_rows[i]);
		}

		//____ Row helpers ________________________________________________________
		//
		// The getter takes a T* and returns the value. Using one helper per
		// value kind makes a getter of the wrong type a compile error at the row.

		template<class Getter>
		static Row textRow(const char* pLabel, Getter get)
		{
			return { pLabel,
				[pLabel](TypedInfoSection* s, TablePanel* t, int r) { s->_initTextEntry(t, r, pLabel); },
				[get](TypedInfoSection* s, TablePanel* t, int r, T* p, Row&) { s->_refreshTextEntry(t, r, get(p)); } };
		}

		template<class Getter>
		static Row intRow(const char* pLabel, Getter get)
		{
			return { pLabel,
				[pLabel](TypedInfoSection* s, TablePanel* t, int r) { s->_initIntegerEntry(t, r, pLabel); },
				[get](TypedInfoSection* s, TablePanel* t, int r, T* p, Row&) { s->_refreshIntegerEntry(t, r, int(get(p))); } };
		}

		template<class Getter>
		static Row decimalRow(const char* pLabel, Getter get)
		{
			return { pLabel,
				[pLabel](TypedInfoSection* s, TablePanel* t, int r) { s->_initDecimalEntry(t, r, pLabel); },
				[get](TypedInfoSection* s, TablePanel* t, int r, T* p, Row&) { s->_refreshDecimalEntry(t, r, float(get(p))); } };
		}

		template<class Getter>
		static Row ptsRow(const char* pLabel, Getter get)
		{
			return { pLabel,
				[pLabel](TypedInfoSection* s, TablePanel* t, int r) { s->_initPtsEntry(t, r, pLabel); },
				[get](TypedInfoSection* s, TablePanel* t, int r, T* p, Row&) { s->_refreshPtsEntry(t, r, pts(get(p))); } };
		}

		template<class Getter>
		static Row spxRow(const char* pLabel, Getter get)
		{
			return { pLabel,
				[pLabel](TypedInfoSection* s, TablePanel* t, int r) { s->_initSpxEntry(t, r, pLabel); },
				[get](TypedInfoSection* s, TablePanel* t, int r, T* p, Row&) { s->_refreshSpxEntry(t, r, spx(get(p))); } };
		}

		template<class Getter>
		static Row boolRow(const char* pLabel, Getter get)
		{
			return { pLabel,
				[pLabel](TypedInfoSection* s, TablePanel* t, int r) { s->_initBoolEntry(t, r, pLabel); },
				[get](TypedInfoSection* s, TablePanel* t, int r, T* p, Row&) { s->_refreshBoolEntry(t, r, bool(get(p))); } };
		}

		template<class Getter>
		static Row pointerRow(const char* pLabel, Getter get)
		{
			return { pLabel,
				[pLabel](TypedInfoSection* s, TablePanel* t, int r) { s->_initPointerEntry(t, r, pLabel); },
				[get](TypedInfoSection* s, TablePanel* t, int r, T* p, Row& row) { s->_refreshPointerEntry(t, r, (void*) get(p), row.pDisplayedPointer); } };
		}

		// Shows class name and address as a link that selects the object.
		// Removes the link's previous route when the pointer changes.

		template<class Getter>
		static Row objectRow(const char* pLabel, Getter get)
		{
			return { pLabel,
				[pLabel](TypedInfoSection* s, TablePanel* t, int r) { s->_initObjectPointerEntry(t, r, pLabel); },
				[get](TypedInfoSection* s, TablePanel* t, int r, T* p, Row& row) { Object* pObj = get(p); s->_refreshObjectPointerEntry(t, r, pObj, row.pDisplayedObject, row.linkRoute); } };
		}

		T*					m_pInspected = nullptr;
		std::vector<Row>	m_rows;
		TablePanel_p		m_pRowTable;
	};

} // namespace wg
#endif //WG_TYPEDINFOSECTION_DOT_H
