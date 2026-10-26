#include "../testframework.h"

class LineRenderTest : public TestCollection
{
public:
	LineRenderTest();
	~LineRenderTest();

	std::string name() const override { return "Line Render Test"; };

private:

	bool capsCodecTest(std::ostream& output);
	bool capsCommandTest(std::ostream& output);
	bool softVsReferenceTest(std::ostream& output);
	bool shortLinesTest(std::ostream& output);
	bool mixedCapsTest(std::ostream& output);
	bool extremesTest(std::ostream& output);
	bool linearBackendTest(std::ostream& output);
	bool clipTest(std::ostream& output);
	bool tintTest(std::ostream& output);
	bool blendModesTest(std::ostream& output);
	bool streamTest(std::ostream& output);
	bool cApiTest(std::ostream& output);
};
