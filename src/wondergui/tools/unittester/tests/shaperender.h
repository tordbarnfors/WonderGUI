
#include "../testframework.h"

class ShapeRenderTest : public TestCollection
{
public:
	ShapeRenderTest();
	~ShapeRenderTest();

	std::string name() const override { return "Shape Render Test"; };

private:

	bool blendModesTest(std::ostream& output);
	bool fillAndOutlineTest(std::ostream& output);
	bool tintColorTest(std::ostream& output);
	bool gradientTintTest(std::ostream& output);
	bool clipListTest(std::ostream& output);
	bool linearBackendTest(std::ostream& output);
	bool streamTest(std::ostream& output);
	bool cApiTest(std::ostream& output);
	bool alignedBoxTest(std::ostream& output);
	bool boxSkinTest(std::ostream& output);
	bool ringTest(std::ostream& output);
	bool undefinedColorTest(std::ostream& output);
	bool elipseTest(std::ostream& output);
	bool referenceCoverageTest(std::ostream& output);
};
