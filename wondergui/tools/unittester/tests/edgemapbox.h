#include "../testframework.h"

class EdgemapBoxTest : public TestCollection
{
public:
	EdgemapBoxTest();
	~EdgemapBoxTest();

	std::string name() const override { return "EdgemapBox Test"; };

private:

	bool accuracyTest(std::ostream& output);
	bool updateTest(std::ostream& output);
	bool nothingDrawnTest(std::ostream& output);
};
