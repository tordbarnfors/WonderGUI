#include "../testframework.h"



class SoftGfxDeviceTest : public TestCollection
{
public:
	SoftGfxDeviceTest();
	~SoftGfxDeviceTest();

	std::string name() const override { return "SoftGfxDevice Test"; };

	bool init(std::ostream& output) override;

private:

	bool drawElipseTest(std::ostream& output);
};
