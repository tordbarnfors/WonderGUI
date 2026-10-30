#include "../testframework.h"

class WarpmapTest : public TestCollection
{
public:
	WarpmapTest();
	~WarpmapTest();

	std::string name() const override { return "Warpmap Test"; };

private:

	bool defaultNodesTest(std::ostream& output);
	bool importExportTest(std::ostream& output);
	bool periodTest(std::ostream& output);
	bool circularWarpTest(std::ostream& output);
	bool circularWarpGenerateTest(std::ostream& output);
	bool roundedRectWarpTest(std::ostream& output);
};
