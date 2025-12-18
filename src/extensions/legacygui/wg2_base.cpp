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
#include <wg2_base.h>
#include <wg_gfxbase.h>
#include <wg_textstyle.h>
#include <wg_texttool.h>
#include <wg_textstylemanager.h>
#include <wg_memstack.h>
#include <wg_gfxdevice.h>
#include <wg_surface.h>
#include <wg_freetypefont.h>
#include <wg_dummyfont.h>

#include <wg2_widget.h>
#include <wg2_container.h>

#include <wg2_smartptr.h>

#include <iostream>


WgContext_p				WgBase::s_pContext = 0;
int WgBase::s_iSoftubeNumberOfInstances = 0;

bool						WgBase::s_bTrackingWidgets = false;
std::vector<WgWidget*>		WgBase::s_trackedWidgets;



//____ Init() __________________________________________________________________

void WgBase::Init( wg::HostBridge * pHostBridge)
{
	s_iSoftubeNumberOfInstances++;
	if(s_iSoftubeNumberOfInstances != 1)
		return;
	
	wg::GfxBase::init();

	s_pContext = new WgContext();

	s_pContext->pGfxContext = GfxBase::context();
	
	s_pContext->pHostBridge = pHostBridge;

	wg::TextTool::setDefaultBreakRules();
	wg::TextStyleManager::init();

	wg::TextStyle::Blueprint textStyleBP;
	textStyleBP.font = wg::DummyFont::create();
	s_pContext->pDefaultTextStyle = wg::TextStyle::create( textStyleBP );

	wg::TextStyle::s_pDefaultStyle = s_pContext->pDefaultTextStyle;
}

//____ Exit() __________________________________________________________________

void WgBase::Exit()
{
	s_iSoftubeNumberOfInstances--;
	if(s_iSoftubeNumberOfInstances != 0)
		return;

	s_pContext = nullptr;
	
	wg::TextStyleManager::exit();
	wg::GfxBase::exit();
}

//____ setContext() ________________________________________________________

WgContext_p WgBase::setContext( const WgContext_p& pNewContext )
{
	auto pOldContext = s_pContext;

	if( pNewContext )
	{
		s_pContext = pNewContext;

		wg::GfxBase::setContext(pNewContext->pGfxContext);
	}
	else
	{
		s_pContext = WgContext_p(new WgContext());

		wg::TextStyle::Blueprint textStyleBP;
		textStyleBP.font = wg::DummyFont::create();
		s_pContext->pDefaultTextStyle = wg::TextStyle::create( textStyleBP );

		wg::GfxBase::setContext(nullptr);
		s_pContext->pGfxContext = wg::GfxBase::context();
	}

	wg::TextStyle::s_pDefaultStyle = s_pContext->pDefaultTextStyle;

	return pOldContext;
}

//____ SetDefaultCursor() ___________________________________________________

void WgBase::SetDefaultCursor( WgCursor * pCursor )
{
	assert( s_pContext != 0 );
	s_pContext->pDefaultCursor = pCursor;
}

//____ SetDoubleClickTresholds() _______________________________________________

bool WgBase::SetDoubleClickTresholds( int time, int distance )
{
	assert( s_pContext != 0 );
	if( time <= 0 || distance <= 0 )
		return false;

	s_pContext->doubleClickTimeTreshold		= time;
	s_pContext->doubleClickDistanceTreshold	= distance;
	return true;
}

//____ SetMouseButtonRepeat() ______________________________________________________

bool WgBase::SetMouseButtonRepeat( int delay, int rate )
{
	assert( s_pContext != 0 );
	if( delay <= 0 || rate <= 0 )
		return false;

	s_pContext->buttonRepeatDelay	= delay;
	s_pContext->buttonRepeatRate	= rate;
	return true;
}

//____ SetKeyRepeat() _________________________________________________________

bool WgBase::SetKeyRepeat( int delay, int rate )
{
	assert( s_pContext != 0 );
	if( delay <= 0 || rate <= 0 )
		return false;

	s_pContext->keyRepeatDelay	= delay;
	s_pContext->keyRepeatRate	= rate;
	return true;
}

//____ MapKey() _______________________________________________________________

void WgBase::MapKey( WgKey translated_keycode, int native_keycode )
{
	assert( s_pContext != 0 );
	s_pContext->keycodeMap[native_keycode] = translated_keycode;
}


//____ UnmapKey() _____________________________________________________________

void WgBase::UnmapKey( WgKey translated_keycode )
{
	assert( s_pContext != 0 );
	std::map<int,WgKey>::iterator it = s_pContext->keycodeMap.begin();

	while( it != s_pContext->keycodeMap.end() )
	{
		if( it->second == translated_keycode )
		{
			std::map<int,WgKey>::iterator it2 = it++;
			s_pContext->keycodeMap.erase(it2);
		}
		else
			++it;
	}
}

//____ ClearKeyMap() __________________________________________________________

void WgBase::ClearKeyMap()
{
	assert( s_pContext != 0 );
	s_pContext->keycodeMap.clear();
}

//____ TranslateKey() __________________________________________________________

WgKey WgBase::TranslateKey( int native_keycode )
{
	assert( s_pContext != 0 );
	std::map<int,WgKey>::iterator it = s_pContext->keycodeMap.find(native_keycode);
	if( it != s_pContext->keycodeMap.end() )
		return  it->second;
	else
		return WG_KEY_UNMAPPED;
}

//____ setDefaultStyle() ______________________________________________________

void WgBase::setDefaultStyle( wg::TextStyle* pStyle )
{
	s_pContext->pDefaultTextStyle = pStyle;
	wg::TextStyle::s_pDefaultStyle = pStyle;
}

//____ setHostBridge() ________________________________________________________

void WgBase::setHostBridge( wg::HostBridge * pHostBridge )
{
	assert(s_pContext != 0);
	s_pContext->pHostBridge = pHostBridge;
}




//____ SetWidgetTracking() __________________________________________________________

void WgBase::SetWidgetTracking(bool bTracking)
{
	s_bTrackingWidgets = bTracking;
	if( !bTracking )
		s_trackedWidgets.clear();
}

//____ PrintWidgetTrees() _________________________________________________

void WgBase::PrintWidgetTrees(std::ostream& stream)
{
	stream << "Widget Trees" << std::endl;
	stream << "----------------" << std::endl;

	for( auto pWidget : s_trackedWidgets )
	{
		if( !pWidget->Parent() )
		{
			PrintWidgetTreeRecursively(stream, pWidget, 1);
		}
	}
}

//____ PrintWidgetTreeRecursively() _____________________________________________________

void WgBase::PrintWidgetTreeRecursively(std::ostream& stream, WgWidget * pWidget, int recursion )
{
	auto geo = pWidget->PointGeo();
	
	char strBuffer[1024];
	
	snprintf(strBuffer,1024, "%s %s id: %d, geo: {%d,%d,%d,%d}\n", std::string(recursion*2, ' ').c_str(), pWidget->Type(), pWidget->Id(), geo.x, geo.y, geo.w, geo.h );
	
	stream << strBuffer;
	
	
//	stream << std::string(recursion*2, ' ') << std::string(pWidget->Type()) << " Id: " << std::to_string(pWidget->Id()) << std::endl;

	if( pWidget->IsContainer() )
	{
		WgContainer * pContainer = pWidget->CastToContainer();
		WgWidget * pChild = pContainer->FirstWidget();
		
		while( pChild )
		{
			PrintWidgetTreeRecursively(stream, pChild, recursion + 1);
			pChild = pChild->NextSibling();
		}
	}
}
