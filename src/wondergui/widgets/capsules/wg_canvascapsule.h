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
#ifndef WG_CANVASCAPSULE_DOT_H
#define WG_CANVASCAPSULE_DOT_H
#pragma once

#include <wg_capsule.h>
#include <wg_surfacefactory.h>
#include <wg_patches.h>
#include <wg_canvaslayers.h>
#include <wg_glow.h>
#include <wg_tintmap.h>
#include <wg_transitions.h>

namespace wg
{

	class CanvasCapsule;
	typedef	StrongPtr<CanvasCapsule>	CanvasCapsule_p;
	typedef	WeakPtr<CanvasCapsule>		CanvasCapsule_wp;

	class CanvasDisplay;

	//____ CanvasCapsule ______________________________________________________
	/**
	* @brief	A widget that provides a separate canvas for its descendants.
	*
	* The CanvasCapsule provides a separate canvas for its descendants into which they
	* are rendered. This then acts as a bitmap cache for fast blits of content when
	* it needs to be re-rendered with none or small changes.
	*
	*/

	class CanvasCapsule : public Capsule
	{
		friend CanvasDisplay;

	public:

		//____ Blueprint __________________________________________

		struct Blueprint
		{
			Object_p		baggage;
			BlendMode		blendMode		= BlendMode::Blend;
			Widget_p		child;
			HiColor			clearColor		= HiColor::Undefined;
			bool			disabled		= false;
			bool			dropTarget		= false;
			Finalizer_p		finalizer		= nullptr;
			Glow::Blueprint glow;
			int				id = 0;
			CanvasLayers_p	layers			= nullptr;
			MarkPolicy		markPolicy		= MarkPolicy::AlphaTest;
			bool			pickable		= false;
			uint8_t			pickCategory	= 0;
			bool			pickHandle		= false;
			PixelFormat		pixelFormat		= PixelFormat::BGRA_8;
			Placement		placement		= Placement::Center;
			PointerStyle	pointer			= PointerStyle::Undefined;
			int				renderLayer		= -1;
			bool			scaleCanvas		= false;
			bool			selectable		= false;
			Skin_p			skin;
			bool			skinAroundCanvas = false;
			bool			stickyFocus		= false;
			SurfaceFactory_p surfaceFactory;
			bool			tabLock			= false;
			HiColor			tintColor		= HiColor::Undefined;
			Tintmap_p		tintmap;
			String			tooltip;
			bool			usePickHandles = false;
		};

		//.____ Creation __________________________________________

		static CanvasCapsule_p	create() { return CanvasCapsule_p(new CanvasCapsule()); }
		static CanvasCapsule_p	create( const Blueprint& bp ) { return CanvasCapsule_p(new CanvasCapsule(bp)); }

		//.____ Components _______________________________________

		Glow				glow;
		
		//.____ Identification __________________________________________

		const TypeInfo& typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Geometry ______________________________________________________

		void				setPlacement(Placement placement);
		Placement			placement() const { return m_placement; }

		void				setScaleCanvas(bool bScale);
		bool				isCanvasScaling() { return m_bScaleCanvas; }

		Rect				canvasGeo() const;

		//.____ Appearance _________________________________________________

		void				setSkin(Skin* pSkin) override;

		void				setTintColor(HiColor color, ColorTransition* pTransition = nullptr);
		void				setTintmap(Tintmap* pTintmap, ColorTransition* pTransition = nullptr);
		void				setBlendMode(BlendMode mode);

		void				setSurfaceFactory(SurfaceFactory* pFactory);
		SurfaceFactory_p	surfaceFactory() const { return m_pFactory; }
		
		void				setFormat(PixelFormat format);
		PixelFormat			format() const { return m_canvasFormat; }
		
		void				setCanvasLayers(CanvasLayers * pLayers);
		CanvasLayers_p	 	canvasLayers() const { return m_pCanvasLayers; }

		void				setClearColor( HiColor color );
		HiColor				clearColor() const { return m_clearColor; }

		void				setRenderLayer(int layer);
		int					renderLayer() const { return m_renderLayer; }

		
		inline HiColor		tintColor() { return m_tintColor; }
		inline Tintmap_p	tintmap() { return m_pTintmap; }
		inline BlendMode	blendMode() { return m_blendMode; }

		//.____ Internal _________________________________________________

		spx					_matchingHeight(spx width, int scale) const override;
		spx					_matchingWidth(spx height, int scale) const override;

		
	protected:
		CanvasCapsule();
		
		template<class BP> CanvasCapsule(const BP& bp) : Capsule(bp), glow(this)
		{
			m_canvasSize = m_size;

			m_pFactory		= bp.surfaceFactory;
			m_canvasFormat	= bp.pixelFormat;
			m_pCanvasLayers = bp.layers;
			m_renderLayer	= bp.renderLayer;

			m_blendMode		= bp.blendMode;
			
			if( bp.tintColor != HiColor::Undefined )
				m_tintColor	= bp.tintColor;

			m_pTintmap		= bp.tintmap;
			m_bScaleCanvas  = bp.scaleCanvas;
			m_placement		= bp.placement;
			m_clearColor	= bp.clearColor;
			
			m_bSkinAroundCanvas = bp.skinAroundCanvas;
			
			glow._initFromBlueprint(bp.glow);
		}
		
		virtual ~CanvasCapsule();

		Root *				_root() override;

		Widget* 			_findWidget(const CoordSPX& ofs, SearchMode mode) override;

		void				_resizeCanvasAndChild();

		void				_update(int microPassed, int64_t microsecTimestamp) override;

		Surface*			_renderCanvas(GfxDevice* pDevice);
		void				_render(GfxDevice* pDevice, const RectSPX& _canvas, const RectSPX& _window) override;
		void				_resize(const SizeSPX& size, int scale) override;

		void				_maskPatches(PatchesSPX& patches, const RectSPX& geo, const RectSPX& clip) override;
		
		void				_releaseChild( StaticSlot * pSlot ) override;
		void				_replaceChild(StaticSlot * pSlot, Widget * pWidget) override;

		
		void				_childRequestRender(StaticSlot* pSlot, const RectSPX& rect) override;
		void				_childRequestResize( StaticSlot * pSlot ) override;

		void				_childRequestInView( StaticSlot * pSlot ) override;
		void				_childRequestInView( StaticSlot * pSlot, const RectSPX& mustHaveArea, const RectSPX& niceToHaveArea ) override;
		
		RectSPX				_slotGeo(const StaticSlot* pSlot) const override;

		RectSPX				_childLocalToGlobal(const StaticSlot* pSlot, const RectSPX& rect) const override;
		RectSPX				_globalToChildLocal(const StaticSlot* pSlot, const RectSPX& rect) const override;
		RectSPX				_globalPtsToChildLocalSpx(const StaticSlot* pSlot, const Rect& rect) const override;
		Rect				_childLocalSpxToGlobalPts(const StaticSlot* pSlot, const RectSPX& rect) const override;

		RectSPX				_canvasWindow( RectSPX window ) const;
		Surface*			_canvasSurface() { return m_pCanvas; }

		void				_addSideDisplay(CanvasDisplay* pSideDisplay);
		void				_removeSideDisplay(CanvasDisplay* pSideDisplay);
		SizeSPX				_canvasSize() const { return m_canvasSize; }

		void				_setCanvasSize( SizeSPX canvasSize );

		Surface_p			m_pCanvas;
		Surface_p			m_pGlowCanvas;	// Extra canvas that may be needed when glow is active.

		CanvasLayers_p		m_pCanvasLayers;
		SizeSPX				m_canvasSize;
		
		SurfaceFactory_p	m_pFactory;
		PixelFormat			m_canvasFormat = PixelFormat::BGRA_8;
		HiColor				m_clearColor = HiColor::Undefined;
		int					m_renderLayer = -1;

		HiColor				m_tintColor = HiColor::White;
		Tintmap_p			m_pTintmap;
		BlendMode			m_blendMode = BlendMode::Blend;

		PatchesSPX			m_patches;

		bool				m_bScaleCanvas = false;
		bool				m_bSkinAroundCanvas = false;
		Placement			m_placement = Placement::Center;
		
		std::vector<CanvasDisplay*>	m_sideDisplays;
		
		// Transitions

		HiColor				m_startTintColor;
		HiColor				m_endTintColor;

		Tintmap_p			m_pStartTintmap;
		Tintmap_p			m_pEndTintmap;

		ColorTransition_p	m_pTintColorTransition;
		ColorTransition_p	m_pTintmapTransition;

		int					m_tintColorTransitionProgress = 0;
		int					m_tintmapTransitionProgress = 0;
	};


} // namespace wg
#endif //WG_CANVASCAPSULE_DOT_H
