#include "../testframework.h"

class EdgemapWaveTest : public TestCollection
{
public:
	EdgemapWaveTest();
	~EdgemapWaveTest();

	std::string name() const override { return "EdgemapWave Test"; };

private:

	bool heldEdgemapTest(std::ostream& output);
	bool drawnTwiceTest(std::ostream& output);
};
