#include <libxml/parser.h>
#include "test_xplbuffer.h"
#include "test_xpldb.h"
#include "test_xplsave.h"

xtsFixturePtr suite[] =
{
	&xtsTestReszBufFixture,
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
	xmlMemSetup(xmlMemFree, xmlMemMalloc, xmlMemRealloc, xmlMemoryStrdup);
	xmlInitParser();
	ret = xtsWrap(suite, &ctxt);
	xmlCleanupParser();
	return ret;
}
