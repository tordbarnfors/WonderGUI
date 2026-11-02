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
	bool createWithNodesTest(std::ostream& output);
	bool periodTest(std::ostream& output);
	bool circularWarpTest(std::ostream& output);
	bool circularWarpGenerateTest(std::ostream& output);
	bool generateUpdatesTest(std::ostream& output);
	bool roundedRectWarpTest(std::ostream& output);
	bool nodeCompressionTest(std::ostream& output);
	bool offsetTest(std::ostream& output);
	bool nonSquareCellsTest(std::ostream& output);
};
