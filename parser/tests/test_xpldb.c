#include "test_xpldb.h"

#include <assert.h>
#include <libxpl/xpldb.h>

#define FAIL(x) do {\
	ctxt->error = BAD_CAST x;\
	goto cleanup;\
} while(0);

static int dealloc_invocations;

static void dummy_dealloc(void* conn)
{
	dealloc_invocations++;
}

static bool xtsTestDB_DBCreateFree(xtsContextPtr ctxt)
{
	xplDBPtr db;

	dealloc_invocations = 0;
	db = xplDBCreate((xplDBPtr) 0x1234, dummy_dealloc);
	assert(db);
	if (db->next != (xplDBPtr) 0x1234)
		FAIL(".next of a newly created xplDB must be equal to aNext parameter");
	if (db->dealloc != dummy_dealloc)
		FAIL(".dealloc of a newly created xplDB must be equal to aDealloc parameter");
	if (db->connection != NULL)
		FAIL(".connection of a newly created xplDB must be NULL");
	if (db->busy)
		FAIL(".busy of a newly created db must be false");
cleanup:
	xplDBFree(db);
	if (ctxt->error)
		return false;
	if (dealloc_invocations != 1)
	{
		ctxt->error = BAD_CAST "deallocator not called";
		return false;
	}
	return true;
}

static bool xtsTestDB_DBListCreateFree(xtsContextPtr ctxt)
{
	bool ok = false;
	xplDBListPtr list;

	list = xplDBListCreate(BAD_CAST "foobar");
	assert(list);
	if (xmlStrcmp(list->conn_string, BAD_CAST "foobar"))
		FAIL("wrong connection string");
	if (list->first != NULL)
		FAIL(".first of a newly created xplDBList must be null");
	if (list->last != NULL)
		FAIL(".last of a newly created xplDBList must be null");
	if (!xprMutexAcquire(&list->lock))
		FAIL("invalid list mutex");
	xprMutexRelease(&list->lock);
	ok = true;
cleanup:
	xplDBListFree(list);
	return ok;
}

static xtsTest db_tests[] =
{
	{
		.id = BAD_CAST "db_create_free",
		.displayName = BAD_CAST "Creation and freeing of a single container",
		.testFunction = xtsTestDB_DBCreateFree,
		.flags = XTS_FLAG_CHECK_MEMORY
	}, {
		.id = BAD_CAST "dblist_create_free",
		.displayName = BAD_CAST "Creation and freeing of a container list",
		.testFunction = xtsTestDB_DBListCreateFree,
		.flags = XTS_FLAG_CHECK_MEMORY
	}
};

xtsFixture xtsTestDBFixture =
{
	.id = BAD_CAST "db",
	.displayName = BAD_CAST "database connection container test group",
	.setup = NULL,
	.teardown = NULL,
	.test_count = sizeof(db_tests) / sizeof(db_tests[0]),
	.tests = db_tests
};
