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
#ifndef WG2_SHADERCAPSULE_DOT_H
#define WG2_SHADERCAPSULE_DOT_H

#ifndef WG2_CAPSULE_DOT_H
#	include <wg2_capsule.h>
#endif

#include <wg2_color.h>

class WgShaderCapsule : public WgCapsule
{
public:
	WgShaderCapsule();
	~WgShaderCapsule();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgShaderCapsule(); };

	void		SetColor( const WgColor& color);
	void		SetTintMode( WgTintMode mode );
	void		SetBlendMode( WgBlendMode mode );

	inline WgColor		Color() { return m_tintColor; }
	inline WgBlendMode	BlendMode() { return m_blendMode; }
	inline WgTintMode	TintMode() { return m_tintMode; }

	WgWidget * 	FindWidget(const WgCoord& ofs, WgSearchMode mode);

protected:
	void		_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches );

	void		_onCollectPatches(WgPatches& container, const WgRect& geo, const WgRect& clip);
	void		_onMaskPatches(WgPatches& patches, const WgRect& geo, const WgRect& clip, WgBlendMode blendMode);

	void		_onCloneContent( const WgWidget * _pOrg );
	WgBlendMode _getBlendMode() const;

private:
	WgColor			m_tintColor;
	WgBlendMode		m_blendMode;
	WgTintMode		m_tintMode;

};

#endif //WG2_SHADERCAPSULE_DOT_H
