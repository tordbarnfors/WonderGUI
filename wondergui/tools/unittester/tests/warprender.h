#include "../testframework.h"

class WarpRenderTest : public TestCollection
{
public:
	WarpRenderTest();
	~WarpRenderTest();

	std::string name() const override { return "Warp Render Test"; };

private:

	bool identityEdgemapTest(std::ostream& output);
	bool identityBlitTest(std::ostream& output);
	bool softVsReferenceTest(std::ostream& output);
	bool blendModesTest(std::ostream& output);
	bool linearBackendTest(std::ostream& output);
	bool offsetPeriodTest(std::ostream& output);
	bool clipTest(std::ostream& output);
	bool opaqueCanvasTest(std::ostream& output);
	bool streamTest(std::ostream& output);
	bool cApiTest(std::ostream& output);
	bool seamColorTest(std::ostream& output);
	bool sourceRectTest(std::ostream& output);
	bool generateTest(std::ostream& output);
};
