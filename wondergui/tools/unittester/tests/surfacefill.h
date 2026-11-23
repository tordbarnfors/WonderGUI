#include "../testframework.h"

// The fill color of Surface::Blueprint: what a surface created without pixels starts out
// with, and how it gets across a stream.

class SurfaceFillTest : public TestCollection
{
public:
	SurfaceFillTest();
	~SurfaceFillTest();

	std::string name() const override { return "SurfaceFill Test"; };

	bool init(std::ostream& output) override;

private:

	bool softSurfaceTest(std::ostream& output);
	bool suppliedPixelsTest(std::ostream& output);
	bool streamTest(std::ostream& output);
	bool oldStreamTest(std::ostream& output);
};
