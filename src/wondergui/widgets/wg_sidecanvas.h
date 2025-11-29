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
#ifndef	WG_SIDECANVAS_DOT_H
#define	WG_SIDECANVAS_DOT_H
#pragma once


#include <wg_widget.h>

namespace wg
{
	
	
	class SideCanvas;
	typedef	StrongPtr<SideCanvas>	SideCanvas_p;
	typedef	WeakPtr<SideCanvas>		SideCanvas_wp;
	
	
	class SideCanvas : public Widget
	{
	public:

		//____ Holder _____________________________________________________
	
		class Holder		 /** @private */
		{
			friend class SideCanvas;
		protected:

			virtual spx			_sideCanvasMatchingHeight( const SideCanvas * pCanvas, spx width, int scale ) const;
			virtual spx			_sideCanvasMatchingWidth( const SideCanvas * pCanvas, spx height, int scale ) const;

			virtual SizeSPX		_sideCanvasDefaultSize( const SideCanvas * pCanvas, int scale) const = 0;
			virtual SizeSPX		_sideCanvasMinSize( const SideCanvas * pCanvas, int scale) const;
			virtual SizeSPX		_sideCanvasMaxSize( const SideCanvas * pCanvas, int scale) const;

			virtual void		_sideCanvasMaskPatches( SideCanvas * pCanvas, PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip );

			virtual void		_sideCanvasRender( SideCanvas * pCanvas, GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) = 0;

			virtual void		_sideCanvasResize( SideCanvas * pCanvas, const SizeSPX& size, int scale ) = 0;
			virtual void		_sideCanvasSetSkin( SideCanvas * pCanvas,  Skin * pSkin ) ;
			virtual void		_sideCanvasSetState( SideCanvas * pCanvas,  State state );

			virtual void		_sideCanvasReceive( SideCanvas * pCanvas,  Msg * pMsg );
			virtual	bool		_sideCanvasAlphaTest( SideCanvas * pCanvas,  const CoordSPX& ofs ) = 0;			
		};

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;


	protected:
	
		SideCanvas( Holder * pHolder );
		virtual ~SideCanvas();

		spx			_matchingHeight(spx width, int scale) const override;
		spx			_matchingWidth(spx height, int scale) const override;

		SizeSPX		_defaultSize(int scale) const override;
		SizeSPX		_minSize(int scale) const override;
		SizeSPX		_maxSize(int scale) const override;

		void		_maskPatches( PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip ) override;

		void		_render( GfxDevice * pDevice, const RectSPX& _canvas, const RectSPX& _window ) override;

		void		_resize( const SizeSPX& size, int scale ) override;
		void		_setState( State state ) override;

		void		_receive( Msg * pMsg ) override;
		bool		_alphaTest( const CoordSPX& ofs ) override;

		Holder * 	m_pHolder;
	};
	
	
	
} // namespace wg




#endif // WG_SIDECANVAS_DOT_H
