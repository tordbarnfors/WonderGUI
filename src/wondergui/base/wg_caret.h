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
#ifndef	WG_CARET_DOT_H
#define WG_CARET_DOT_H
#pragma once

#include <wg_geo.h>
#include <wg_pointers.h>

namespace wg
{

	class GfxDevice;

	class Caret;
	typedef	StrongPtr<Caret>		Caret_p;
	typedef	WeakPtr<Caret>		Caret_wp;


	enum class CaretMode
	{
		Insert,
		Overwrite,
		Eol
	};

	/**
	 * @brief Graphical presentation and rendering of a text-field caret
	 *
	 * A Caret object contains information about the graphical presentation
	 * of a caret in an editable text-field and methods for rendering it.
	 *
	 * Displaying the caret in a text-field is the responsibility of a TextLayout
	 * object, which might have a custom implementation, but typically use a Caret
	 * object for this.
	 *
	 * The default Caret class displays a simple, vertical blinking line, but can
	 * be subclassed to use animations or something else.
	 *
	 * Typically you create only one caret and provide it to the TextLayout(s) being
	 * used.
	 */

	class Caret : public Object
	{
	public:
		//.____ Creation __________________________________________

		static Caret_p	create() { return Caret_p(new Caret()); }

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance ________________________________________________________

		virtual bool		setCycleLength( int millisec );
		inline int			cycleLength() const { return m_cycleLength; }	///@brief Returns the length the carrets animation cycle (=blink frequency)


		//.____ Internal ___________________________________________________________

		virtual bool		setMode( CaretMode mode );
		inline CaretMode	mode() const { return m_mode; }
		virtual bool		restartCycle();
		virtual spx			eolWidth( const SizeSPX& eolCell, int scale ) const;
		virtual RectSPX		dirtyRect( const RectSPX& cell, int scale ) const;
		virtual bool		tick( int millisec );
		inline bool			needToRender() const { return m_bNeedToRender; }
		virtual void		render( GfxDevice * pDevice, const RectSPX& cell, int scale );


	protected:
		Caret();
		~Caret() {}

		bool		_updateNeedToRender( int oldTicks, int newTicks );


		CaretMode	m_mode;
		int			m_ticks;
		int			m_cycleLength;
		bool		m_bNeedToRender;

	};


} // namespace wg
#endif //WG_CARET_DOT_H
