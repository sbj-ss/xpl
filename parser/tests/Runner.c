#include "test_xplbuffer.h"
#include "test_xpldb.h"

xtsFixturePtr suite[] =
{
	&xtsTestReszBufFixture,
	&xtsTestDBFixture,
	NULL
};

int main(int argc, char **argv)
{
	return xtsRun(argc, argv, suite);
}
