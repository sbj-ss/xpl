#include "Tests.h"
#include <assert.h>
#include <stdio.h>
#include <libxml/chvalid.h>

#define SINGLE_TEST_PREAMBLE "  "
#define FIXTURE_PREAMBLE "------------------------------------------\n"
#define SUMMARY_PREAMBLE "==========================================\n"
#define UNKNOWN_ERROR_MESSAGE BAD_CAST "(Unknown error)"

void xtsInitContext(xtsContextPtr ctxt, xtsFailMode fail_mode, xtsVerbosity verbosity)
{
	ctxt->env = NULL;
	ctxt->error = NULL;
	ctxt->error_malloced = FALSE;
	ctxt->fail_mode = fail_mode;
	ctxt->verbosity = verbosity;
	ctxt->passed = 0;
	ctxt->skipped = 0;
	ctxt->total = 0;
}

BOOL xtsRunTest(const xtsTestPtr test, xtsContextPtr ctxt)
{
	BOOL ok;
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
			xmlGenericError(xmlGenericErrorContext, "%s%03d Skipping %s (#%s)...\n",
				SINGLE_TEST_PREAMBLE, ctxt->total, test->displayName, test->id);
		ctxt->skipped++;
		return TRUE;
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
					"%s%03d %s (#%s): FAILED: expected %d used memory blocks, got %d\n",
					SINGLE_TEST_PREAMBLE, ctxt->total, test->displayName, test->id,
					test->expectedMemoryDelta, xmlMemBlocks() - mem_blocks);
			return FALSE;
		}
		if (ctxt->verbosity >= XTS_VERBOSITY_TEST)
			xmlGenericError(xmlGenericErrorContext, "%s%03d %s (#%s): OK\n",
				SINGLE_TEST_PREAMBLE, ctxt->total, test->displayName, test->id);
		ctxt->passed++;
	} else {
		if (ctxt->verbosity >= XTS_VERBOSITY_TEST)
			xmlGenericError(xmlGenericErrorContext, "%s%03d %s (#%s): FAILED: %s\n",
				SINGLE_TEST_PREAMBLE, ctxt->total, test->displayName, test->id,
				ctxt->error? ctxt->error: UNKNOWN_ERROR_MESSAGE);
	}
	if (ctxt->error && ctxt->error_malloced)
	{
		xmlFree(ctxt->error);
		ctxt->error_malloced = FALSE;
	}
	ctxt->error = NULL;
	return ok;
}

BOOL xtsRunFixture(const xtsFixturePtr fixture, xtsContextPtr ctxt)
{
	BOOL ok = TRUE;
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
			xmlHashFree(ctxt->env, NULL);
			if (ctxt->verbosity >= XTS_VERBOSITY_FIXTURE)
				xmlGenericError(xmlGenericErrorContext, "Fixture.setup() failed: %s\n",
					ctxt->error? ctxt->error: UNKNOWN_ERROR_MESSAGE);
			return FALSE;
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

BOOL xtsRunSuite(const xtsFixturePtr *fixtures, xtsContextPtr ctxt)
{
	xtsFixturePtr fixture;
	BOOL ok = TRUE;

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

void xtsSkipSingleTest(xtsTestPtr test, BOOL skip)
{
	assert(test);
	if (skip)
		test->flags |= XTS_FLAG_SKIP;
	else
		test->flags &= ~XTS_FLAG_SKIP;
}

void xtsSkipAllTestsInFixture(xtsFixturePtr fixture, BOOL skip)
{
	xtsTestPtr test;
	int i;

	assert(fixture);
	for (i = 0, test = fixture->tests; i < fixture->test_count; i++, test++)
		xtsSkipSingleTest(test, skip);
}

void xtsSkipAllTestsInSuite(xtsFixturePtr *suite, BOOL skip)
{
	xtsFixturePtr fixture;

	assert(suite);
	for (; (fixture = *suite); suite++)
		xtsSkipAllTestsInFixture(fixture, skip);
}

BOOL xtsSkipTest(xtsFixturePtr *suite, const xmlChar *fixture_id, const xmlChar *test_id, BOOL skip)
{
	xtsFixturePtr fixture = NULL;
	xtsTestPtr test;

	assert(suite);
	assert(fixture_id);
	assert(test_id);


	if (!(fixture = xtsLocateFixture(suite, fixture_id)))
		return FALSE;
	if (!(test = xtsLocateTest(fixture, test_id)))
		return FALSE;
	xtsSkipSingleTest(test, skip);
	return TRUE;
}

typedef struct _xtsSkipListParserCtxt
{
	const xmlChar *cur;
	const xmlChar *new_cur;
	BOOL skip;
	xtsFixturePtr *suite;
	xtsFixturePtr fixture;
	xmlChar *error;
} xtsSkipListParserCtxt, *xtsSkipListParserCtxtPtr;

static void xtsOmitSkipListSpace(xtsSkipListParserCtxtPtr ctxt)
{
	while (xmlIsBlank_ch(*(ctxt->cur)))
		ctxt->cur++;
}

static xmlChar* xtsExtractEntityId(xtsSkipListParserCtxtPtr ctxt)
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
	ent_id = (xmlChar*) xmlMalloc(ent_id_len + 1);
	strncpy(ent_id, ctxt->cur, ent_id_len);
	*(ent_id + ent_id_len) = 0;
	ctxt->new_cur = ent_end;
	return ent_id;
}

static BOOL xtsParseSkipListTest(xtsSkipListParserCtxtPtr ctxt)
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
		return TRUE; /* we don't expect #names afterwards */
	}
	if (*(ctxt->cur) != (xmlChar) '#')
	{
		ctxt->error = BAD_CAST "test names must start with #";
		return FALSE;
	}
	ctxt->cur++;
	xtsOmitSkipListSpace(ctxt);
	test_id = xtsExtractEntityId(ctxt);
	test = xtsLocateTest(ctxt->fixture, test_id);
	xmlFree(test_id);
	if (!test)
	{
		ctxt->error = BAD_CAST "can't locate test";
		return FALSE;
	}
	xtsSkipSingleTest(test, ctxt->skip);
	ctxt->cur = ctxt->new_cur;
	xtsOmitSkipListSpace(ctxt);
	if (*(ctxt->cur) == (xmlChar) '#')
		return xtsParseSkipListTest(ctxt);
	return TRUE;
}

static BOOL xtsParseSkipListFixture(xtsSkipListParserCtxtPtr ctxt)
{
	xmlChar *fixture_id;

	assert(ctxt);
	assert(ctxt->cur);
	if (*(ctxt->cur) == (xmlChar) '*')
	{
		xtsSkipAllTestsInSuite(ctxt->suite, ctxt->skip);
		ctxt->cur++;
		return TRUE;
	}
	if (*(ctxt->cur) != (xmlChar) '@')
	{
		ctxt->error = BAD_CAST "fixture names must start with @";
		return FALSE;
	}
	ctxt->cur++;
	xtsOmitSkipListSpace(ctxt);
	fixture_id = xtsExtractEntityId(ctxt);
	ctxt->fixture = xtsLocateFixture(ctxt->suite, fixture_id);
	xmlFree(fixture_id);
	if (!ctxt->fixture)
	{
		ctxt->error = "can't locate fixture";
		return FALSE;
	}
	ctxt->cur = ctxt->new_cur;
	xtsOmitSkipListSpace(ctxt);
	if (!xtsParseSkipListTest(ctxt))
		return FALSE;
	xtsOmitSkipListSpace(ctxt);
	if (*ctxt->cur == (xmlChar) '@')
		return xtsParseSkipListFixture(ctxt);
	return TRUE;
}

static BOOL xtsParseSkipListStatement(xtsSkipListParserCtxtPtr ctxt)
{
	assert(ctxt);
	assert(ctxt->cur);
	if (!*(ctxt->cur))
		return TRUE;
	if (*(ctxt->cur) == (xmlChar) 'i')
		ctxt->skip = FALSE;
	else if (*(ctxt->cur) == (xmlChar) 's')
		ctxt->skip = TRUE;
	else {
		ctxt->error = BAD_CAST "unknown operation";
		return FALSE;
	}
	ctxt->cur++;
	xtsOmitSkipListSpace(ctxt);
	if (*(ctxt->cur) != (xmlChar) ':')
	{
		ctxt->error = BAD_CAST "missing colon after operation";
		return FALSE;
	}
	ctxt->cur++;
	xtsOmitSkipListSpace(ctxt);
	if (!xtsParseSkipListFixture(ctxt))
		return FALSE;
	if (*(ctxt->cur) != (xmlChar) ';')
	{
		ctxt->error = "statements must end with a semicolon";
		return FALSE;
	}
	ctxt->cur++;
	xtsOmitSkipListSpace(ctxt);
	if (*(ctxt->cur))
		return xtsParseSkipListStatement(ctxt);
	return TRUE;
}

BOOL xtsApplySkipList(const xmlChar *s, xtsFixturePtr *suite, xmlChar **error)
{
	xtsSkipListParserCtxt ctxt;
	BOOL ok;
	static const xmlChar *fmt = BAD_CAST "%s at ...%s...";
	xmlChar part[20];
	int error_len;

	assert(s);
	assert(suite);
	assert(error);
	memset(&ctxt, 0, sizeof(ctxt));
	ctxt.suite = suite;
	ctxt.cur = s;
	xtsOmitSkipListSpace(&ctxt);
	ok = xtsParseSkipListStatement(&ctxt);
	if (!ok)
	{
		strncpy(part, ctxt.cur, sizeof(part));
		part[20] = 0;
		error_len = 128; /* introducing XPR dependency is bad */
		*error = xmlMalloc(error_len + 1);
		snprintf(*error, error_len, fmt, ctxt.error, part);
		(*error)[error_len] = 0;
	} else
		*error = NULL;
	return ok;
}
