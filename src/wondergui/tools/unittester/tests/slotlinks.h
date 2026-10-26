#include "../testframework.h"



class SlotLinksTest : public TestCollection
{
public:
	SlotLinksTest();
	~SlotLinksTest();

	std::string name() const override { return "SlotLinks Test"; };

	bool init(std::ostream& output) override;

private:

	bool packPanelSlotLinksTest(std::ostream& output);
	bool flexPanelSlotLinksTest(std::ostream& output);
};
