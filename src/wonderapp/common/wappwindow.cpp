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

#include "wappwindow.h"
#include "wappapi.h"
#include <wg_debugcapsule.h>
#include <wg_debugfrontend.h>

using namespace wg;

namespace wapp
{


const TypeInfo Window::TYPEINFO = { "Window", &Object::TYPEINFO };

//____ create() _______________________________________________________________

Window_p Window::create(API* pAPI, const Blueprint& blueprint)
{
	if (!pAPI)
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Window::create() - pAPI is nullptr.", nullptr, &TYPEINFO, __func__, __FILE__, __LINE__);
		return nullptr;
	}

	Window_p p = new Window(pAPI, blueprint);

	if( p->m_pSysCalls == nullptr )
		return nullptr;

	return p;
}

//____ constructor _____________________________________________________________

Window::Window(API* pAPI, const Blueprint& blueprint)
{
	if( blueprint.finalizer )
		setFinalizer(blueprint.finalizer);

	auto result = pAPI->_createWindow( this,
										blueprint.origin,
										blueprint.pos,
										blueprint.size,
										blueprint.title,
										blueprint.resizable,
										blueprint.open);

	if( !result.success )
	{
		Base::throwError(ErrorLevel::Error, ErrorCode::Internal, result.errorMsg.c_str(), this, &TYPEINFO, __func__, __FILE__, __LINE__);
		m_pSysCalls = nullptr;
		return;
	}

	m_geo = result.geo;
	m_pSysCalls = result.pSysCalls;
	m_pRoot = result.root;

	Overlay_p lastOverlay = nullptr;

	if (blueprint.tooltipOverlay)
	{
		auto pTooltipOverlay = TooltipOverlay::create();
		m_pRoot->slot = pTooltipOverlay;
		lastOverlay = pTooltipOverlay;
	}

	if( blueprint.dndOverlay )
	{
		auto pDragNDropOverlay = DragNDropOverlay::create();

		if( lastOverlay )
			lastOverlay->mainSlot = pDragNDropOverlay;
		else
			m_pRoot->slot = pDragNDropOverlay;

		lastOverlay = pDragNDropOverlay;
	}

	if( blueprint.popupOverlay )
	{
		auto pPopupOverlay = PopupOverlay::create();
		if( lastOverlay )
			lastOverlay->mainSlot = pPopupOverlay;
		else
			m_pRoot->slot = pPopupOverlay;
		lastOverlay = pPopupOverlay;
	}

	if( blueprint.modalOverlay )
	{
		auto pModalOverlay = ModalOverlay::create();
		if( lastOverlay )
			lastOverlay->mainSlot = pModalOverlay;
		else
			m_pRoot->slot = pModalOverlay;
		lastOverlay = pModalOverlay;
	}

	m_pMainCapsule = RenderLayerCapsule::create();
	if( lastOverlay )
		lastOverlay->mainSlot = m_pMainCapsule;
	else
		m_pRoot->slot = m_pMainCapsule;
}

//____ destructor _____________________________________________________

Window::~Window()
{
	m_pSysCalls->destroy();
}


//____ typeInfo() _________________________________________________________

const wg::TypeInfo& Window::typeInfo(void) const
{
	return TYPEINFO;
}


//____ setGeo() _____________________________________________________________

wg::Rect Window::setGeo(const wg::Rect& geo)
{
	m_geo = _updateWindowGeo(geo);
	return m_geo;
}

//____ setPos() _____________________________________________________________

wg::Coord Window::setPos(wg::Coord pos)
{
	m_geo = _updateWindowGeo({ pos, m_geo.size() } );
	return m_geo.pos();
}

//____ setSize() _____________________________________________________________

wg::Size Window::setSize(wg::Size size)
{
	m_geo = _updateWindowGeo({ m_geo.pos(), size });
	return m_geo.size();
}

//____ setTitle() ____________________________________________________________

bool Window::setTitle(std::string& title)
{
	//TODO: Implement!
	return false;
}

//____ title() _______________________________________________________________

std::string Window::title() const
{
	//TODO: Implement!
	return "";
}

//____ setIcon() _____________________________________________________________

bool Window::setIcon(wg::Surface* pIcon)
{
	//TODO: Implement!
	return false;
}

//____ _updateWindowGeo() ___________________________________________________

wg::Rect Window::_updateWindowGeo(const wg::Rect& geo)
{
	//TODO: Implement!
	return geo;
}


//____ _setDebugger() ______________________________________________________

void Window::_setDebugger( DebugFrontend * pDebugger )
{
/*
	auto pNextWidget = m_pRootPanel->slot.widget();
	auto pDebugCapsule = DebugCapsule::create( { .frontend = pDebugger });

	m_pRootPanel->slot = pDebugCapsule;
	pDebugCapsule->slot = pNextWidget;
*/
}
//____ onClose() ____________________________________________________________

bool Window::onClose()
{
	//TODO: Implement!
	return true;
}

//____ adjustSize() _________________________________________________________

wg::Size Window::adjustSize(wg::Size size)
{
	return size;
}


//____ onResize() __________________________________________________________

void Window::onResize(wg::Size size)
{
	//TODO: Implement!
}

//____ onMove() ____________________________________________________________

void Window::onMove(wg::Coord pos)
{
	//TODO: Implement!
}

//____ onFocusGained() ____________________________________________________

void Window::onFocusGained()
{
	//TODO: Implement!
}

//____ onFocusLost() ______________________________________________________

void Window::onFocusLost()
{
	//TODO: Implement!
}

//____ onMinimized() ______________________________________________________

void Window::onMinimized()
{
	//TODO: Implement!
}

//____ onRestored() ______________________________________________________

void Window::onRestored()
{
	//TODO: Implement!
}

} // namespace wapp

