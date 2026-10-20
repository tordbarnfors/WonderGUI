#include "../testframework.h"

class EdgemapTest : public TestCollection
{
public:
	EdgemapTest();
	~EdgemapTest();

	std::string name() const override { return "Edgemap Test"; };

private:

	bool importSamplesTest(std::ostream& output);
	bool importSamplesPitchTest(std::ostream& output);
};
