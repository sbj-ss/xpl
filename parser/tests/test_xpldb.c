#include "test_xpldb.h"

#include <assert.h>
#include <libxpl/xpldb.h>

#define FAIL(x) do {\
	ctxt->error = BAD_CAST x;\
	goto cleanup;\
} while(0);

static bool xtsTestDB_DBCreateFree(xtsContextPtr ctxt)
{
	return true;
}

static xtsTest db_tests[] =
{
	{
		SFINIT(.id, BAD_CAST "db_create_free"),
		SFINIT(.displayName, BAD_CAST "Creation and freeing of a single container"),
		SFINIT(.testFunction, xtsTestDB_DBCreateFree),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY)
	}
};

xtsFixture xtsTestDBFixture =
{
	SFINIT(.id, BAD_CAST "db"),
	SFINIT(.displayName, BAD_CAST "database connection container test group"),
	SFINIT(.setup, NULL),
	SFINIT(.teardown, NULL),
	SFINIT(.test_count, sizeof(db_tests) / sizeof(db_tests[0])),
	SFINIT(.tests, db_tests)
};
