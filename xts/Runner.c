#include "Sanity.h"

xtsFixturePtr suite[] =
{
	&xtsTestSanityFixture,
	NULL
};

int main(int argc, char **argv)
{
	return xtsRun(argc, argv, suite);
}
