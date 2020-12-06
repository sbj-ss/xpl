#include "xts.h"
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <libxml/chvalid.h>
#include <Configuration.h>

#define SINGLE_TEST_PREAMBLE "  "
#define FIXTURE_PREAMBLE "------------------------------------------\n"
#define SUMMARY_PREAMBLE "==========================================\n"
#define UNKNOWN_ERROR_MESSAGE BAD_CAST "(Unknown error)"

static bool use_ansi_colors = false;

void xtsInitContext(xtsContextPtr ctxt, xtsFailMode fail_mode, xtsVerbosity verbosity)
{
	ctxt->env = NULL;
	ctxt->error = NULL;
	ctxt->error_malloced = false;
	ctxt->fail_mode = fail_mode;
	ctxt->verbosity = verbosity;
	ctxt->passed = 0;
	ctxt->skipped = 0;
	ctxt->total = 0;
	ctxt->initial_mem_blocks = xmlMemBlocks();
}

bool xtsRunTest(const xtsTestPtr test, xtsContextPtr ctxt)
{
	bool ok;
	int mem_blocks;

	assert(test);
	assert(ctxt);
	assert(test->id);
	assert(test->displayName);
	assert(test->testFunction);

	ctxt->total++;
	if (test->flags & XTS_FLAG_SKIP)
	{
		if (ctxt->verbosity >= XTS_VERBOSITY_TEST)
			xmlGenericError(xmlGenericErrorContext, "%s%03d %sSkipping%s %s (#%s)...\n",
				SINGLE_TEST_PREAMBLE, ctxt->total,
				use_ansi_colors? "\033[33m": "", use_ansi_colors? "\033[0m": "",
				test->displayName, test->id);
		ctxt->skipped++;
		return true;
	}
	if (test->flags & XTS_FLAG_CHECK_MEMORY)
		mem_blocks = xmlMemBlocks();
	ok = test->testFunction(ctxt);
	if (ok)
	{
		if ((test->flags & XTS_FLAG_CHECK_MEMORY) &&
			xmlMemBlocks() - test->expectedMemoryDelta != mem_blocks)
		{
			if (ctxt->verbosity >= XTS_VERBOSITY_TEST)
				xmlGenericError(xmlGenericErrorContext,
					"%s%03d %s (#%s): %sFAILED%s: expected %d used memory blocks, got %d\n",
					SINGLE_TEST_PREAMBLE, ctxt->total, test->displayName, test->id,
					use_ansi_colors? "\033[31m": "", use_ansi_colors? "\033[0m": "",
					test->expectedMemoryDelta, xmlMemBlocks() - mem_blocks);
			return false;
		}
		if (ctxt->verbosity >= XTS_VERBOSITY_TEST)
			xmlGenericError(xmlGenericErrorContext, "%s%03d %s (#%s): %sOK%s\n",
				SINGLE_TEST_PREAMBLE, ctxt->total, test->displayName, test->id,
				use_ansi_colors? "\033[32m": "", use_ansi_colors? "\033[0m": "");
		ctxt->passed++;
	} else {
		if (ctxt->verbosity >= XTS_VERBOSITY_TEST)
			xmlGenericError(xmlGenericErrorContext, "%s%03d %s (#%s): %sFAILED%s: %s\n",
				SINGLE_TEST_PREAMBLE, ctxt->total, test->displayName, test->id,
				use_ansi_colors? "\033[31m": "", use_ansi_colors? "\033[0m": "",
				ctxt->error? ctxt->error: UNKNOWN_ERROR_MESSAGE);
	}
	if (ctxt->error && ctxt->error_malloced)
	{
		XPL_FREE(ctxt->error);
		ctxt->error_malloced = false;
	}
	ctxt->error = NULL;
	return ok;
}

bool xtsRunFixture(const xtsFixturePtr fixture, xtsContextPtr ctxt)
{
	bool ok = true;
	int i, old_passed, old_skipped, old_total;
	xtsTestPtr test;

	assert(fixture);
	assert(ctxt);
	assert(fixture->id);
	assert(fixture->displayName);
	if (fixture->test_count)
		assert(fixture->tests);

	old_passed = ctxt->passed;
	old_skipped = ctxt->skipped;
	old_total = ctxt->total;

	if (ctxt->verbosity >= XTS_VERBOSITY_FIXTURE)
		xmlGenericError(xmlGenericErrorContext, "%sSetting up fixture: %s (@%s)\n",
			FIXTURE_PREAMBLE, fixture->displayName, fixture->id);
	ctxt->env = xmlHashCreate(16);
	if (fixture->setup)
		if (!fixture->setup(ctxt))
		{
			if (fixture->teardown)
				fixture->teardown(ctxt);
			xmlHashFree(ctxt->env, NULL);
			if (ctxt->verbosity >= XTS_VERBOSITY_FIXTURE)
				xmlGenericError(xmlGenericErrorContext, "Fixture.setup() %sFAILED%s: %s\n",
					use_ansi_colors? "\033[31m": "", use_ansi_colors? "\033[0m": "",
					ctxt->error? ctxt->error: UNKNOWN_ERROR_MESSAGE);
			return false;
		}

	for (i = 0, test = fixture->tests; i < fixture->test_count; i++, test++)
	{
		ok &= xtsRunTest(test, ctxt);
		if (!ok && (ctxt->fail_mode == XTS_FAIL_FIRST))
			break;
	}

	if (fixture->teardown)
		fixture->teardown(ctxt);
	xmlHashFree(ctxt->env, NULL);
	if (ctxt->verbosity >= XTS_VERBOSITY_FIXTURE)
		xmlGenericError(xmlGenericErrorContext, "Fixture @%s: %d passed, %d failed, %d skipped, %d total\n",
			fixture->id,
			ctxt->passed - old_passed,
			(ctxt->total - old_total) - (ctxt->passed - old_passed) - (ctxt->skipped - old_skipped),
			ctxt->skipped - old_skipped,
			ctxt->total - old_total
		);
	return ok;
}

bool xtsRunSuite(const xtsFixturePtr *fixtures, xtsContextPtr ctxt)
{
	xtsFixturePtr fixture;
	bool ok = true;

	assert(fixtures);
	assert(ctxt);
	if (ctxt->verbosity > XTS_VERBOSITY_QUIET)
		xmlGenericError(xmlGenericErrorContext, "Running test suite...\n");
	for (; (fixture = *fixtures); fixtures++)
	{
		ok &= xtsRunFixture(fixture, ctxt);
		if (!ok && (ctxt->fail_mode == XTS_FAIL_FIRST))
			break;
	}
	if (ctxt->verbosity > XTS_VERBOSITY_QUIET)
		xmlGenericError(xmlGenericErrorContext, "%sDone. %d passed, %d failed, %d skipped, %d total\n",
			SUMMARY_PREAMBLE, ctxt->passed, ctxt->total - ctxt->passed - ctxt->skipped,
			ctxt->skipped, ctxt->total);
	return ok;
}

/* skipping control */
#define XTS_SKIP_HASH_SIZE 16

xtsTestPtr xtsLocateTest(const xtsFixturePtr fixture, const xmlChar *test_id)
{
	xtsTestPtr test;
	int i;

	assert(fixture);
	assert(test_id);
	for (i = 0, test = fixture->tests; i < fixture->test_count; i++, test++)
		if (!xmlStrcasecmp(test->id, test_id))
			return test;
	return NULL;
}

xtsFixturePtr xtsLocateFixture(const xtsFixturePtr *suite, const xmlChar *fixture_id)
{
	xtsFixturePtr fixture;

	assert(suite);
	assert(fixture_id);
	for (; (fixture = *suite); suite++)
		if (!xmlStrcasecmp(fixture->id, fixture_id))
			return fixture;
	return NULL;
}

void xtsSkipSingleTest(xtsTestPtr test, bool skip)
{
	assert(test);
	if (skip)
		test->flags |= XTS_FLAG_SKIP;
	else
		test->flags &= ~XTS_FLAG_SKIP;
}

void xtsSkipAllTestsInFixture(xtsFixturePtr fixture, bool skip)
{
	xtsTestPtr test;
	int i;

	assert(fixture);
	for (i = 0, test = fixture->tests; i < fixture->test_count; i++, test++)
		xtsSkipSingleTest(test, skip);
}

void xtsSkipAllTestsInSuite(xtsFixturePtr *suite, bool skip)
{
	xtsFixturePtr fixture;

	assert(suite);
	for (; (fixture = *suite); suite++)
		xtsSkipAllTestsInFixture(fixture, skip);
}

bool xtsSkipTest(xtsFixturePtr *suite, const xmlChar *fixture_id, const xmlChar *test_id, bool skip)
{
	xtsFixturePtr fixture = NULL;
	xtsTestPtr test;

	assert(suite);
	assert(fixture_id);
	assert(test_id);

	if (!(fixture = xtsLocateFixture(suite, fixture_id)))
		return false;
	if (!(test = xtsLocateTest(fixture, test_id)))
		return false;
	xtsSkipSingleTest(test, skip);
	return true;
}

typedef struct _xtsSkipListParserCtxt
{
	const xmlChar *cur;
	const xmlChar *new_cur;
	bool skip;
	xtsFixturePtr *suite;
	xtsFixturePtr fixture;
	xmlChar *error;
} xtsSkipListParserCtxt, *xtsSkipListParserCtxtPtr;

static void _omitSkipListSpace(xtsSkipListParserCtxtPtr ctxt)
{
	while (xmlIsBlank_ch(*(ctxt->cur)))
		ctxt->cur++;
}

static xmlChar* _extractEntityId(xtsSkipListParserCtxtPtr ctxt)
{
	const xmlChar *ent_end;
	xmlChar *ent_id;
	int ent_id_len;

	ent_end = ctxt->cur;
	while (*ent_end
		&& *ent_end != (xmlChar) '*'
		&& *ent_end != (xmlChar) '#'
		&& *ent_end != (xmlChar) '@'
		&& *ent_end != (xmlChar) ';'
		&& !xmlIsBlank_ch(*ent_end)
	)
		ent_end++;
	ent_id_len = ent_end - ctxt->cur;
	ent_id = (xmlChar*) XPL_MALLOC(ent_id_len + 1);
	strncpy((char*) ent_id, (char*) ctxt->cur, ent_id_len);
	*(ent_id + ent_id_len) = 0;
	ctxt->new_cur = ent_end;
	return ent_id;
}

static bool _parseSkipListTests(xtsSkipListParserCtxtPtr ctxt)
{
	xmlChar *test_id;
	xtsTestPtr test;

	assert(ctxt);
	assert(ctxt->cur);
	assert(ctxt->fixture);
	if (*(ctxt->cur) == (xmlChar) '*')
	{
		xtsSkipAllTestsInFixture(ctxt->fixture, ctxt->skip);
		ctxt->cur++;
		return true; /* we don't expect #names afterwards */
	}
	if (*(ctxt->cur) != (xmlChar) '#')
	{
		ctxt->error = BAD_CAST "test names must start with #";
		return false;
	}
	ctxt->cur++;
	_omitSkipListSpace(ctxt);
	test_id = _extractEntityId(ctxt);
	test = xtsLocateTest(ctxt->fixture, test_id);
	XPL_FREE(test_id);
	if (!test)
	{
		ctxt->error = BAD_CAST "can't locate test";
		return false;
	}
	xtsSkipSingleTest(test, ctxt->skip);
	ctxt->cur = ctxt->new_cur;
	_omitSkipListSpace(ctxt);
	if (*(ctxt->cur) == (xmlChar) '#')
		return _parseSkipListTests(ctxt);
	return true;
}

static bool _parseSkipListFixture(xtsSkipListParserCtxtPtr ctxt)
{
	xmlChar *fixture_id;

	assert(ctxt);
	assert(ctxt->cur);
	if (*(ctxt->cur) == (xmlChar) '*')
	{
		xtsSkipAllTestsInSuite(ctxt->suite, ctxt->skip);
		ctxt->cur++;
		return true;
	}
	if (*(ctxt->cur) != (xmlChar) '@')
	{
		ctxt->error = BAD_CAST "fixture names must start with @";
		return false;
	}
	ctxt->cur++;
	_omitSkipListSpace(ctxt);
	fixture_id = _extractEntityId(ctxt);
	ctxt->fixture = xtsLocateFixture(ctxt->suite, fixture_id);
	XPL_FREE(fixture_id);
	if (!ctxt->fixture)
	{
		ctxt->error = BAD_CAST "can't locate fixture";
		return false;
	}
	ctxt->cur = ctxt->new_cur;
	_omitSkipListSpace(ctxt);
	if (!_parseSkipListTests(ctxt))
		return false;
	_omitSkipListSpace(ctxt);
	if (*ctxt->cur == (xmlChar) '@')
		return _parseSkipListFixture(ctxt);
	return true;
}

static bool _parseSkipListStatements(xtsSkipListParserCtxtPtr ctxt)
{
	assert(ctxt);
	assert(ctxt->cur);
	_omitSkipListSpace(ctxt);
	if (!*(ctxt->cur))
		return true;
	if (*(ctxt->cur) == (xmlChar) 'i')
		ctxt->skip = false;
	else if (*(ctxt->cur) == (xmlChar) 's')
		ctxt->skip = true;
	else {
		ctxt->error = BAD_CAST "unknown operation";
		return false;
	}
	ctxt->cur++;
	_omitSkipListSpace(ctxt);
	if (*(ctxt->cur) != (xmlChar) ':')
	{
		ctxt->error = BAD_CAST "missing colon after operation";
		return false;
	}
	ctxt->cur++;
	_omitSkipListSpace(ctxt);
	if (!_parseSkipListFixture(ctxt))
		return false;
	if (*(ctxt->cur) != (xmlChar) ';')
	{
		ctxt->error = BAD_CAST "statements must end with a semicolon";
		return false;
	}
	ctxt->cur++;
	if (*(ctxt->cur))
		return _parseSkipListStatements(ctxt);
	return true;
}

bool xtsApplySkipList(const xmlChar *s, xtsFixturePtr *suite, xmlChar **error)
{
	xtsSkipListParserCtxt ctxt;
	bool ok;
	static const char *fmt = "%s at ...%s...";
	xmlChar part[20];
	int error_len;

	assert(s);
	assert(suite);
	assert(error);
	memset(&ctxt, 0, sizeof(ctxt));
	ctxt.suite = suite;
	ctxt.cur = s;
	ok = _parseSkipListStatements(&ctxt);
	if (!ok)
	{
		strncpy((char*) part, (char*) ctxt.cur, sizeof(part));
		part[sizeof(part) - 1] = 0;
		error_len = 128; /* introducing XPR dependency is bad */
		*error = XPL_MALLOC(error_len + 1);
		snprintf((char*) *error, error_len, fmt, ctxt.error, part);
		(*error)[error_len] = 0;
	} else
		*error = NULL;
	return ok;
}

void xtsDisplayUsage(char *prog_name)
{
	printf("Usage:\n");
	printf("%s [-h] [-c] [-s LIST]\n", prog_name);
	printf("\t-h: show usage and exit\n");
	printf("\t-c: use ANSI colors\n");
	printf("\t-s LIST: skip/include various tests\n");
}

int xtsInit(int argc, char** argv, xtsContextPtr ctxt, xtsFixturePtr *suite)
{
	int c;
	xmlChar *skip_list = NULL, *error;

	while (1)
	{
		c = getopt(argc, argv, "hcs:");
		if (c == -1)
			break;
		switch(c)
		{
			case 'h':
				xtsDisplayUsage(argv[0]);
				return 0;
			case 'c':
				use_ansi_colors = true;
				break;
			case 's':
				skip_list = BAD_CAST optarg;
				break;
			default:
				xtsDisplayUsage(argv[0]);
				return 1;
		}
	}
	xmlMemSetup(xmlMemFree, xmlMemMalloc, xmlMemRealloc, xmlMemoryStrdup);
	xtsInitContext(ctxt, XTS_FAIL_ALL, XTS_VERBOSITY_TEST); /* TODO command line switches */
	if (skip_list)
		if (!xtsApplySkipList(skip_list, suite, &error))
		{
			xmlGenericError(xmlGenericErrorContext, "error parsing skip list: %s\n", error);
			XPL_FREE(error);
			return 1;
		}
	return 0;
}

int xtsWrap(xtsFixturePtr *suite, xtsContextPtr ctxt)
{
	bool ok;
	int mem_delta;

	ok = xtsRunSuite(suite, ctxt);
	if (!ok)
		return 2;
	mem_delta = xmlMemBlocks() - ctxt->initial_mem_blocks;
	if (mem_delta)
	{
		xmlGenericError(xmlGenericErrorContext, "ERROR: expected 0 extra memory blocks, got %d\n", mem_delta);
		return 3;
	}
	return 0;
}
