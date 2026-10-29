


#include "testframework.h"
#include <stdio.h>
#include <string.h>

#include "tests/gfxstream.h"
#include "tests/softgfxdevice.h"
#include "tests/softtint.h"
#include "tests/slotlinks.h"
#include "tests/qoi.h"
#include "tests/surfacefile.h"
#include "tests/softshapes.h"
#include "tests/shaperender.h"
#include "tests/edgemap.h"
#include "tests/edgemapring.h"
#include "tests/edgemapbox.h"
#include "tests/warpmap.h"
#include "tests/warprender.h"
#include "tests/linerender.h"





//____ Constructor _____________________________________________________________

TestFramework::TestFramework( std::ostream& output ) :
	m_log(output)
{
	addCollection( new GfxStreamTest() );
	addCollection( new SoftGfxDeviceTest() );
	addCollection( new SoftTintTest() );
	addCollection( new SlotLinksTest() );
	addCollection( new QOITest() );
	addCollection( new SurfaceFileTest() );
	addCollection( new SoftShapesTest() );
	addCollection( new ShapeRenderTest() );
	addCollection( new EdgemapTest() );
	addCollection( new EdgemapRingTest() );
	addCollection( new EdgemapBoxTest() );
	addCollection( new WarpmapTest() );
	addCollection( new WarpRenderTest() );
	addCollection( new LineRenderTest() );
}

//____ Destructor ______________________________________________________________

TestFramework::~TestFramework()
{
	for (auto p : m_testCollections)
		delete p;
}

//____ initAllTests() ________________________________________________________________

int TestFramework::initAllTests()
{
	m_log << "Initializing Tests" << std::endl;

	int nFailed = 0;
	for (auto p : m_testCollections)
	{
		if (!p->init(m_log))
			nFailed++;
	}

	return nFailed;
}

//____ runAllTests() ________________________________________________________________

int TestFramework::runAllTests()
{
	m_log << "Running all tests in the framework:\n";

	int nFailed = 0;
	for (auto p : m_testCollections)
	{
		for (int i = 0; i < p->nbTests(); i++)
		{
			if (!p->getTest(i).run(m_log))
				nFailed++;
		}
	}

	if( nFailed == 0 )
	{
		m_log << "\nAll tests ran successfully!\n\n" << std::flush;
	}
	else
	{
		m_log << "\n" << nFailed << " tests failed!\n\n" << std::flush;
	}

	return nFailed;
}

//____ runCollection() _________________________________________________________

int TestFramework::runCollection( int collIdx )
{
	TestCollection* pCollection = m_testCollections[collIdx];

	m_log << "Running all tests of collection '" << pCollection->name() << "':\n";

	int nFailed = 0;
	for (int i = 0; i < pCollection->nbTests(); i++)
	{
		if (!pCollection->getTest(i).run(m_log))
			nFailed++;
	}

	if (nFailed == 0)
	{
		m_log << "\nAll tests ran successfully!\n\n" << std::flush;
	}
	else
	{
		m_log << "\n" << nFailed << " tests failed!\n\n" << std::flush;
	}

	return nFailed;
}


//____ RunTest() _______________________________________________________________

bool TestFramework::runTest( int collIdx, int testIdx )
{
	TestCollection * p = m_testCollections[collIdx];

	Test& test = p->getTest(testIdx);

	m_log << "Running test '" << test.name() << "' of collection '" << p->name() << "':\n";
	bool bSuccess = test.run(m_log);

	return bSuccess;
}

//____ loadBlob() _________________________________________________________

Blob_p TestCollection::loadBlob(const std::string& path)
{
	FILE* fp;

#ifdef WIN32
	errno_t err = fopen_s(&fp, path.c_str(), "rb");
#else
	fp = fopen(path.c_str(), "rb");
#endif
	if (!fp)
		return 0;

	fseek(fp, 0, SEEK_END);
	int size = (int)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	Blob_p pBlob = Blob::create(size);

	int nRead = (int)fread(pBlob->data(), 1, size, fp);
	fclose(fp);

	if (nRead < size)
		return 0;

	return pBlob;
}
