#include <string.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include <libxpl/xplwrappers.h>
#include <oniguruma.h>

static XPR_MUTEX mapper_interlock;

xplDocRole xplDocRoleFromString(xmlChar *role)
{
	if (!role)
		return XPL_DOC_ROLE_UNKNOWN;
	if (!xmlStrcasecmp(role, BAD_CAST "prologue"))
		return XPL_DOC_ROLE_PROLOGUE;
	if (!xmlStrcasecmp(role, BAD_CAST "main"))
		return XPL_DOC_ROLE_MAIN;
	if (!xmlStrcasecmp(role, BAD_CAST "epilogue"))
		return XPL_DOC_ROLE_EPILOGUE;
	return XPL_DOC_ROLE_UNKNOWN;
}

const xmlChar* xplDocRoleToString(xplDocRole role)
{
	switch (role)
	{
		case XPL_DOC_ROLE_UNKNOWN: return BAD_CAST "unknown";
		case XPL_DOC_ROLE_PROLOGUE: return BAD_CAST "prologue";
		case XPL_DOC_ROLE_MAIN: return BAD_CAST "main";
		case XPL_DOC_ROLE_EPILOGUE: return BAD_CAST "epilogue";
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			return BAD_CAST "unknown role";
	}
}

const xmlChar* xplDocSourceToString(xplDocSource src)
{
	switch (src)
	{
		case XPL_DOC_SOURCE_ORIGINAL: return BAD_CAST "original";
		case XPL_DOC_SOURCE_MAPPED: return BAD_CAST "mapped";
		case XPL_DOC_SOURCE_SWEATOUT: return BAD_CAST "sweatout";
		case XPL_DOC_SOURCE_ABSENT: return BAD_CAST "absent";
		case XPL_DOC_SOURCE_OVERRIDDEN: return BAD_CAST "overridden";
		default:
			DISPLAY_INTERNAL_ERROR_MESSAGE();
			return BAD_CAST "unknown source";
	}
}

/* prologue/epilogue/etc path mapper */
typedef struct _xplWrapperMapEntry xplWrapperMapEntry;
typedef xplWrapperMapEntry *xplWrapperMapEntryPtr;
struct _xplWrapperMapEntry {
	xmlChar *regex_string;
	xmlChar *wrapper_file;
	regex_t *regex;
	xplWrapperMapEntryPtr next;
};

static xplWrapperMapEntryPtr wrapper_map[XPL_DOC_ROLE_MAX + 1] = { 0 };

typedef struct _xplWrapperMapEntryDecl
{
	xplDocRole role;
	xmlChar *name;
} xplWrapperMapEntryDecl, *xplWrapperMapEntryDeclPtr;

xplWrapperMapEntryDecl wrapper_map_entries[] =
{
	{ XPL_DOC_ROLE_PROLOGUE, BAD_CAST "Prologue" },
	{ XPL_DOC_ROLE_MAIN,     BAD_CAST "Main" },
	{ XPL_DOC_ROLE_EPILOGUE, BAD_CAST "Epilogue" }
};
#define WRAPPER_MAP_ENTRIES_COUNT (sizeof(wrapper_map_entries) / sizeof(wrapper_map_entries[0]))

static xplWrapperMapEntryPtr xplWrapperMapEntryCreate(xmlChar *regexString, xmlChar *wrapperFile, regex_t *regex)
{
	xplWrapperMapEntryPtr ret = (xplWrapperMapEntryPtr) XPL_MALLOC(sizeof(xplWrapperMapEntry));
	if (!ret)
		return NULL;
	ret->regex_string = regexString;
	ret->wrapper_file = wrapperFile;
	ret->regex = regex;
	ret->next = NULL;
	return ret;
}

static void xplFreeWrapperMapEntry(xplWrapperMapEntryPtr cur)
{
	if (cur)
	{
		if (cur->regex_string) XPL_FREE(cur->regex_string);
		if (cur->wrapper_file) XPL_FREE(cur->wrapper_file);
		if (cur->regex) onig_free(cur->regex);
		XPL_FREE(cur);
	}
}

void xplCleanupWrapperMap(void)
{
	xplWrapperMapEntryPtr cur, list;
	int i;

	for (i = 0; i <= XPL_DOC_ROLE_MAX; i++)
	{
		list = wrapper_map[i];
		while (list)
		{
			cur = list;
			list = list->next;
			xplFreeWrapperMapEntry(cur);
		}
		wrapper_map[i] = NULL;
	}
	if (!xprMutexCleanup(&mapper_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

static void xplAssignWrapperMapEntry(xplWrapperMapEntryPtr *head, xplWrapperMapEntryPtr *tail, xmlNodePtr cur)
{
	xmlChar *regex_string, *wrapper_file;
	xplWrapperMapEntryPtr entry;
	regex_t *regex;
	int ret_code;
	OnigErrorInfo err_info;
	xmlChar err_str[ONIG_MAX_ERROR_MESSAGE_LEN+1];

	regex_string = xmlGetNoNsProp(cur, BAD_CAST "regex");
	if (!regex_string)
	{
		xplDisplayMessage(xplMsgWarning, BAD_CAST "Missing regex attribute at prologue/epilogue in config file (line %d), ignored", cur->line);
		return;
	}
	if (!xplCheckNodeListForText(cur->children))
	{
		xplDisplayMessage(xplMsgWarning, BAD_CAST "Non-text content in prologue/epilogue in config file (line %d), ignored.", cur->line);
		XPL_FREE(regex_string);
		return;
	}
	wrapper_file = xmlNodeListGetString(cur->doc, cur->children, 1);
	if ((ret_code = onig_new(&regex, regex_string, regex_string + xmlStrlen(regex_string),
		ONIG_OPTION_NONE, ONIG_ENCODING_UTF8, ONIG_SYNTAX_PERL_NG, &err_info)) != ONIG_NORMAL)
	{
		if (onig_error_code_to_str(err_str, ret_code) != ONIG_NORMAL)
			strcpy((char*) err_str, "unknown error");
		xplDisplayMessage(xplMsgWarning, BAD_CAST "Invalid prologue/epilogue path regex \"%s\" in config file (line %d), ignored.", regex_string, cur->line);
		XPL_FREE(regex_string);
		XPL_FREE(wrapper_file);
		return;
	}
	entry = xplWrapperMapEntryCreate(regex_string, wrapper_file, regex);
	if (entry)
	{
		if (*tail)
			(*tail)->next = entry;
		*tail = entry;
		if (!*head)
			*head = entry;
	} else {
		XPL_FREE(regex_string);
		XPL_FREE(wrapper_file);
		onig_free(regex);
	}
}

void xplReadWrapperMap(xmlNodePtr cur)
{
	xplWrapperMapEntryPtr tail[WRAPPER_MAP_ENTRIES_COUNT] = { 0 };
	unsigned int i;

	xplCleanupWrapperMap();
	if (!xprMutexInit(&mapper_interlock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return;
	}
	cur = cur->children;
	while (cur)
	{
		if (cur->type == XML_ELEMENT_NODE)
		{
			for (i = 0; i < WRAPPER_MAP_ENTRIES_COUNT; i++)
			{
				if (!xmlStrcmp(cur->name, wrapper_map_entries[i].name) && (!cur->ns || !cur->ns->prefix))
				{
					xplAssignWrapperMapEntry(&wrapper_map[wrapper_map_entries[i].role], &tail[wrapper_map_entries[i].role], cur);
					break;
				}
			}
			if (i == WRAPPER_MAP_ENTRIES_COUNT)
				xplDisplayMessage(xplMsgWarning, BAD_CAST "Unknown node \"%s\" in config file, line %d", cur->name, cur->line);
		}
		cur = cur->next;
	}
}

xplWrapperMapEntryPtr xplWrapperMapAddEntry(xmlChar *regexString, xmlChar *wrapperFile, xplDocRole role)
{
	regex_t *regex;
	OnigErrorInfo err_info;
	xplWrapperMapEntryPtr entry, cur;

	if (onig_new(&regex, regexString, regexString + xmlStrlen(regexString),
		ONIG_OPTION_NONE, ONIG_ENCODING_UTF8, ONIG_SYNTAX_PERL_NG, &err_info) != ONIG_NORMAL)
		return NULL;
	entry = xplWrapperMapEntryCreate(regexString, wrapperFile, regex);
	if (!entry)
		return NULL;
	if ((cur = wrapper_map[role]))
	{
		while (cur->next)
			cur = cur->next;
		cur->next = entry;
	} else
		wrapper_map[role] = entry;
	return entry;
}

xmlChar* xplMapDocWrapper(xmlChar *filename, xplDocRole role)
{
	xplWrapperMapEntryPtr cur;
	OnigRegion *region;
	xmlChar *ret = NULL, *fn_end;

	cur = wrapper_map[role];
	if (!cur)
		return NULL; /* speedup */
	region = onig_region_new();
	if (!region)
		return NULL;
	fn_end = filename + xmlStrlen(filename);
	if (!xprMutexAcquire(&mapper_interlock)) /* regex isn't thread safe */
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	while (cur)
	{
		if (onig_match(cur->regex, filename, fn_end, filename, region, 0) != ONIG_MISMATCH)
		{
			ret = *cur->wrapper_file? cur->wrapper_file: NULL; /* Пустое значение = "не использовать" */
			break;
		}
		cur = cur->next;
	}
	if (!xprMutexRelease(&mapper_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	onig_region_free(region, 1);
	return ret;
}
