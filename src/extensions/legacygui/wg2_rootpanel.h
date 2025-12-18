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
#ifndef WG2_ROOTPANEL_DOT_H
#define WG2_ROOTPANEL_DOT_H

#include <deque>
#include <vector>

#ifndef WG2_CONTAINER_DOT_H
#	include <wg2_container.h>
#endif

#ifndef WG2_GEO_DOT_H
#	include <wg2_geo.h>
#endif

#include <wg2_root.h>
#include <wg2_patches.h>
#include <wg_skin.h>
#include <wg_gfxdevice.h>
#include <wg_surface.h>

class WgWidget;

class WgRootPanel : public WgWidgetHolder, public WgRoot
{
	friend class WgWidget;

public:
	WgRootPanel();
	WgRootPanel( wg::Surface * pCanvasSurface, wg::GfxDevice * pGfxDevice = nullptr );
    WgRootPanel( wg::CanvasRef canvasRef, wg::GfxDevice * pGfxDevice = nullptr );
	~WgRootPanel();

	void					SetMaxDirtyRects( int max );
	inline int				MaxDirtyRects() const { return m_maxDirtyRects; }

	void					SetDirtyRectAlignment( int alignment );
	int						DirtyRectAlignment() const { return m_dirtyRectAlignment; }

	void					SetMaxDirtyRectPixels( int maxDirtyRectPixels );
	inline int				MaxDirtyRectPixels() const { return m_maxDirtyRectPixels; }

	
	bool					SetGfxDevice( wg::GfxDevice * pDevice );
	inline wg::GfxDevice_p 	GfxDevice() const { return m_pGfxDevice; };

    bool                    SetCanvas(wg::Surface* pCanvasSurface);
    bool                    SetCanvas(wg::CanvasRef canvasRef);
    inline const wg::CanvasInfo&   CanvasInfo() const { return m_canvas; }
    inline wg::SizeI        CanvasSize() const { return m_canvas.size; }
    
	inline WgEventHandler *	EventHandler() const override { return m_pEventHandler; }

	bool					SetPixelGeo( const WgRect& geo );
	WgRect					PixelGeo() const;

	bool					SetVisible( bool bVisible );
	bool					IsVisible() const { return m_bVisible; }

	inline WgWidget *		Child() const { return m_hook.Widget(); }
	bool					SetChild( WgWidget * pWidget ) override;
	inline void				DeleteChild() { SetChild(0); }
	WgWidget * 				ReleaseChild();

	void					SetScale( int scale );
	int						Scale() const override { return m_scale; }

	// Freezes RootPanels geo and scale.
	// Calls to SetPixelGeo() and SetScale() will just store value
	// and not apply it until unfrozen. This does not prevent direct changes to geo/scale on its children.

	void					SetFreezeGeo(bool bFreeze);
	bool					IsGeoFrozen(bool bFreeze) { return m_bGeoFrozen; }

	// Inherited from WgWidgetHolder

	bool					DeleteChild( WgWidget * pWidget ) override;
	WgWidget *				ReleaseChild( WgWidget * pWidget ) override;

	bool					DeleteAllChildren() override;
	bool					ReleaseAllChildren() override;

	bool					IsRoot() const override { return true; }

	WgRoot *				CastToRoot() override { return this; }
	const WgRoot *			CastToRoot() const override { return this; }


	inline int				NbDirtyRects() const { return m_dirtyPatches.size(); }
	inline const WgRect*	FirstDirtyRect() const { return m_dirtyPatches.isEmpty() ? 0 : m_dirtyPatches.begin(); }

	inline int				NbUpdatedRects() const { return m_updatedPatches.size(); }
	inline const WgRect*	FirstUpdatedRect() const { return m_updatedPatches.isEmpty() ? 0 : m_updatedPatches.begin(); }


	bool	Render();
	bool	Render( const WgRect& clip );

	bool	BeginRender();
	bool	RenderSection( const WgRect& clip );
	bool	EndRender();

	void	SetUpdatedRectOverlay( wg::Skin * pUpdatedRectOverlay, int afterglowFrames = 3 );

	void 	AddDirtyPatch( const WgRect& rect );
    inline void ClearDirtyPatches( const WgRect& rect ) { m_dirtyPatches.clear(); }

	WgWidget *	FindWidget( const WgCoord& ofs, WgSearchMode mode ) override;

protected:
	class Hook : public WgHook
	{
		friend class WgRootPanel;
	public:
		Hook() : m_pRoot(0) {};				// So we can make them members and then make placement new...
		~Hook();

		const char *Type( void ) const;
		static const char * ClassType();

		WgCoord			PixelPos() const;
		WgSize			PixelSize() const;
		WgRect			PixelGeo() const;
		WgCoord			ScreenPixelPos() const;
		WgRect			ScreenPixelGeo() const;

		WgRootPanel*	Root() const;

	protected:

		void			_requestRender();
		void			_requestRender( const WgRect& rect );
		void			_requestResize();

		WgHook *		_prevHook() const;
		WgHook *		_nextHook() const;
		WgWidgetHolder * _holder() const;
		WgContainer *	_parent() const;

		WgRootPanel *		m_pRoot;
	};

	inline void         _addPreRenderCall(WgWidget * pWidget) override { m_preRenderCalls.push_back(pWidget); }

	WgHook*				_firstHook() const override { return m_hook.Widget()? const_cast<Hook*>(&m_hook):0; }
	WgHook*				_lastHook() const override { return m_hook.Widget()? const_cast<Hook*>(&m_hook):0; }

	bool 				_focusRequested( WgHook * pBranch, WgWidget * pWidgetRequesting ) override;
	bool 				_focusReleased( WgHook * pBranch, WgWidget * pWidgetReleasing ) override;

	void                _inViewRequested( WgHook * pChild ) override;
	void                _inViewRequested( WgHook * pChild, const WgRect& mustHaveArea, const WgRect& niceToHaveArea ) override;

	WgPatches			m_dirtyPatches;		// Dirty patches that needs to be rendered.
	WgPatches			m_updatedPatches;	// Patches that were updated in last rendering session.

	int					m_maxDirtyRects = 0;	// 0 = unlimited
	int					m_dirtyRectAlignment = 1;
	int					m_maxDirtyRectPixels = 0;

	bool				m_bWasAlreadyRendering = false;
	
	std::vector<WgWidgetWeakPtr>   m_preRenderCalls;

	wg::Skin_p			m_pUpdatedRectOverlay;
	int					m_afterglowFrames;

	std::deque<WgPatches>	m_afterglowRects;	// Afterglow rects are placed in this queue.


	wg::GfxDevice_p		m_pGfxDevice;
    wg::CanvasInfo		m_canvas;
	WgEventHandler *	m_pEventHandler;
	Hook				m_hook;
	WgRect				m_geo;
	int					m_scale;
	bool				m_bHasGeo;
	bool				m_bVisible;
	bool				m_bGeoFrozen = false;
	
	WgRect				m_frozenGeo;
	int					m_frozenScale;
};



#endif //WG2_ROOTPANEL_DOT_H
