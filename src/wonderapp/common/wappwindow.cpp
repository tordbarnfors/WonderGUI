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

	DynamicSlot* pSlot = &m_pRoot->slot;

	Overlay_p lastOverlay = nullptr;

	if(result.debugger && blueprint.debug)
	{
		auto pDebugCapsule = DebugCapsule::create( { .frontend = result.debugger });
		m_pRoot->slot = pDebugCapsule;
		pSlot = &pDebugCapsule->slot;
	}

	if (blueprint.tooltipOverlay)
	{
		auto pOverlay = TooltipOverlay::create();
		pSlot->setWidget(pOverlay);
		pSlot = &pOverlay->mainSlot;
	}

	if( blueprint.dndOverlay )
	{
		auto pOverlay = DragNDropOverlay::create();
		pSlot->setWidget(pOverlay);
		pSlot = &pOverlay->mainSlot;
	}

	if( blueprint.popupOverlay )
	{
		auto pOverlay = PopupOverlay::create();
		pSlot->setWidget(pOverlay);
		pSlot = &pOverlay->mainSlot;
	}

	if( blueprint.modalOverlay )
	{
		auto pOverlay = ModalOverlay::create();
		pSlot->setWidget(pOverlay);
		pSlot = &pOverlay->mainSlot;
	}

	m_pMainCapsule = RenderLayerCapsule::create();
	pSlot->setWidget(m_pMainCapsule);
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
	m_pSysCalls->setTitle(title);
	return false;
}

//____ title() _______________________________________________________________

std::string Window::title() const
{
	return m_pSysCalls->title();
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

