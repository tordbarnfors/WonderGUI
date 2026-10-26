//#include <conio.h>
#include "testframework.h"

#include <wondergui.h>

using namespace wg;

int main( int argc, char * argv[] )
{
	Base::init(nullptr);


	TestFramework	t(std::cout);

	int nFailed = t.runAllTests();

//	_getch();

	Base::exit();
	
	return nFailed;
}
