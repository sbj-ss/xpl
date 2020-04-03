/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __options_H
#define __options_H

#include "Configuration.h"
#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

XPLPUBFUN int XPLCALL
	xplGetBooleanValue(xmlChar* str);
/* Получить строковое значение параметра конфигурации. Память нужно
 * будет освободить. */
XPLPUBFUN xmlChar* XPLCALL
	xplGetOptionValue(xmlChar *optionName, BOOL showPasswords);
/* Получить список узлов со всеми параметрами конфигурации. */
XPLPUBFUN xmlNodePtr XPLCALL
	xplOptionsToList(xmlDocPtr doc, xmlNodePtr parent, xmlChar *tagName, BOOL showTags, BOOL showPasswords);

typedef enum _xplSetOptionResult
{
	XPL_SET_OPTION_OK = 0,
	XPL_SET_OPTION_UNKNOWN_OPTION = -1,
	XPL_SET_OPTION_INVALID_VALUE = -2,
	XPL_SET_OPTION_INTERNAL_ERROR = -3
} xplSetOptionResult;

/* Установить значение параметра конфигурации. Перед этим необходимо
 * вызвать блокировку создания новых потоков. Права доступа не проверяются! */
XPLPUBFUN xplSetOptionResult XPLCALL
	xplSetOptionValue(xmlChar *optionName, xmlChar *value, BOOL byDefault);

XPLPUBFUN int XPLCALL
	xplReadOptions(xmlNodePtr opt_root);
XPLPUBFUN void XPLCALL
	xplAssignDefaultsToAllOptions(void);
XPLPUBFUN void XPLCALL
	xplCleanupOptions(void);

/* config variables */
XPLPUBVAR int cfgCheckDbOnStartup;
XPLPUBVAR int cfgCheckSAMode;
XPLPUBVAR xmlChar *cfgDebugSaveFile;
XPLPUBVAR xmlChar *cfgDefaultEncoding;
XPLPUBVAR xmlChar *cfgDocRoot;
XPLPUBVAR int cfgEnableAssertions;
XPLPUBVAR int cfgErrorsToConsole;
XPLPUBVAR int cfgFoolproofDestructiveCommands;
XPLPUBVAR int cfgInitialMacroTableSize;
XPLPUBVAR xmlChar *cfgLogFileName;
XPLPUBVAR int cfgLuciferCompat;
XPLPUBVAR int cfgMacroContentCachingThreshold;
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
XPLPUBVAR int cfgWarnOnExpandedMacroContent;
XPLPUBVAR int cfgWarnOnInvalidXplNsUri;
XPLPUBVAR int cfgWarnOnMacroRedefinition;
XPLPUBVAR int cfgWarnOnMissingInheritBase;
XPLPUBVAR int cfgWarnOnMissingMacroContent;
XPLPUBVAR int cfgWarnOnNoExpectParam;
XPLPUBVAR int cfgWarnOnUnknownCommand;
XPLPUBVAR xmlChar *cfgXplNsUri;

/* Config defaults - for reference */
#define DEFAULT_CHECK_DB_ON_STARTUP TRUE
#define DEFAULT_CHECK_SA_MODE TRUE
#define DEFAULT_DEFAULT_ENCODING (BAD_CAST "utf-8")
#define DEFAULT_ENABLE_ASSERTIONS FALSE
#define DEFAULT_ERRORS_TO_CONSOLE FALSE
#define DEFAULT_FOOLPROOF_DESTRUCTIVE_COMMANDS FALSE
#define DEFAULT_INITIAL_MACRO_TABLE_SIZE 16
#define DEFAULT_LUCIFER_COMPAT TRUE
#define DEFAULT_MACRO_CONTENT_CACHING_THRESHOLD -1 /* disabled */
#define DEFAULT_MAX_RECURSION_DEPTH 100
#define DEFAULT_MIN_DEBUG_PRINT_LEVEL 0
#define DEFAULT_PRINT_TIDY_INFO FALSE
#define DEFAULT_PROXY_PORT 80
#define DEFAULT_SESSION_LIFETIME 86400
#define DEFAULT_STACK_TRACE FALSE
#define DEFAULT_USE_CONSOLE_COLORS TRUE
#define DEFAULT_USE_WRAPPERS FALSE
#define DEFAULT_WARN_ON_ANCESTOR_MODIFICATION_ATTEMPT FALSE
#define DEFAULT_WARN_ON_DELETED_NODE_REFERENCE FALSE
#define DEFAULT_WARN_ON_EXPANDED_MACRO_CONTENT FALSE
#define DEFAULT_WARN_ON_INVALID_XPL_NS_URI FALSE
#define DEFAULT_WARN_ON_MACRO_REDEFINITION FALSE
#define DEFAULT_WARN_ON_MISSING_MACRO_CONTENT FALSE
#define DEFAULT_WARN_ON_MISSING_INHERIT_BASE FALSE
#define DEFAULT_WARN_ON_NO_EXPECT_PARAM TRUE
#define DEFAULT_WARN_ON_UNKNOWN_COMMAND FALSE
#define DEFAULT_XPL_NS_URI (BAD_CAST "http://tch.org/xpl")

#ifdef __cplusplus
}
#endif
#endif
