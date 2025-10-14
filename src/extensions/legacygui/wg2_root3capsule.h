#pragma once

#include <wg_widget.h>
#include <wg2_widget.h>
#include <wg_rootpanel.h>

// The purpose of this class is to allow for modern WG widgets in a legacy WG
// widget hierarchy. In order for this to work, wg::Base needs to have a proper
// context and wg::Base::update has to be called in order for update receivers
// to work. The contexts of wg::Base and WgBase has to be compatible with one
// another (it's recommended that the GfxContext of both has the same members).

// There is currently no user interaction implemented, so clicking or hovering
// the modern WG widgets does not do anything.

class WgRoot3Capsule : public WgWidget, public wg::RootPanel
{
public:
    // Legacy WG methods (overrides WgWidget methods)
	WgRoot3Capsule();
	const char *Type( void ) const override;
	bool MarkTest( const WgCoord& ofs ) override;
	WgWidget * NewOfMyType() const override { return new WgRoot3Capsule(); }

	int MatchingPixelHeight( int pixelWidth ) const override;
	int MatchingPixelWidth( int pixelHeight ) const override;

	WgSize PreferredPixelSize() const override;

    // Modern WG methods (overrides wg::RootPanel methods)
	bool addPreRenderCall(wg::Widget* pWidget) override;

protected:
    // Legacy WG methods (overrides WgWidget methods)
    void _onCloneContent( const WgWidget * _pOrg ) override;
	void _onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window ) override;
	void _onNewSize( const WgSize& size ) override;
	void _setScale( int scale ) override;
	void _onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler );
	void _setState( WgState state ) override;

    // Modern WG methods (overrides wg::RootPanel methods)
	void _childRequestResize( wg::StaticSlot * pSlot ) override;
	void _childRequestRender( wg::StaticSlot* pSlot, const wg::RectSPX& rect ) override;
	void _replaceChild( wg::StaticSlot * pSlot, wg::Widget * pNewChild ) override;
    void _preRender() override;


	wg::Surface_p m_canvasSurface;
};