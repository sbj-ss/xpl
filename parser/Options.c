#include "Options.h"
#include "Messages.h"
#include "Utils.h"
#include "abstraction/xpr.h"
#include <openssl/ripemd.h>

int cfgCheckDbOnStartup;
int cfgCheckSAMode;
xmlChar *cfgDefaultEncoding;
xmlChar *cfgDebugSaveFile;
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
int cfgWarnOnInvalidXplNsUri;
int cfgWarnOnMacroRedefinition;
int cfgWarnOnMissingInheritBase;
int cfgWarnOnMissingMacroContent;
int cfgWarnOnNoExpectParam;
int cfgWarnOnUnknownCommand;
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

xplConfigEntry configEntries[] =
{
/* cfg_type, value_ptr, name, default_value */
	{
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgCheckDbOnStartup),
		SFINIT(.name, BAD_CAST "CheckDbOnStartup"),
		SFINIT(.default_value, (void*) DEFAULT_CHECK_DB_ON_STARTUP)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgCheckSAMode),
		SFINIT(.name, BAD_CAST "CheckSAMode"),
		SFINIT(.default_value, (void*) DEFAULT_CHECK_SA_MODE)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING),
		SFINIT(.value_ptr, &cfgDebugSaveFile),
		SFINIT(.name, BAD_CAST "DebugSaveFile"),
		SFINIT(.default_value, NULL)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING),
		SFINIT(.value_ptr, &cfgDefaultEncoding),
		SFINIT(.name, BAD_CAST "DefaultEncoding"),
		SFINIT(.default_value, DEFAULT_DEFAULT_ENCODING)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING),
		SFINIT(.value_ptr, &cfgDocRoot),
		SFINIT(.name, BAD_CAST "DocRoot"),
		SFINIT(.default_value, NULL)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgEnableAssertions),
		SFINIT(.name, BAD_CAST "EnableAssertions"),
		SFINIT(.default_value, (void*) DEFAULT_ENABLE_ASSERTIONS)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING),
		SFINIT(.value_ptr, NULL),
		SFINIT(.name, BAD_CAST "ErrorSourceName"),
		SFINIT(.default_value, NULL),
		SFINIT(.options, CFG_OPTION_DEPRECATED)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgErrorsToConsole),
		SFINIT(.name, BAD_CAST "ErrorsToConsole"),
		SFINIT(.default_value, (void*) DEFAULT_ERRORS_TO_CONSOLE)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgFoolproofDestructiveCommands),
		SFINIT(.name, BAD_CAST "FoolproofDestructiveCommands"),
		SFINIT(.default_value, (void*) DEFAULT_FOOLPROOF_DESTRUCTIVE_COMMANDS)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_INT),
		SFINIT(.value_ptr, &cfgInitialMacroTableSize),
		SFINIT(.name, BAD_CAST "InitialMacroTableSize"),
		SFINIT(.default_value, (void*) DEFAULT_INITIAL_MACRO_TABLE_SIZE)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING),
		SFINIT(.value_ptr, &cfgLogFileName),
		SFINIT(.name, BAD_CAST "LogFile"),
		SFINIT(.default_value, NULL)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgLuciferCompat),
		SFINIT(.name, BAD_CAST "LuciferCompatibility"),
		SFINIT(.default_value, (void*) DEFAULT_LUCIFER_COMPAT)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_INT),
		SFINIT(.value_ptr, &cfgMacroContentCachingThreshold),
		SFINIT(.name, BAD_CAST "MacroContentCachingThreshold"),
		SFINIT(.default_value, (void*) DEFAULT_MACRO_CONTENT_CACHING_THRESHOLD)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_INT),
		SFINIT(.value_ptr, &cfgMaxRecursionDepth),
		SFINIT(.name, BAD_CAST "MaxRecursionDepth"),
		SFINIT(.default_value, (void*) DEFAULT_MAX_RECURSION_DEPTH)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_INT),
		SFINIT(.value_ptr, &cfgMinDebugPrintLevel),
		SFINIT(.name, BAD_CAST "MinDebugPrintLevel"),
		SFINIT(.default_value, (void*) DEFAULT_MIN_DEBUG_PRINT_LEVEL)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL), /* TODO this should be somehow moved to XEF */
		SFINIT(.value_ptr, &cfgPrintTidyInfo),
		SFINIT(.name, BAD_CAST "PrintTidyInfo"),
		SFINIT(.default_value, (void*) DEFAULT_PRINT_TIDY_INFO)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING), /* TODO this too - together with all other proxy settings */
		SFINIT(.value_ptr, &cfgProxyPassword),
		SFINIT(.name, BAD_CAST "ProxyPassword"),
		SFINIT(.default_value, NULL),
		SFINIT(.options, CFG_OPTION_IS_PASSWORD)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_INT),
		SFINIT(.value_ptr, &cfgProxyPort),
		SFINIT(.name, BAD_CAST "ProxyPort"),
		SFINIT(.default_value, (void*) DEFAULT_PROXY_PORT)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING),
		SFINIT(.value_ptr, &cfgProxyServer),
		SFINIT(.name, BAD_CAST "ProxyServer"),
		SFINIT(.default_value, NULL)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING),
		SFINIT(.value_ptr, &cfgProxyUser),
		SFINIT(.name, BAD_CAST "ProxyUser"),
		SFINIT(.default_value, NULL)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING),
		SFINIT(.value_ptr, &cfgSaPassword),
		SFINIT(.name, BAD_CAST "SAPassword"),
		SFINIT(.default_value, (void*) "0BD4BC9866BA6963A3CA0563FD00578D90338EB5"), /* 1111111 */
		SFINIT(.options, CFG_OPTION_IS_PASSWORD | CFG_OPTION_STORED_AS_HASH)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_INT),
		SFINIT(.value_ptr, &cfgSessionLifetime),
		SFINIT(.name, BAD_CAST "SessionLifetime"),
		SFINIT(.default_value, (void*) DEFAULT_SESSION_LIFETIME)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgStackTrace),
		SFINIT(.name, BAD_CAST "StackTrace"),
		SFINIT(.default_value, (void*) DEFAULT_STACK_TRACE)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgUseConsoleColors),
		SFINIT(.name, BAD_CAST "UseConsoleColors"),
		SFINIT(.default_value, (void*) DEFAULT_USE_CONSOLE_COLORS)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgUseWrappers),
		SFINIT(.name, BAD_CAST "UseWrappers"),
		SFINIT(.default_value, (void*) DEFAULT_USE_WRAPPERS)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL), /* TODO should be registered by xpl:with (or xpl:edge?) */
		SFINIT(.value_ptr, &cfgWarnOnAncestorModificationAttempt),
		SFINIT(.name, BAD_CAST "WarnOnAncestorModificationAttempt"),
		SFINIT(.default_value, (void*) DEFAULT_WARN_ON_ANCESTOR_MODIFICATION_ATTEMPT)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgWarnOnDeletedNodeReference),
		SFINIT(.name, BAD_CAST "WarnOnDeletedNodeReference"),
		SFINIT(.default_value, (void*) DEFAULT_WARN_ON_DELETED_NODE_REFERENCE)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgWarnOnExpandedMacroContent),
		SFINIT(.name, BAD_CAST "WarnOnExpandedMacroContent"),
		SFINIT(.default_value, (void*) DEFAULT_WARN_ON_EXPANDED_MACRO_CONTENT)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgWarnOnInvalidXplNsUri),
		SFINIT(.name, BAD_CAST "WarnOnInvalidXplNsUri"),
		SFINIT(.default_value, (void*) DEFAULT_WARN_ON_INVALID_XPL_NS_URI)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgWarnOnMacroRedefinition),
		SFINIT(.name, BAD_CAST "WarnOnMacroRedefinition"),
		SFINIT(.default_value, (void*) DEFAULT_WARN_ON_MACRO_REDEFINITION)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL), /* TODO should be registered by xpl:inherit */
		SFINIT(.value_ptr, &cfgWarnOnMissingInheritBase),
		SFINIT(.name, BAD_CAST "WarnOnMissingInheritBase"),
		SFINIT(.default_value, (void*) DEFAULT_WARN_ON_MISSING_INHERIT_BASE)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgWarnOnMissingMacroContent),
		SFINIT(.name, BAD_CAST "WarnOnMissingMacroContent"),
		SFINIT(.default_value, (void*) DEFAULT_WARN_ON_MISSING_MACRO_CONTENT)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL), /* TODO should be registered by xpl:get-param */
		SFINIT(.value_ptr, &cfgWarnOnNoExpectParam),
		SFINIT(.name, BAD_CAST "WarnOnNoExpectParam"),
		SFINIT(.default_value, (void*) DEFAULT_WARN_ON_NO_EXPECT_PARAM)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_BOOL),
		SFINIT(.value_ptr, &cfgWarnOnUnknownCommand),
		SFINIT(.name, BAD_CAST "WarnOnUnknownCommand"),
		SFINIT(.default_value, (void*) DEFAULT_WARN_ON_UNKNOWN_COMMAND)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING),
		SFINIT(.value_ptr, NULL),
		SFINIT(.name, BAD_CAST "XplNs"),
		SFINIT(.default_value, NULL),
		SFINIT(.options, CFG_OPTION_DEPRECATED)
	}, {
		SFINIT(.cfg_type, CFG_TYPE_STRING),
		SFINIT(.value_ptr, &cfgXplNsUri),
		SFINIT(.name, BAD_CAST "XplNsUri"),
		SFINIT(.default_value, DEFAULT_XPL_NS_URI)
	}
};

#define CONFIG_ENTRIES_COUNT (sizeof(configEntries) / sizeof(configEntries[0]))

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
		xplDisplayMessage(xplMsgWarning, BAD_CAST "Option \"%s\" is deprecated. Setting it has no effect.", opt->name);
		return;
	}
	switch (opt->cfg_type)
	{
		case CFG_TYPE_STRING:
			*((xmlChar**) opt->value_ptr) = xmlStrdup(value);
			break;
		case CFG_TYPE_INT:
			if (sscanf((char*) value, "%d", (int*) opt->value_ptr) != 1)
			{
				xplDisplayMessage(xplMsgWarning, BAD_CAST "\"%s\" is not a valid integer value. Setting config option \"%s\" to its default value \"%d\"\n",
					value, opt->name, VOID_PTR_TO_INT(opt->default_value));
				*((int*) opt->value_ptr) = VOID_PTR_TO_INT(opt->default_value);
			}
			break;
		case CFG_TYPE_BOOL:
			int_value = xplGetBooleanValue(value);
			if (int_value == -1)
			{
				xplDisplayMessage(xplMsgWarning, BAD_CAST "\"%s\" is not a valid boolean value. Setting config option \"%s\" to its default value \"%d\"\n",
					value, opt->name, VOID_PTR_TO_INT(opt->default_value));
				*((int*) opt->value_ptr) = VOID_PTR_TO_INT(opt->default_value);
			} else
				*((int*) opt->value_ptr) = int_value;
			break;
		default: /* Если мы здесь, значит, что-то добавлено в перечисление типов, но забыто */
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
				*((xmlChar**) opt->value_ptr) = xmlStrdup((xmlChar*) opt->default_value);
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
		if (configEntries[i].state == CFG_STATE_UNASSIGNED)
			xplAssignDefaultToOption(&configEntries[i]);
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
	config_entries_hash = xmlHashCreate(32);

	for (i = 0; i < CONFIG_ENTRIES_COUNT; i++)
	{
		configEntries[i].state = CFG_STATE_UNASSIGNED;
		xmlHashAddEntry(config_entries_hash, configEntries[i].name, &configEntries[i]);
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
						xplDisplayMessage(xplMsgWarning, BAD_CAST "unknown config option \"%s\" (line %d), ignored\n", opt_name, cur->line);
					xmlFree(opt_name);
				} else
					xplDisplayMessage(xplMsgWarning, BAD_CAST "missing option name in config file (line %d), ignored\n", cur->line);
			} else
				xplDisplayMessage(xplMsgWarning, BAD_CAST "unknown node \"%s\" in Options section in config file (line %d), ignored", cur->name, cur->line);
		}
		cur = cur->next;
	}
	xplAssignDefaultsToAllOptions();
	/* Импликация */
	if (cfgStackTrace)
		cfgErrorsToConsole = 1;
	if (cfgProxyServer && *cfgProxyServer)
		xplDisplayMessage(xplMsgInfo, BAD_CAST "Using proxy server \"%s:%d\"", cfgProxyServer, cfgProxyPort);
	return 1;
}

void xplCleanupOptions(void)
{
	unsigned int i;

	for (i = 0; i < CONFIG_ENTRIES_COUNT; i++)
	{
		if (configEntries[i].options & CFG_OPTION_DEPRECATED)
			continue;
		if ((configEntries[i].cfg_type == CFG_TYPE_STRING) && *((xmlChar**) configEntries[i].value_ptr))
		{
			xmlFree(*((xmlChar**)configEntries[i].value_ptr));
			*((xmlChar**) configEntries[i].value_ptr) = NULL;
		}
	}
	if (config_entries_hash)
	{
		xmlHashFree(config_entries_hash, NULL);
		config_entries_hash = NULL;
	}
}

static xmlChar *xplGetOptionValueInner(xplConfigEntryPtr p, BOOL showPasswords)
{
	xmlChar int_buf[12];

	switch (p->cfg_type)
	{
		case CFG_TYPE_BOOL:
			if (*((int*) p->value_ptr))
				return xmlStrdup(BAD_CAST "true");
			else
				return xmlStrdup(BAD_CAST "false");
		case CFG_TYPE_INT:
			snprintf((char*) int_buf, 12, "%d", *((int*) p->value_ptr));
			return xmlStrdup(int_buf);
		case CFG_TYPE_STRING:
			if ((p->options & CFG_OPTION_IS_PASSWORD) && !showPasswords)
				return NULL;
			else
				return xmlStrdup(*((xmlChar**) p->value_ptr));
		case CFG_TYPE_DEFERRED: /* ToDo */
			break;
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
	return NULL;
}

xmlChar *xplGetOptionValue(xmlChar *optionName, BOOL showPasswords)
{
	xplConfigEntryPtr p;

	if (!config_entries_hash)
		return NULL;
	p = (xplConfigEntryPtr) xmlHashLookup(config_entries_hash, optionName);
	if (!p)
		return NULL;
	if (p->options & CFG_OPTION_DEPRECATED)
		return NULL;
	return xplGetOptionValueInner(p, showPasswords);
}

xmlNodePtr xplOptionsToList(xmlDocPtr doc, xmlNodePtr parent, xmlChar *tagName, BOOL showTags, BOOL showPasswords)
{
	unsigned int i;
	xmlNodePtr ret = NULL, tail, cur, value;
	xmlNsPtr ns;
	xmlChar *tag_name;

	EXTRACT_NS_AND_TAGNAME(tagName, ns, tag_name, parent);
	for (i = 0; i < CONFIG_ENTRIES_COUNT; i++)
	{
		if (configEntries[i].options & CFG_OPTION_DEPRECATED)
			continue;
		cur = xmlNewDocNode(doc, ns, showTags?configEntries[i].name:tag_name, NULL);
		value = xmlNewDocText(doc, NULL);
		value->content = xplGetOptionValueInner(&configEntries[i], showPasswords);
		value->parent = cur;
		cur->children = cur->last = value;
		if (!showTags)
			xmlNewProp(cur, BAD_CAST "name", configEntries[i].name);
		switch(configEntries[i].cfg_type)
		{
			case CFG_TYPE_BOOL: xmlNewProp(cur, BAD_CAST "type", BAD_CAST "bool"); break;
			case CFG_TYPE_INT: xmlNewProp(cur, BAD_CAST "type", BAD_CAST "int"); break;
			case CFG_TYPE_STRING: xmlNewProp(cur, BAD_CAST "type", BAD_CAST "string"); break;
			case CFG_TYPE_DEFERRED: break; /* ToDo */
			default:
				DISPLAY_INTERNAL_ERROR_MESSAGE();
		}
		if (configEntries[i].state == CFG_STATE_DEFAULT)
			xmlNewProp(cur, BAD_CAST "default", BAD_CAST "true");
		if (configEntries[i].options & CFG_OPTION_IS_PASSWORD)
			xmlNewProp(cur, BAD_CAST "ispassword", BAD_CAST "true");
		if (configEntries[i].options & CFG_OPTION_RESTART_REQUIRED)
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

xplSetOptionResult xplSetOptionValue(xmlChar *optionName, xmlChar *value, BOOL byDefault)
{
	xplConfigEntryPtr p;
	int int_value = 0;
	unsigned char *digest;

	p = (xplConfigEntryPtr) xmlHashLookup(config_entries_hash, optionName);
	if (!p)
		return XPL_SET_OPTION_UNKNOWN_OPTION;
	if (p->options & CFG_OPTION_DEPRECATED)
		return XPL_SET_OPTION_OK;
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
				digest = RIPEMD160((unsigned char*) value, xmlStrlen(value), NULL);
				value = bufferToHex(digest, RIPEMD160_DIGEST_LENGTH, FALSE);
				*((xmlChar**) p->value_ptr) = value;
			} else
				*((xmlChar**) p->value_ptr) = xmlStrdup(value);
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
