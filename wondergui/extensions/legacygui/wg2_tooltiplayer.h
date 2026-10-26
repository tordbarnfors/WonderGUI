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
#ifndef WG2_TOOLTIPLAYER_DOT_H
#define WG2_TOOLTIPLAYER_DOT_H
#pragma once

#include <wg2_layer.h>



class WgTooltipLayer;

//____ WgTooltipLayer ____________________________________________________________

class WgTooltipLayer : public WgLayer
{
public:

    WgTooltipLayer();
    virtual ~WgTooltipLayer();

    virtual const char *Type(void) const override;
    static const char * GetClass();
    virtual WgWidget * NewOfMyType() const override { return new WgTooltipLayer(); };

    
    bool            DeleteChild(WgWidget * pWidget) override { return 0; }
    WgWidget *        ReleaseChild(WgWidget * pWidget) override { return 0; }

    bool            DeleteAllChildren() override { return 0; }
    bool            ReleaseAllChildren() override { return 0; }
    
    //

    struct Placement
    {
        WgOrigo	    direction;
        WgBorders	spacing;
        bool	    bTooltipAroundPointer;
    };


    void SetTooltipGenerator( const std::function<WgWidget*(Placement& placement, const WgWidget* pHoveredWidget, const WgBorders& widgetMargins )> func );
    void SetTooltipPlacement(WgOrigo direction, WgBorders spacing, bool bTooltipAroundPointer = true );

    void SetDisplayTooltips(bool bDisplay);
    bool DisplayTooltips() const { return m_bDisplayTooltips;  }
    
    void SetPopupDelayTimer(int iMillisec) { m_hoverMillisec = iMillisec;}

    
    virtual WgWidget *     FindWidget( const WgCoord& ofs, WgSearchMode mode ) override;


protected:

    class TooltipHook : public WgLayerHook        /** @private */
    {
        friend class WgTooltipLayer;
    public:
        const char *Type(void) const;
        static const char * ClassType();

        // Standard Hook methods

        TooltipHook *    Prev() const { return nullptr; }
        TooltipHook *    Next() const { return nullptr; }

        WgTooltipLayer *    Parent() const;

    protected:
        TooltipHook(WgTooltipLayer * pParent) : m_pParent(pParent) {}

        void        _requestRender();
        void        _requestRender(const WgRect& rect);
        void        _requestResize();

        WgHook *    _prevHook() const { return nullptr; }
        WgHook *    _nextHook() const { return nullptr; }
        WgContainer* _parent() const { return m_pParent; }


        WgTooltipLayer * m_pParent;
    };
    
    void			_updateTooltipGeo();

    void			_openTooltip();
    void			_closeTooltip();

    static WgWidget *		_defaultTooltipGenerator(Placement& placement, const WgWidget*, const WgBorders&);

    // Overloaded from Widget

    void            _onCloneContent(const WgWidget * _pOrg) override;
    void            _onEvent(const WgEvent::Event * pEvent, WgEventHandler * pHandler) override;

    // Overloaded from Container

    void            _childRequestResize(TooltipHook * pSlot);

    // Overloaded from Layer

    WgLayerHook *   _firstLayerHook() const override;

    WgHook*         _firstHook() const override;        // Fist Hook returned is the normal child, then follows the modal ones.
    WgHook*         _lastHook() const override;        //

    WgHook *        _firstHookWithGeo(WgRect& geo) const override;
    WgHook *        _nextHookWithGeo(WgRect& geo, WgHook * pHook) const override;

    WgHook *        _lastHookWithGeo(WgRect& geo) const override;
    WgHook *        _prevHookWithGeo(WgRect& geo, WgHook * pHook) const override;

    
    
    TooltipHook		m_tooltipHook;

    int			m_hoverMillisec		= 300;				// Number of millisec to hover before tooltip is displayed.
    WgCoord		m_hoverPos			= { -1,-1 };
    int			m_hoverCountdown	= 0;
    WgWidget*	m_pHoverWidget = nullptr;

    int			m_openTooltipMaxMovement = 8;			// How many points the pointer may move in any direction
                                                        // from hoverPos once tooltip is displayed before it is closed.
    bool		m_bBlockReopen		= false;
    bool		m_bDisplayTooltips	= true;

    Placement	m_defaultPlacement = { WgOrigo::SouthEast, {2,16,16,2}, true };
    Placement	m_activePlacement;

    std::function<WgWidget*(Placement& placement, const WgWidget* pHovered, const WgBorders& margin)>	m_tooltipGenerator = _defaultTooltipGenerator;

};

#endif //WG2_POPUPLAYER_DOT_H
