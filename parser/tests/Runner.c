#include "Test_ReszBuf.h"

xtsFixturePtr suite[] =
{
	&xtsTestReszBufFixture,
	NULL
};

int main(int argc, char **argv)
{
	return xtsRun(argc, argv, suite);
}
