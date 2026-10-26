#include "../testframework.h"



class QOITest : public TestCollection
{
public:
	QOITest();
	~QOITest();

	std::string name() const override { return "QOI Test"; };

	bool init(std::ostream& output) override;

private:

	bool compressTest(std::ostream& output);
	bool decompressTest(std::ostream& output);
	bool writeQOITest(std::ostream& output);
	bool readQOITest(std::ostream& output);
	bool surfaceFileTest(std::ostream& output);
};
