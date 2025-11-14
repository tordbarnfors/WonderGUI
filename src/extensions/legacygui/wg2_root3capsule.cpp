#include "wg2_root3capsule.h"

static const char c_widgetType[] = {"WgRoot3Capsule"};

WgRoot3Capsule::WgRoot3Capsule()
: WgWidget(), wg::RootPanel()
{}

const char *WgRoot3Capsule::Type(void) const
{
	return c_widgetType;
}

bool WgRoot3Capsule::MarkTest(const WgCoord& ofs)
{
	if(slot.widget() != nullptr)
	{
		return slot->markTest(wg::Coord(ofs));
	}
	return true;
}

int WgRoot3Capsule::MatchingPixelHeight(int pixelWidth) const
{
	if(slot.widget() != nullptr)
	{
		return slot->_matchingHeight(pixelWidth*64, RootPanel::m_scale) / 64;
	}
	return 0;
}

int WgRoot3Capsule::MatchingPixelWidth(int pixelHeight) const
{
	if(slot.widget() != nullptr)
	{
		return slot->_matchingWidth(pixelHeight*64, RootPanel::m_scale) / 64;
	}
	return 0;
}

WgSize WgRoot3Capsule::PreferredPixelSize() const
{
	if(slot.widget() != nullptr)
	{
		return slot->_defaultSize(RootPanel::m_scale) / 64;
	}
	return {0, 0};
}

bool WgRoot3Capsule::addPreRenderCall(wg::Widget* pWidget)
{
	// Also notify WG2 holder
	if(_requestPreRenderCall())
	{
		RootPanel::addPreRenderCall(pWidget);
		return true;
	}
	return false;
}

void WgRoot3Capsule::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	WgWidget::_onRender(pDevice, _canvas, _window);

	m_skin.render(pDevice, _canvas * 64, wg::RootPanel::m_scale, wg::State::Default);

	if( !slot.isEmpty() )
		slot._widget()->_render( pDevice, _canvas * 64, _canvas * 64 );

}

void WgRoot3Capsule::_onNewSize( const WgSize& size )
{
	WgWidget::_onNewSize(size);
	if(slot.widget() != nullptr)
	{
		slot->_resize(size * 64, RootPanel::m_scale);
	}
}

void WgRoot3Capsule::_setScale( int scale )
{
	WgWidget::_setScale(scale);
	setScale(scale / 64);
}

void WgRoot3Capsule::_onEvent( const WgEvent::Event * pEvent, WgEventHandler * pHandler )
{
	WgWidget::_onEvent(pEvent, pHandler);
}

void WgRoot3Capsule::_setState( WgState state )
{
	WgWidget::_setState(state);
}

void WgRoot3Capsule::_childRequestResize( wg::StaticSlot * pSlot )
{
	RootPanel::_childRequestResize(pSlot);
	_requestResize();
}

void WgRoot3Capsule::_childRequestRender(wg::StaticSlot* pSlot, const wg::RectSPX& rect)
{
	if (m_bVisible)
	{
		// Pass it on to legacy parent
		WgWidget::_requestRender(rect / 64);
	}
}

void WgRoot3Capsule::_replaceChild(wg::StaticSlot * pSlot, wg::Widget * pNewChild)
{
	RootPanel::_replaceChild(pSlot, pNewChild);
	_requestResize();
}

void WgRoot3Capsule::_preRender()
{
	for (auto& pWidget : m_preRenderCalls)
		pWidget->_preRender();

	m_preRenderCalls.clear();
}
