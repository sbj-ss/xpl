#include <string.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include <libxpl/xplwrappers.h>
#include <oniguruma.h>

static XPR_MUTEX mapper_interlock;
static bool mapper_interlock_initialized = false;

/* prologue/epilogue/etc path mapper */
typedef struct _xplWrapperMapEntry xplWrapperMapEntry;
typedef xplWrapperMapEntry *xplWrapperMapEntryPtr;
struct _xplWrapperMapEntry {
	xmlChar *regex_string;
	xmlChar *wrapper_file;
	regex_t *regex;
	xplWrapperMapEntryPtr next;
};

static xplWrapperMapEntryPtr wrappers;

static xplWrapperMapEntryPtr _createWrapperMapEntry(xmlChar *regexString, xmlChar *wrapperFile, regex_t *regex)
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

static void _freeWrapperMapEntry(xplWrapperMapEntryPtr cur)
{
	if (cur)
	{
		if (cur->regex_string) XPL_FREE(cur->regex_string);
		if (cur->wrapper_file) XPL_FREE(cur->wrapper_file);
		if (cur->regex) onig_free(cur->regex);
		XPL_FREE(cur);
	}
}

bool xplInitWrapperMap()
{
	if (!mapper_interlock_initialized)
	{
		if (!xprMutexInit(&mapper_interlock))
			return false;
		mapper_interlock_initialized = true;
	}
	return true;
}

void xplCleanupWrapperMap(void)
{
	xplWrapperMapEntryPtr cur = wrappers, next;

	while (cur)
	{
		next = cur->next;
		_freeWrapperMapEntry(cur);
		cur = next;
	}
	wrappers = NULL;
	if (mapper_interlock_initialized)
	{
		xprMutexCleanup(&mapper_interlock);
		mapper_interlock_initialized = false;
	}
}

xplWrapperMapEntryPtr xplWrapperMapAddEntry(xmlChar *regexString, xmlChar *wrapperFile)
{
	regex_t *regex;
	OnigErrorInfo err_info;
	xplWrapperMapEntryPtr entry, cur;

	if (onig_new(&regex, regexString, regexString + xmlStrlen(regexString),
		ONIG_OPTION_NONE, ONIG_ENCODING_UTF8, ONIG_SYNTAX_PERL_NG, &err_info) != ONIG_NORMAL)
		return NULL;
	entry = _createWrapperMapEntry(regexString, wrapperFile, regex);
	if (!entry)
		return NULL;
	if ((cur = wrappers))
	{
		while (cur->next)
			cur = cur->next;
		cur->next = entry;
	} else
		wrappers = entry;
	return entry;
}

xmlChar* xplMapDocWrapper(xmlChar *filename)
{
	xplWrapperMapEntryPtr cur;
	OnigRegion *region;
	xmlChar *ret = NULL, *fn_end;

	cur = wrappers;
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
			ret = *cur->wrapper_file? cur->wrapper_file: NULL; /* empty value = don't use and stop searching */
			break;
		}
		cur = cur->next;
	}
	if (!xprMutexRelease(&mapper_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	onig_region_free(region, 1);
	return ret;
}
