#include "Sanity.h"
#include <getopt.h>

xtsFixturePtr suite[] =
{
	&xtsTestSanityFixture,
	NULL
};

void usage(char *my_name)
{
	printf("Usage:\n");
	printf("%s [-h] [-s LIST]\n", my_name);
	printf("\t-h: show usage and exit\n");
	printf("\t-s LIST: skip/include various tests\n");
}

int main(int argc, char** argv)
{
	int c;
	xmlChar *skip_list = NULL, *error;
	bool ok;
	xtsContext ctxt;
	int initial_blocks, mem_delta;

	while (1)
	{
		c = getopt(argc, argv, "hs:");
		if (c == -1)
			break;
		switch(c)
		{
			case 'h':
				usage(argv[0]);
				return 0;
			case 's':
				skip_list = BAD_CAST optarg;
				break;
			default:
				usage(argv[0]);
				return 1;
		}
	}
	xmlMemSetup(xmlMemFree, xmlMemMalloc, xmlMemRealloc, xmlMemoryStrdup);
	initial_blocks = xmlMemBlocks();
	xtsInitContext(&ctxt, XTS_FAIL_ALL, XTS_VERBOSITY_TEST);
	if (skip_list)
		if (!xtsApplySkipList(skip_list, suite, &error))
		{
			xmlGenericError(xmlGenericErrorContext, "error parsing skip list: %s\n", error);
			xmlFree(error);
			return 1;
		}
	ok = xtsRunSuite(suite, &ctxt);
	if (!ok)
		return 2;
	mem_delta = xmlMemBlocks() - initial_blocks;
	if (mem_delta)
	{
		xmlGenericError(xmlGenericErrorContext, "ERROR: expected 0 extra mem blocks, got %d\n", mem_delta);
		return 3;
	}
	return 0;
}
