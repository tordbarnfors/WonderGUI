#include "../testframework.h"



class BitmapFileTest : public TestCollection
{
public:
	BitmapFileTest();
	~BitmapFileTest();

	std::string name() const override { return "BitmapFile Test"; };

	bool init(std::ostream& output) override;

private:

	bool paddedPixelsTest(std::ostream& output);
	bool compressedPaletteTest(std::ostream& output);
};
