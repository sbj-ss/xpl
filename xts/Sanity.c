#include "xts.h"
#include <assert.h>
#include <Configuration.h>

static bool _passingFunction(xtsContextPtr ctxt)
{
	return true;
}
//------------------------
static xtsContext test_context;

static bool _testInitContext(xtsContextPtr ctxt)
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
	.id = BAD_CAST "passing_test",
	.displayName = BAD_CAST "single successful test",
	.testFunction = _passingFunction
};

static bool _testPassingTest(xtsContextPtr ctxt)
{
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	return xtsRunTest(&passing_test, &test_context);
}
//----------------------------------------------
static bool _failingFunction(xtsContextPtr ctxt)
{
	return false;
}

static xtsTest failing_test =
{
	.id = BAD_CAST "failing_test",
	.displayName = BAD_CAST "single failing test",
	.testFunction = _failingFunction
};

static bool _testFailingTest(xtsContextPtr ctxt)
{
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	return !xtsRunTest(&failing_test, &test_context);
}
//---------------------------
static xtsTest skipped_test =
{
	.id = BAD_CAST "skipped_test",
	.displayName = BAD_CAST "single skipped test",
	.testFunction = _failingFunction,
	.flags = XTS_FLAG_SKIP
};

static bool _testSkippedTest(xtsContextPtr ctxt)
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
	.id = BAD_CAST "passing_fixture",
	.displayName = BAD_CAST "passing fixture",
	.setup = NULL,
	.teardown = NULL,
	.test_count = 1,
	.tests = &passing_test
};

static bool _testPassingFixture(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	ok = xtsRunFixture(&passing_fixture, &test_context);
	if (!ok)
	{
		ctxt->error = BAD_CAST "this fixture must pass";
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
		.id = BAD_CAST "failing_test",
		.displayName = BAD_CAST "single failing test",
		.testFunction = _failingFunction
	}, {
		.id = BAD_CAST "passing_test",
		.displayName = BAD_CAST "single successful test",
		.testFunction = _passingFunction
	}, {
		.id = BAD_CAST "skipped_test",
		.displayName = BAD_CAST "single skipped test",
		.testFunction = _failingFunction,
		.flags = XTS_FLAG_SKIP
	}
};

static xtsFixture mixed_fixture =
{
	.id = BAD_CAST "mixed_fixture",
	.displayName = BAD_CAST "mixed fixture",
	.setup = NULL,
	.teardown = NULL,
	.test_count = 3,
	.tests = &mixed_tests[0]
};

static bool _testEarlyFailingFixture(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	ok = xtsRunFixture(&mixed_fixture, &test_context);
	if (ok)
	{
		ctxt->error = BAD_CAST "this fixture must fail";
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
static bool _testLateFailingFixture(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_ALL, XTS_VERBOSITY_QUIET);
	ok = xtsRunFixture(&mixed_fixture, &test_context);
	if (ok)
	{
		ctxt->error = BAD_CAST "this fixture must fail";
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
	.id = BAD_CAST "sf_fixture",
	.displayName = BAD_CAST "fixture with failing .setup()",
	.setup = setup_fixture_fail,
	.teardown = NULL,
	.test_count = 1,
	.tests = &passing_test
};

static bool _testFixtureSetupFail(xtsContextPtr ctxt)
{
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	if (xtsRunFixture(&setup_fail_fixture, &test_context))
	{
		ctxt->error = BAD_CAST "this fixture must fail";
		return false;
	}
	if (test_context.total > 0)
	{
		ctxt->error = BAD_CAST "no tests must be invoked";
		return false;
	}
	return true;
}
//--------------------------------------------------------
static bool setup_fixture_called, teardown_fixture_called;
static bool fixture_env_present, fixture_env_param_passed;

static bool _setupFixtureMock(xtsContextPtr ctxt)
{
	setup_fixture_called = true;
	if (ctxt->env)
	{
		fixture_env_present = true;
		xmlHashAddEntry(ctxt->env, BAD_CAST "test", (void*) 5);
	}
	return true;
}

static void _teardownFixtureMock(xtsContextPtr ctxt)
{
	teardown_fixture_called = true;
	if (ctxt->env)
		fixture_env_param_passed = (xmlHashLookup(ctxt->env, BAD_CAST "test") == (void*) 5);
}

static xtsFixture setup_teardown_fixture = {
	.id = BAD_CAST "st_fixture",
	.displayName = BAD_CAST "setup and teardown fixture",
	.setup = _setupFixtureMock,
	.teardown = _teardownFixtureMock,
	.test_count = 1,
	.tests = &passing_test
};

static bool _testFixtureSetupAndTeardown(xtsContextPtr ctxt)
{
	setup_fixture_called = teardown_fixture_called = false;
	fixture_env_present = fixture_env_param_passed = false;
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	xtsRunFixture(&setup_teardown_fixture, &test_context);
	if (!setup_fixture_called)
	{
		ctxt->error = BAD_CAST "fixture.setup() not called";
		return false;
	}
	if (!teardown_fixture_called)
	{
		ctxt->error = BAD_CAST "fixture.teardown() not called";
		return false;
	}
	if (!fixture_env_present)
	{
		ctxt->error = BAD_CAST "fixture.env = NULL";
		return false;
	}
	if (!fixture_env_param_passed)
	{
		ctxt->error = BAD_CAST "fixture env param lost";
		return false;
	}
	return true;
}
//----------------------------------------
static bool _leakyTest(xtsContextPtr ctxt)
{
	xmlHashAddEntry(ctxt->env, BAD_CAST "leak", XPL_MALLOC(sizeof(int)));
	return true;
}

static xtsTest unexpected_leak_test =
{
	.id = BAD_CAST "unexpected_leak",
	.displayName = BAD_CAST "unexpected leak",
	.testFunction = _leakyTest,
	.flags = XTS_FLAG_CHECK_MEMORY,
	.expectedMemoryDelta = 0
};

static void _leakDeallocator(void *payload, XML_HCBNC xmlChar *name)
{
	XPL_FREE(payload);
}

static void _teardownLeakyFixture(xtsContextPtr ctxt)
{
	xmlHashRemoveEntry(ctxt->env, BAD_CAST "leak", _leakDeallocator);
}

static xtsFixture unexpected_leak_fixture =
{
	.id = BAD_CAST "unexpected_leak_fixture",
	.displayName = BAD_CAST "fixture with unexpected leak",
	.setup = NULL,
	.teardown = _teardownLeakyFixture,
	.test_count = 1,
	.tests = &unexpected_leak_test
};

static bool _testUnexpectedLeakInFixture(xtsContextPtr ctxt)
{
	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	return !xtsRunFixture(&unexpected_leak_fixture, &test_context);
}
//---------------------------------
static xtsTest expected_leak_test =
{
	.id = BAD_CAST "expected_leak",
	.displayName = BAD_CAST "expected leak",
	.testFunction = _leakyTest,
	.flags = XTS_FLAG_CHECK_MEMORY,
	.expectedMemoryDelta = 2 /* 2 due to hash entry */
};

static xtsFixture expected_leak_fixture =
{
	.id = BAD_CAST "expected_leak_fixture",
	.displayName = BAD_CAST "fixture with expected leak",
	.setup = NULL,
	.teardown = _teardownLeakyFixture,
	.test_count = 1,
	.tests = &expected_leak_test
};

static bool _testExpectedLeakInFixture(xtsContextPtr ctxt)
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

static bool _testPassingSuite(xtsContextPtr ctxt)
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

static bool _testEarlyFailingSuite(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_FIRST, XTS_VERBOSITY_QUIET);
	ok = xtsRunSuite(mixed_suite, &test_context);
	if (ok)
	{
		ctxt->error = BAD_CAST "this suite must fail";
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
static bool _testLateFailingSuite(xtsContextPtr ctxt)
{
	bool ok;

	xtsInitContext(&test_context, XTS_FAIL_ALL, XTS_VERBOSITY_QUIET);
	ok = xtsRunSuite(mixed_suite, &test_context);
	if (ok)
	{
		ctxt->error = BAD_CAST "this suite must fail";
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
		.id = BAD_CAST "test_a",
		.displayName = BAD_CAST "",
		.testFunction = _passingFunction
	}, {
		.id = BAD_CAST "test_b",
		.displayName = BAD_CAST "",
		.testFunction = _passingFunction
	}
};

static xtsFixture fixture_to_skip_a =
{
	.id = BAD_CAST "fixture_a",
	.displayName = BAD_CAST "",
	.setup = NULL,
	.teardown = NULL,
	.test_count = 2,
	.tests = fixture_to_skip_a_tests
};

static xtsTest fixture_to_skip_b_tests[] =
{
	{
		.id = BAD_CAST "test_c",
		.displayName = BAD_CAST "",
		.testFunction = _passingFunction
	}, {
		.id = BAD_CAST "test_d",
		.displayName = BAD_CAST  "",
		.testFunction = _passingFunction
	}
};

static xtsFixture fixture_to_skip_b =
{
	.id = BAD_CAST "fixture_b",
	.displayName = BAD_CAST "",
	.setup = NULL,
	.teardown = NULL,
	.test_count = 2,
	.tests = fixture_to_skip_b_tests
};

static xtsFixturePtr suite_to_skip[] =
{
	&fixture_to_skip_a,
	&fixture_to_skip_b,
	NULL
};

static bool _testLocateFixture(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_b, unknown_fixture;

	fixture_b = xtsLocateFixture(suite_to_skip, BAD_CAST "fixture_b");
	if (!fixture_b)
	{
		ctxt->error = BAD_CAST "fixture_b not found";
		return false;
	}
	if (xmlStrcasecmp(fixture_b->id, BAD_CAST "fixture_b"))
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
static bool _testLocateTest(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_a;
	xtsTestPtr test_b, unknown_test;

	fixture_a = xtsLocateFixture(suite_to_skip, BAD_CAST "fixture_a");
	if (!fixture_a)
	{
		ctxt->error = BAD_CAST "fixture_a not found";
		return false;
	}
	test_b = xtsLocateTest(fixture_a, BAD_CAST "test_b");
	if (!test_b)
	{
		ctxt->error = BAD_CAST "test_b not found";
		return false;
	}
	if (xmlStrcasecmp(test_b->id, BAD_CAST "test_b"))
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
static bool _testSkipSingleTest(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_b;
	xtsTestPtr test_d;

	fixture_b = xtsLocateFixture(suite_to_skip, BAD_CAST "fixture_b");
	if (!fixture_b)
	{
		ctxt->error = BAD_CAST "fixture_b not found";
		return false;
	}
	test_d = xtsLocateTest(fixture_b, BAD_CAST "test_d");
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
static bool _testSkipTest(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_b;
	xtsTestPtr test_d;

	fixture_b = xtsLocateFixture(suite_to_skip, BAD_CAST "fixture_b");
	if (!fixture_b)
	{
		ctxt->error = BAD_CAST "fixture_b not found";
		return false;
	}
	test_d = xtsLocateTest(fixture_b, BAD_CAST "test_d");
	if (!test_d)
	{
		ctxt->error = BAD_CAST "test_d not found";
		return false;
	}
	if (!xtsSkipTest(suite_to_skip, BAD_CAST "fixture_b", BAD_CAST "test_d", true))
	{
		ctxt->error = BAD_CAST "skipping test_d failed";
		return false;
	}
	if (!(test_d->flags & XTS_FLAG_SKIP))
	{
		ctxt->error = BAD_CAST "XTS_FLAG_SKIP not set";
		return false;
	}
	if (!xtsSkipTest(suite_to_skip, BAD_CAST "fixture_b", BAD_CAST "test_d", false))
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
static bool _testSkipAllTestsInFixture(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_b;
	xtsTestPtr test_c, test_d;

	fixture_b = xtsLocateFixture(suite_to_skip, BAD_CAST "fixture_b");
	test_c = xtsLocateTest(fixture_b, BAD_CAST "test_c");
	test_d = xtsLocateTest(fixture_b, BAD_CAST "test_d");
	if (!fixture_b || !test_c || !test_d)
	{
		ctxt->error = BAD_CAST "xtsLocateFixture() or xtsLocateTest() failed";
		return false;
	}
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
static bool _testSkipAllTestsInSuite(xtsContextPtr ctxt)
{
	xtsFixturePtr fixture_a, fixture_b;
	xtsTestPtr test_a, test_d;

	fixture_a = xtsLocateFixture(suite_to_skip, BAD_CAST "fixture_a");
	fixture_b = xtsLocateFixture(suite_to_skip, BAD_CAST "fixture_b");
	test_a = xtsLocateTest(fixture_a, BAD_CAST "test_a");
	test_d = xtsLocateTest(fixture_b, BAD_CAST "test_d");
	if (!fixture_a || !fixture_b || !test_a || !test_d)
	{
		ctxt->error = BAD_CAST "xtsLocateFixture() or xtsLocateTest() failed";
		return false;
	}
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

static bool _testApplySkipList(xtsContextPtr ctxt)
{
	static const xmlChar *fixture_names[] =
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
			BAD_CAST "s:@fixture_a#test_a @fixture_b#test_d;", { false, true, true, false }
		}, {
			BAD_CAST "i:*;", { true, true, true, true }
		}
	};
	int i, j;
	xmlChar *error;

	assert(sizeof(fixture_names) == sizeof(test_names));
	assert(sizeof(test_names) == sizeof(tests));
	for (i = 0; i < sizeof(fixture_names) / sizeof(fixture_names[0]); i++)
	{
		fixture = xtsLocateFixture(suite_to_skip, fixture_names[i]);
		tests[i] = xtsLocateTest(fixture, test_names[i]);
		if (!fixture || !tests[i])
		{
			ctxt->error = BAD_CAST "xtsLocateFixture() or xtsLocateTest() failed";
			return false;
		}
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
static bool _testApplySkipListInvalidInput(xtsContextPtr ctxt)
{
	xmlChar* erratic[] = {
		BAD_CAST "q:@fixture_a#test_b",	/* invalid operation */
		BAD_CAST "i@fixture_a#test_b",	/* no colon after operation */
		BAD_CAST "i:fixture_a#test_b",	/* no "@" before fixture name */
		BAD_CAST "i:@fixture_a test_b",	/* no "#" before test name */
		BAD_CAST "i:@fixture_a#test_b",	/* no semicolon at the end of statement */
		BAD_CAST "i:@fixture_z#test_a", /* nonexistent feature */
		BAD_CAST "i:@fixture_a#test_q", /* nonexistent test */
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
		XPL_FREE(error);
	} while (1);
	return true;
}
//------------------------------------------------
static bool _testMallocingError(xtsContextPtr ctxt)
{
	ctxt->error = BAD_CAST XPL_STRDUP("error");
	ctxt->error_malloced = true;
	return false;
}

static xtsTest test_mallocing_error =
{
	.id = BAD_CAST "test_mallocing_error",
	.displayName = BAD_CAST "",
	.testFunction = _testMallocingError
};

bool _testRunTestAndFreeError(xtsContextPtr ctxt)
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
		.id = BAD_CAST "basic",
		.displayName = BAD_CAST "basic functionality",
		.testFunction = _passingFunction
	}, {
		.id = BAD_CAST "context",
		.displayName = BAD_CAST "xtsInitContext()",
		.testFunction = _testInitContext
	}, {
		.id = BAD_CAST "passing_test",
		.displayName = BAD_CAST "xtsRunTest(): passing test",
		.testFunction = _testPassingTest
	}, {
		.id = BAD_CAST "failing_test",
		.displayName = BAD_CAST "xtsRunTest(): failing test",
		.testFunction = _testFailingTest
	}, {
		.id = BAD_CAST "skipped_test",
		.displayName = BAD_CAST "xtsRunTest(): skipped test",
		.testFunction = _testSkippedTest
	}, {
		.id = BAD_CAST "passing_fixture",
		.displayName = BAD_CAST "xtsRunFixture(): passing fixture",
		.testFunction = _testPassingFixture
	}, {
		.id = BAD_CAST "failing_early_fixture",
		.displayName = BAD_CAST "xtsRunFixture(): fixture failing early",
		.testFunction = _testEarlyFailingFixture
	}, {
		.id = BAD_CAST "failing_late_fixture",
		.displayName = BAD_CAST "xtsRunFixture(): fixture failing late",
		.testFunction = _testLateFailingFixture
	}, {
		.id = BAD_CAST "failing_setup_fixture",
		.displayName = BAD_CAST "xtsRunFixture(): failing .setup()",
		.testFunction = _testFixtureSetupFail
	}, {
		.id = BAD_CAST "setup_teardown_fixture",
		.displayName = BAD_CAST "xtsRunFixture(): .setup() and .teardown()",
		.testFunction = _testFixtureSetupAndTeardown
	}, {
		.id = BAD_CAST "unexpected_leak",
		.displayName = BAD_CAST "xtsRunFixture(): unexpected leak",
		.testFunction = _testUnexpectedLeakInFixture
	}, {
		.id = BAD_CAST "expected_leak",
		.displayName = BAD_CAST "xtsRunFixture(): expected leak",
		.testFunction = _testExpectedLeakInFixture
	}, {
		.id = BAD_CAST "passing_suite",
		.displayName = BAD_CAST "xtsRunSuite(): passing suite",
		.testFunction = _testPassingSuite
	}, {
		.id = BAD_CAST "failing_early_suite",
		.displayName = BAD_CAST "xtsRunSuite(): suite failing early",
		.testFunction = _testEarlyFailingSuite
	}, {
		.id = BAD_CAST "failing_late_suite",
		.displayName = BAD_CAST "xtsRunSuite(): suite failing late",
		.testFunction = _testLateFailingSuite
	}, {
		.id = BAD_CAST "locate_fixture",
		.displayName = BAD_CAST "xtsLocateFixture()",
		.testFunction = _testLocateFixture,
	}, {
		.id = BAD_CAST "locate_test",
		.displayName = BAD_CAST "xtsLocateTest()",
		.testFunction = _testLocateTest,
	}, {
		.id = BAD_CAST "skip_single_test",
		.displayName = BAD_CAST "xtsSkipSingleTest()",
		.testFunction = _testSkipSingleTest,
	}, {
		.id = BAD_CAST "skip_test",
		.displayName = BAD_CAST "xtsSkipTest()",
		.testFunction = _testSkipTest,
	}, {
		.id = BAD_CAST "skip_all_tests_in_fixture",
		.displayName = BAD_CAST "xtsSkipAllTestsInFixture()",
		.testFunction = _testSkipAllTestsInFixture,
	}, {
		.id = BAD_CAST "skip_all_tests_in_suite",
		.displayName = BAD_CAST "xtsSkipAllTestsInSuite()",
		.testFunction = _testSkipAllTestsInSuite,
	}, {
		.id = BAD_CAST "apply_skip_list",
		.displayName = BAD_CAST "xtsApplySkipList()",
		.testFunction = _testApplySkipList,
		.flags = XTS_FLAG_CHECK_MEMORY,
		.expectedMemoryDelta = 0
	}, {
		.id = BAD_CAST "apply_skip_list_invalid_input",
		.displayName = BAD_CAST "xtsApplySkipList(): invalid input",
		.testFunction = _testApplySkipListInvalidInput,
		.flags = XTS_FLAG_CHECK_MEMORY,
		.expectedMemoryDelta = 0
	}, {
		.id = BAD_CAST "run_test_free_error",
		.displayName = BAD_CAST "xtsRunTest(): malloc'ed error",
		.testFunction = _testRunTestAndFreeError,
		.flags = XTS_FLAG_CHECK_MEMORY,
		.expectedMemoryDelta = 0
	}
};

xtsFixture xtsTestSanityFixture =
{
	.id = BAD_CAST "sanity",
	.displayName = BAD_CAST "test system sanity test group",
	.setup = NULL,
	.teardown = NULL,
	.test_count = sizeof(sanity_tests) / sizeof(sanity_tests[0]),
	.tests = &sanity_tests[0]
};
