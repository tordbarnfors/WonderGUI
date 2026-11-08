#include "../testframework.h"

class WarpmapTest : public TestCollection
{
public:
	WarpmapTest();
	~WarpmapTest();

	std::string name() const override { return "Warpmap Test"; };

private:

	bool defaultMeshTest(std::ostream& output);
	bool importExportTest(std::ostream& output);
	bool createWithMeshTest(std::ostream& output);
	bool periodTest(std::ostream& output);
	bool fillRuleTest(std::ostream& output);
	bool perspectivePositionTest(std::ostream& output);
	bool circularWarpTest(std::ostream& output);
	bool circularWarpGenerateTest(std::ostream& output);
	bool generateUpdatesTest(std::ostream& output);
	bool roundedRectWarpTest(std::ostream& output);
	bool meshCompressionTest(std::ostream& output);
	bool offsetTest(std::ostream& output);
	bool nonSquareCellsTest(std::ostream& output);
	bool cylinderWarpTest(std::ostream& output);
	bool perspectiveWarpTest(std::ostream& output);
	bool perspectiveRotatedTest(std::ostream& output);
	bool limitsTest(std::ostream& output);
};
