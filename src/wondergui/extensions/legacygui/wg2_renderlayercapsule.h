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
#ifndef WG2_RENDERLAYERCAPSULE_DOT_H
#define WG2_RENDERLAYERCAPSULE_DOT_H

#ifndef WG2_CAPSULE_DOT_H
#	include <wg2_capsule.h>
#endif


class WgRenderLayerCapsule : public WgCapsule
{
public:
    WgRenderLayerCapsule();
	WgRenderLayerCapsule(WgWidget * pChild, int layer);
	~WgRenderLayerCapsule();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgRenderLayerCapsule(); };

	void		SetRenderLayer( int layer);
    inline int 	RenderLayer() { return m_renderLayer; }

protected:
	void		_renderPatches( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window, WgPatches * _pPatches );


	void		_onCloneContent( const WgWidget * _pOrg );

private:
    int         m_renderLayer = -1;
};

#endif //WG2_RENDERLAYERCAPSULE_DOT_H
