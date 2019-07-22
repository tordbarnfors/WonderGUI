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
#ifndef WG_CANVAS_DOT_H
#define WG_CANVAS_DOT_H
#pragma once


#include <wg_widget.h>
#include <wg_gfxdevice.h>
#include <wg_icanvas.h>


namespace wg
{

	class Canvas;
	typedef	StrongPtr<Canvas>	Canvas_p;
	typedef	WeakPtr<Canvas>		Canvas_wp;





	//____ Canvas ____________________________________________________________
	/**
	 * @brief Widget displaying a surface on which rendering operations can be performed.
	 *
	 * The Canvas widget displays a surface onto which various rendering operations can be
	 * performed using a GfxDevice.
	 *
	 * To setup a Canvas, you at least need to provide a GfxDevice of your choice, that
	 * should be unique to this Canvas and not used for anything else. Optionally you
	 * can also provide a SurfaceFactory and specify PixelFormat and size for the surface. Please see
	 * the CCanvas interface documentation for details.
	 *
	 * Please note that "canvas" is not only the name of this widget, but used throughout
	 * WonderGUI for surfaces and rectangular areas that are destinations for drawing operations.
	 * To make a clearer distinction we try to write Canvas with a capital C when
	 * referring to the widget and a small c everywhere else, but through
	 * CamelCasing we still end up writing canvas with a capital C when not referring to this widget
	 * (like "setCanvas()" or "pCanvas").
	 *
	 **/


	class Canvas : public Widget
	{
		friend class ICanvas;

	public:

		//.____ Creation __________________________________________

		static Canvas_p	create() { return Canvas_p(new Canvas()); }

		//.____ Interfaces _______________________________________

		ICanvas		canvas;


		//.____ Identification __________________________________________

		bool				isInstanceOf( const char * pClassName ) const;
		const char *		className( void ) const;
		static const char	CLASSNAME[];
		static Canvas_p		cast( Object * pObject );

		//.____ Geometry ____________________________________________

		virtual int		matchingHeight(int width) const;
		virtual int		matchingWidth( int height ) const;

		SizeI			preferredSize() const;


	protected:
		Canvas();
		virtual ~Canvas();
		virtual Widget* _newOfMyType() const { return new Canvas(); };

		void			_cloneContent( const Widget * _pOrg );
		void			_setSize(const SizeI& size);
		virtual void	_setSkin(Skin * pSkin);
		virtual void	_render(GfxDevice * pDevice, const RectI& _canvas, const RectI& _window);
		virtual	bool	_alphaTest(const CoordI& ofs);


		CCanvas	m_canvas;
	};



} // namespace wg
#endif //WG_CANVAS_DOT_H
