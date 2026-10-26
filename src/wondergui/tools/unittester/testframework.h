#ifndef TESTFRAMEWORK_DOT_H
#define TESTFRAMEWORK_DOT_H



#include <wax_chain.h>
#include <wax_blob.h>
#include <wg_types.h>

#include <stdio.h>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <sstream>

using namespace wg;

//____ Macros __________________________________________________________________

#define DECL_TEST( _class_, _method_ )										\
	bool _method_(void);													\
	static bool _method_##cb(void * p) {return ((_class_*)p)->_method_();}

#define ADD_TEST( _method_ )												\
	m_tests.push_back( Test( #_method_, [this](std::ostream& output){ return this->_method_(output); } ) );

#define TEST_ASSERT( _expression_ )											\
	if( !(_expression_) )													\
	{																		\
		output << "Failed assert '" << #_expression_ << "' at line " << __LINE__ << " of " << __FILE__ << "." << std::endl; \
		return false;														\
	}




//____ Test ____________________________________________________________________


class Test
{
public:
	Test( std::string _name, const std::function<bool(std::ostream& output)>& func ) { m_name = _name; m_func = func; }

	std::string		name() const { return m_name; }
	bool			run(std::ostream& output) 
	{ 
		output << "    Running " << m_name << "...";

		std::ostringstream ss;

		bool success = m_func(ss);
		
		if (success)
			output << "  OK" << std::endl;
		else
		{

			output << "  FAILED!" << std::endl;
			output << "    " << ss.str() << std::endl;
		}

		return success; 
	}

protected:
	std::string				m_name;
	std::function<bool(std::ostream&)>	m_func;
};


//____ TestCollection __________________________________________________________

class TestCollection
{
public:
	TestCollection() {};
	virtual ~TestCollection() {};

	virtual bool	init(std::ostream& output) { return true; };

	int				nbTests() const { return m_tests.size(); }
	Test&			getTest(int index) { return m_tests[index]; }
	


	virtual std::string		name() const = 0;

protected:

	Blob_p 			loadBlob(const std::string& path);

	std::vector<Test>	m_tests;

};

//____ TestFramework ___________________________________________________________

class TestFramework
{
	friend class TestCollection;
public:
	TestFramework( std::ostream& output );
	~TestFramework();

	int				nbCollections() const { return m_testCollections.size(); }
	TestCollection* getCollection(int index) { return m_testCollections[index]; }

	int				initAllTests();

	int				runAllTests();
	int				runCollection( int collIdx );
	bool			runTest(int collIdx, int testIdx );

	int				addCollection(TestCollection* pCollection) { m_testCollections.push_back(pCollection); return m_testCollections.size() - 1; };

protected:

	std::vector<TestCollection*>	m_testCollections;

	std::ostream&					m_log;
};

#endif // TESTFRAMEWORK_DOT_H
