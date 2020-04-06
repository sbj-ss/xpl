/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __tests_H
#define __tests_H

#include "Configuration.h"
#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	XTS_FAIL_FIRST,
	XTS_FAIL_ALL
} xtsFailMode;

typedef enum {
	XTS_VERBOSITY_QUIET,
	XTS_VERBOSITY_SUMMARY,
	XTS_VERBOSITY_FIXTURE,
	XTS_VERBOSITY_TEST
} xtsVerbosity;

typedef struct _xtsContext
{
	xmlChar *error;
	BOOL error_malloced;
	xmlHashTablePtr env;
	xtsFailMode fail_mode;
	xtsVerbosity verbosity;
	int total;
	int passed;
	int skipped;
} xtsContext, *xtsContextPtr;

void xtsInitContext(xtsContextPtr ctxt, xtsFailMode fail_mode, xtsVerbosity verbosity);

/* single test */
typedef BOOL (*xtsFunction) (xtsContextPtr ctxt);

#define XTS_FLAG_CHECK_MEMORY 0x0001
#define XTS_FLAG_SKIP 0x0002

typedef struct _xtsTest
{
	xmlChar *id;
	xmlChar *displayName;
	xtsFunction testFunction;
	int flags;
	int expectedMemoryDelta;
} xtsTest, *xtsTestPtr;

BOOL xtsRunTest(const xtsTestPtr test, xtsContextPtr ctxt);

/* test fixture */
typedef BOOL (*xtsFixtureSetupFunction) (xtsContextPtr ctxt);
typedef void (*xtsFixtureTeardownFunction) (xtsContextPtr ctxt);

typedef struct _xtsFixture
{
	xmlChar *id;
	xmlChar *displayName;
	xtsFixtureSetupFunction setup;
	xtsFixtureTeardownFunction teardown;
	int test_count;
	xtsTestPtr tests;
} xtsFixture, *xtsFixturePtr;

BOOL xtsRunFixture(const xtsFixturePtr fixture, xtsContextPtr ctxt);
xtsTestPtr xtsLocateTest(const xtsFixturePtr fixture, const xmlChar *test_id);

/* entire suite */
/* fixtures must be NULL-terminated */
BOOL xtsRunSuite(const xtsFixturePtr *suite, xtsContextPtr ctxt);
xtsFixturePtr xtsLocateFixture(const xtsFixturePtr *suite, const xmlChar *fixture_id);

/* skipping control */
/* grammar ("s" for "skip", "i" for "include"):
 * LIST = { STATEMENT ";" }
 * LINE = OPERATION ":" FIXTURE { FIXTURE }
 * OPERATION = "s" | "i"
 * FIXTURE = "*" | ("@" identifier TEST { TEST })
 * TEST = "*" | ("#" identifier)
 *
 * last command wins
 */

void xtsSkipSingleTest(xtsTestPtr test, BOOL skip);
void xtsSkipAllTestsInFixture(xtsFixturePtr fixture, BOOL skip);
void xtsSkipAllTestsInSuite(xtsFixturePtr *suite, BOOL skip);
BOOL xtsSkipTest(xtsFixturePtr *suite, const xmlChar *fixture_id, const xmlChar *test_id, BOOL skip);
BOOL xtsApplySkipList(const xmlChar *s, xtsFixturePtr *suite, xmlChar **error);

#ifdef __cplusplus
}
#endif
#endif
