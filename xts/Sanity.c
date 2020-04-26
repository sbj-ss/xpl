#include "xts.h"
#include <assert.h>

static bool passing_function(xtsContextPtr ctxt)
{
	return true;
}
//------------------------
static xtsContext test_context;

static bool xtsTestSanity_InitContext(xtsContextPtr ctxt)
{
	test_context.total = test_context.skipped = test_context.passed = 883;
	test_context.error = (xmlChar*) 0x883;
	test_context.env = (xmlHashTablePtr) 0x883;
	test_context.verbosity = 883;
	test_context.fail_mode = 883;
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	if (test_context.total != 0)
	{
		ctxt->error = BAD_CAST "total != 0";
		return false;
	}
	if (test_context.skipped != 0)
	{
		ctxt->error = BAD_CAST "skipped != 0";
		return false;
	}
	if (test_context.passed != 0)
	{
		ctxt->error = BAD_CAST "passed != 0";
		return false;
	}
	if (test_context.error != NULL)
	{
		ctxt->error = BAD_CAST "error != NULL";
		return false;
	}
	if (test_context.env != NULL)
	{
		ctxt->error = BAD_CAST "env != NULL";
		return false;
	}
	if (test_context.verbosity != XTS_VERBOSITY_QUIET)
	{
		ctxt->error = BAD_CAST "context.verbosity != XTS_VERBOSITY_QUIET";
		return false;
	}
	if (test_context.fail_mode != XTS_FAIL_FIRST)
	{
		ctxt->error = BAD_CAST "context.fail_mode != XTS_FAIL_FIRST";
		return false;
	}
	return true;
}
//---------------------------
static xtsTest passing_test =
{
	SFINIT(.id, "passing_test"),
	SFINIT(.displayName, "single successful test"),
	SFINIT(.testFunction, passing_function)
};

static bool xtsTestSanity_TestPass(xtsContextPtr ctxt)
{
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	return xtsRunTest(&passing_test, &test_context);
}
//----------------------------------------------
static bool failing_function(xtsContextPtr ctxt)
{
	return false;
}

static xtsTest failing_test =
{
	SFINIT(.id, "failing_test"),
	SFINIT(.displayName, "single failing test"),
	SFINIT(.testFunction, failing_function)
};

static bool xtsTestSanity_TestFail(xtsContextPtr ctxt)
{
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	return !xtsRunTest(&failing_test, &test_context);
}
//---------------------------
static xtsTest skipped_test =
{
	SFINIT(.id, "skipped_test"),
	SFINIT(.displayName, "single skipped test"),
	SFINIT(.testFunction, failing_function),
	SFINIT(.flags, XTS_FLAG_SKIP)
};

static bool xtsTestSanity_TestSkip(xtsContextPtr ctxt)
{
	bool ok;
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	ok = xtsRunTest(&skipped_test, &test_context);
	if (!ok)
		ctxt->error = BAD_CAST "the failing test wasn't skipped";
	return ok;
}
//-----------------------------------
static xtsFixture passing_fixture = {
	SFINIT(.id, "passing_fixture"),
	SFINIT(.displayName, "passing fixture"),
	SFINIT(.setup, NULL),
	SFINIT(.teardown, NULL),
	SFINIT(.test_count, 1),
	SFINIT(.tests, &passing_test)
};

static bool xtsTestSanity_FixtureSinglePass(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	ok = xtsRunFixture(&passing_fixture, &test_context);
	if (!ok)
	{
		ctxt->error = BAD_CAST "this fixture should pass";
		return false;
	}
	if (test_context.total != 1)
	{
		ctxt->error = BAD_CAST "total != 1";
		return false;
	}
	if (test_context.skipped != 0)
	{
		ctxt->error = BAD_CAST "skipped != 0";
		return false;
	}
	if (test_context.passed != 1)
	{
		ctxt->error = BAD_CAST "passed != 1";
		return false;
	}
	return true;
}
//----------------------------
static xtsTest mixed_tests[] =
{
	{
		SFINIT(.id, "failing_test"),
		SFINIT(.displayName, "single failing test"),
		SFINIT(.testFunction, failing_function)
	}, {
		SFINIT(.id, "passing_test"),
		SFINIT(.displayName, "single successful test"),
		SFINIT(.testFunction, passing_function)
	}, {
		SFINIT(.id, "skipped_test"),
		SFINIT(.displayName, "single skipped test"),
		SFINIT(.testFunction, failing_function),
		SFINIT(.flags, XTS_FLAG_SKIP)
	}
};

static xtsFixture mixed_fixture =
{
	SFINIT(.id, "mixed_fixture"),
	SFINIT(.displayName, "mixed fixture"),
	SFINIT(.setup, NULL),
	SFINIT(.teardown, NULL),
	SFINIT(.test_count, 3),
	SFINIT(.tests, &mixed_tests[0])
};

static bool xtsTestSanity_FixtureFailImmediately(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	ok = xtsRunFixture(&mixed_fixture, &test_context);
	if (ok)
	{
		ctxt->error = BAD_CAST "this fixture should fail";
		return false;
	}
	if (test_context.total != 1)
	{
		ctxt->error = BAD_CAST "total != 1";
		return false;
	}
	if (test_context.skipped != 0)
	{
		ctxt->error = BAD_CAST "skipped != 0";
		return false;
	}
	if (test_context.passed != 0)
	{
		ctxt->error = BAD_CAST "passed != 0";
		return false;
	}
	return true;
}
//------------------------------------------------------------
static bool xtsTestSanity_FixtureFailAfter(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_ALL, XTS_VERBOSITY_QUIET);
	ok = xtsRunFixture(&mixed_fixture, &test_context);
	if (ok)
	{
		ctxt->error = BAD_CAST "this fixture should fail";
		return false;
	}
	if (test_context.total != 3)
	{
		ctxt->error = BAD_CAST "total != 3";
		return false;
	}
	if (test_context.skipped != 1)
	{
		ctxt->error = BAD_CAST "skipped != 1";
		return false;
	}
	if (test_context.passed != 1)
	{
		ctxt->error = BAD_CAST "passed != 1";
		return false;
	}
	return true;
}
//------------------------------------------------
static bool setup_fixture_fail(xtsContextPtr ctxt)
{
	return false;
}

static xtsFixture setup_fail_fixture = {
	SFINIT(.id, "sf_fixture"),
	SFINIT(.displayName, "fixture with failing .setup()"),
	SFINIT(.setup, setup_fixture_fail),
	SFINIT(.teardown, NULL),
	SFINIT(.test_count, 1),
	SFINIT(.tests, &passing_test)
};

static bool xtsTestSanity_FixtureSetupFail(xtsContextPtr ctxt)
{
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	if (xtsRunFixture(&setup_fail_fixture, &test_context))
	{
		ctxt->error = BAD_CAST "this fixture should fail";
		return false;
	}
	if (test_context.total > 0)
	{
		ctxt->error = BAD_CAST "no tests should be invoked";
		return false;
	}
	return true;
}
//--------------------------------------------------------
static bool setup_fixture_called, teardown_fixture_called;
static bool fixture_env_present, fixture_env_param_passed;

static bool setup_fixture_mock(xtsContextPtr ctxt)
{
	setup_fixture_called = true;
	if (ctxt->env)
	{
		fixture_env_present = true;
		xmlHashAddEntry(ctxt->env, "test", (void*) 5);
	}
	return true;
}

static void teardown_fixture_mock(xtsContextPtr ctxt)
{
	teardown_fixture_called = true;
	if (ctxt->env)
		fixture_env_param_passed = (xmlHashLookup(ctxt->env, "test") == (void*) 5);
}

static xtsFixture setup_teardown_fixture = {
	SFINIT(.id, "sd_fixture"),
	SFINIT(.displayName, "setup and teardown fixture"),
	SFINIT(.setup, setup_fixture_mock),
	SFINIT(.teardown, teardown_fixture_mock),
	SFINIT(.test_count, 1),
	SFINIT(.tests, &passing_test)
};

static bool xtsTestSanity_FixtureSetupTeardown(xtsContextPtr ctxt)
{
	setup_fixture_called = teardown_fixture_called = false;
	fixture_env_present = fixture_env_param_passed = false;
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	xtsRunFixture(&setup_teardown_fixture, &test_context);
	if (!setup_fixture_called)
	{
		ctxt->error = "fixture.setup() not called";
		return false;
	}
	if (!teardown_fixture_called)
	{
		ctxt->error = "fixture.teardown() not called";
		return false;
	}
	if (!fixture_env_present)
	{
		ctxt->error = "fixture.env = NULL";
		return false;
	}
	if (!fixture_env_param_passed)
	{
		ctxt->error = "fixture env param lost";
		return false;
	}
	return true;
}
//----------------------------------------
static bool leaky_test(xtsContextPtr ctxt)
{
	xmlHashAddEntry(ctxt->env, BAD_CAST "leak", xmlMalloc(sizeof(int)));
	return true;
}

static xtsTest unexpected_leak_test =
{
	SFINIT(.id, BAD_CAST "expected_leak"),
	SFINIT(.displayName, BAD_CAST "expected leak"),
	SFINIT(.testFunction, leaky_test),
	SFINIT(.flags, XTS_FLAG_CHECK_MEMORY),
	SFINIT(.expectedMemoryDelta, 0)
};

static void leak_deallocator(void *payload, xmlChar *name)
{
	xmlFree(payload);
}

static void teardown_leaky_fixture(xtsContextPtr ctxt)
{
	xmlHashRemoveEntry(ctxt->env, "leak", leak_deallocator);
}

static xtsFixture unexpected_leak_fixture =
{
	SFINIT(.id, BAD_CAST "unexpected_leak_fixture"),
	SFINIT(.displayName, BAD_CAST "fixture with unexpected leak"),
	SFINIT(.setup, NULL),
	SFINIT(.teardown, teardown_leaky_fixture),
	SFINIT(.test_count, 1),
	SFINIT(.tests, &unexpected_leak_test)
};

static bool xtsTestSanity_UnexpectedLeakFixture(xtsContextPtr ctxt)
{
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	return !xtsRunFixture(&unexpected_leak_fixture, &test_context);
}
//---------------------------------
static xtsTest expected_leak_test =
{
	SFINIT(.id, BAD_CAST "expected_leak"),
	SFINIT(.displayName, BAD_CAST "expected leak"),
	SFINIT(.testFunction, leaky_test),
	SFINIT(.flags, XTS_FLAG_CHECK_MEMORY),
	SFINIT(.expectedMemoryDelta, 2) /* 2 due to hash entry */
};

static xtsFixture expected_leak_fixture =
{
	SFINIT(.id, BAD_CAST "expected_leak_fixture"),
	SFINIT(.displayName, BAD_CAST "fixture with expected leak"),
	SFINIT(.setup, NULL),
	SFINIT(.teardown, teardown_leaky_fixture),
	SFINIT(.test_count, 1),
	SFINIT(.tests, &expected_leak_test)
};

static bool xtsTestSanity_ExpectedLeakFixture(xtsContextPtr ctxt)
{
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	return xtsRunFixture(&expected_leak_fixture, &test_context);
}
//------------------------------------
static xtsFixturePtr passing_suite[] =
{
	&passing_fixture,
	NULL
};

static bool xplTestSanity_PassingSuite(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	ok = xtsRunSuite(passing_suite, &test_context);
	if (!ok)
	{
		ctxt->error = BAD_CAST "this suite should pass";
		return false;
	}
	if (test_context.total != 1)
	{
		ctxt->error = BAD_CAST "total != 1";
		return false;
	}
	if (test_context.skipped != 0)
	{
		ctxt->error = BAD_CAST "skipped != 0";
		return false;
	}
	if (test_context.passed != 1)
	{
		ctxt->error = BAD_CAST "passed != 1";
		return false;
	}
	return true;
}
//----------------------------------
static xtsFixturePtr mixed_suite[] =
{
	&passing_fixture,
	&mixed_fixture,
	NULL
};

static bool xplTestSanity_SuiteFailingEarly(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	ok = xtsRunSuite(mixed_suite, &test_context);
	if (ok)
	{
		ctxt->error = BAD_CAST "this suite should fail";
		return false;
	}
	if (test_context.total != 2)
	{
		ctxt->error = BAD_CAST "total != 2";
		return false;
	}
	if (test_context.skipped != 0)
	{
		ctxt->error = BAD_CAST "skipped != 0";
		return false;
	}
	if (test_context.passed != 1)
	{
		ctxt->error = BAD_CAST "passed != 1";
		return false;
	}
	return true;
}
//------------------------------------------------------------
static bool xplTestSanity_SuiteFailingLate(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_ALL, XTS_VERBOSITY_QUIET);
	ok = xtsRunSuite(mixed_suite, &test_context);
	if (ok)
	{
		ctxt->error = BAD_CAST "this suite should fail";
		return false;
	}
	if (test_context.total != 4)
	{
		ctxt->error = BAD_CAST "total != 4";
		return false;
	}
	if (test_context.skipped != 1)
	{
		ctxt->error = BAD_CAST "skipped != 1";
		return false;
	}
	if (test_context.passed != 2)
	{
		ctxt->error = BAD_CAST "passed != 2";
		return false;
	}
	return true;
}
//----------------------------------------
static xtsTest fixture_to_skip_a_tests[] =
{
	{
		SFINIT(.id, "test_a"),
		SFINIT(.displayName, ""),
		SFINIT(.testFunction, passing_function)
	}, {
		SFINIT(.id, "test_b"),
		SFINIT(.displayName, ""),
		SFINIT(.testFunction, passing_function)
	}
};

static xtsFixture fixture_to_skip_a =
{
	SFINIT(.id, "fixture_a"),
	SFINIT(.displayName, ""),
	SFINIT(.setup, NULL),
	SFINIT(.teardown, NULL),
	SFINIT(.test_count, 2),
	SFINIT(.tests, fixture_to_skip_a_tests)
};

static xtsTest fixture_to_skip_b_tests[] =
{
	{
		SFINIT(.id, "test_c"),
		SFINIT(.displayName, ""),
		SFINIT(.testFunction, passing_function)
	}, {
		SFINIT(.id, "test_d"),
		SFINIT(.displayName, ""),
		SFINIT(.testFunction, passing_function)
	}
};

static xtsFixture fixture_to_skip_b =
{
	SFINIT(.id, "fixture_b"),
	SFINIT(.displayName, ""),
	SFINIT(.setup, NULL),
	SFINIT(.teardown, NULL),
	SFINIT(.test_count, 2),
	SFINIT(.tests, fixture_to_skip_b_tests)
};

static xtsFixturePtr suite_to_skip[] =
{
	&fixture_to_skip_a,
	&fixture_to_skip_b,
	NULL
};

static bool xplTestSanity_LocateFixture(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_b, unknown_fixture;

	fixture_b = xtsLocateFixture(suite_to_skip, BAD_CAST "fixture_b");
	if (!fixture_b)
	{
		ctxt->error = BAD_CAST "fixture_b not found";
		return false;
	}
	if (xmlStrcasecmp(fixture_b->id, "fixture_b"))
	{
		ctxt->error = BAD_CAST "wrong fixture found instead of fixture_b";
		return false;
	}
	unknown_fixture = xtsLocateFixture(suite_to_skip, BAD_CAST "unknown_fixture");
	if (unknown_fixture)
	{
		ctxt->error = BAD_CAST "unknown_fixture must not be found";
		return false;
	}
	return true;
}
//------------------------------------------------------
static bool xplTestSanity_LocateTest(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_a;
	xtsTestPtr test_b, unknown_test;

	fixture_a = xtsLocateFixture(suite_to_skip, BAD_CAST "fixture_a");
	test_b = xtsLocateTest(fixture_a, BAD_CAST "test_b");
	if (!test_b)
	{
		ctxt->error = BAD_CAST "test_b not found";
		return false;
	}
	if (xmlStrcasecmp(test_b->id, "test_b"))
	{
		ctxt->error = BAD_CAST "wrong test found instead of test_b";
		return false;
	}
	unknown_test = xtsLocateTest(fixture_a, BAD_CAST "unknown_test");
	if (unknown_test)
	{
		ctxt->error = BAD_CAST "unknown_test must not be found";
		return false;
	}
	return true;
}
//----------------------------------------------------------
static bool xplTestSanity_SkipSingleTest(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_b;
	xtsTestPtr test_d;

	fixture_b = xtsLocateFixture(suite_to_skip, "fixture_b");
	if (!fixture_b)
	{
		ctxt->error = BAD_CAST "fixture_b not found";
		return false;
	}
	test_d = xtsLocateTest(fixture_b, "test_d");
	if (!test_d)
	{
		ctxt->error = BAD_CAST "test_d not found";
		return false;
	}
	xtsSkipSingleTest(test_d, true);
	if (!(test_d->flags & XTS_FLAG_SKIP))
	{
		ctxt->error = BAD_CAST "XTS_FLAG_SKIP not set";
		return false;
	}
	xtsSkipSingleTest(test_d, false);
	if (test_d->flags & XTS_FLAG_SKIP)
	{
		ctxt->error = BAD_CAST "XTS_FLAG_SKIP is set";
		return false;
	}
	return true;
}
//----------------------------------------------------
static bool xplTestSanity_SkipTest(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_b;
	xtsTestPtr test_d;

	fixture_b = xtsLocateFixture(suite_to_skip, "fixture_b");
	if (!fixture_b)
	{
		ctxt->error = BAD_CAST "fixture_b not found";
		return false;
	}
	test_d = xtsLocateTest(fixture_b, "test_d");
	if (!test_d)
	{
		ctxt->error = BAD_CAST "test_d not found";
		return false;
	}
	if (!xtsSkipTest(suite_to_skip, "fixture_b", "test_d", true))
	{
		ctxt->error = BAD_CAST "skipping test_d failed";
		return false;
	}
	if (!(test_d->flags & XTS_FLAG_SKIP))
	{
		ctxt->error = BAD_CAST "XTS_FLAG_SKIP not set";
		return false;
	}
	if (!xtsSkipTest(suite_to_skip, "fixture_b", "test_d", false))
	{
		ctxt->error = BAD_CAST "un-skipping test_d failed";
		return false;
	}
	if (test_d->flags & XTS_FLAG_SKIP)
	{
		ctxt->error = BAD_CAST "XTS_FLAG_SKIP is set";
		return false;
	}
	return true;
}
//-----------------------------------------------------------------
static bool xplTestSanity_SkipAllTestsInFixture(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_b;
	xtsTestPtr test_c, test_d;

	fixture_b = xtsLocateFixture(suite_to_skip, "fixture_b");
	test_c = xtsLocateTest(fixture_b, "test_c");
	test_d = xtsLocateTest(fixture_b, "test_d");
	xtsSkipAllTestsInFixture(fixture_b, true);
	if (!(test_c->flags & XTS_FLAG_SKIP) || !(test_d->flags & XTS_FLAG_SKIP))
	{
		ctxt->error = BAD_CAST "XTS_FLAG_SKIP not set";
		return false;
	}
	xtsSkipAllTestsInFixture(fixture_b, false);
	if ((test_c->flags & XTS_FLAG_SKIP) || (test_d->flags & XTS_FLAG_SKIP))
	{
		ctxt->error = BAD_CAST "XTS_FLAG_SKIP is set";
		return false;
	}
	return true;
}
//---------------------------------------------------------------
static bool xplTestSanity_SkipAllTestsInSuite(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_a, fixture_b;
	xtsTestPtr test_a, test_d;

	fixture_a = xtsLocateFixture(suite_to_skip, "fixture_a");
	fixture_b = xtsLocateFixture(suite_to_skip, "fixture_b");
	test_a = xtsLocateTest(fixture_a, "test_a");
	test_d = xtsLocateTest(fixture_b, "test_d");
	xtsSkipAllTestsInSuite(suite_to_skip, true);
	if (!(test_a->flags & XTS_FLAG_SKIP) || !(test_d->flags & XTS_FLAG_SKIP))
	{
		ctxt->error = BAD_CAST "XTS_FLAG_SKIP not set";
		return false;
	}
	xtsSkipAllTestsInSuite(suite_to_skip, false);
	if ((test_a->flags & XTS_FLAG_SKIP) || (test_d->flags & XTS_FLAG_SKIP))
	{
		ctxt->error = BAD_CAST "XTS_FLAG_SKIP is set";
		return false;
	}
	return true;
}
//---------------------------
typedef struct _skipTestState
{
	xmlChar *skip_list;
	bool enabled[4];
} skipTestState;

static bool xplTestSanity_ApplySkipList(xtsContextPtr ctxt)
{
	static const xmlChar *feature_names[] =
	{
		BAD_CAST "fixture_a", BAD_CAST "fixture_a", BAD_CAST "fixture_b", BAD_CAST "fixture_b"
	};
	static const xmlChar *test_names[] =
	{
		BAD_CAST "test_a", BAD_CAST "test_b", BAD_CAST "test_c", BAD_CAST "test_d"
	};
	xtsFixturePtr fixture;
	xtsTestPtr tests[4];
	static const skipTestState states[] =
	{
		{
			BAD_CAST "s:@fixture_a#test_b;", { true, false, true, true }
		}, {
			BAD_CAST "i:@fixture_a*;", { true, true, true, true }
		}, {
			BAD_CAST "s:@fixture_a#test_a@fixture_b#test_d;", { false, true, true, false }
		}, {
			BAD_CAST "i:*;", { true, true, true, true }
		}
	};
	int i, j;
	xmlChar *error;

	assert(sizeof(feature_names) == sizeof(test_names));
	assert(sizeof(test_names) == sizeof(tests));
	for (i = 0; i < sizeof(feature_names) / sizeof(feature_names[0]); i++)
	{
		fixture = xtsLocateFixture(suite_to_skip, feature_names[i]);
		tests[i] = xtsLocateTest(fixture, test_names[i]);
	}
	for (i = 0; i < sizeof(states) / sizeof(states[0]); i++)
	{
		if (!xtsApplySkipList(states[i].skip_list, suite_to_skip, &error))
		{
			ctxt->error = error;
			ctxt->error_malloced = true;
			return false;
		}
		for (j = 0; j < sizeof(tests) / sizeof(tests[0]); j++)
		{
			if (states[i].enabled[j] != (tests[j]->flags & XTS_FLAG_SKIP? false: true))
			{
				ctxt->error = BAD_CAST "unexpected skip state";
				return false;
			}
		}
	}
	return true;
}
//---------------------------------------------------------------------
static bool xplTestSanity_ApplySkipListInvalidInput(xtsContextPtr ctxt)
{
	xmlChar* erratic[] = {
		BAD_CAST "q:@feature_a#test_b",	/* invalid operation */
		BAD_CAST "i@feature_a#test_b",	/* no colon after operation */
		BAD_CAST "i:feature_a#test_b",	/* no "@" before fixture name */
		BAD_CAST "i:@feature_a test_b",	/* no "#" before test name */
		BAD_CAST "i:@feature_a#test_b",	/* no semicolon at the end of statement */
		BAD_CAST "i:@feature_c#test_a", /* nonexistent feature */
		BAD_CAST "i:@feature_a#test_q", /* nonexistent test */
		NULL
	}, *s, *error;
	int i = 0;

	do {
		s = erratic[i++];
		if (!s)
			break;
		error = NULL;
		if (xtsApplySkipList(s, suite_to_skip, &error))
		{
			ctxt->error = BAD_CAST "these lists must fail";
			return false;
		}
		if (!error)
		{
			ctxt->error = BAD_CAST "error not filled in";
			return false;
		}
		xmlFree(error);
	} while (1);
	return true;
}
//------------------------------------------------
static bool testMallocingError(xtsContextPtr ctxt)
{
	ctxt->error = xmlStrdup(BAD_CAST "error");
	ctxt->error_malloced = true;
	return false;
}

static xtsTest test_mallocing_error =
{
	SFINIT(.id, BAD_CAST "test_mallocing_error"),
	SFINIT(.displayName, BAD_CAST ""),
	SFINIT(.testFunction, testMallocingError)
};

bool xplTestSanity_RunTestFreeError(xtsContextPtr ctxt)
{
	test_context.error = NULL;
	test_context.error_malloced = false;
	xtsRunTest(&test_mallocing_error, &test_context);
	if (test_context.error)
	{
		ctxt->error = BAD_CAST "xtsRunTest() didn't clean up ctxt.error";
		return false;
	}
	if (test_context.error_malloced)
	{
		ctxt->error = BAD_CAST "xtsRunTest() didn't clean up ctxt.error_malloced";
		return false;
	}
	return true;
}


static xtsTest sanity_tests[] =
{
	{
		SFINIT(.id, BAD_CAST "basic"),
		SFINIT(.displayName, BAD_CAST "basic functionality"),
		SFINIT(.testFunction, passing_function)
	}, {
		SFINIT(.id, BAD_CAST "context"),
		SFINIT(.displayName, BAD_CAST "xtsInitContext()"),
		SFINIT(.testFunction, xtsTestSanity_InitContext)
	}, {
		SFINIT(.id, BAD_CAST "passing_test"),
		SFINIT(.displayName, BAD_CAST "xtsRunTest(): passing test"),
		SFINIT(.testFunction, xtsTestSanity_TestPass)
	}, {
		SFINIT(.id, BAD_CAST "failing_test"),
		SFINIT(.displayName, BAD_CAST "xtsRunTest(): failing test"),
		SFINIT(.testFunction, xtsTestSanity_TestFail)
	}, {
		SFINIT(.id, BAD_CAST "skipped_test"),
		SFINIT(.displayName, BAD_CAST "xtsRunTest(): skipped test"),
		SFINIT(.testFunction, xtsTestSanity_TestSkip)
	}, {
		SFINIT(.id, BAD_CAST "passing_fixture"),
		SFINIT(.displayName, BAD_CAST "xtsRunFixture(): passing fixture"),
		SFINIT(.testFunction, xtsTestSanity_FixtureSinglePass)
	}, {
		SFINIT(.id, BAD_CAST "failing_early_fixture"),
		SFINIT(.displayName, BAD_CAST "xtsRunFixture(): fixture failing early"),
		SFINIT(.testFunction, xtsTestSanity_FixtureFailImmediately)
	}, {
		SFINIT(.id, BAD_CAST "failing_late_fixture"),
		SFINIT(.displayName, BAD_CAST "xtsRunFixture(): fixture failing late"),
		SFINIT(.testFunction, xtsTestSanity_FixtureFailAfter)
	}, {
		SFINIT(.id, BAD_CAST "failing_setup_fixture"),
		SFINIT(.displayName, BAD_CAST "xtsRunFixture(): failing .setup()"),
		SFINIT(.testFunction, xtsTestSanity_FixtureSetupFail)
	}, {
		SFINIT(.id, BAD_CAST "setup_teardown_fixture"),
		SFINIT(.displayName, BAD_CAST "xtsRunFixture(): .setup() and .teardown()"),
		SFINIT(.testFunction, xtsTestSanity_FixtureSetupTeardown)
	}, {
		SFINIT(.id, BAD_CAST "unexpected_leak"),
		SFINIT(.displayName, BAD_CAST "xtsRunFixture(): unexpected leak"),
		SFINIT(.testFunction, xtsTestSanity_UnexpectedLeakFixture)
	}, {
		SFINIT(.id, BAD_CAST "expected_leak"),
		SFINIT(.displayName, BAD_CAST "xtsRunFixture(): expected leak"),
		SFINIT(.testFunction, xtsTestSanity_ExpectedLeakFixture)
	}, {
		SFINIT(.id, BAD_CAST "passing_suite"),
		SFINIT(.displayName, BAD_CAST "xtsRunSuite(): passing suite"),
		SFINIT(.testFunction, xplTestSanity_PassingSuite)
	}, {
		SFINIT(.id, BAD_CAST "failing_early_suite"),
		SFINIT(.displayName, BAD_CAST "xtsRunSuite(): suite failing early"),
		SFINIT(.testFunction, xplTestSanity_SuiteFailingEarly)
	}, {
		SFINIT(.id, BAD_CAST "failing_late_suite"),
		SFINIT(.displayName, BAD_CAST "xtsRunSuite(): suite failing late"),
		SFINIT(.testFunction, xplTestSanity_SuiteFailingLate)
	}, {
		SFINIT(.id, BAD_CAST "locate_fixture"),
		SFINIT(.displayName, BAD_CAST "xtsLocateFixture()"),
		SFINIT(.testFunction, xplTestSanity_LocateFixture),
	}, {
		SFINIT(.id, BAD_CAST "locate_test"),
		SFINIT(.displayName, BAD_CAST "xtsLocateTest()"),
		SFINIT(.testFunction, xplTestSanity_LocateTest),
	}, {
		SFINIT(.id, BAD_CAST "skip_single_test"),
		SFINIT(.displayName, BAD_CAST "xtsSkipSingleTest()"),
		SFINIT(.testFunction, xplTestSanity_SkipSingleTest),
	}, {
		SFINIT(.id, BAD_CAST "skip_test"),
		SFINIT(.displayName, BAD_CAST "xtsSkipTest()"),
		SFINIT(.testFunction, xplTestSanity_SkipTest),
	}, {
		SFINIT(.id, BAD_CAST "skip_all_tests_in_fixture"),
		SFINIT(.displayName, BAD_CAST "xtsSkipAllTestsInFixture()"),
		SFINIT(.testFunction, xplTestSanity_SkipAllTestsInFixture),
	}, {
		SFINIT(.id, BAD_CAST "skip_all_tests_in_suite"),
		SFINIT(.displayName, BAD_CAST "xtsSkipAllTestsInSuite()"),
		SFINIT(.testFunction, xplTestSanity_SkipAllTestsInSuite),
	}, {
		SFINIT(.id, BAD_CAST "apply_skip_list"),
		SFINIT(.displayName, BAD_CAST "xtsApplySkipList()"),
		SFINIT(.testFunction, xplTestSanity_ApplySkipList),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY),
		SFINIT(.expectedMemoryDelta, 0)
	}, {
		SFINIT(.id, BAD_CAST "apply_skip_list_invalid_input"),
		SFINIT(.displayName, BAD_CAST "xtsApplySkipList(): invalid input"),
		SFINIT(.testFunction, xplTestSanity_ApplySkipListInvalidInput),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY),
		SFINIT(.expectedMemoryDelta, 0)
	}, {
		SFINIT(.id, BAD_CAST "run_test_free_error"),
		SFINIT(.displayName, BAD_CAST "xtsRunTest(): malloc'ed error"),
		SFINIT(.testFunction, xplTestSanity_RunTestFreeError),
		SFINIT(.flags, XTS_FLAG_CHECK_MEMORY),
		SFINIT(.expectedMemoryDelta, 0)
	}
};

xtsFixture xtsTestSanityFixture =
{
	SFINIT(.id, BAD_CAST "sanity"),
	SFINIT(.displayName, BAD_CAST "test system sanity test group"),
	SFINIT(.setup, NULL),
	SFINIT(.teardown, NULL),
	SFINIT(.test_count, sizeof(sanity_tests) / sizeof(sanity_tests[0])),
	SFINIT(.tests, &sanity_tests[0])
};
