#include <string.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplmiddleware.h>
#include <libxpl/xpltree.h>
#include <oniguruma.h>

static XPR_MUTEX middleware_interlock;
static bool middleware_interlock_initialized = false;

typedef struct _xplMWEntry *xplMWEntryPtr;

typedef struct _xplMWEntry
{
	xmlChar *regex_string;
	xmlChar *wrapper_file;
	regex_t *regex;
	xplMWEntryPtr prev;
	xplMWEntryPtr next;
} xplMWEntry;

typedef struct _xplMWEntries
{
	xplMWEntryPtr first;
	xplMWEntryPtr last;
} xplMWEntries, *xplMWEntriesPtr;

static xplMWEntries middlewares;

static void _clearMWEntry(xplMWEntryPtr cur, bool removeCarrier)
{
	if (!cur)
		return;
	if (cur->regex_string)
		XPL_FREE(cur->regex_string);
	if (cur->wrapper_file)
		XPL_FREE(cur->wrapper_file);
	if (cur->regex)
		onig_free(cur->regex);
	if (removeCarrier)
		XPL_FREE(cur);
}

static xplMWResult _changeMWEntry(xplMWEntryPtr cur, const xmlChar *regexString, const xmlChar *wrapperFile, bool clear)
{
	regex_t *regex;
	OnigErrorInfo err_info;

	if (onig_new(&regex, regexString, regexString + xmlStrlen(regexString),
		ONIG_OPTION_NONE, ONIG_ENCODING_UTF8, ONIG_SYNTAX_PERL_NG, &err_info) != ONIG_NORMAL)
		return XPL_MW_BAD_REGEX;
	if (clear)
		_clearMWEntry(cur, false);
	cur->regex_string = BAD_CAST XPL_STRDUP((char*) regexString);
	cur->wrapper_file = BAD_CAST XPL_STRDUP((char*) wrapperFile);
	cur->regex = regex;
	return XPL_MW_OK;
}

static xplMWResult _createMWEntry(const xmlChar *regexString, const xmlChar *wrapperFile)
{
	xplMWEntryPtr entry;
	xplMWResult res;

	entry = (xplMWEntryPtr) XPL_MALLOC(sizeof(xplMWEntry));
	if (!entry)
		return XPL_MW_OUT_OF_MEMORY;
	memset(entry, 0, sizeof(xplMWEntry));
	res = _changeMWEntry(entry, regexString, wrapperFile, false);
	if (res != XPL_MW_OK)
	{
		_clearMWEntry(entry, true);
		return res;
	}
	APPEND_NODE_TO_LIST(middlewares.first, middlewares.last, entry);
	return XPL_MW_OK;
}

static xplMWEntryPtr _locateMWEntry(const xmlChar *regexString)
{
	xplMWEntryPtr cur;

	for (cur = middlewares.first; cur; cur = cur->next)
		if (!xmlStrcmp(cur->regex_string, regexString))
			return cur;
	return NULL;
}

xplMWResult xplMWAddEntry(const xmlChar *regexString, const xmlChar *wrapperFile, bool allowReplace)
{
	xplMWEntryPtr old;

	if ((old = _locateMWEntry(regexString)))
	{
		if (allowReplace)
			return _changeMWEntry(old, regexString, wrapperFile, false);
		else
			return XPL_MW_ALREADY_EXISTS;
	}
	return _createMWEntry(regexString, wrapperFile);
}

xplMWResult xplMWChangeEntry(const xmlChar *regexString, const xmlChar *wrapperFile, bool allowCreate)
{
	xplMWEntryPtr old;

	if (!(old = _locateMWEntry(regexString)))
	{
		if (allowCreate)
			return _createMWEntry(regexString, wrapperFile);
		else
			return XPL_MW_NOT_FOUND;
	}
	return _changeMWEntry(old, regexString, wrapperFile, true);
}

xplMWResult xplMWRemoveEntry(const xmlChar *regexString, bool ignoreMissing)
{
	xplMWEntryPtr old;

	if (!(old = _locateMWEntry(regexString)))
	{
		if (ignoreMissing)
			return XPL_MW_OK;
		else
			return XPL_MW_NOT_FOUND;
	}
	if (middlewares.first == old)
		middlewares.first = old->next;
	if (middlewares.last == old)
		middlewares.last = old->prev;
	if (old->prev)
		old->prev->next = old->next;
	if (old->next)
		old->next->prev = old->prev;
	_clearMWEntry(old, true);
	return XPL_MW_OK;
}

xmlNodePtr xplMWListEntries(xmlDocPtr doc, xplQName qname)
{
	xplMWEntryPtr entry;
	xmlNodePtr head = NULL, tail = NULL, cur;

	for (entry = middlewares.first; entry; entry = entry->next)
	{
		cur = xmlNewDocNode(doc, qname.ns, qname.ncname, NULL);
		xmlNewProp(cur, BAD_CAST "regex", entry->regex_string);
		xmlNewProp(cur, BAD_CAST "file", entry->wrapper_file);
		APPEND_NODE_TO_LIST(head, tail, cur);
	}
	return head;
}

xmlChar* xplMWGetWrapper(const xmlChar *filename)
{
	xplMWEntryPtr cur;
	OnigRegion *region;
	xmlChar *ret = NULL;
	const xmlChar *fn_end;

	cur = middlewares.first;
	if (!cur)
		return NULL; /* speedup */
	region = onig_region_new();
	if (!region)
		return NULL;
	fn_end = filename + xmlStrlen(filename);
	SUCCEED_OR_DIE(xprMutexAcquire(&middleware_interlock)); /* regex isn't thread safe */
	while (cur)
	{
		if (onig_match(cur->regex, filename, fn_end, filename, region, 0) != ONIG_MISMATCH)
		{
			ret = *cur->wrapper_file? cur->wrapper_file: NULL; /* empty value = don't use and stop searching */
			break;
		}
		cur = cur->next;
	}
	SUCCEED_OR_DIE(xprMutexRelease(&middleware_interlock));
	onig_region_free(region, 1);
	return ret;
}

bool xplInitMiddleware()
{
	if (!middleware_interlock_initialized)
	{
		if (!xprMutexInit(&middleware_interlock))
			return false;
		middleware_interlock_initialized = true;
	}
	return true;
}

void xplCleanupMiddleware(void)
{
	xplMWEntryPtr cur = middlewares.first, next;

	while (cur)
	{
		next = cur->next;
		_clearMWEntry(cur, true);
		cur = next;
	}
	middlewares.first = middlewares.last = NULL;
	if (middleware_interlock_initialized)
	{
		xprMutexCleanup(&middleware_interlock);
		middleware_interlock_initialized = false;
	}
}

