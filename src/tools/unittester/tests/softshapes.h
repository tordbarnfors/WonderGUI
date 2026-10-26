
#include "../testframework.h"

#include <wax_softshapes.h>

class SoftShapesTest : public TestCollection
{
public:
	SoftShapesTest();
	~SoftShapesTest();

	std::string name() const override { return "SoftShapes Test"; };

private:

	bool roundedBoxCoverageTest(std::ostream& output);
	bool roundedBoxSeamTest(std::ostream& output);
	bool roundedBoxSymmetryTest(std::ostream& output);
	bool roundedBoxClipTest(std::ostream& output);
	bool clampCornerRadiiTest(std::ostream& output);
	bool ringCoverageTest(std::ostream& output);
	bool ringClipTest(std::ostream& output);
	bool ringWithoutBackTest(std::ostream& output);
	bool mirrorBufferTest(std::ostream& output);
};
