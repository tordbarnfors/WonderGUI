#include "wg2_root3capsule.h"

static const char c_widgetType[] = {"WgRoot3Capsule"};

WgRoot3Capsule::WgRoot3Capsule(wg::HiColor clearColor)
: WgWidget(), wg::RootPanel(), m_clearColor{clearColor}
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
	RootPanel::addPreRenderCall(pWidget);
	// Also notify WG2 holder
	_requestPreRenderCall();
	return true;
}

void WgRoot3Capsule::_onCloneContent( const WgWidget * _pOrg )
{
	// TODO: what?
}

void WgRoot3Capsule::_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window )
{
	WgWidget::_onRender(pDevice, _canvas, _window);
	if(m_canvasSurface == nullptr || m_canvasSurface->pixelSize() != _canvas.size())
	{
		m_canvasSurface = pDevice->surfaceFactory()->createSurface(WGBP(Surface,
			_.size = _canvas.size(),
			_.format = wg::PixelFormat::BGRA_8,
			_.canvas = true
		));
		m_canvasSurface->fill(wg::HiColor::Transparent);
		RootPanel::setCanvas(m_canvasSurface);
	}
	if(pDevice != m_pGfxDevice)
	{
		RootPanel::setGfxDevice(pDevice);
	}

	RootPanel::beginRender(); // Does the pre-render calls and such, if any

	// If clear color is set, clear canvas where it is going to be drawn on
	// (otherwise this widget has to be completely opaque in order to render
	// correctly)
	if(m_dirtyPatches.size() > 0 && m_clearColor != wg::HiColor::Undefined)
	{
		pDevice->beginCanvasUpdate(m_canvasSurface, m_dirtyPatches.size(), m_dirtyPatches.begin());
		pDevice->setBlendMode(wg::BlendMode::Replace);

		pDevice->fill(m_clearColor);

		pDevice->setBlendMode(wg::BlendMode::Blend);
		pDevice->endCanvasUpdate();
	}
	RootPanel::renderSection(geo());

	// These things are otherwise done in RootPanel::endRender()
	m_updatedPatches.clear();
	m_updatedPatches.add(&m_dirtyPatches);
	m_dirtyPatches.clear();

	// Blit the result
	pDevice->setBlitSource(m_canvasSurface);
	pDevice->stretchBlit(_canvas * 64);
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
		addDirtyPatch(rect + m_geo.pos());
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
