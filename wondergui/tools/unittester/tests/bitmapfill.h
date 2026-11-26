#include "../testframework.h"

// The fill color of Bitmap::Blueprint: what a bitmap created without pixels starts out
// with, and how it gets across a stream.

class BitmapFillTest : public TestCollection
{
public:
	BitmapFillTest();
	~BitmapFillTest();

	std::string name() const override { return "BitmapFill Test"; };

	bool init(std::ostream& output) override;

private:

	bool softBitmapTest(std::ostream& output);
	bool suppliedPixelsTest(std::ostream& output);
	bool streamTest(std::ostream& output);
	bool oldStreamTest(std::ostream& output);
};
