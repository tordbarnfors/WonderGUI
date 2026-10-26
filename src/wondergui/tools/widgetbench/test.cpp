
#include "test.h"

//____ instance() _____________________________________________________________

TestRegistry& TestRegistry::instance()
{
	static TestRegistry registry;
	return registry;
}

//____ add() ___________________________________________________________________

void TestRegistry::add(std::string name, TestBuildFunc build)
{
	m_tests.push_back({ std::move(name), std::move(build) });
}
