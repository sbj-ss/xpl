#include <libxpl/abstraction/xef.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>
#include <stdint.h>

int cfgCheckDbOnStartup;
int cfgCheckSAMode;
xmlChar *cfgDebugSaveFile;
xmlChar *cfgDefaultEncoding;
xmlChar *cfgDocRoot;
int cfgEnableAssertions;
int cfgErrorsToConsole;
int cfgFoolproofDestructiveCommands;
int cfgInitialMacroTableSize;
xmlChar *cfgLogFileName;
int cfgLuciferCompat;
int cfgMacroContentCachingThreshold;
int cfgMaxRecursionDepth;
int cfgMinDebugPrintLevel;
int cfgPrintTidyInfo;
xmlChar *cfgProxyPassword;
int cfgProxyPort;
xmlChar *cfgProxyServer;
xmlChar *cfgProxyUser;
xmlChar *cfgSaPassword;
int cfgSessionLifetime;
int cfgStackTrace;
int cfgUseConsoleColors;
int cfgUseWrappers;
int cfgWarnOnAncestorModificationAttempt;
int cfgWarnOnDeletedNodeReference;
int cfgWarnOnExpandedMacroContent;
int cfgWarnOnFatalErrorsInIsolatedDocuments;
int cfgWarnOnInvalidXplNsUri;
int cfgWarnOnInvalidNodeType;
int cfgWarnOnJsonxSerializationIssues;
int cfgWarnOnMacroRedefinition;
int cfgWarnOnMissingInheritBase;
int cfgWarnOnMissingMacroContent;
int cfgWarnOnMultipleSelection;
int cfgWarnOnNoExpectParam;
int cfgWarnOnUninitializedTimer;
int cfgWarnOnUnknownCommand;
int cfgWarnOnWontReplace;
xmlChar *cfgXplNsUri;

static xmlHashTablePtr config_entries_hash = NULL;

typedef enum {
	CFG_TYPE_STRING,
	CFG_TYPE_INT,
	CFG_TYPE_BOOL,
	CFG_TYPE_DEFERRED /* waiting until a command registers it and decodes */
} xplCfgValueType;

typedef enum {
	CFG_STATE_UNASSIGNED,
	CFG_STATE_ASSIGNED,
	CFG_STATE_DEFAULT
} xplCfgState;

#define CFG_OPTION_IS_PASSWORD 0x01
#define CFG_OPTION_STORED_AS_HASH 0x02
#define CFG_OPTION_RESTART_REQUIRED 0x04
#define CFG_OPTION_DEPRECATED 0x08

typedef struct _xplConfigEntry
{
	xplCfgValueType cfg_type;
	void *value_ptr;
	xmlChar *name;
	void *default_value;
	int options;
	xplCfgState state;
} xplConfigEntry, *xplConfigEntryPtr;

xplConfigEntry config_entries[] =
{
/* cfg_type, value_ptr, name, default_value */
	{
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgCheckDbOnStartup,
		.name = BAD_CAST "CheckDbOnStartup",
		.default_value = (void*) DEFAULT_CHECK_DB_ON_STARTUP
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgCheckSAMode,
		.name = BAD_CAST "CheckSAMode",
		.default_value = (void*) DEFAULT_CHECK_SA_MODE
	}, {
		.cfg_type = CFG_TYPE_STRING,
		.value_ptr = &cfgDebugSaveFile,
		.name = BAD_CAST "DebugSaveFile",
		.default_value = NULL
	}, {
		.cfg_type = CFG_TYPE_STRING,
		.value_ptr = &cfgDefaultEncoding,
		.name = BAD_CAST "DefaultEncoding",
		.default_value = (void*) DEFAULT_DEFAULT_ENCODING
	}, {
		.cfg_type = CFG_TYPE_STRING,
		.value_ptr = &cfgDocRoot,
		.name = BAD_CAST "DocRoot",
		.default_value = NULL
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgEnableAssertions,
		.name = BAD_CAST "EnableAssertions",
		.default_value = (void*) DEFAULT_ENABLE_ASSERTIONS
	}, {
		.cfg_type = CFG_TYPE_STRING,
		.value_ptr = NULL,
		.name = BAD_CAST "ErrorSourceName",
		.default_value = NULL,
		.options = CFG_OPTION_DEPRECATED
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgErrorsToConsole,
		.name = BAD_CAST "ErrorsToConsole",
		.default_value = (void*) DEFAULT_ERRORS_TO_CONSOLE
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgFoolproofDestructiveCommands,
		.name = BAD_CAST "FoolproofDestructiveCommands",
		.default_value = (void*) DEFAULT_FOOLPROOF_DESTRUCTIVE_COMMANDS
	}, {
		.cfg_type = CFG_TYPE_INT,
		.value_ptr = &cfgInitialMacroTableSize,
		.name = BAD_CAST "InitialMacroTableSize",
		.default_value = (void*) DEFAULT_INITIAL_MACRO_TABLE_SIZE
	}, {
		.cfg_type = CFG_TYPE_STRING,
		.value_ptr = &cfgLogFileName,
		.name = BAD_CAST "LogFile",
		.default_value = NULL
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgLuciferCompat,
		.name = BAD_CAST "LuciferCompatibility",
		.default_value = (void*) DEFAULT_LUCIFER_COMPAT
	}, {
		.cfg_type = CFG_TYPE_INT,
		.value_ptr = &cfgMacroContentCachingThreshold,
		.name = BAD_CAST "MacroContentCachingThreshold",
		.default_value = (void*) DEFAULT_MACRO_CONTENT_CACHING_THRESHOLD
	}, {
		.cfg_type = CFG_TYPE_INT,
		.value_ptr = &cfgMaxRecursionDepth,
		.name = BAD_CAST "MaxRecursionDepth",
		.default_value = (void*) DEFAULT_MAX_RECURSION_DEPTH
	}, {
		.cfg_type = CFG_TYPE_INT,
		.value_ptr = &cfgMinDebugPrintLevel,
		.name = BAD_CAST "MinDebugPrintLevel",
		.default_value = (void*) DEFAULT_MIN_DEBUG_PRINT_LEVEL
	}, {
		.cfg_type = CFG_TYPE_BOOL, /* TODO this should be somehow moved to XEF */
		.value_ptr = &cfgPrintTidyInfo,
		.name = BAD_CAST "PrintTidyInfo",
		.default_value = (void*) DEFAULT_PRINT_TIDY_INFO
	}, {
		.cfg_type = CFG_TYPE_STRING, /* TODO this too - together with all other proxy settings */
		.value_ptr = &cfgProxyPassword,
		.name = BAD_CAST "ProxyPassword",
		.default_value = NULL,
		.options = CFG_OPTION_IS_PASSWORD
	}, {
		.cfg_type = CFG_TYPE_INT,
		.value_ptr = &cfgProxyPort,
		.name = BAD_CAST "ProxyPort",
		.default_value = (void*) DEFAULT_PROXY_PORT
	}, {
		.cfg_type = CFG_TYPE_STRING,
		.value_ptr = &cfgProxyServer,
		.name = BAD_CAST "ProxyServer",
		.default_value = NULL
	}, {
		.cfg_type = CFG_TYPE_STRING,
		.value_ptr = &cfgProxyUser,
		.name = BAD_CAST "ProxyUser",
		.default_value = NULL
	}, {
		.cfg_type = CFG_TYPE_STRING,
		.value_ptr = &cfgSaPassword,
		.name = BAD_CAST "SAPassword",
		.default_value = (void*) "0BD4BC9866BA6963A3CA0563FD00578D90338EB5", /* 1111111 */
		.options = CFG_OPTION_IS_PASSWORD | CFG_OPTION_STORED_AS_HASH
	}, {
		.cfg_type = CFG_TYPE_INT,
		.value_ptr = &cfgSessionLifetime,
		.name = BAD_CAST "SessionLifetime",
		.default_value = (void*) DEFAULT_SESSION_LIFETIME
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgStackTrace,
		.name = BAD_CAST "StackTrace",
		.default_value = (void*) DEFAULT_STACK_TRACE
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgUseConsoleColors,
		.name = BAD_CAST "UseConsoleColors",
		.default_value = (void*) DEFAULT_USE_CONSOLE_COLORS
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgUseWrappers,
		.name = BAD_CAST "UseWrappers",
		.default_value = (void*) DEFAULT_USE_WRAPPERS
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnAncestorModificationAttempt,
		.name = BAD_CAST "WarnOnAncestorModificationAttempt",
		.default_value = (void*) DEFAULT_WARN_ON_ANCESTOR_MODIFICATION_ATTEMPT
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnDeletedNodeReference,
		.name = BAD_CAST "WarnOnDeletedNodeReference",
		.default_value = (void*) DEFAULT_WARN_ON_DELETED_NODE_REFERENCE
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnExpandedMacroContent,
		.name = BAD_CAST "WarnOnExpandedMacroContent",
		.default_value = (void*) DEFAULT_WARN_ON_EXPANDED_MACRO_CONTENT
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnFatalErrorsInIsolatedDocuments,
		.name = BAD_CAST "WarnOnFatalErrorsInIsolatedDocuments",
		.default_value = (void*) DEFAULT_WARN_ON_FATAL_ERRORS_IN_ISOLATED_DOCUMENTS
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnInvalidNodeType,
		.name = BAD_CAST "WarnOnInvalidNodeType",
		.default_value = (void*) DEFAULT_WARN_ON_INVALID_NODE_TYPE,
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnInvalidXplNsUri,
		.name = BAD_CAST "WarnOnInvalidXplNsUri",
		.default_value = (void*) DEFAULT_WARN_ON_INVALID_XPL_NS_URI
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnJsonxSerializationIssues,
		.name = BAD_CAST "WarnOnJsSerializationIssues",
		.default_value = DEFAULT_WARN_ON_JSONX_SERIALIZATION_ISSUES
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnMacroRedefinition,
		.name = BAD_CAST "WarnOnMacroRedefinition",
		.default_value = (void*) DEFAULT_WARN_ON_MACRO_REDEFINITION
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnMissingInheritBase,
		.name = BAD_CAST "WarnOnMissingInheritBase",
		.default_value = (void*) DEFAULT_WARN_ON_MISSING_INHERIT_BASE
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnMissingMacroContent,
		.name = BAD_CAST "WarnOnMissingMacroContent",
		.default_value = (void*) DEFAULT_WARN_ON_MISSING_MACRO_CONTENT
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnMultipleSelection,
		.name = BAD_CAST "WarnOnMultipleSelection",
		.default_value = (void*) DEFAULT_WARN_ON_MULTIPLE_SELECTION
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnNoExpectParam,
		.name = BAD_CAST "WarnOnNoExpectParam",
		.default_value = (void*) DEFAULT_WARN_ON_NO_EXPECT_PARAM
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnUninitializedTimer,
		.name = BAD_CAST "WarnOnUninitializedTimer",
		.default_value = (void*) DEFAULT_WARN_ON_UNINITIALIZED_TIMER
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnUnknownCommand,
		.name = BAD_CAST "WarnOnUnknownCommand",
		.default_value = (void*) DEFAULT_WARN_ON_UNKNOWN_COMMAND
	}, {
		.cfg_type = CFG_TYPE_BOOL,
		.value_ptr = &cfgWarnOnWontReplace,
		.name = BAD_CAST "WarnOnWontReplace",
		.default_value = (void*) DEFAULT_WARN_ON_WONT_REPLACE
	}, {
		.cfg_type = CFG_TYPE_STRING,
		.value_ptr = NULL,
		.name = BAD_CAST "XplNs",
		.default_value = NULL,
		.options = CFG_OPTION_DEPRECATED
	}, {
		.cfg_type = CFG_TYPE_STRING,
		.value_ptr = &cfgXplNsUri,
		.name = BAD_CAST "XplNsUri",
		.default_value = (void*) DEFAULT_XPL_NS_URI
	}
};

#define CONFIG_ENTRIES_COUNT (sizeof(config_entries) / sizeof(config_entries[0]))

int xplGetBooleanValue(xmlChar* str)
{
	if (!str)
		return 0;
	if (!xmlStrcasecmp(str, BAD_CAST "true")) return 1;
	if (!xmlStrcasecmp(str, BAD_CAST "false")) return 0;
	if (!xmlStrcasecmp(str, BAD_CAST "yes")) return 1;
	if (!xmlStrcasecmp(str, BAD_CAST "no")) return 0;
	if (!xmlStrcmp(str, BAD_CAST "_T")) return 1;
	if (!xmlStrcmp(str, BAD_CAST "_F")) return 0;
	return -1;
}

#define VOID_PTR_TO_INT(x) ((int) (((uintptr_t) (x)) & UINTPTR_MAX))

static void xplReadOption(xplConfigEntryPtr opt, xmlChar *value)
{
	int int_value = 0;

	if (opt->options & CFG_OPTION_DEPRECATED)
	{
		xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "Option \"%s\" is deprecated. Setting it has no effect.", opt->name);
		return;
	}
	switch (opt->cfg_type)
	{
		case CFG_TYPE_STRING:
			*((xmlChar**) opt->value_ptr) = BAD_CAST XPL_STRDUP((char*) value);
			break;
		case CFG_TYPE_INT:
			if (sscanf((char*) value, "%d", (int*) opt->value_ptr) != 1)
			{
				xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "\"%s\" is not a valid integer value. Setting config option \"%s\" to its default value \"%d\"\n",
					value, opt->name, VOID_PTR_TO_INT(opt->default_value));
				*((int*) opt->value_ptr) = VOID_PTR_TO_INT(opt->default_value);
			}
			break;
		case CFG_TYPE_BOOL:
			int_value = xplGetBooleanValue(value);
			if (int_value == -1)
			{
				xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "\"%s\" is not a valid boolean value. Setting config option \"%s\" to its default value \"%d\"\n",
					value, opt->name, VOID_PTR_TO_INT(opt->default_value));
				*((int*) opt->value_ptr) = VOID_PTR_TO_INT(opt->default_value);
			} else
				*((int*) opt->value_ptr) = int_value;
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			return;
	}
	opt->state = CFG_STATE_ASSIGNED;
}

static void xplAssignDefaultToOption(xplConfigEntryPtr opt)
{
	if (!opt || !opt->value_ptr)
		return;
	switch (opt->cfg_type)
	{
		case CFG_TYPE_STRING:
			if (opt->default_value)
				*((xmlChar**) opt->value_ptr) = BAD_CAST XPL_STRDUP(opt->default_value);
			else
				*((xmlChar**) opt->value_ptr) = NULL;
			break;
		case CFG_TYPE_INT:
		case CFG_TYPE_BOOL:
			*((int*) opt->value_ptr) = VOID_PTR_TO_INT(opt->default_value);
			break;
		default:
			return;
	}
	opt->state = CFG_STATE_DEFAULT;
}

void xplAssignDefaultsToAllOptions(void)
{
	unsigned int i;

	for (i = 0; i < CONFIG_ENTRIES_COUNT; i++)
	{
		if (config_entries[i].state == CFG_STATE_UNASSIGNED)
			xplAssignDefaultToOption(&config_entries[i]);
	}
}

int xplReadOptions(xmlNodePtr opt_root)
{
	xmlNodePtr cur;
	xmlChar *opt_name;
	xplConfigEntryPtr opt;
	unsigned int i;

	if (config_entries_hash)
		xplCleanupOptions();
	config_entries_hash = xmlHashCreate(sizeof(config_entries) / sizeof(config_entries[0]) * 2);

	for (i = 0; i < CONFIG_ENTRIES_COUNT; i++)
	{
		config_entries[i].state = CFG_STATE_UNASSIGNED;
		xmlHashAddEntry(config_entries_hash, config_entries[i].name, &config_entries[i]);
	}
	cur = opt_root->children;
	while (cur)
	{
		if (cur->type == XML_ELEMENT_NODE)
		{
			if (!xmlStrcmp(cur->name, BAD_CAST "Option"))
			{
				opt_name = xmlGetNoNsProp(cur, BAD_CAST "name");
				if (opt_name)
				{
					opt = (xplConfigEntryPtr) xmlHashLookup(config_entries_hash, opt_name);
					if (opt)
						xplReadOption(opt, cur->children?cur->children->content:NULL);
					else
						xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "unknown config option \"%s\" (line %d), ignored\n", opt_name, cur->line);
					XPL_FREE(opt_name);
				} else
					xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "missing option name in config file (line %d), ignored\n", cur->line);
			} else
				xplDisplayMessage(XPL_MSG_WARNING, BAD_CAST "unknown node \"%s\" in Options section in config file (line %d), ignored", cur->name, cur->line);
		}
		cur = cur->next;
	}
	xplAssignDefaultsToAllOptions();
	/* implication */
	if (cfgStackTrace)
		cfgErrorsToConsole = 1;
	if (cfgProxyServer && *cfgProxyServer)
		xplDisplayMessage(XPL_MSG_INFO, BAD_CAST "Using proxy server \"%s:%d\"", cfgProxyServer, cfgProxyPort);
	return 1;
}

void xplCleanupOptions(void)
{
	unsigned int i;

	for (i = 0; i < CONFIG_ENTRIES_COUNT; i++)
	{
		if (config_entries[i].options & CFG_OPTION_DEPRECATED)
			continue;
		if ((config_entries[i].cfg_type == CFG_TYPE_STRING) && *((xmlChar**) config_entries[i].value_ptr))
		{
			XPL_FREE(*((xmlChar**)config_entries[i].value_ptr));
			*((xmlChar**) config_entries[i].value_ptr) = NULL;
		}
	}
	if (config_entries_hash)
	{
		xmlHashFree(config_entries_hash, NULL);
		config_entries_hash = NULL;
	}
}

static xmlChar *xplGetOptionValueInner(xplConfigEntryPtr p, bool showPasswords)
{
	char int_buf[12];

	switch (p->cfg_type)
	{
		case CFG_TYPE_BOOL:
			if (*((int*) p->value_ptr))
				return BAD_CAST XPL_STRDUP("true");
			else
				return BAD_CAST XPL_STRDUP("false");
		case CFG_TYPE_INT:
			snprintf(int_buf, 12, "%d", *((int*) p->value_ptr));
			return BAD_CAST XPL_STRDUP_NO_CHECK(int_buf);
		case CFG_TYPE_STRING:
			if ((p->options & CFG_OPTION_IS_PASSWORD) && !showPasswords)
				return BAD_CAST XPL_STRDUP("[hidden]");
			else
				return BAD_CAST XPL_STRDUP(*((char**) p->value_ptr));
		case CFG_TYPE_DEFERRED: /* ToDo */
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
	return NULL;
}

xmlChar *xplGetOptionValue(xmlChar *optionName, bool showPasswords, bool *found)
{
	xplConfigEntryPtr p;

	*found = false;
	if (!config_entries_hash)
		return NULL;
	p = (xplConfigEntryPtr) xmlHashLookup(config_entries_hash, optionName);
	if (!p)
		return NULL;
	*found = true;
	if (p->options & CFG_OPTION_DEPRECATED)
		return NULL;
	return xplGetOptionValueInner(p, showPasswords);
}

xmlNodePtr xplOptionsToList(xmlNodePtr parent, xplQName tagname, bool showPasswords)
{
	unsigned int i;
	xmlNodePtr ret = NULL, tail, cur, value;

	for (i = 0; i < CONFIG_ENTRIES_COUNT; i++)
	{
		if (config_entries[i].options & CFG_OPTION_DEPRECATED)
			continue;
		if (!tagname.ncname)
			cur = xmlNewDocNode(parent->doc, NULL, config_entries[i].name, NULL);
		else
			cur = xmlNewDocNode(parent->doc, tagname.ns, tagname.ncname, NULL);
		value = xmlNewDocText(parent->doc, NULL);
		value->content = xplGetOptionValueInner(&config_entries[i], showPasswords);
		value->parent = cur;
		cur->children = cur->last = value;
		if (tagname.ncname)
			xmlNewProp(cur, BAD_CAST "name", config_entries[i].name);
		switch(config_entries[i].cfg_type)
		{
			case CFG_TYPE_BOOL: xmlNewProp(cur, BAD_CAST "type", BAD_CAST "bool"); break;
			case CFG_TYPE_INT: xmlNewProp(cur, BAD_CAST "type", BAD_CAST "int"); break;
			case CFG_TYPE_STRING: xmlNewProp(cur, BAD_CAST "type", BAD_CAST "string"); break;
			case CFG_TYPE_DEFERRED: break; /* ToDo */
			default:
				DISPLAY_INTERNAL_ERROR_MESSAGE();
		}
		if (config_entries[i].state == CFG_STATE_DEFAULT)
			xmlNewProp(cur, BAD_CAST "default", BAD_CAST "true");
		if (config_entries[i].options & CFG_OPTION_IS_PASSWORD)
			xmlNewProp(cur, BAD_CAST "ispassword", BAD_CAST "true");
		if (config_entries[i].options & CFG_OPTION_RESTART_REQUIRED)
			xmlNewProp(cur, BAD_CAST "restartrequired", BAD_CAST "true");
		if (ret)
		{
			tail->next = cur;
			cur->prev = tail;
			tail = cur;
		} else
			ret = tail = cur;
	}
	return ret;
}

xplSetOptionResult xplSetOptionValue(xmlChar *optionName, xmlChar *value, bool byDefault)
{
	xplConfigEntryPtr p;
	int int_value = 0;
	xefCryptoDigestParams dp;

	p = (xplConfigEntryPtr) xmlHashLookup(config_entries_hash, optionName);
	if (!p)
		return XPL_SET_OPTION_UNKNOWN_OPTION;
	if (p->options & CFG_OPTION_DEPRECATED)
		return XPL_SET_OPTION_OK;
	if (p->cfg_type == CFG_TYPE_STRING && *((xmlChar**) p->value_ptr))
		XPL_FREE(*((xmlChar**) p->value_ptr));
	if (byDefault)
	{
		xplAssignDefaultToOption(p);
		return XPL_SET_OPTION_OK;
	}
	switch (p->cfg_type)
	{
		case CFG_TYPE_STRING:
			if (p->options & CFG_OPTION_STORED_AS_HASH)
			{
				if (value)
				{
					dp.input = value;
					dp.input_size = xmlStrlen(value);
					dp.digest_method = XEF_CRYPTO_DIGEST_METHOD_RIPEMD160;
					if (!xefCryptoDigest(&dp))
						return XPL_SET_OPTION_INTERNAL_ERROR;
				} else
					dp.digest = NULL;
				value = xstrBufferToHex(dp.digest, dp.digest_size, false);
				if (dp.digest)
					XPL_FREE(dp.digest);
				*((xmlChar**) p->value_ptr) = value;
			} else
				*((xmlChar**) p->value_ptr) = BAD_CAST XPL_STRDUP((char*) value);
			return XPL_SET_OPTION_OK;
		case CFG_TYPE_INT:
			if (!sscanf((char*) value, "%d", &int_value))
				return XPL_SET_OPTION_INVALID_VALUE;
			*((int*) p->value_ptr) = int_value;
			return XPL_SET_OPTION_OK;
		case CFG_TYPE_BOOL:
			int_value = xplGetBooleanValue(value);
			if (int_value == -1)
				return XPL_SET_OPTION_INVALID_VALUE;
			*((int*) p->value_ptr) = int_value;
			return XPL_SET_OPTION_OK;
		default:
			return XPL_SET_OPTION_INTERNAL_ERROR;
	}
}
