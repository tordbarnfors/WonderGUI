#include "../testframework.h"



class GfxStreamTest : public TestCollection
{
public:
	GfxStreamTest();
	~GfxStreamTest();

	std::string name() const override { return "GfxStream Test"; };

	bool init(std::ostream& output) override;

private:

	bool streamLoopWrapperTest(std::ostream& output);
	bool streamReaderPumpWithOptimizationTest(std::ostream& output);

	Blob_p	_createTestStream();
};
