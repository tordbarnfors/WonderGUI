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
#ifndef WG_TESTWIDGET_DOT_H
#define WG_TESTWIDGET_DOT_H


#include <wg_widget.h>

namespace wg
{
	class TestWidget;
	typedef	StrongPtr<TestWidget>		TestWidget_p;
	typedef	WeakPtr<TestWidget>       TestWidget_wp;


	//____ TestWidget ____________________________________________________________

	class TestWidget : public Widget
	{
	public:
		static TestWidget_p	create() { return TestWidget_p(new TestWidget()); }
	
		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		void	start();
		void	stop();

		Size			preferredSize() const override;


	protected:
		TestWidget();
		virtual ~TestWidget();

		virtual Widget* _newOfMyType() const { return new TestWidget(); };
	
		void			_cloneContent( const Widget * _pOrg ) override;
		void			_update(int microPassed, int64_t microsecTimestamp) override;
		void			_render( GfxDevice * pDevice, const Rect& _canvas, const Rect& _window ) override;

	private:

		CoordI 	m_coord[2];
		int		m_speed[2];
		bool	m_bPointsInitialized;
		bool	m_bAnimated;
        
	};

} // namespace wg

#endif //WG_TESTWIDGET_DOT_H
