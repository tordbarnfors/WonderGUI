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
#ifndef WG_CUSTOMSKIN_DOT_H
#define WG_CUSTOMSKIN_DOT_H
#pragma once

#include <wg_skin.h>
#include <functional>

namespace wg
{

	class LambdaSkin;

	typedef	StrongPtr<LambdaSkin>	LambdaSkin_p;
	typedef	WeakPtr<LambdaSkin>	LambdaSkin_wp;


	class LambdaSkin : public Skin
	{
	public:

		typedef std::function<bool(const CoordSPX&,const RectSPX&,int,State,float,float,int)> MarkTestFunc;
		typedef std::function<void(GfxDevice*,const RectSPX&,int,State,float,float)> RenderFunc;

		//____ Blueprint ______________________________________________________

		struct Blueprint
		{
			Size			defaultSize;
			Finalizer_p		finalizer = nullptr;
			bool			ignoreState = false;
			bool			ignoreValue = false;
			int				layer = -1;
			int				markAlpha = 1;
			MarkTestFunc	markTestFunc;
			bool			opaque = false;
			Border			overflow;
			Border			padding;
			RenderFunc		renderFunc;
			Border			spacing;
		};

		//.____ Creation __________________________________________

		static LambdaSkin_p create( const Blueprint& blueprint );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Internal ____________________________________________________

		bool		_markTest(	const CoordSPX& ofs, const RectSPX& canvas, int scale, State state,
								float value = 1.f, float value2 = -1.f, int alphaOverride = -1) const override;

		void		_render(	GfxDevice * pDevice, const RectSPX& canvas, int scale, State state,
							float value = 1.f, float value2 = -1.f, int animPos = 0, float* pStateFractions = nullptr) const override;

		SizeSPX		_defaultSize(int scale) const override;

		RectSPX		_dirtyRect(const RectSPX& canvas, int scale, State newState, State oldState,
									float newValue = 1.f, float oldValue = 1.f,
									float newValue2 = -1.f, float oldValue2 = -1.f,
									int newAnimPos = 0, int oldAnimPos = 0,
									float* pNewStateFractions = nullptr, float* pOldStateFractions = nullptr) const override;

		RectSPX		_coverage(const RectSPX& geo, int scale, State state) const override;

	protected:
		LambdaSkin( const Blueprint& blueprint );
		~LambdaSkin() {};

		MarkTestFunc	m_markTestFunc;
		RenderFunc		m_renderFunc;
		Size			m_defaultSize;
		bool			m_bOpaque;
	};


} // namespace wg
#endif //WG_CUSTOMSKIN_DOT_H
