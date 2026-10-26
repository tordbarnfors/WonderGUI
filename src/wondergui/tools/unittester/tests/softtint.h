#include "../testframework.h"



class SoftTintTest : public TestCollection
{
public:
	SoftTintTest();
	~SoftTintTest();

	std::string name() const override { return "SoftTint Test"; };

	bool init(std::ostream& output) override;

private:

	bool lookupTableTest(std::ostream& output);
	bool tintRowsTest(std::ostream& output);
	bool tintedRenderingTest(std::ostream& output);
};
