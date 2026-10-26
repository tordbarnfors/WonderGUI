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
#ifndef WG2_IMAGE_DOT_H
#define WG2_IMAGE_DOT_H


#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#ifndef WG2_BLOCKSET_DOT_H
#	include <wg2_blockset.h>
#endif


//____ WgImage __________________________________________________________

class WgImage : public WgWidget
{
public:
	WgImage();
	virtual ~WgImage();

	virtual const char *Type( void ) const;
	static const char * GetClass();
	virtual WgWidget * NewOfMyType() const { return new WgImage(); };

	void			SetSkin(wg::Skin * pSkin);

	void			SetSource(wg::Skin * pSkin) { SetSkin(pSkin); };

	void            SetImage( wg::Surface * pSurface );
	wg::Surface_p   GetImage() const { return m_pImage; }

	void			SetImageTint( wg::Color color);
	wg::Color		ImageTint() const { return m_imageTint; }

	WgSize			PreferredPixelSize() const;

	int				MatchingPixelHeight(int pixelWidth) const;
	int				MatchingPixelWidth(int pixelHeight) const;

	void			SetKeepAspectRatio(bool bKeepAspectRatio) { m_bKeepAspectRatio = bKeepAspectRatio;  }

    void SetScale(int scale) { _setScale(scale); }

protected:

	WgRect	_imageRect(const WgRect& _canvas);

	void	_onCloneContent( const WgWidget * _pOrg );
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window );
	bool	_onAlphaTest( const WgCoord& ofs );
	void	_onEnable();
	void	_onDisable();

	void    _setScale( int scale );

private:

	wg::Surface_p   m_pImage;
	bool			m_bKeepAspectRatio = false;
	wg::Color		m_imageTint = wg::Color::White;
};


#endif //WG2_FILLER_DOT_H
