//#include <conio.h>
#include "testframework.h"

#include <wondergui.h>

using namespace wg;

int main( int argc, char * argv[] )
{
	Base::init(nullptr);


	TestFramework	t(std::cout);

	int nFailed = 0;

	if( argc > 1 )
	{
		// Only run collections whose name contains the argument, like "TextSystem".

		for( int i = 0 ; i < t.nbCollections() ; i++ )
			if( t.getCollection(i)->name().find(argv[1]) != std::string::npos )
				nFailed += t.runCollection(i);
	}
	else
		nFailed = t.runAllTests();

//	_getch();

	Base::exit();
	
	return nFailed;
}
