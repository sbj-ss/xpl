/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2021     */
/******************************************/
#ifndef __xploptions_H
#define __xploptions_H

#include "Configuration.h"
#include <stdbool.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxpl/xpltree.h>

#ifdef __cplusplus
extern "C" {
#endif

XPLPUBFUN int XPLCALL
	xplGetBooleanValue(const xmlChar* str);
/* get option value. result must be freed. */
XPLPUBFUN xmlChar* XPLCALL
	xplGetOptionValue(const xmlChar *optionName, bool showPasswords, bool *found);
/* get all options in a node list */
XPLPUBFUN xmlNodePtr XPLCALL
	xplOptionsToList(const xmlNodePtr parent, const xplQName tagname, bool showPasswords);

typedef enum _xplSetOptionResult
{
	XPL_SET_OPTION_OK = 0,
	XPL_SET_OPTION_UNKNOWN_OPTION = -1,
	XPL_SET_OPTION_INVALID_VALUE = -2,
	XPL_SET_OPTION_INTERNAL_ERROR = -3
} xplSetOptionResult;

/* set option value. threads must be locked first. access rights aren't checked here */
XPLPUBFUN xplSetOptionResult XPLCALL
	xplSetOptionValue(const xmlChar *optionName, const xmlChar *value, bool byDefault);

XPLPUBFUN bool XPLCALL
	xplInitOptions(void);
XPLPUBFUN void XPLCALL
	xplCleanupOptions(void);

/* config variables */
XPLPUBVAR int cfgCheckSAMode;
XPLPUBVAR xmlChar *cfgDebugSaveFile;
XPLPUBVAR xmlChar *cfgDefaultEncoding;
XPLPUBVAR xmlChar *cfgDocRoot;
XPLPUBVAR int cfgEnableAssertions;
XPLPUBVAR int cfgErrorsToConsole;
XPLPUBVAR int cfgFlushLogFile;
XPLPUBVAR int cfgFoolproofDestructiveCommands;
XPLPUBVAR int cfgInitialMacroTableSize;
XPLPUBVAR xmlChar *cfgLogFileName;
XPLPUBVAR int cfgLuciferCompat;
XPLPUBVAR int cfgMacroContentCachingThreshold;
XPLPUBVAR int cfgMaxDatabaseConnectionLifetime; // in seconds
XPLPUBVAR int cfgMaxRecursionDepth;
XPLPUBVAR int cfgMinDebugPrintLevel;
XPLPUBVAR int cfgPrintTidyInfo;
XPLPUBVAR xmlChar *cfgProxyPassword;
XPLPUBVAR int cfgProxyPort;
XPLPUBVAR xmlChar *cfgProxyServer;
XPLPUBVAR xmlChar *cfgProxyUser;
extern xmlChar *cfgSaPassword;
XPLPUBVAR int cfgSessionLifetime;
XPLPUBVAR int cfgStackTrace;
XPLPUBVAR int cfgUseConsoleColors;
XPLPUBVAR int cfgUseWrappers;
XPLPUBVAR int cfgWarnOnAncestorModificationAttempt;
XPLPUBVAR int cfgWarnOnDeletedNodeReference;
XPLPUBVAR int cfgWarnOnDeletedThreadLandingPoint;
XPLPUBVAR int cfgWarnOnExpandedMacroContent;
XPLPUBVAR int cfgWarnOnFatalErrorsInIsolatedDocuments;
XPLPUBVAR int cfgWarnOnInvalidNodeType;
XPLPUBVAR int cfgWarnOnInvalidXplNsUri;
XPLPUBVAR int cfgWarnOnJsonxSerializationIssues;
XPLPUBVAR int cfgWarnOnMacroRedefinition;
XPLPUBVAR int cfgWarnOnMissingInheritBase;
XPLPUBVAR int cfgWarnOnMissingMacroContent;
XPLPUBVAR int cfgWarnOnMultipleSelection;
XPLPUBVAR int cfgWarnOnNeverLaunchedThreads;
XPLPUBVAR int cfgWarnOnNoAwaitableThreads;
XPLPUBVAR int cfgWarnOnNoExpectParam;
XPLPUBVAR int cfgWarnOnUninitializedTimer;
XPLPUBVAR int cfgWarnOnUnknownCommand;
XPLPUBVAR int cfgWarnOnWontReplace;
XPLPUBVAR xmlChar *cfgXplNsUri;

/* Config defaults - for reference */
#define DEFAULT_CHECK_SA_MODE true
#define DEFAULT_DEFAULT_ENCODING (BAD_CAST "utf-8")
#define DEFAULT_ENABLE_ASSERTIONS false
#define DEFAULT_ERRORS_TO_CONSOLE false
#define DEFAULT_FLUSH_LOG_FILE false
#define DEFAULT_FOOLPROOF_DESTRUCTIVE_COMMANDS false
#define DEFAULT_INITIAL_MACRO_TABLE_SIZE 16
#define DEFAULT_LUCIFER_COMPAT true
#define DEFAULT_MACRO_CONTENT_CACHING_THRESHOLD -1 /* disabled */
#define DEFAULT_MAX_DATABASE_CONNECTION_LIFETIME 86400
#define DEFAULT_MAX_RECURSION_DEPTH 100
#define DEFAULT_MIN_DEBUG_PRINT_LEVEL 0
#define DEFAULT_PRINT_TIDY_INFO false
#define DEFAULT_PROXY_PORT 80
#define DEFAULT_SESSION_LIFETIME 86400
#define DEFAULT_STACK_TRACE false
#define DEFAULT_USE_CONSOLE_COLORS true
#define DEFAULT_USE_WRAPPERS false
#define DEFAULT_WARN_ON_ANCESTOR_MODIFICATION_ATTEMPT true
#define DEFAULT_WARN_ON_DELETED_NODE_REFERENCE true
#define DEFAULT_WARN_ON_DELETED_THREAD_LANDING_POINT true
#define DEFAULT_WARN_ON_FATAL_ERRORS_IN_ISOLATED_DOCUMENTS true
#define DEFAULT_WARN_ON_EXPANDED_MACRO_CONTENT true
#define DEFAULT_WARN_ON_INVALID_NODE_TYPE true
#define DEFAULT_WARN_ON_INVALID_XPL_NS_URI true
#define DEFAULT_WARN_ON_JSONX_SERIALIZATION_ISSUES true
#define DEFAULT_WARN_ON_MACRO_REDEFINITION false
#define DEFAULT_WARN_ON_MISSING_MACRO_CONTENT false
#define DEFAULT_WARN_ON_MISSING_INHERIT_BASE false
#define DEFAULT_WARN_ON_MULTIPLE_SELECTION true
#define DEFAULT_WARN_ON_NEVER_LAUNCHED_THREADS true
#define DEFAULT_WARN_ON_NO_AWAITABLE_THREADS true
#define DEFAULT_WARN_ON_NO_EXPECT_PARAM true
#define DEFAULT_WARN_ON_UNINITIALIZED_TIMER true
#define DEFAULT_WARN_ON_UNKNOWN_COMMAND true
#define DEFAULT_WARN_ON_WONT_REPLACE false
#define DEFAULT_XPL_NS_URI (BAD_CAST "urn:x-xpl:xpl")

#ifdef __cplusplus
}
#endif
#endif
