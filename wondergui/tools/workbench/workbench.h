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
#ifndef WORKBENCH_DOT_H
#define WORKBENCH_DOT_H
#pragma once

//____ Workbench ______________________________________________________________
//
// A place to try things out. Each test builds some widgets, which the workbench
// shows next to a list of all tests. Pick a test in the list to run it.
//
// To add a test, put it in a .cpp file in tests/ (a new file is picked up the
// next time CMake runs):
//
//     #include "../workbench.h"
//
//     using namespace wg;
//
//     WORKBENCH_TEST("Skins", "Rounded box")
//     {
//         return Filler::create({ .skin = BoxSkin::create({ .cornerRadius = 10 }) });
//     }
//
// The test gets a workbench::Context, ctx, and returns the widget to show. The
// widget fills the test area, so wrap it in a capsule or panel to give it a
// size of its own.
//
// Routes, update callbacks and exit callbacks added through ctx are removed
// when the test is left, so a test can be run again and again.
//
// Keys:
//     F5                      Run the current test again.
//     F12                     Show or hide the debugger.
//     Ctrl+PageUp/PageDown    Previous/next test.
//
// The test to start with can be given on the command line, as "Group/Name" or
// any part of it. Otherwise the test that was run last is started.
//
// Software rendering through SoftBackend, so all tests run the same everywhere.

#include <wondergui.h>

#include <functional>
#include <string>
#include <vector>

namespace workbench
{
	//____ Context ____________________________________________________________

	class Context
	{
		friend class App;

	public:

		// Loads from the resources directory.

		wg::Surface_p	loadSurface(const std::string& path, const wg::Surface::Blueprint& blueprint = wg::Surface::Blueprint());
		wg::Blob_p		loadBlob(const std::string& path);

		// Removed when the test is left.

		wg::RouteId		addRoute(wg::Object* pSource, wg::MsgType type, std::function<void(wg::Msg* pMsg)> func);
		wg::RouteId		addRoute(wg::MsgType type, std::function<void(wg::Msg* pMsg)> func);

		// Called once per frame with the time in microseconds, while the test runs.

		void			onUpdate(std::function<void(int64_t microsec)> func);

		// Called when the test is left, before its widgets are released.

		void			onExit(std::function<void()> func);

	private:

		Context() {}
		~Context();

		void			_update(int64_t microsec);

		std::vector<wg::RouteId>						m_routes;
		std::vector<std::function<void(int64_t)>>		m_updateFuncs;
		std::vector<std::function<void()>>				m_exitFuncs;
	};

	//____ Test _______________________________________________________________

	typedef	wg::Widget_p(*TestFunc)(Context& ctx);

	struct Test
	{
		std::string		group;
		std::string		name;
		TestFunc		func;
	};

	// All tests, sorted by group and name.

	const std::vector<Test>&	tests();

	bool			registerTest(const char* pGroup, const char* pName, TestFunc func);
}

#define WORKBENCH_CONCAT2(a, b)	a##b
#define WORKBENCH_CONCAT(a, b)	WORKBENCH_CONCAT2(a, b)

#define WORKBENCH_TEST(group, name)\
	static wg::Widget_p	WORKBENCH_CONCAT(workbenchTest_, __LINE__)(workbench::Context& ctx);\
	static bool			WORKBENCH_CONCAT(workbenchRegistered_, __LINE__) = workbench::registerTest(group, name, &WORKBENCH_CONCAT(workbenchTest_, __LINE__));\
	static wg::Widget_p	WORKBENCH_CONCAT(workbenchTest_, __LINE__)([[maybe_unused]] workbench::Context& ctx)

#endif //WORKBENCH_DOT_H
