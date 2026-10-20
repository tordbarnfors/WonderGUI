#include "../testframework.h"

class EdgemapRingTest : public TestCollection
{
public:
	EdgemapRingTest();
	~EdgemapRingTest();

	std::string name() const override { return "EdgemapRing Test"; };

private:

	bool accuracyTest(std::ostream& output);
	bool updateTest(std::ostream& output);
	bool nothingDrawnTest(std::ostream& output);
};
