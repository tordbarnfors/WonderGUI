#include "../testframework.h"



class SurfaceFileTest : public TestCollection
{
public:
	SurfaceFileTest();
	~SurfaceFileTest();

	std::string name() const override { return "SurfaceFile Test"; };

	bool init(std::ostream& output) override;

private:

	bool paddedPixelsTest(std::ostream& output);
	bool compressedPaletteTest(std::ostream& output);
};
