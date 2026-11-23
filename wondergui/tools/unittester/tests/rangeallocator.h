#include "../testframework.h"



class RangeAllocatorTest : public TestCollection
{
public:
	RangeAllocatorTest();
	~RangeAllocatorTest();

	std::string name() const override { return "RangeAllocator Test"; };

	bool init(std::ostream& output) override;

private:

	bool allocTest(std::ostream& output);
	bool alignmentTest(std::ostream& output);
	bool joinTest(std::ostream& output);
	bool badFreeTest(std::ostream& output);
	bool randomTest(std::ostream& output);
};
