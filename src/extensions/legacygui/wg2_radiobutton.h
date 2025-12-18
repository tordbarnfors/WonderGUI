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
#ifndef	WG2_RADIOBUTTON_DOT_H
#define	WG2_RADIOBUTTON_DOT_H

#ifndef	WG2_CHECKBOX_DOT_H
#	include <wg2_checkbox.h>
#endif

// Radiobutton is almost exactly the same as Checkbox so it inherits from it now
class	WgRadioButton : public WgCheckBox
{

public:
	WgRadioButton();
	~WgRadioButton();

	virtual const char * Type() const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgRadioButton(); };


	bool	SetState( bool state );

	inline void	AllowUnchecking( bool bAllow ) {m_bAllowUnchecking = bAllow; }	///< Should we allow this radiobutton to be unchecked without any other being checked?
	inline bool AllowUnchecking( ) const { return m_bAllowUnchecking; }

private:

	void			_onCloneContent( const WgWidget * _pOrg );
	WgWidgetHolder * _findRadioGroup();
	void			_unselectRecursively( WgWidgetHolder * pParent );


	bool			m_bAllowUnchecking;

};




#endif //	WgRadioButton_DOT_H
