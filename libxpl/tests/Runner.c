#include <libxml/parser.h>
#include "test_xpldb.h"
#include "test_xplsave.h"

xtsFixturePtr suite[] =
{
	&xtsTestSaveFixture,
	&xtsTestDBFixture,
	NULL
};

int main(int argc, char **argv)
{
	xtsContext ctxt;
	int ret;

	LIBXML_TEST_VERSION
	ret = xtsInit(argc, argv, &ctxt, suite);
	if (ret)
		return ret;
	xmlInitParser();
	ret = xtsWrap(suite, &ctxt);
	xmlCleanupParser();
	return ret;
}
