
#pragma once

// ---------------------------------------------------------------------------
// Test registry.
//
// A "test" is a name plus a function that builds some widget(s) to look at
// and poke at. To add a new one:
//
//   1. Add a new .cpp file under tests/ (tests/text_test.cpp is the
//      shortest example to copy from).
//   2. Write a build(TestContext&) function that fills ctx.pContent->slot
//      with whatever you want to try out.
//   3. Call REGISTER_TEST("Some Name", build) at file scope.
//   4. Add the file to TEST_SOURCES in CMakeLists.txt.
//
// widgetbench.h/.cpp and this file never need to change - the test shows up
// in the picker automatically, sorted by name.
// ---------------------------------------------------------------------------

#include <wonderapp.h>
#include <wondergui.h>

#include <functional>
#include <string>
#include <vector>

// Everything a test gets handed. Extend this struct (not the individual
// tests) when a test needs something new from the framework.

struct TestContext
{
	wapp::API *		pAPI;		// Platform API: load resources, show dialogs, get the resource dir, etc.
	wg::Capsule_p	pContent;	// Fill pContent->slot with the widget(s) to test. Cleared for you before build() runs.
};

using TestBuildFunc = std::function<void(TestContext&)>;

struct Test
{
	std::string		name;
	TestBuildFunc	build;
};


class TestRegistry
{
public:
	static TestRegistry&	instance();

	void			add(std::string name, TestBuildFunc build);

	const std::vector<Test>& tests() const { return m_tests; }

private:
	std::vector<Test>	m_tests;
};


// Macro concatenation needs one extra indirection to expand __LINE__ first.

#define WGBENCH_CONCAT_(a,b) a##b
#define WGBENCH_CONCAT(a,b) WGBENCH_CONCAT_(a,b)

// Drop this at file scope in a tests/*.cpp file to register a test at
// static-init time. No edits needed anywhere else but CMakeLists.txt.

#define REGISTER_TEST(displayName, buildFunc)								\
	namespace {																\
		const bool WGBENCH_CONCAT(g_wgbenchTestRegistered_, __LINE__) =	\
			(TestRegistry::instance().add((displayName), (buildFunc)), true); \
	}
