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
#ifndef WG_RENDERLAYERCAPSULE_DOT_H
#define WG_RENDERLAYERCAPSULE_DOT_H
#pragma once

#include <wg_capsule.h>

namespace wg
{
	class RenderLayerCapsule;
	typedef	StrongPtr<RenderLayerCapsule>	RenderLayerCapsule_p;
	typedef	WeakPtr<RenderLayerCapsule>		RenderLayerCapsule_wp;

	//____ RenderLayerCapsule ______________________________________________________

	class RenderLayerCapsule : public Capsule
	{
	public:
		//____ Blueprint __________________________________________

		struct Blueprint
		{

			Object_p		baggage;
			Widget_p		child;
			bool			disabled		= false;
			bool			dropTarget		= false;
			Finalizer_p		finalizer		= nullptr;
			int				id				= 0;
			MarkPolicy		markPolicy		= MarkPolicy::AlphaTest;
			bool			pickable		= false;
			uint8_t			pickCategory	= 0;
			bool			pickHandle		= false;
			PointerStyle	pointer			= PointerStyle::Undefined;
			int				renderLayer		= -1;
			bool			selectable		= false;
			Skin_p			skin;
			bool			stickyFocus		= false;
			bool			tabLock			= false;
			String			tooltip;
			bool			usePickHandles	= false;
		};


		//.____ Creation __________________________________________

		static RenderLayerCapsule_p	create();
		static RenderLayerCapsule_p	create(const Blueprint& blueprint);

		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Rendering _________________________________________________

		void	setRenderLayer(int layer);
		int		renderLayer() const { return m_renderLayer; }

	protected:
		RenderLayerCapsule();

		template<class BP> RenderLayerCapsule( const BP& bp ) : Capsule(bp)
		{
			 m_renderLayer = bp.renderLayer;

			 if (bp.child)
				 slot.setWidget(bp.child);
		}

		virtual ~RenderLayerCapsule();

		void		_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;

		int			m_renderLayer = -1;
	};


} // namespace wg
#endif //WG_RENDERLAYERRCAPSULE_DOT_H
