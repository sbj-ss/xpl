#include "Sanity.h"

xtsFixturePtr suite[] =
{
	&xtsTestSanityFixture,
	NULL
};

int main(int argc, char **argv)
{
	xtsContext ctxt;
	int ret;

	ret = xtsInit(argc, argv, &ctxt, suite);
	if (ret)
		return ret;
	return xtsWrap(suite, &ctxt);
}
