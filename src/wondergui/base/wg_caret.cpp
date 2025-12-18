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
#include <wg_caret.h>
#include <wg_gfxdevice.h>
#include <wg_util.h>

namespace wg
{

	const TypeInfo Caret::TYPEINFO = { "Caret", &Object::TYPEINFO };


	//____ constructor _____________________________________________________________

	Caret::Caret()
	{
		m_mode = CaretMode::Eol;
		m_ticks = 0;
		m_cycleLength = 1000;
		m_bNeedToRender = false;
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& Caret::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ setMode() _______________________________________________________________
	/**
	 * @brief Set the carets animation mode (insert, overwrite or end of line).
	 *
	 * Set the carets animation mode (insert, overwrite or end of line).
	 *
	 * @param mode One of Insert, Overwrite or Eol (end of line)
	 * @return True if caret needs to be redrawn.
	 */

	bool Caret::setMode( CaretMode mode )
	{
		if( mode != m_mode )
		{
			m_mode = mode;
			m_ticks = 0;
			m_bNeedToRender = true;
		}
		return m_bNeedToRender;
	}

	//____ setCycleLength() ____________________________________________________
	/**
	 * @brief Set the length of the caret animation cycle (blink frequency)
	 *
	 * Sets the length in milliseconds of the caret animation cycle. For a normal, blinking
	 * cursor this means the time between when the cursor appears and reappears again.
	 *
	 * Default is 1000 milliseconds.
	 *
	 * @param millisec Number of milliseconds for a full animation cycle.
	 * @return True as long as millisec is >=2.
	 */
	bool Caret::setCycleLength( int millisec )
	{
		if( millisec < 2 )
			return false;

		m_cycleLength = millisec;
		return true;
	}

	//____ restartCycle() ______________________________________________________
	/**
	 * @brief Restart the caret animation cycle
	 *
	 * Restarts the carets animation cycle. This is typically done when the caret is moved.
	 *
	 * @return True if the caret needs to be redrawn.
	 */
 	bool Caret::restartCycle()
	{
		int oldTicks = m_ticks;
		m_ticks = 0;

		return _updateNeedToRender( oldTicks, m_ticks );
	}


	//____ eolWidth() ______________________________________________________________
	/**
	 * @brief Pixels needed for the end of line character.
	 *
	 * Calculates the number of pixels needed for rendering an end-of-line caret.
	 * 
	 * Since the end-of-line caret is placed after the last character of the line, the
	 * TextLayout needs to know how much extra space needs to be reserved past the end of
	 * the line to fit the caret in an editable text-field.
	 * 
	 * @param eolCell The size of the last cell of the line in pixels. Typically the
	 * height of the previous cell is used to determine the carets width.
	 *
	 * @return Number of extra pixels needed to fit the caret after the line.
	 */

	spx Caret::eolWidth( const SizeSPX& eolCell, int scale ) const
	{
		return std::max( 1*scale, eolCell.h / 16);
	}

	//____ tick() __________________________________________________________________
	/**
	 * @brief Increment time for caret animation.
	 *
	 * Increments the time for the caret animation and returns whether caret needs to be redrawn.
	 *
	 * @return True if the caret needs to be redrawn.
	 */

	bool Caret::tick( int ms )
	{
		int oldTicks = m_ticks;
		m_ticks = (m_ticks + ms) % m_cycleLength;

		return _updateNeedToRender( oldTicks, m_ticks );
	}

	//____ dirtyRect() _____________________________________________________________
	/**
	 * @brief Returns the rectangle occupied by the caret.
	 *
	 * Returns the rectangle occupied by the caret, the area that needs to be redrawn 
	 * when the caret animation is updated or caret is moved.
	 *
	 * @param cell The position and size of the character cell with the caret.
	 * @return The position and size of the caret.
	 */

	RectSPX Caret::dirtyRect( const RectSPX& cell, int scale ) const
	{
		switch( m_mode )
		{
			case CaretMode::Overwrite:
				return cell;
			case CaretMode::Eol:
				return Util::align(RectSPX( cell.x, cell.y, std::max(1*scale, cell.h/ 16), cell.h ));
			default: // CaretMode::Insert:
				return Util::align(RectSPX(cell.x, cell.y, std::max(1 * scale, cell.h / 16), cell.h));
		}
	}

	//____ render() ________________________________________________________________
	/**
	 * @brief Render the caret.
	 *
	 * Renders the caret in position for the character occupying the 'cell' specified.
	 *
	 * @param cell The position and size of the character cell with the caret.
	 *
	 * Please note that the caret might be rendered outside the cell specified. Call dirtyRect()
	 * with the same cell rectangle to get the area that a call to render() will affect.
	 *
	 * @return Void.
	 */

	void Caret::render( GfxDevice * pDevice, const RectSPX& cell, int scale )
	{
		if( m_ticks < m_cycleLength / 2 )
		{
			RectSPX r = dirtyRect(cell, scale);
			BlendMode oldMode = pDevice->blendMode();
			pDevice->setBlendMode(BlendMode::Invert);
			pDevice->fill( r, HiColor::White );
			pDevice->setBlendMode(oldMode);
		}

		m_bNeedToRender = false;
	}

	//____ _updateNeedToRender() _______________________________________________

	bool Caret::_updateNeedToRender( int oldTicks, int newTicks )
	{
		int halfCycle = m_cycleLength / 2;
		int oldBlink = oldTicks / halfCycle;

		int newBlink = newTicks / halfCycle;
		if( newBlink != oldBlink )
			m_bNeedToRender = true;

		return m_bNeedToRender;
	}


} // namespace wg
